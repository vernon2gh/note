### 1. rebase

当开发某一个功能时，通过`git commit`提交多次修改，想将这此修改合并成一个`commit`进行发布，可能通过`git rebase`进行操作

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

# 1. 选择需要合并的范围, b2dba72~20b6f52(不包括b2dba72)
$ git rebase -i b2dba72 20b6f52
or
$ git rebase -i b2dba72
or
$ git rebase -i HEAD~3
pick 373f063 2
pick f845618 3
pick 20b6f52 add: 1
## 2. 修改上述信息，重新排列以及合并commit，如 将20b6f52重新排列到373f063后面，同时与373f063合并
pick 373f063 2
squash 20b6f52 add: 1
pick f845618 3
## 3. 然后保存退出，修改commit信息，再保存退出即可。
 $ git log
commit 4612f3b2bf08ef705b2b70dc8f2e1d8f14023a9b (HEAD)
    3

commit c02e774166119ff4830b5a2f4db48b97b74dd290
    2
    
    test: add: 1

commit b2dba726e7606b9f5374218c236c4e2d5efb7877
    Init
```

