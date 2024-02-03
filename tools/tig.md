### BRIEF INTRODUCTION

Tig is an ncurses-based text-mode interface for git. It functions mainly as a Git repository browser, but can also assist in staging changes for commit at chunk level and act as a pager for output from various Git commands.

### INSTALL

```bash
$ sudo apt install tig
```

### EXPLICATE

```bash
$ man tig
```

### EXAMPLE

* Displays the author of each line of a file

  ```bash
  $ tig blame <file>
  ## same as
  $ git blame <file>
  ```
