alias proxy='export https_proxy=http://127.0.0.1:7890 http_proxy=http://127.0.0.1:7890 all_proxy=socks5://127.0.0.1:7890'
alias unproxy='export https_proxy= http_proxy= all_proxy='

alias claude_deepseek='
export ANTHROPIC_BASE_URL=https://api.deepseek.com/anthropic
export ANTHROPIC_AUTH_TOKEN=xxx
export ANTHROPIC_MODEL=deepseek-v4-pro[1m]
export ANTHROPIC_DEFAULT_OPUS_MODEL=deepseek-v4-pro[1m]
export ANTHROPIC_DEFAULT_SONNET_MODEL=deepseek-v4-pro[1m]
export ANTHROPIC_DEFAULT_HAIKU_MODEL=deepseek-v4-flash
export CLAUDE_CODE_SUBAGENT_MODEL=deepseek-v4-pro[1m]
export CLAUDE_CODE_EFFORT_LEVEL=max
export ENABLE_LSP_TOOL=1
'
alias claude_glm='
export ANTHROPIC_BASE_URL=https://open.bigmodel.cn/api/anthropic
export ANTHROPIC_AUTH_TOKEN=xxx
export ANTHROPIC_MODEL=glm-5.1
export ANTHROPIC_DEFAULT_OPUS_MODEL=glm-5.1
export ANTHROPIC_DEFAULT_SONNET_MODEL=glm-5.1
export ANTHROPIC_DEFAULT_HAIKU_MODEL=glm-5-turbo
export CLAUDE_CODE_SUBAGENT_MODEL=glm-5.1
export CLAUDE_CODE_EFFORT_LEVEL=max
export ENABLE_LSP_TOOL=1
'
alias claude_default='
export ANTHROPIC_BASE_URL=
export ANTHROPIC_AUTH_TOKEN=
export ANTHROPIC_MODEL=
export ANTHROPIC_DEFAULT_OPUS_MODEL=
export ANTHROPIC_DEFAULT_SONNET_MODEL=
export ANTHROPIC_DEFAULT_HAIKU_MODEL=
export CLAUDE_CODE_SUBAGENT_MODEL=
export CLAUDE_CODE_EFFORT_LEVEL=
export ENABLE_LSP_TOOL=1
'
