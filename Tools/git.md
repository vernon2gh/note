## DESCRIPTION

The git is source code version control software.

1. install git, example Ubuntu/Debian.

   ```bash
   $ sudo apt install git
   ```

2. Configure personal information

   ```bash
   $ git config --global user.name "Your Name Comes Here"
   $ git config --global user.email you@yourdomain.example.com
   ```

3. Check personal information

   ```bash
   $ git config -l
   ```

## IMPORTING A NEW PROJECT

init git repositories

```bash
$ git init .
```

add files that want to trace

```bash
$ git add <files>
```

commit files to git repositories for tracing

```bash
$ git commit -m '
```

## VIEWING PROJECT HISTORY

```bash
$ git log [--reverse] [file]
$ git show <commit>
```

## MANAGING BRANCHES

Review branch

```bash
$ git brach
```

Create new branch from the current tag/commit

```bash
$ git branch <new_branch> [tag_or_commint]
or
$ git checkout -b <new_branch> [tag_or_commint]
```

Create new branch from the previous commit of a commit

```bash
$ git branch <new_branch> <commint>~1
## such as:
$ git branch slub 81819f0fc828~1
```

Checkout branch

```bash
$ git checkout <branch>
```

Delete branch

```bash
$ git branch -d <branch>
```

Modify branch name

```bash
$ git branch -M <branch>
```

## REMOTE

The remote server has github, gitlab, gitee and private git server.

Clone remote server git repositories

```bash
$ git clone <remote_url>
```

Push code/tags to remote server

```bash
$ git push [--tags] <remote_url> <local_branch>[:<remote_branch>]
```

Pull code/tags from remote server

```bash
$ git pull [--tags] <remote_url> <remote_branch>[:<local_branch>]
or
$ git fetch <remote_url> <remote_branch>:<local_branch>
```

## PATCH

output `<commit>` patch only

```bash
$ git format-patch -1 <commit>
```

Output the patch of a certain range, such as the patch of `commit1~commit3`, excluding `commit1` patch

```bash
$ git format-patch <commit1>..<commit3>
```

output all patches of a file

```bash
## such as: mm/slub.c
$ i=0; for line in `git log --reverse --pretty=format:"%h" mm/slub.c`; do i=$[$i+1]; git format-patch -1 --start-number $i $line -o patches/; done
```

see if the patch is applicable to the current working tree and/or the index file and detects errors. Turns off apply.

```bash
$ git apply --check <patch>
```

apply patch to git repositories

```bash
$ git am <patch>
```

## SUB-MODULES

Add sub-modules into a repository

```bash
$ git submodule add <repository URL> [path]
$ git commit -m '
```

Cloning a repository that contains submodules

```bash
## The first method
$ git clone <repository URL>
$ git submodule update --init [--recursive]

## The second method
$ git clone --recurse-submodules <repository URL>
```

Update sub-modules

```bash
## The first method
$ cd <sub-modules>
$ git pull
$ cd <main-repository>
$ git add .
$ git commit -m '

## The second method
$ cd <main-repository>
$ git submodule update --remote [sub-modules]
$ git add .
$ git commit -m '
```

## EMAIL

> 前提：Shell 终端能够访问 gmail 服务器

使用基于 gmail 的 `git send-email` 功能，需要完成如下：

1. gmail 开启 IMAP 与 应用专用密码，
   参考 [通过其他电子邮件平台查看 Gmail](https://support.google.com/mail/answer/7126229)
2. 执行 `apt install git-email`，安装 `git send-email` 功能,
   详细配置参考 `git help send-email` 的 `EXAMPLES` 小节

完成以上两点后，就能够使用 `git send-email` 进行发送邮件

如果想要给 Linux Kernel 提交 patch，需要按照以下步骤来执行：

1. 修改源码，编译运行，测试，确保能够正常运行 patch 对应的功能

2. 提交源码，如下：

   ```bash
   $ git add xxx
   $ git commit -s -m '
   module name: fix some bugs

   why and how
   '
   ```

3. 生成 patch

```bash
$ git format-patch -M origin/master -o patch/
```

将与 origin/master 不同的 commit 都生成 patch，存储在 `patch/` 目录下

可选：

* `--cover-letter` ：生成以 `0000-` 为前缀的邮件封面，需要指定标题、简介。
* `-v <version>`   ：指定新版本
  注意：第二版本后的邮件封面，除了需要指定标题、简介外，
  还需要写 版本改变日志、之前的版本在 `https://lore.kernel.org` 的链接
* `--base=auto`    ：显示是基于哪一个 commit 进行修改

4. 通过 `./scripts/checkpatch.pl xxx.patch` 检查 patch 格式是否符合要求

5. 通过 `./scripts/get_maintainer.pl xxx.patch` 获得 maintainer 邮箱地址

6. 通过 `git send-email --to xxx --cc xxx xxx.patch` 来发送 patch 给 maintainer。
   如果需要发送给多个邮箱地址，用 `,` 作为分隔符。如果是 patchset，发送时选择 a[ll]

## TIPS

tag

```bash
## view
$ git tag
## create
$ git tag <version>
```

look for string

```bash
$ git grep "string"
```

compare differ branch

```bash
$ git diff <branch_1> <branch_2> [files_path_or_dir]
```

let's some commit of other branch to current branch

```bash
$ git cherry-pick <commit>
```

get files of some commit of other branch

```bash
$ git checkout <commit> <file name>
```

When you develop a feature, commit multiple changes through `git commit`. If you want to merge this change into a single `commit`, you may do so through `git rebase`

```bash
$ git log
commit 20b6f52f5d70a07117f6d11b2902d72f69ff1ae5 (HEAD -> test)
    add: 1

commit f845618b13247321e7b3fcd27fd94d0392dbe6f9
    3

commit 373f063d4f68a1cdec3b6b29676d4203a86b4387
    2

commit b2dba726e7606b9f5374218c236c4e2d5efb7877
    Init

# 1. Select the range that you want to merge, b2dba72~20b6f52(exception b2dba72)
$ git rebase -i b2dba72 20b6f52
or
$ git rebase -i b2dba72
or
$ git rebase -i HEAD~3
pick 373f063 2
pick f845618 3
pick 20b6f52 add: 1
## 2. Modify the above information, rearrange, and merge commit, such as rearranging 20B6f52 after 373f063 and squash to 373f063
pick 373f063 2
squash 20b6f52 add: 1
pick f845618 3
## 3. Then save and exit, change the commit information, and save the exit.
 $ git log
commit 4612f3b2bf08ef705b2b70dc8f2e1d8f14023a9b (HEAD)
    3

commit c02e774166119ff4830b5a2f4db48b97b74dd290
    2

    test: add: 1

commit b2dba726e7606b9f5374218c236c4e2d5efb7877
    Init
```

The remote repository has new commits, but the local repository also has new commits. How to synchronize the new commits of the remote repository to the local repository?

```bash
$ git rebase <branch>
```

When we need to actively merge new feature branches

```bash
$ git merge <branch>
```

Displays the commit of each line of a file

```bash
$ git blame <file>
```

当我们想要知道commit A是在哪一个新merge feature合入时，如何找到对应的merge commit ？

> https://blog.csdn.net/qq_39734650/article/details/116658540

```bash
## ~/.gitconfig
[alias]
	find-merge = "!sh -c 'commit=$0 && branch=${1:-HEAD} && (git rev-list $commit..$branch --ancestry-path | cat -n; git rev-list $commit..$branch --first-parent | cat -n) | sort -k2 -s | uniq -f1 -d | sort -n | tail -1 | cut -f2'"
	show-merge = "!sh -c 'merge=$(git find-merge $0 $1) && [ -n \"$merge\" ] && git show $merge'"

$ git find-merge <commit A>
```

当我们想要研究某一个新merge feature时，merge commit会显示feature commit范围，比如 （A，F]，其中不包括A，包括F。如何显示此feature所有commit ？

```bash
$ git log --online --graph <merge commit>
```
