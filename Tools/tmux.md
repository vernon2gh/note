进入`tmux`环境

```bash
$ tmux
```

基础操作

```bash
ctrl-b c # 创建新窗口
ctrl-b n # 切换到后一个窗口
ctrl-b p # 切换到前一个窗口

ctrl-b % # 分割新垂直窗口
ctrl-b " # 分割新水平窗口
ctrl-b o # 光标聚焦到下一个分割窗口
```

复制粘贴

```bash
ctrl-b [     # 进入复制模式
ctrl-b space # 选择需要复制的内容
ctrl-b w     # 将需要复制的内容保存到缓冲区
ctrl-b ]     # 粘贴

$ tmux list-buffers                             # 查看缓冲区
$ tmux save-buffer -b [buffer_name] [save_path] # 将缓冲区保存到文件中
```

后台执行

```bash
$ ctrl-b d    # 退出 session
$ tmux attach # 重新进入 session
```

