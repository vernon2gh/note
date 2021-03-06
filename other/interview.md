#### 问：面试前需要刷题吗？

答：适当刷题即可，大部分时间总结下自己的项目，用到了哪些技术，是怎么用的，怎么体现自己比较牛逼

#### 面试官一般

1. 先简单介绍下你自己吧
2. 你都做过什么项目，简单介绍下吧
3. 项目中遇到过什么难题，是怎么解决的

提前在家把自己多问几遍，不要到面试的时候再组织

#### 工作1年，3年，5年 侧重点也是不一样的

1年就看你C语言基础，一些基本概念知不知道，爱不爱学习，培养价值大不大。

3年就看你项目做的怎么样，一些API的底层实现知道不知道。比如调用了C语言的Open函数，一步一步是怎么到磁盘上的，

5年就看你的架构能力，系统设计能力，带人能力

3年以内的可能会笔试，3年以上的很少笔试了

**NOTE:** 跨行业也要注意下，因为你的项目经验对他们没用，面试官也可能不懂。这时候基本就会和你拼基础了。

1. 比如面试网络开发，从一个http请求发起，到物理层是怎样一步步走的，必须得很熟悉，不然人家就会说还行，但是基础一般，批发价收了吧。这个主要是针对做应用的。
2. 比如面试linux开发底层开发的。一定会问你你对linux哪个模块最熟，是怎样实现的，要讲的很清楚

#### 问：就说一个模块就可以吗
答：可以，就把你做的这个模块说清楚就行了，如果模块太简单，可能会认为你不牛逼，但是不至于不诚实，如果你说整个系统，你说不好，人家就会觉得你说大话

这是真实的案例，不过要看你面试的是什么岗位，比如你面试的系统软件部，专门解决程序运行过程中的各种跑飞问题，内存问题等疑难杂症，他就要考察你的计算机系统知识扎实不扎实。

#### 问：这个模块的在项目中的作用？还是说内核调用过程用那个函数？还是说调试出那些Bug？
答：不用说函数

1. 把这个模块当做一个黑盒子，输入什么，输出什么

2. 这个黑盒子内部分几个模块，每个模块的作用，怎么联系起来的

一般这么一聊，面试官就开始和你互通起来了，这时候就注意把话题往考点上引

#### 考点就是工作中大家都可能会卡主或者吃亏的点

比如你说你做的这个模块通过串口个别的模块通信，那么，串口过来的帧来不及处理怎么办，是不是一个循环缓冲区的概念出来，有帧了，帧肯定会粘连在一起，应用怎么分帧呢，怎么保证可靠呢，是不是又引出了一个考点…

当然我说的这些都是正常的面试官，不包含比价二的，比如问你某个函数有几个参数