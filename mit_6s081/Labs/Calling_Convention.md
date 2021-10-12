### 栈

调用函数时，需要保存上下文环境，比如：返回地址、参数、局部变量等等，这些上下文环境就保存在**栈**中

`frame / stack frame`，也叫 栈帧。一个函数对应一个栈帧，而且栈帧的前两个8字节必须分别存储 **返回地址** 与 **上一个栈帧的帧指针**

`fp / frame pointer`，也叫 帧指针，指向 栈帧开头

`sp / stack pointer`，也叫 栈指针，一直指向 栈顶

### gdb

```bash
(gdb) apropos tui # 显示tui帮助信息

(gdb) tui enable # 启动tui，默认只显示源码

(gdb) layout asm   # 显示汇编
(gdb) layout regs  # 显示寄存器
(gdb) layout src   # 显示源码
(gdb) layout split # 显示源码与汇编

(gdb) focus asm    # 光标定位到汇编窗口
(gdb) focus src    # 光标定位到源码窗口
(gdb) focus cmd    # 光标定位到命令窗口

(gdb) break main       # 在main()开头打一个断点 
(gdb) info breakpoints # 显示全部断点
(gdb) info registers   # 显示所有寄存器
(gdb) delete breakpoints # 删除全部断点

(gdb) backtrace    # 显示所有栈帧
(gdb) info frame   # 显示当前栈帧的内容，默认是0号栈帧，即leaf(叶子栈帧)
(gdb) info frame 1 # 指定显示1号栈帧的内容
(gdb) frame 1      # 将当前栈帧切换到1号栈帧

(gdb) ni # 单步执行一条汇编指令，但是不进入函数
(gdb) si # 单步执行一条汇编指令，可以进入函数

## 如果gcc优化某些变量，gdb无法将这些变量打印出来，需要将gcc优化标志设置成-O0后，再对源码重新编译即可
(gdb) info args        # 显示当前函数的参数值
(gdb) print argc       # 打印main() argc变量的值
(gdb) print argv       # 打印main() argv变量的值
(gdb) print *argv      # 打印main() argv[0]变量的值
(gdb) print *argv@2    # 打印main() argv[0~1]变量的值
(gdb) print *argv@argc # 打印main() argv所有变量的值

(gdb) info locals        # 显示所有局部变量
(gdb) watch i            # 当变量i的值发生变化时，中断，显示变量i的值
(gdb) break func if i==5 # 当i==5时，中断，进入func()
```

### tmux

```bash
$ tmux
ctrl-b c # 创建新窗口
ctrl-b n # 切换到后一个窗口
ctrl-b p # 切换到前一个窗口

ctrl-b % # 分割新垂直窗口
ctrl-b " # 分割新水平窗口
ctrl-b o # 光标聚焦到下一个分割窗口

ctrl-b [     # 进入复制模式
ctrl-b space # 选择需要复制的内容
ctrl-b w     # 将需要复制的内容保存到缓冲区
ctrl-b ]     # 粘贴

$ tmux list-buffers                             # 查看缓冲区
$ tmux save-buffer -b [buffer_name] [save_path] # 将缓冲区保存到文件中
```

