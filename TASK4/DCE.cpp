#include "../../include/ir/opt/DCE.hpp"

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
  for(auto user:inst->Getuserlist()){
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