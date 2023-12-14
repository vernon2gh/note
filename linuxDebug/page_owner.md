> Documentation/mm/page_owner.rst

1. build Linux Kernel

```
CONFIG_PAGE_OWNER
```

page owner is disabled by default.

2. enable

add `page_owner=on` to boot cmdline.

3. do the job that you want to debug.

4. the tracking about who allocated each page.

```bash
$ cat /sys/kernel/debug/page_owner
```

5. (options) build user-space helper

```bash
$ cd tools/mm
$ make page_owner_sort

$ cat /sys/kernel/debug/page_owner > page_owner_full.txt
$ ./page_owner_sort page_owner_full.txt sorted_page_owner.txt
```
