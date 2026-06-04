#!/usr/bin/env python3

"""
tmux copy-pipe 回调：捕获光标 → 退出copy-mode → 加载环境 → AI翻译 → 浮窗展示

依赖：pip install anthropic
配置: 从 ~/.bashrc 加载 ANTHROPIC_* 环境变量
用法: 由 tmux copy-pipe 调用，选中文本通过 stdin 传入
"""

import os
import sys
import re
import subprocess
import unicodedata
import anthropic

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


def load_env():
    try:
        with open(os.path.expanduser("~/.bashrc")) as f:
            for line in f:
                m = re.match(
                    r"^export\s+(ANTHROPIC_[\w]+)=[\"']?(.*?)[\"']?\s*$",
                    line,
                )
                if m and m.group(1) not in os.environ:
                    os.environ[m.group(1)] = m.group(2)
    except FileNotFoundError:
        pass


def is_english(text: str) -> bool:
    return text.isascii()


def translate(text: str, api_base: str, api_key: str, model: str) -> str:
    if is_english(text):
        source_lang, target_lang = "English", "Simplified Chinese"
    else:
        source_lang, target_lang = "Chinese", "English"
    system_prompt = (
        "You are a professional translator. "
        f"Translate the following {source_lang} text into {target_lang}.\n\n"
        "Rules:\n"
        "- Preserve all technical terms, code, function names, variable names in original form\n"
        "- Preserve patch diff markers (+, -, @@, etc.) and git headers unchanged\n"
        "- Preserve email quoting levels (>, >>) unchanged\n"
        "- Keep the original formatting and line breaks\n"
        "- Only translate natural language sentences, not code or technical markers\n"
        "- Make the translation natural and fluent"
    )
    client = anthropic.Anthropic(api_key=api_key, base_url=api_base)
    resp = client.messages.create(
        model=model,
        max_tokens=16384,
        thinking={"type": "disabled"},
        system=system_prompt,
        messages=[{"role": "user", "content": text}],
    )
    return next(b.text for b in resp.content if b.type == "text")


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


def paste_to_pane(text: str, cursor_y: int):
    """若当前 pane 是 vim/nvim，把 text 粘贴到选区结束行下方"""
    pane = get_tmux_pane()
    cmd = run_cmd(f"tmux display -t {pane} -p '#{{pane_current_command}}'")
    if cmd not in ("vim", "nvim", "vi"):
        return
    subprocess.run("tmux load-buffer -", shell=True, input=f"\n{text.strip()}\n\n", text=True)
    motion = "H" if cursor_y == 0 else f"H {cursor_y}gj"
    subprocess.run(
        f"tmux send-keys -t {pane} Escape Escape {motion} "
        f"':r !tmux show-buffer' Enter",
        shell=True,
    )


def main():
    cursor_x, cursor_y = capture_cursor()
    exit_copy_mode()
    load_env()

    sel = sys.stdin.read()
    if not sel.strip():
        sys.exit(0)
    if len(sel) > 80000:
        sel = sel[:80000] + "\n\n[... truncated ...]"

    required = ("ANTHROPIC_BASE_URL", "ANTHROPIC_AUTH_TOKEN", "ANTHROPIC_MODEL")
    missing = [k for k in required if k not in os.environ]
    if missing:
        text = (
            "请设置：\n"
            + "\n".join(f"  echo \"export {k}=xxx\" >> ~/.bashrc" for k in missing)
        )
        show_chinese(text, cursor_x, cursor_y)
        return

    show_loading("正在翻译...", cursor_x, cursor_y)
    try:
        text = translate(
            sel,
            os.environ["ANTHROPIC_BASE_URL"],
            os.environ["ANTHROPIC_AUTH_TOKEN"],
            os.environ["ANTHROPIC_MODEL"],
        )
    except Exception as e:
        text = f"[翻译失败: {e}]"
    close_loading()

    if is_english(sel):
        show_chinese(text, cursor_x, cursor_y)
    else:
        paste_to_pane(text, cursor_y)


if __name__ == "__main__":
    main()
