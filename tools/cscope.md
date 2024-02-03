## 安装

安装`cscope`，以`ubuntu`为例：

```bash
$ sudo apt install cscope
```

## 生成数据库

生成`cscope`数据库，以`linux kernel`为例：

```bash
$ make ARCH=arm cscope          # 只生成arm的数据库
或
$ cscope -b -q -k -f cscope.out # 不建议使用，默认会生成所有arch的数据库
```

## 查询

通过`cscope`查询 函数在哪里定义 或 函数在哪里被调用等等，下面分别介绍`vim`与`vscode`是如何进行查询

A. 当通过`vim`进行查询

首先，在`/etc/vim/vimrc.local`添加如下配置

```bash
set cscopeprg=cscope

if filereadable("cscope.out")
    cs add cscope.out
else
    let cscope_file=findfile("cscope.out", ".;")
    let cscope_pre=matchstr(cscope_file, ".*/")
    if !empty(cscope_file) && filereadable(cscope_file)
        exe "cs add" cscope_file cscope_pre
    endif
endif

nmap <C-@>g :cs find g <C-R>=expand("<cword>")<CR><CR> " Find function definition
nmap <C-@>c :cs find c <C-R>=expand("<cword>")<CR><CR> " Find function call
```

然后，用`vim`打开某一个文件后，就可以通过`ctrl-@ g`查询 函数在哪里定义，通过`ctrl-@ c`查询 函数在哪里被调用

B. 通过`vscode`进行查询

首先，安装 `scope4code`插件

接着，配置`xxx.code-workspace`的`settings`属性，如下:

```bash
"scope4code.databasePath": "${workspaceRoot}/",
"scope4code.engineCommands": {
    "config_index": {
        "cscope": {
            "linux": 0
        }
    },
    "config": [
        {
            "find_cmd": "find ${src_path} -type f -name *.c -o -type f -name *.h -o -type f -name *.cpp -o -type f -name *.cc -o -type f -name *.mm",
            // "database_cmd": "cscope -b -q -k -f ${database_path}/cscope.out",
            "database_cmd": "make ARCH=x86 cscope",
            "find_all_ref": "cscope -q -k -f ${database_path}/cscope.out -L0 ${text}",
            "find_define": "cscope -q -k -f ${database_path}/cscope.out -L1 ${text}",
            "find_callee": "cscope -q -k -f ${database_path}/cscope.out -L2 ${text}",
            "find_caller": "cscope -q -k -f ${database_path}/cscope.out -L3 ${text}",
            "find_text": "cscope -q -k -f ${database_path}/cscope.out -L4 ${text}"
        }
    ]
}
```

然后，用`vscode`打开某一个文件后，就可以通过`右键->xxx`查询 函数在哪里定义 或 函数在哪里被调用

