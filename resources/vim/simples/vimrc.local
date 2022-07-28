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
nmap <C-]>g :cs find g <C-R>=expand("<cword>")<CR><CR>
" Find function call
nmap <C-]>c :cs find c <C-R>=expand("<cword>")<CR><CR>
