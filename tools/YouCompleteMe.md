
## 概念
YouCompleteMe 插件是一种基于语义分析的补齐：通过分析源文件，经过语法分析以后进行智能补全。

简称 YCM，是一款 Vim 下非常流行的自动代码补齐神器。


## 安装依赖软件和库
编译 YouCompleteMe 插件时需要依赖 cmake 构建 Makefile，且依赖 Python 源码头文件，Ubuntu 系统可以使用以下命令安装相关依赖。
```bash
$ sudo apt-get install build-essential cmake python-dev python3-dev
```

clang 是一个面向 C 族语言(C-family languages)的轻量级编译器，YouCompleteMe 插件依赖 clang 实现对 C 族语言的语义补全，
```bash
$ sudo apt-get install clang
```


## 下载 YouCompleteMe 源码
使用 Vundle 或 vim-plug 等 Vim 插件管理器从 github 获取 YouCompleteMe 最新的源码，官方推荐使用 Vundle

* 基于Vundle安装 YouCompleteMe 源码
在~/.vimrc中加入以下
```
Plugin 'Valloric/YouCompleteMe'
```
然后输入 **:PluginInstall** 进行


## 编译 YouCompleteMe
经历过上述3个步骤后，YouCompleteMe 插件还没法使用，此时打开 Vim 时会看到如下的报错：

> The ycmd server SHUT DOWN (restart with ‘:YcmRestartServer’). YCM core library not detected; you need to compile YCM before using it. Follow the instructions in the documentation.


这是因为，YouCompleteMe 需要手工编译出库文件 ycm_core.so (以及依赖的libclang.so) 才可使用。

使用 Vundle 下载的 YouCompleteMe 源码保存在目录 ~/.vim/bundle/YouCompleteMe，在该目录下执行 .
```bash
$ cd .vim/bundle/YouCompleteMe/
$ ./install.py --clang-completer
```
即可编译具有C族语言的语义补全功能的 YouCompleteMe 插件。

至此，YouCompleteMe 插件已经**安装完成**

## 配置 YCM
ycm安装成功后，还不能代码补全提示，需要配置 **.ycm_extra_conf.py**

参考YCM自带的 YouCompleteMe/third_party/ycmd/examples/.ycm_extra_conf.py

* 设置全局配置文件，根据实际情况进行修改
```bash
$ cp YouCompleteMe/third_party/ycmd/examples/.ycm_extra_conf.py ~/
或
$ cp note/vim/.ycm_extra_conf.py ~/

$ vim /etc/vim/vimrc.local
let g:ycm_global_ycm_extra_conf='~/.ycm_extra_conf.py'
```

note/vim 目录有已经修改好的全局配置文件.ycm_extra_conf.py，可直接用于**Ｃ项目**

* 设置项目配置文件，根据实际情况进行修改
```bash
$ cp ~/.ycm_extra_conf.py <project root>
```

YCM配置文件的查找顺序是**当前目录>上层目录>...>根目录>全局配置文件**
搜索头文件的顺序是：**-I 指定目录、-isystem 指定目录、标准系统目录**

**此时正常进行语义补全!！!**

## vim配置(可选)
```
""""""""""""""""YouCompleteMe""""""""""""""""""""
" 指定全局配置文件
let g:ycm_global_ycm_extra_conf='~/.ycm_extra_conf.py'
" 自动加载 .ycm_extra_conf.py 配置文件
let g:ycm_confirm_extra_conf = 0
" 输入两个字母以上，自动进行语义补全
let g:ycm_semantic_triggers =  {
			\ 'c,cpp,python,java,go,erlang,perl': ['re!\w{2}'],
			\ 'cs,lua,javascript': ['re!\w{2}'],
			\ }
" 修改语义补全时的背景颜色为灰色
highlight PMenu ctermfg=0 ctermbg=242 guifg=black guibg=darkgrey
highlight PMenuSel ctermfg=242 ctermbg=8 guifg=darkgrey guibg=black

" 函数申明
nmap <C-d>c :YcmCompleter GoToDeclaration<CR>
" 函数定义
nmap <C-d>f :YcmCompleter GoToDefinition<CR>
" 显示语法错误详情
nmap <F4> :YcmDiags<CR>
" YCM提供的跳跃功能采用了vim的jumplist，
" 往前跳和往后跳的快捷键为Ctrl+O以及Ctrl+I
```

## 参考链接：
[YouCompleteMe 中容易忽略的配置](https://zhuanlan.zhihu.com/p/33046090)
[为YCM配置ycm_extra_conf.py脚本](https://www.jianshu.com/p/5aaae8f036c1)


