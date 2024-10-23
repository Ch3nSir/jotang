# TASK4-1 IR & SSA & CFG

## IR中**φ函数**的作用

完成这题的过程中主要学习了这篇文档[《什么是SSA》](https://fail.lingfei.xyz/_posts/SSA/)

1.当控制流中出现多个分支，且每个分支中都对某个元素进行了赋值操作，这个时候底部的基本块中就无法确定该元素的下标。

![image-20220827230246087.png](https://s2.loli.net/2022/08/29/TNlVD3nFg1Xj7sy.png)

此时我们引入**φ函数**，来根据实际的执行流来选择元素的赋值。（下面的例子就是根据执行流选择y.1或y.2用以确定y.3的值）

![image-20220828160404389.png](https://s2.loli.net/2022/08/29/ite5fR7GqHr9gM3.png)

然后因为大多数机器都没有**φ函数**对应的特殊指令，所以我们通过在每个基本块后面插入move的操作来代替。（上面的例子中，编译器就会在左侧的基本块中插入move y.3,y.1的操作，右侧基本块则是mov y.2,y.1)

了解完**φ函数**的基本原理之后，你可能会产生一个疑问，我们在实际的code中应该如何确定插入**φ函数**的位置呢？

这个时候就要用到我们卓中卓的一轮招新的第三题《图中的支配关系》，使用Dominance Tree和Dominance Frontier，帮助我们确定在哪里插入**φ函数**。

下面我们先给出文章中原文的**Dominance Frontier定义**：

首先需要定义dominator的概念：如果任何到达节点B的执行路径都必须首先经过节点A，那么就称节点A严格控制节点B。如果A严格控制B或A =  B，那么就说A控制B（B被A控制）。并且通过严格控制，我们可以得到如下结论：如果当前指令执行到了B，那么节点A中的指令必然已经全部执行完毕。

接下来定义dominance  frontier：如果A没有严格控制B，但是控制了一些B的直接前驱，或A是B的直接前驱，并且由于任何节点控制其自身，那么我们称节点B在A的dominance frontier中。从节点A的角度来看，这些节点是其他不经过A的控制路径中最早出现的节点（换句话说，其他路径可能带来的**同一变量的其他定义**，导致冲突，例如上图的y.3）。

总的来说，如果一个节点X定义了一个变量，那么这个定义将到达X支配的所有节点。但是，当我们离开这些节点并进入X的dominance  frontier时（进入dominance  frontier就说明了此时的节点并不是由X严格支配，可能在其他分支上面可能产生同一变量的其他定义），我们必须考虑**其他路径可能带来的冲突**。因为在编译期间无法确定将采用哪一条分支，所以我们需要在适当的位置放置**φ函数**来合并这些可能的不同定义。

最后文章提出了一种根据变量的地址赋值的方法，但是带来了新问题，寄存器的直接访问变为了对于内存的访问（访问效率会变慢）。此时LLVM optimizer提供了一个称为mem2reg的pass来将alloca提升为SSA寄存器，并在合适的地方插入φ结点。

## SSA形式

wiki百科上面介绍了许多经典的SSAhttps://en.wikipedia.org/wiki/Static_single-assignment_form#History

- ### **经典SSA**

1.简化编译优化，每个变量只赋值一次，消除了重定义问题。

2.通过 `Φ` 函数直接合并多个值，极大地简化了数据流分析。

- ### "Minimal" SSA 

1.在经典SSA中，每个变量在可能需要合并的地方都会引入 `Φ` 函数，但有些地方的 `Φ` 函数可能是不必要的。“Minimal" SSA则消除了部分 冗余的`Φ` 函数。

- ### **Pruned SSA**

  作者提出Pruned SSA算法是因为“Minimal" SSA中并没检测“死代码”。

  1.于是Pruned SSA通过追踪变量是否“活跃”（即该变量是否在之后的某条执行路径上会被使用），只为那些活跃的变量引入 `Φ` 函数。如果某个变量在 `Φ` 函数插入点之后不会被使用，就不会插入这个 `Φ` 函数，这样就忽略了死代码。

- ### Semi-pruned SSA

  作者提出Semi-pruned SSA是因为上面的Pruned SSA追踪变量是否“活跃”时，对性能的开销太大了，如果项目比较大型会更加耗时。

  1.  Semi-pruned SSA通过一种更简单的方式减少 `Φ` 函数：只检查一个变量是否在某个基本块的开头是“本地定义的”（也就是只在这个块中使用）。如果一个变量从未在进入一个基本块时活跃，就不需要为这个块的开头插入 `Φ` 函数。这种方式虽然没有修剪SSA那么精确，但计算起来更高效。

## **LLVM IR的基本语法**

找到了一个介绍LLVM IR的博客讲解的不错喵https://csstormq.github.io/blog/LLVM%20%E4%B9%8B%20IR%20%E7%AF%87%EF%BC%881%EF%BC%89%EF%BC%9A%E9%9B%B6%E5%9F%BA%E7%A1%80%E5%BF%AB%E9%80%9F%E5%85%A5%E9%97%A8%20LLVM%20IR

#### Identifiers

LLVM IR 中的标识符分为：全局标识符和局部标识符。全局标识符以`@`开头，比如：全局函数、全局变量。局部标识符以`%`开头，类似于汇编语言中的寄存器。

标识符有如下 3 种形式：

- 有名称的值（Named Value），表示为带有前缀（`@`或`%`）的字符串。比如：%val、@name。
- 无名称的值（Unnamed Value），表示为带前缀（`@`或`%`）的无符号数值。比如：%0、%1、@2。
- 常量。

#### Functions

`define`用于定义一个函数。

语法：

```
define [linkage] [PreemptionSpecifier] [visibility] [DLLStorageClass]
       [cconv] [ret attrs]
       <ResultType> @<FunctionName> ([argument list])
       [(unnamed_addr|local_unnamed_addr)] [AddrSpace] [fn Attrs]
       [section "name"] [comdat [($name)]] [align N] [gc] [prefix Constant]
       [prologue Constant] [personality Constant] (!name !N)* { ... }
```

示例（in Ubuntu 20.04）：

```
define dso_local void @foo(i32 %x) #0 {
  ; 省略 ...
}
```


# TASK4-2

## Value 、User 、Use

这个部分我主要学习了一篇博客https://www.cnblogs.com/Five100Miles/p/14083814.html

>  The `Value` class is the  most important class in the LLVM Source base. It represents a typed  value that may be used (among other things) as an operand to an  instruction. There are many different types of `Value`s, such as [Constant](https://link.zhihu.com/?target=https%3A//llvm.org/docs/ProgrammersManual.html%23constant)s, [Argument](https://link.zhihu.com/?target=https%3A//llvm.org/docs/ProgrammersManual.html%23argument)s. Even [Instruction](https://link.zhihu.com/?target=https%3A//llvm.org/docs/ProgrammersManual.html%23instruction)s and [Function](https://link.zhihu.com/?target=https%3A//llvm.org/docs/ProgrammersManual.html%23c-function)s are `Value`s.

`Value` 类是LLVM源代码库中最重要的类。它代表一个有类型的值，可以被用作指令的操作数（等等）。有许多不同类型的 `Value` ，比如常量、参数。甚至指令和函数也是 `Value` 。(继承自Value)

**LLVM中一切皆Value**. Value类是LLVM中非常重要的类, 程序中的变量/常量/表达式/符号都可以被视作一个Value.

![img](https://pic1.zhimg.com/80/v2-0c3a0f3b9faa946d0a44394dab4385c8_1440w.jpg)

**Value** 

- 含义：最基础的类，文中说一切皆Value，程序中的变量/常量/表达式/符号都可以被视作一个Value.

- 三大核心成员：

  - Use_list：记录了所有使用该操作数的指令的列表

  - name：名字

  - VTy：类型，一个type类，表示操作数的类型（Void也要记录）

    


    常见API（名字可能不太对）：
    
    ```cpp
    Type *get_type() const //返回这个操作数的类型
    std::list<Use> &get_use_list() // 返回value的使用者链表
    void add_use(Value *val, unsigned arg_no = 0);
    // 添加val至this的使用者链表上
    void replace_all_use_with(Value *new_val);
    // 将this在所有的地方用new_val替代，并且维护好use_def与def_use链表
    void remove_use(Value *val);
    // 将val从this的use_list_中移除
    ```

  


正如招新题所说的，我们需要一个有效方便的数据结构来表示哪个Value使用了哪个Value。于是产生了Use类。
如果一个Value使用到了另一个Value即产生一条有向边, Use类(defined in include/llvm/IR/Use.h)被用来描述这种边.
下面来看看Use类的源代码

```cpp
class Use {
public:
  friend class Value;
  friend class User; //Use声明了友元以供Value与User访问其成员, 因此Value/User可以调用Use的成员函数.

  operator Value *() const { return Val; }
  Value *get() const { return Val; }
  User *getUser() const { return Parent; };
  inline void set(Value *Val);
  inline Value *operator=(Value *RHS);
  inline const Use &operator=(const Use &RHS);
  Value *operator->() { return Val; }
  const Value *operator->() const { return Val; }
  Use *getNext() const { return Next; }
  unsigned getOperandNo() const;
  static void zap(Use *Start, const Use *Stop, bool del = false);

private:
  Value *Val = nullptr;
  Use *Next = nullptr;
  Use **Prev = nullptr;
  User *Parent = nullptr;

  // ......
};
```

Use类的设计是一个经典的双链表结构, 一共包含4个成员, 分别是:

1. Val - 指向被使用的Value对象.

2. Next - **下一个指向同一个值的`Use`节点**.

   当一个`Value`对象（即某个值，如一个变量、常量或指令的结果）被多个地方使用时，这些“使用”（`Use`）会形成一个链表。每个`Use`节点都指向下一个使用同一个`Value`对象的`Use`节点。`Next`成员就是指向这个链表中的**下一个`Use`节点**。

   eg:

   假设有一个 `Value` 对象 `V` 被三个不同的指令 `I1`, `I2`, `I3` 使用，那么会有三个对应的 `Use` 对象 `U1`, `U2`, `U3`，它们的 `Next` 链接如下：

   ```
   V  <-- (指向 V 的使用)
   ^        ^
   |        |
   U1 --> U2 --> U3 --> null
   ```

3. Prev - **上一个指向同一个值的`Use`节点**, 注意是双向链表的设计, 这里用的二层指针.(这里与上面同理了就)

4. Parent - 使用该Value的User(边的源节点).

下面介绍更复杂一点的**User**

1. 首先从抽象的角度User必须继承自Value, 因为从直观上来讲一个Value可以同时是一个Value的Usee且又是另一个Value的User.
2. 另一方面Use是依赖于User存在的(只有User知道它需要多少依赖以及依赖指向哪个Value), 因此User负责对Use的管理(申请与释放).
3. 由于User是一个通用的基础类, 而不同指令的操作数个数又不同, 导致Use不能作为User的成员存在(容器方式存储可以, 但是效率降低).
    让我们来看下User类(defined in include/llvm/IR/User.h)的定义.

```cpp
class User : public Value {
protected:
  User(Type *ty, unsigned vty, Use *, unsigned NumOps)
      : Value(ty, vty) {
    assert(NumOps < (1u << NumUserOperandsBits) && "Too many operands");
    NumUserOperands = NumOps;
    // If we have hung off uses, then the operand list should initially be
    // null.
    assert((!HasHungOffUses || !getOperandList()) &&
           "Error in initializing hung off uses for User");
  }
};
```

在构造User对象时额外分配一个指针用来保存Use数组

User最重要的特性就是，提供一个操作数表，表中每个操作数都直接指向一个 Value, 提供了 use-def 信息，它本身是 Value 的子类， Value 类会维护一个该数据使用者的列表，提供def-use信息。

同时User类最重要的两个成员如下：

- operands_：参数列表，表示这个使用者所用到的参数
- num_ops_：表示该使用者使用的参数的个数

下面是对他的一些API的介绍

```cpp
Value *get_operand(unsigned i) const;
// 从user的操作数链表中取出第i个操作数
void set_operand(unsigned i, Value *v);
// 将user的第i个操作数设为v
void add_operand(Value *v);
// 将v挂到User的操作数链表上
unsigned get_num_operand() const;
// 得到操作数链表的大小
void remove_use_of_ops();
// 从User的操作数链表中的所有操作数处的use_list_ 移除该User;
void remove_operands(int index1,int index2);
// 移除操作数链表中索引为index1-index2的操作数，例如想删除第0个操作数：remove_operands(0,0)
```



总结可知三者的关系

1.**`Value`**：表示LLVM IR中的值，它可以是指令、变量、常量、函数参数等。

2。**`Use`**：表示 `Value` 被使用的情况，链接了一个 `Value` 和其所有的 `User`。

3.**`User`**：表示使用了一个或多个 `Value` 的对象，典型的例子是指令。`User` 本身也是 `Value` 的子类，因此它也可以被其他指令或操作所引用。

下图则是链表操作，已经在Use的设计中讲到了许多部分（Val Next Prev Parent UseLitt Vty



![img](https://img2020.cnblogs.com/blog/1335902/202012/1335902-20201206025748757-322294594.jpg)

## User维护Usee、UseList

User持有对Value的引用，并且可以有多个Value作为其操作数（Operands），可以通过一些函数API接口调用例如remove_use_of_ops( )从User的操作数链表中的所有操作数处的Use_list 移除该User，

## 关于Use维护Value、User的关系

每个Use对象持有一个Value的引用，并且知道这个Value是由哪个User使用的。Use允许LLVM跟踪每个Value的所有使用情况，并且当Value被修改或删除时，可以更新所有引用它的地方。User和Use之间存在一个双向关系：User持有一个Use链表，每个Use指向一个Value；Value持有一个User链表，每个User指向一个使用该Value的User对象。

具体的实现就是通过调用API接口（但是这代码稍微有点复杂了目前还没怎么看明白）

```cpp
class User : public Value {
private:
  const Use *getHungOffOperands() const {
    return *(reinterpret_cast<const Use *const *>(this) - 1);
  }

  Use *&getHungOffOperands() { return *(reinterpret_cast<Use **>(this) - 1); }

  const Use *getIntrusiveOperands() const {
    return reinterpret_cast<const Use *>(this) - NumUserOperands;
  }

  Use *getIntrusiveOperands() {
    return reinterpret_cast<Use *>(this) - NumUserOperands;
  }

public:
  const Use *getOperandList() const {
    return HasHungOffUses ? getHungOffOperands() : getIntrusiveOperands();
  }
  Use *getOperandList() {
    return const_cast<Use *>(static_cast<const User *>(this)->getOperandList());
  }

  Value *getOperand(unsigned i) const {
    assert(i < NumUserOperands && "getOperand() out of range!");
    return getOperandList()[i];
  }
};
```

# TASK4-3

## **DCE(DeadCodeElimination)**

感觉题目提示的已经比较明显了（

```cpp
bool DCE::Run() {
  /*
  要求：
  1. 顺序遍历function中的每个基本块,并顺序遍历基本块中的每个指令
  2. 如果一个指令没有副作用, 并且是一个"死"指令，那么就删除这个指令
  */
   Singleton<Module>();
   Singleton<Module>().GetFuncTion();
   for(auto bb:(*func)){
     for(auto iter=bb->begin();iter!=bb->end();++iter){
        auto inst = *iter;
        if(!HasSideEffect(inst)){
          if(0 == inst->GetUserListSize()){
            RemoveInst(inst);
          }
        }
     }
   }
  return true;
}

bool DCE::HasSideEffect(User *inst) {
  /*
  要求：
  1. 判断一个指令是否有副作用
  2. 如果有副作用, 返回true, 否则返回false
  */

  if(inst->IsTerminateInst()){//对于return语句，或者分支语句，不应该去除。
    return true;
  }
  auto id = inst->GetInstId();
  if(id == User::OpID::Memcpy || //有效的内存操作也是副作用
  id == User::OpID::Store || 
  id == User::OpID::Load || 
  id == User::OpID::Alloca||
  id == User::OpID::Call||//函数调用也是副作用
  id == User::OpID::Phi //phi函数也是副作用
  ) 
  {
    return true;
  }
    return false;//检测了所有情况，没有副作用

  /*
  思考：
  1. 什么情况下一条指令会有副作用？
  2. 如何判断一条指令是否有副作用？
  */

}

void DCE::RemoveInst(User *inst) {
  /*
  要求：
  正确删除一条指令, 并使用User类中正确的函数维护关系。
  */
  for(auto user:inst->GetUserList()){
    auto x=user->GetUser(); //获取inst的User
    x->RemoveFromUseList(inst); //将inst从User的UseList中删除
  }
  inst->EraseFromParent();
  //遍历userlist并删除每一个Use，然后使用EraseFromParent函数删除修改指令的父节点
  inst->ClearRelation();
  //先检查Userlist是否为空，接着遍历User中的UseList，将inst从UseList中删除
  delete inst;
  /*
  思考：
  1. 如何删除一条指令？
  2. 如何维护一条指令的Use\User\Usee关系?
  */
}
```



## **ConstantProp**

```cpp
#include "../../include/ir/opt/ConstantProp.hpp"
#include <cmath>
#include <set>

bool ConstantProp::Run() {
  /*
  要求：
  1. 顺序遍历function中的每个基本块,并顺序遍历基本块中的每个指令
  2. 如果一个指令是常量传播的结果，那么就将这个指令替换为常量，并删除这个指令
  */
  Singleton<Module>().GetFuncTion();
  for (auto bb : (*_func)) {
    for (auto iter = bb->begin(); iter != bb->end(); ++iter) {
      auto inst = *iter;
      auto val = ConstFoldInst(inst);
      if (val != nullptr) {
        inst->RAUW(val);
        delete inst;
      }
    }
  }
  return true;
}

Value *ConstantProp::ConstFoldInst(User *inst) {
  /*
  要求：
  1. 考虑一条指令是否能进行常量折叠
  2. 如果能够进行常量折叠，那么返回折叠后的常量
  3. 正确判断指令的类型，对于不同的类型需要有不同的处理方法
  这个过程可以自行在 ConstantProp类中增加函数来实现功能
  */
   for(auto &use:inst->Getuselist())
      {
        if(!use->GetValue()->isConst()) {//先检查uselist中是否为常量
           return nullptr;
        }
        if(!inst->IsBinary()){ //在检查是否为二元操作符(因为只有+-*/|&!^<>这些操作符才能进行常量折叠)
          return nullptr;
        }
      }
      
}

```

ConstFoldInst函数还没实现完（但是有点思路

我的想法是通过irtual Operand GetDef(); 先获取到Operand的类型

  然后通过使用inline Operand GetOperand(int i){return uselist[i]->GetValue();}来获取到操作数

   ConstantProp类中增加个函数来实现Operand的类型判断和转化