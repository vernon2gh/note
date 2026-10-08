#!/usr/bin/env python3

"""
tmux copy-pipe 回调：捕获光标 → 退出copy-mode → AI翻译
- 中→英：写入 tmux buffer
- 英→中：浮窗展示

依赖：pip install openai
用法: 由 tmux copy-pipe 调用，选中文本通过 stdin 传入
"""

import os
import sys
import subprocess
import unicodedata
import openai

# ============================== 配置 ==============================
# OpenAI 兼容格式
BASE_URL = "https://api.deepseek.com"
# API Key
API_KEY = ""
# 模型名：deepseek-flash（推荐，1M 上下文 / 最大 384K 输出 / 关闭思维链）
MODEL = "deepseek-flash"
CONTEXT_WINDOW = 1000000
MAX_OUTPUT_TOKENS = 262144
REASONING_EFFORT = "none"
# ================================================================


def run_cmd(cmd: str) -> str:
    try:
        return subprocess.run(
            cmd, shell=True, capture_output=True, text=True
        ).stdout.strip()
    except Exception:
        return ""


def get_tmux_pane() -> str:
    pane = os.environ.get("TMUX_PANE", "")
    if not pane:
        pane = run_cmd("tmux display -p '#{pane_id}'")
    return pane


def capture_cursor() -> tuple[int, int]:
    pane = get_tmux_pane()
    out = run_cmd(
        f"tmux display -t {pane} -p '#{{copy_cursor_x}},#{{copy_cursor_y}}'"
    )
    parts = out.split(",")
    if len(parts) == 2 and parts[0].isdigit() and parts[1].isdigit():
        return int(parts[0]), int(parts[1])
    return 0, 5


def exit_copy_mode():
    pane = get_tmux_pane()
    run_cmd(f"tmux send-keys -t {pane} -X cancel")


def has_chinese(text: str) -> bool:
    """是否含汉字（仅汉字本身，不含 ，。："" 等中文标点）"""
    for ch in text:
        cp = ord(ch)
        if (
            0x4E00 <= cp <= 0x9FFF      # CJK 统一汉字（基本区）
            or 0x3400 <= cp <= 0x4DBF   # 扩展 A
            or 0xF900 <= cp <= 0xFAFF   # 兼容汉字
            or 0x20000 <= cp <= 0x2EBEF  # 扩展 B–F
            or cp == 0x3007             # 〇
        ):
            return True
    return False


def translate(text: str, base_url: str, api_key: str, model: str) -> str:
    if has_chinese(text):
        source_lang, target_lang = "Chinese", "English"
    else:
        source_lang, target_lang = "English", "Simplified Chinese"
    system_prompt = (
        "You are a professional translator. "
        f"Translate the following {source_lang} text into {target_lang}.\n\n"
        "Rules:\n"
        "- Preserve all technical terms, code, function names, variable names in original form\n"
        "- Preserve patch diff markers (+, -, @@, etc.) and git headers unchanged\n"
        "- Preserve email quoting levels (>, >>) unchanged\n"
        "- Keep the original formatting and line breaks\n"
        "- Only translate natural language sentences, not code or technical markers\n"
        "- If the text mixes Chinese and English, translate all natural "
        "language into the target language\n"
        "- Make the translation natural and fluent"
    )
    client = openai.OpenAI(api_key=api_key, base_url=base_url or None)
    budget = CONTEXT_WINDOW - est_tokens(system_prompt) - MAX_OUTPUT_TOKENS

    resp = client.responses.create(
        model=model,
        instructions=system_prompt,
        input=truncate_to_tokens(text, max(budget, 1)),
        reasoning={"effort": REASONING_EFFORT},
        max_output_tokens=MAX_OUTPUT_TOKENS,
    )
    return resp.output_text or ""


def _char_width(ch: str) -> int:
    return 2 if unicodedata.east_asian_width(ch) in ("F", "W") else 1


def display_width(text: str) -> int:
    """文本最长行的显示宽度（全角 2 列，半角 1 列）"""
    return max(
        (sum(_char_width(c) for c in line) for line in text.splitlines() or [""]),
        default=0,
    )


def display_height(text: str, max_w: int) -> int:
    """文本按 max_w 宽度 wrap 后的总行数"""
    if max_w <= 0:
        return 1
    return sum(
        max(1, (sum(_char_width(c) for c in line) + max_w - 1) // max_w)
        for line in text.splitlines() or [""]
    )


def get_pane_bounds() -> tuple[int, int, int, int]:
    pane = get_tmux_pane()
    out = run_cmd(
        f"tmux display -t {pane} -p "
        f"'#{{pane_left}},#{{pane_top}},#{{pane_width}},#{{pane_height}}'"
    )
    parts = out.split(",")
    try:
        return int(parts[0]), int(parts[1]), int(parts[2]), int(parts[3])
    except (ValueError, IndexError):
        return 0, 0, 80, 24


def calc_popup_pos(cursor_x: int, cursor_y: int, text: str,
                   status_lines: int = 0) -> tuple[int, int, int, int]:
    """光标右侧 ≥80 列时在右侧，否则下一行开头；status_lines 为内容下方额外预留行数（如 less 状态行）。"""
    pane_left, pane_top, pane_w, pane_h = get_pane_bounds()

    # 底部留 1 行余量，避免贴 pane 底
    right_space = pane_w - cursor_x - 1
    if right_space >= 80:
        popup_x = pane_left + cursor_x + 1
        top_in_pane = cursor_y + 1
        avail_w = right_space
    else:
        popup_x = pane_left
        top_in_pane = cursor_y + 2
        avail_w = pane_w
    avail_h = pane_h - top_in_pane - 1

    # popup 边框上下左右各占 1；宽度上限 80，下限 8（容纳 "(END)" + 边框）
    popup_w = max(min(80, display_width(text) + 2, avail_w), 8)

    # 按真实内容区宽度算 wrap 后的行数；高度下限 4（边框 2 + 状态行 1 + 内容 1）
    wrapped_h = display_height(text, max(popup_w - 2, 1))
    popup_h = max(min(wrapped_h + status_lines + 2, avail_h), 4)

    top_row = pane_top + top_in_pane
    popup_y = top_row + popup_h - 1  # tmux -y 指 popup 底部行号

    return popup_x, popup_y, popup_w, popup_h


def show_chinese(text: str, cursor_x: int, cursor_y: int):
    px, py, pw, ph = calc_popup_pos(cursor_x, cursor_y, text, status_lines=1)
    pane = get_tmux_pane()
    subprocess.run("tmux load-buffer -", shell=True, input=text, text=True)
    subprocess.run(
        f'tmux display-popup -t {pane} -x {px} -y {py} '
        f'-w {pw} -h {ph} -E "tmux show-buffer | less -R -~"',
        shell=True,
    )


def show_loading(text: str, cursor_x: int, cursor_y: int):
    px, py, pw, ph = calc_popup_pos(cursor_x, cursor_y, text)
    pane = get_tmux_pane()
    subprocess.Popen(
        f'tmux display-popup -t {pane} -x {px} -y {py} '
        f'-w {pw} -h {ph} "echo {text}"',
        shell=True,
    )


def close_loading():
    pane = get_tmux_pane()
    run_cmd(f"tmux display-popup -C -t {pane}")


def est_tokens(text: str) -> int:
    cjk = sum(
        1 for ch in text
        if "\u2e80" <= ch <= "\u9fff" or "\uf900" <= ch <= "\ufaff"
        or "\uff00" <= ch <= "\uffef"
    )
    return cjk + (len(text) - cjk + 3) // 4


def truncate_to_tokens(text: str, budget: int) -> str:
    if est_tokens(text) <= budget:
        return text
    marker = "\n\n[... truncated ...]"
    budget = max(budget - est_tokens(marker), 0)
    lo, hi = 0, len(text)
    while lo < hi:
        mid = (lo + hi + 1) // 2
        if est_tokens(text[:mid]) <= budget:
            lo = mid
        else:
            hi = mid - 1
    return text[:lo] + marker


def load_to_buffer(text: str):
    """翻译结果写入 tmux buffer"""
    subprocess.run(["tmux", "load-buffer", "-"], input=text.strip(), text=True)


def main():
    cursor_x, cursor_y = capture_cursor()
    exit_copy_mode()

    sel = sys.stdin.read()
    if not sel.strip():
        sys.exit(0)

    if not API_KEY or not MODEL:
        text = (
            "请设置环境变量：\n"
            '  API_KEY = "sk-xxxx"'
        )
        show_chinese(text, cursor_x, cursor_y)
        return

    show_loading("正在翻译...", cursor_x, cursor_y)
    try:
        text = translate(sel, BASE_URL, API_KEY, MODEL)
    except Exception as e:
        text = f"[翻译失败: {e}]"
    close_loading()

    if has_chinese(sel):
        load_to_buffer(text)
    else:
        show_chinese(text, cursor_x, cursor_y)


if __name__ == "__main__":
    main()
