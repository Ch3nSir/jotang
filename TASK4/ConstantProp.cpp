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
      //然后我的想法是通过irtual Operand GetDef(); 先获取到Operand的类型
      //然后通过使用inline Operand GetOperand(int i){return uselist[i]->GetValue();}来获取到操作数
      //通过在ConstantProp类中增加个函数来实现Operand的类型判断和转化
}
