alias proxy='export https_proxy=http://127.0.0.1:7890 http_proxy=http://127.0.0.1:7890 all_proxy=socks5://127.0.0.1:7890'
alias unproxy='export https_proxy= http_proxy= all_proxy='

alias claude_deepseek='
export ANTHROPIC_BASE_URL=https://api.deepseek.com/anthropic
export ANTHROPIC_AUTH_TOKEN=xxx
export API_TIMEOUT_MS=600000
export ANTHROPIC_MODEL=deepseek-chat
export ANTHROPIC_SMALL_FAST_MODEL=deepseek-chat
export CLAUDE_CODE_DISABLE_NONESSENTIAL_TRAFFIC=1
export ENABLE_LSP_TOOL=1
'
alias claude_glm='
export ANTHROPIC_BASE_URL=https://open.bigmodel.cn/api/anthropic
export ANTHROPIC_AUTH_TOKEN=xxx
export API_TIMEOUT_MS=3000000
export CLAUDE_CODE_DISABLE_NONESSENTIAL_TRAFFIC=1
export ENABLE_LSP_TOOL=1
'
alias claude_default='
export ANTHROPIC_BASE_URL=
export ANTHROPIC_AUTH_TOKEN=
export API_TIMEOUT_MS=
export ANTHROPIC_MODEL=
export ANTHROPIC_SMALL_FAST_MODEL=
export CLAUDE_CODE_DISABLE_NONESSENTIAL_TRAFFIC=
export ENABLE_LSP_TOOL=1
'
