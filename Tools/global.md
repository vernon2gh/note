## 安装

安装`GNU global`，以`ubuntu`为例：

```bash
$ sudo apt install global
```

安装后有如下命令

- gtags  : 生成GNU global的数据库
- global : 在shell命令行中进行查询
- gtags-cscope : 在vim中利用 cscope一样的界面进行查询

## 生成数据库

生成`GNU global`数据库，以`linux kernel`为例：

```bash
$ make ARCH=arm gtags # 只生成arm的数据库
或
$ gtags               # 不建议使用，默认会生成所有arch的数据库
```

生成如下文件

- GTAGS   : definition database
- GRTAGS:  reference database
- GPATH  :  path name database

## 查询

通过`GNU global`查询 函数在哪里定义 或 函数在哪里被调用等等，下面分别介绍`shell`, `vim`与`vscode`是如何进行查询

Ａ. 通过`shell`进行查询

```bash
$ global -x  <funcName> # 查找函数定义
$ global -rx <funcName> # 查找函数调用
```

Ｂ. 通过`vim`进行查询

首先，在`/etc/vim/vimrc.local`添加如下配置

```bash
set cscopeprg=gtags-cscope

if filereadable("GTAGS")
    cs add GTAGS
else
    let gtags_file=findfile("GTAGS", ".;")
    if !empty(gtags_file) && filereadable(gtags_file)
        exe "cs add" gtags_file
    endif
endif

nmap <C-@>g :cs find g <C-R>=expand("<cword>")<CR><CR> " Find function definition
nmap <C-@>c :cs find c <C-R>=expand("<cword>")<CR><CR> " Find function call
```

然后，用`vim`打开某一个文件后，就可以通过`ctrl-@ g`查询 函数在哪里定义，通过`ctrl-@ c`查询 函数在哪里被调用

Ｃ. 通过`vscode`进行查询

>  参考网址 : [GNU GLOBAL Source Code Tag System](https://www.gnu.org/software/global/globaldoc.html)

首先，安装 `C/C++ GNU Global`插件

然后，用`vscode`打开某一个文件后，就可以通过`右键->xxx`查询 函数在哪里定义 或 函数在哪里被调用

