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
$ git log
$ git show <commit>
```

## MANAGING BRANCHES

Review branch

```bash
$ git brach
```

Create new branch

```bash
$ git branch <new_branch> [tag_or_commint]
or
$ git checkout -b <new_branch> [tag_or_commint]
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

Push code to remote server

```bash
$ git push <remote_url> <local_branch>[:<remote_branch>]
```

Pull code from remote server

```bash
$ git pull <remote_url> <remote_branch>[:<local_branch>]
or
$ git fetch <remote_url> <remote_branch>:<local_branch>
```

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

