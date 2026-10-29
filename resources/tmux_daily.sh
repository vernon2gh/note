#!/usr/bin/env bash

CMD=$(cat <<'EOS'
echo hello
echo world
exec "${SHELL:-bash}" -l
EOS
)

if tmux list-sessions >/dev/null 2>&1; then
	# 情况 2：已有 session，交给 tmux 选最近活动的 session
	tmux new-window -d "$CMD"
else
	# 情况 1：没有任何 session，新建 session
	tmux new-session -d "$CMD"
fi
