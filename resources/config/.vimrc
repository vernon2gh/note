syntax on

set number
set hlsearch

" startup code folding
set fdm=indent
" By default, not collapsed when they are opened
set foldlevelstart=99

" when save file, auto delete line tail space
autocmd BufWritePre * %s/\s\+$//e

" global
set cscopeprg=gtags-cscope

if filereadable("GTAGS")
    cs add GTAGS
else
    let gtags_file=findfile("GTAGS", ".;")
    if !empty(gtags_file) && filereadable(gtags_file)
        exe "cs add" gtags_file
    endif
endif

" cscope
"set cscopeprg=cscope
"
"if filereadable("cscope.out")
"    cs add cscope.out
"else
"    let cscope_file=findfile("cscope.out", ".;")
"    let cscope_pre=matchstr(cscope_file, ".*/")
"    if !empty(cscope_file) && filereadable(cscope_file)
"        exe "cs add" cscope_file cscope_pre
"    endif
"endif

" Find function definition
nmap ;d :cs find g <C-R>=expand("<cword>")<CR><CR>
" Find function call
nmap ;r :cs find c <C-R>=expand("<cword>")<CR><CR>
" grep
nmap ;gg :!git grep <C-R>=expand("<cword>")<CR><CR>

" Directional keys
" normal mode
nmap i <up>
nmap k <down>
nmap j <left>
nmap l <right>
nmap h <insert>
nmap ;i 10<up>
nmap ;k 10<down>
" visual mode
vmap i <up>
vmap k <down>
vmap j <left>
vmap l <right>
vmap h <insert>
vmap ;i 10<up>
vmap ;k 10<down>
