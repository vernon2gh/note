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
'
alias claude_glm='
export ANTHROPIC_BASE_URL=https://open.bigmodel.cn/api/anthropic
export ANTHROPIC_AUTH_TOKEN=xxx
export ANTHROPIC_MODEL=glm-5.2[1m]
export ANTHROPIC_DEFAULT_OPUS_MODEL=glm-5.2[1m]
export ANTHROPIC_DEFAULT_SONNET_MODEL=glm-5.2[1m]
export ANTHROPIC_DEFAULT_HAIKU_MODEL=glm-5-turbo
export CLAUDE_CODE_SUBAGENT_MODEL=glm-5.2[1m]
'
alias claude_kimi='
export ANTHROPIC_BASE_URL=https://api.kimi.com/coding/
export ANTHROPIC_API_KEY=xxx
export ANTHROPIC_MODEL=k3[1m]
export ANTHROPIC_DEFAULT_FABLE_MODEL=k3[1m]
export ANTHROPIC_DEFAULT_OPUS_MODEL=k3[1m]
export ANTHROPIC_DEFAULT_SONNET_MODEL=k3[1m]
export ANTHROPIC_DEFAULT_HAIKU_MODEL=k3[1m]
export CLAUDE_CODE_SUBAGENT_MODEL=k3[1m]
export CLAUDE_CODE_AUTO_COMPACT_WINDOW=1048576
export CLAUDE_CODE_MAX_CONTEXT_TOKENS=1048576
'
alias claude_agent_teams='
export CLAUDE_CODE_EFFORT_LEVEL=max
export CLAUDE_CODE_EXPERIMENTAL_AGENT_TEAMS=1
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
export CLAUDE_CODE_EXPERIMENTAL_AGENT_TEAMS=
'
