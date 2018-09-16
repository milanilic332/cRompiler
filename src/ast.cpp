#include "ast.hpp"
#include <iostream>

LLVMContext TheContext;
Module* TheModule;
IRBuilder<> Builder(TheContext);
map<string,  AllocaInst*> NamedValues;
FunctionPassManager *TheFPM;


Value* VariableNode::codegen() const {
    AllocaInst *Alloca = NamedValues[id_];
    if (!Alloca) {
        cerr << "Variable doesn't exist: " << id_ << endl;
        exit(1);
    }
    return Builder.CreateLoad(Alloca);
}

Value* ConstantNode::codegen() const {
  return ConstantFP::get(TheContext, APFloat(num_));
}

Value* AssignmentNode::codegen() {
    Value *Val = e_->codegen();
    if (!Val)
        return NULL;
    AllocaInst* Alloca = NamedValues[id_];
    if (!Alloca) {
        cerr << "Variable doesn't exist" << id_ << endl;
        exit(1);
    }
    Builder.CreateStore(Val, Alloca);
    return Val;
}

Value* BinaryOperatorNode::codegen() const {
    Value *l = l_->codegen();
    Value *d = r_->codegen();
    if (!l || !d)
        return NULL;
    switch(op_){
        case bin_op::plus:
            return Builder.CreateFAdd(l, d, "addtmp");
        case bin_op::minus:
            return Builder.CreateFSub(l, d, "subtmp");
        case bin_op::mul:
            return Builder.CreateFMul(l, d, "multmp");
        case bin_op::div:
            return Builder.CreateFDiv(l, d, "divtmp");
        case bin_op::gt:
            Value *tmp = Builder.CreateFCmpUGT(l, d, "gttmp");
            return Builder.CreateUIToFP(tmp, Type::getDoubleTy(TheContext), "booltmp");
        case bin_op::lt:
            Value *tmp = Builder.CreateFCmpULT(l, d, "lttmp");
            return Builder.CreateUIToFP(tmp, Type::getDoubleTy(TheContext), "booltmp");
        case bin_op::geq:
            Value *tmp = Builder.CreateFCmpUGE(l, d, "geqtmp");
            return Builder.CreateUIToFP(tmp, Type::getDoubleTy(TheContext), "booltmp");
        case bin_op::leq:
            Value *tmp = Builder.CreateFCmpULE(l, d, "leqtmp");
            return Builder.CreateUIToFP(tmp, Type::getDoubleTy(TheContext), "booltmp");
        case bin_op::eq:
            Value *tmp = Builder.CreateFCmpEQ(l, d, "eqtmp");
            return Builder.CreateUIToFP(tmp, Type::getDoubleTy(TheContext), "booltmp");
        case bin_op::neq:
            Value *tmp = Builder.CreateFCmpNE(l, d, "neqtmp");
            return Builder.CreateUIToFP(tmp, Type::getDoubleTy(TheContext), "booltmp");
    }
}

Value* ReturnNode::codegen() const {
    return e_->codegen();
}

Value* BlockNode::codegen() const {
  for(unsigned i = 0; i < nodes_.size() - 1; i++) {
    Value *tmp = nodes_[i]->codegen();
    if (tmp == NULL)
      return NULL;
  }
  return nodes_[nodes_.size() - 1]->codegen();
}

Value* FunctionCallNode::codegen() const {
    Function* F = TheModule->getFunction(id_);
    if(!F) {
        cerr << "Function is not defined: " << id_ << endl;
        exit(EXIT_FAILURE);
    }
    if(F->arg_size() != _v.size()) {
        cerr << "Wrong argument size: given " << _f << ", expected " << F->arg_size() << endl ;
        exit(1);
    }

    vector<Value*> a;
    for(auto const& value: params_)
        a.push_back(value->codegen());

    return Builder.CreateCall(F, a, "calltmp");
}

Value* IfElseNode::codegen() const {
    Value *Cond = cond_->codegen();
    if (!Cond)
        return NULL;
    Value* Tmp = Builder.CreateFCmpONE(Cond, ConstantFP::get(TheContext, APFloat(0.0)), "ifcond");

    Function *F = Builder.GetInsertBlock()->getParent();
    BasicBlock *ThenBB = BasicBlock::Create(TheContext, "then", F);
    BasicBlock *ElseBB = BasicBlock::Create(TheContext, "else");
    BasicBlock *MergeBB = BasicBlock::Create(TheContext, "ifcont");

    Builder.CreateCondBr(Tmp, ThenBB, ElseBB);

    Builder.SetInsertPoint(ThenBB);
    Value* Then = then_->codegen();
    if (!Then)
        return NULL;
    Builder.CreateBr(MergeBB);
    ThenBB = Builder.GetInsertBlock();

    F->getBasicBlockList().push_back(ElseBB);
    Builder.SetInsertPoint(ElseBB);
    Value* Else = else_->codegen();
    if (!Else)
        return NULL;
    Builder.CreateBr(MergeBB);
    ElseBB = Builder.GetInsertBlock();

    F->getBasicBlockList().push_back(MergeBB);
    Builder.SetInsertPoint(MergeBB);
    PHINode* PHI = Builder.CreatePHI(Type::getDoubleTy(TheContext), 2, "iftmp");
    PHI->addIncoming(Then, ThenBB);
    PHI->addIncoming(Else, ElseBB);

    return PHI;
}


Value* ForLoopNode::codegen() const {
    Value* Start = start_->codegen();
    if (!Start)
        return NULL;

    Function *F = Builder.GetInsertBlock()->getParent();
    BasicBlock *LoopBB = BasicBlock::Create(TheContext, "loop", F);

    AllocaInst* Alloca = CreateEntryBlockAlloca(F, id_);
    Builder.CreateStore(Start, Alloca);
    Builder.CreateBr(LoopBB);

    Builder.SetInsertPoint(LoopBB);
    AllocaInst* Old = NamedValues[id_];
    NamedValues[id_] = Alloca;

    Value* Body = body_->codegen();
    if (!Body)
        return NULL;

    Value* Inc = ConstantFP::get(TheContext, APFloat(1));
    Value* Curr = Builder.CreateLoad(Alloca);
    Value* Next = Builder.CreateFAdd(Curr, Inc, "nextvar");
    Builder.CreateStore(Next, Alloca);

    Value* End = end->codegen();
    if (!End)
        return NULL;

    Value* Tmp = Builder.CreateFCmpULE(Curr, End, "leqtmp");
    Tmp = Builder.CreateUIToFP(tmp, Type::getDoubleTy(TheContext), "booltmp");
    Tmp = Builder.CreateFCmpONE(CondV, ConstantFP::get(TheContext, APFloat(0.0)), "loopcond");

    BasicBlock *AfterLoopBB = BasicBlock::Create(TheContext, "afterloop", F);
    Builder.CreateCondBr(Tmp, LoopBB, AfterLoopBB);

    Builder.SetInsertPoint(AfterLoopBB);

    LoopBB = Builder.GetInsertBlock();

    if (Old != NULL)
        NamedValues[id_] = Old;
    else
        NamedValues.erase(id_);
    return ConstantFP::get(TheContext, APFloat(0.0));
}

Function* FunctionPrototypeNode::codegen() const {
    vector<Type*> d(paramas_.size(), Type::getDoubleTy(TheContext));
    FunctionType* FT = FunctionType::get(Type::getDoubleTy(TheContext), d, false);
    Function *F = Function::Create(FT, Function::ExternalLinkage, id_, TheModule);

    unsigned i = 0;
    for (auto &Arg : F->args())
        Arg.setName(params_[i++]);

    return F;
}


Function* FunctionNode::codegen() const {
    Function* F = TheModule->getFunction(prototype_.getName());
    if(!F)
        F = prototype_.codegen();

    if(!F) {
        cerr << "Can't generate code for function: " << prototype_.getName() << endl;
        exit(1);
    }

    if(!F->empty()) {
        cerr << "Can't redefine function: " << prototype_.getName() << endl;
        exit(1);
    }

    BasicBlock *BB = BasicBlock::Create(TheContext, "entry", F);
    Builder.SetInsertPoint(BB);

    NamedValues.clear();
    for(auto &Arg : F->args()) {
        AllocaInst* Alloca = CreateEntryBlockAlloca(F, Arg.getName());
        NamedValues[Arg.getName()] = Alloca;
        Builder.CreateStore(&Arg, Alloca);
    }

    Value* RetVal;
    if(RetVal = body_->codegen()) {
        Builder.CreateRet(RetVal);
        verifyFunction(*F);
        return F;
    }
    else {
        F->eraseFromParent();
        return NULL;
    }
}


void InitializeModuleAndPassManager() {
    TheModule = new llvm::Module("Module", TheContext);
    TheFPM = new FunctionPassManager(TheModule);

    TheFPM->add(createInstructionCombiningPass());
    TheFPM->add(createReassociatePass());
    TheFPM->add(createGVNPass());
    TheFPM->add(createCFGSimplificationPass());
    TheFPM->doInitialization();
}

AllocaInst *CreateEntryBlockAlloca(Function *TheFunction, const string &VarName) {
    IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
    return TmpB.CreateAlloca(Type::getDoubleTy(TheContext), 0, VarName.c_str());
}
