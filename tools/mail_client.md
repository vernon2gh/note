# Mail Client

## 简介

一个完整的邮件客户端需要包括三大部分：收取邮件、发送邮件、阅读邮件。

* 收取邮件一般是由 IMAP、POP3 协议来进行
* 发送邮件是由 SMTP 协议来进行

Linux 邮件客户端有 TUI 或 GUI 类型，其中 TUI 类型比较著名的是 mutt、neomutt 等等，
GUI 类型比较著名的是 Thunderbird 等等

我的目的只是单纯想回复 LKML 的某一个邮件，所以选择 mutt

## mutt

mutt 收取/发送/阅读邮件

通过执行 `$ sudo apt install mutt` 来安装 mutt，然后参考
[Email clients info for Linux¶ ](https://www.kernel.org/doc/html/latest/process/email-clients.html)
Mutt (TUI) 小节，配置 `~/.muttrc`，然后将用户名/密码替换一下，运行 `$ mutt` 就可以正常使用。

> 目前在国内使用 gmail，Mutt 读取邮箱的速度太慢了，这是 gmail 被墙的问题，所以需要
将 gmail 邮件下载到本地，这样就不用每次都访问 gmail 读取邮箱，从而使用下面的软件搭配。

## offlineimap + mutt

使用 offlineimap 收取邮件，mutt 发送/阅读邮件

通过执行 `$ sudo apt install offlineimap` 来安装 offlineimap，参考
`/usr/share/doc/offlineimap/examples/offlineimap.conf[.minimal]` 配置
`~/.offlineimaprc`，如下：

```
[general]
accounts = Gmail

[Account Gmail]
localrepository = Local
remoterepository = Remote

[Repository Local]
type = Maildir
localfolders = ~/Mail

[Repository Remote]
type = IMAP
remotehost = imap.gmail.com
remoteuser = xxx@gmail.com
remotepass = xxx
sslcacertfile = OS-DEFAULT
folderfilter = lambda foldername: foldername in ['INBOX']
```

配置完成后，通过 `$ offlineimap` 将 gmail 邮件下载到本地，接着配置 `~/.muttrc` 读取
offlineimap 下载到本地的邮件，如下：

```
# ================  IMAP ====================
set folder = ~/Mail
set spoolfile = "+INBOX"

# ================  SMTP  ====================
set smtp_url = "smtp://xxx@smtp.gmail.com:587/"
set smtp_pass = "xxx"
set ssl_force_tls = yes # Require encrypted connection

# ================  Composition  ====================
set editor = "vi"
set edit_headers = yes  # See the headers when editing
set charset = UTF-8     # value of $LANG; also fallback for send_charset
# Sender, email address, and sign-off line must match
unset use_domain        # because joe@localhost is just embarrassing
set realname = "xxx xxx"
set from = "xxx@gmail.com"
set use_from = yes

# ================  Others  ====================
set sort = threads
set sort_aux = reverse-last-date-received
```

这时候运行 `$ mutt` 就可以阅读本地邮件了，以后 gmail 有新邮件时，需要手动执行
`$ offlineimap` 将新邮件下载到本地。

### 未来

ubuntu 默认安装 Python2 版本的 offlineimap，官方已经停止维持，最新是 Python3版本
（offlinemap3）

neomutt 是 mutt 的下游产品，添加更多功能

因为刚刚开始，所以准备先用 offlineimap + mutt，后面慢慢优化，
更新到 offlineimap3 + neomutt

## wget + mutt

有时候我们想要回复 LKML 的邮件，但是又没有将邮件抄送给我们，
我们要如何回复 LKML 的邮件？

在 LKML 找到对应想要回复的邮件对应的 raw 链接，即 mbox 文件。
然后通过 wget 下载 mbox 文件，导入到 mutt 中

```bash
$ wget https://lore.kernel.org/xxx/raw
$ mutt -f raw
```

此时已经进入到 mutt，能够按照正常操作回复此邮件了

## mutt 颜色配置

* Patch 高亮

```
# ==============  Patch color  =================
color   body    red             black    "^-.*"
color   body    green           black    "^[+].*"
color   body    brightwhite     black    "^diff --git.*"
color   body    brightwhite     black    "^index [a-f0-9].*"
color   body    brightwhite     black    "^\-\-\- a.*"
color   body    brightwhite     black    "^[\+]{3} b.*"
color   body    brightyellow    black    "^@@.*"
color   body    brightmagenta   black    "^(Signed-off-by).*"
color   body    brightmagenta   black    "^(Reported-by).*"
color   body    brightmagenta   black    "^(Suggested-by).*"
color   body    brightmagenta   black    "^(Acked-by).*"
color   body    brightmagenta   black    "^(Reviewed-by).*"
color   body    brightmagenta   black    "^\-\-\-$"
color   body    white           black    "^( \#define).*"
color   body    white           black    "^( \#include).*"
color   body    white           black    "^( \#if).*"
color   body    white           black    "^( \#el).*"
color   body    white           black    "^( \#endif).*"

color   body    green           black    "LGTM"
color   body    brightmagenta   black    "-- Commit Summary --"
color   body    brightmagenta   black    "-- File Changes --"
color   body    brightmagenta   black    "-- Patch Links --"
color   body    green           black    "^Merged #.*"
color   body    red             black    "^Closed #.*"
color   body    brightblue      black    "^Reply to this email.*"
```

* 嵌套引用高亮

```
# ==============  Quoted Color  =================
set quote_regexp = "^([ \t]*[>])+"

color   quoted  blue            black
color   quoted1 magenta         black
color   quoted2 cyan            black
color   quoted3 yellow          black
color   quoted4 red             black
```

## 参考

* [offlineimap](http://www.offlineimap.org/about/)
* [mutt](http://www.mutt.org/doc/manual/)
* [Mutt: 阅读邮件列表](https://fancyseeker.github.io/2015/08/19/mutt/)
