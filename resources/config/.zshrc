# ...
# ...
# ...

function arch_prompt_info() {
	FILE=compile_commands.json

	if [ -e $FILE ]; then
		ARCH=$(sed -n '3p' $FILE | grep -oE 'arch/[^/]*' | head -n 1 | cut -d '/' -f 2)
		echo -e "\033[36;1m($ARCH)\033[0m "
	fi
}

PROMPT+='$(arch_prompt_info)'

alias proxy='export https_proxy=http://127.0.0.1:7890 http_proxy=http://127.0.0.1:7890 all_proxy=socks5://127.0.0.1:7891'
alias unproxy='export https_proxy= http_proxy= all_proxy='
