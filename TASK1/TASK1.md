# TASK1

1.编译器是一种可执行文件，跟我们常见的window上面的exe，linux上面的ELF文件类似

编译器的输入是高级语言(例如C,C++,Rust等等)

输出是一个可执行文件

至于大致工作流程，我在[#4 MakeFile & CMakeList](https://d.jotang.club/t/topic/942#h-4-makefile-cmakelist-33) : C/C++代码构建工具里面大概介绍了

分为四个阶段，预处理，编译，汇编，链接

2.前端：

过程分为 词法分析 语法分析 语义分析 IR生成

主要任务就是把输入(一串高级语言的字符串)通过上面的四个过程，生成输出（IR）

3.中端：

中端的主要任务就是通过Pass(对IR进行遍历)对IR进行优化和转化

所以Pass分成两种

分析Pass：寻找IR特性，发掘优化机会

转化Pass：生成必要的数据结构给后端使用