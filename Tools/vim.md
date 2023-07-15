## vim 快捷移动按键

* 左下上右

```
hjkl
```

* 将光标移动到 窗口顶部/中部/底部 位置

```
shift + l (window low)
shift + m (window middle)
shift + h (window high)
```

* 行内移动以单词为单位

```
w (word)
e (end)
b (begin)
^ (line begin)
$ (line end)
```

* 向前/后翻页

```
ctrl+f (forward page)
ctrl+b (back page)
```

* 将光标移动到 文件开头行与结束行

```
gg (file begin)
G  (file end)
```

* 删除一个字母/删除一个单词/改变一个单词

```
d  (delete)
dw (delete word)
cw (change word)
```

## 一些常见的配置

* vim

在`~/.vimrc`中加入如下配置：

```bash
set number   " display line number
set hlsearch " hight light search
autocmd BufWritePre * %s/\s\+$//e " when save file, auto delete line tail space
```

完整配置参考：[vimrc](../resources/config/.vimrc)

* neovim

复用 `~/.vimrc` 配置文件，执行如下命令：

```bash
$ ln -s ~/.vimrc ~/.config/nvim/init.vim
```

或者使用 `~/.config/nvim/init.lua` 来配置，如下：

```bash
vim.o.number = true
vim.o.mouse = ""
vim.cmd([[autocmd BufWritePre * %s/\s\+$//e]])
```

完整配置参考：[init.lua](../resources/config/init.lua)，
记得先手动安装 [packer](http://neovimcraft.com/plugin/wbthomason/packer.nvim/index.html) 插件，
然后在 nvim 中执行 `:PackerSync` 命令来安装插件

