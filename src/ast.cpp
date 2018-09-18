#include "ast.hpp"
#include <iostream>

LLVMContext TheContext;
Module* TheModule;
IRBuilder<> Builder(TheContext);
map<string,  AllocaInst*> NamedValues;
llvm::legacy::FunctionPassManager *TheFPM;
extern Function *MainFja;
extern Function *PrintfFja;
extern Value* Str;

Value* VariableNode::codegen() const {
    cerr << "Entered VariableNode" << endl;
    AllocaInst *Alloca = NamedValues[id_];
    if (!Alloca) {
        cerr << "Variable doesn't exist: " << id_ << endl;
        exit(1);
    }
    return Builder.CreateLoad(Alloca);
}

Value* ConstantNode::codegen() const {
    cerr << "Entered ConstantNode" << endl;
    return ConstantInt::get(TheContext, APInt(32, num_));
}

Value* AssignmentNode::codegen() const {
    cerr << "Entered AssignmentNode" << endl;
    Value *Val = e_->codegen();
    vector<AllocaInst*> OldValues;
    Function *F = Builder.GetInsertBlock()->getParent();
    if (!Val)
        return nullptr;
    AllocaInst* Alloca = CreateEntryBlockAlloca(F, id_);
    Builder.CreateStore(Val, Alloca);
    OldValues.push_back(NamedValues[id_]);
    NamedValues[id_] = Alloca;
    return Val;
}

Value* BinaryOperatorNode::codegen() const {
    cerr << "Entered BinaryOperatorNode" << endl;
    Value *l = l_->codegen();
    Value *d = r_->codegen();
    if (!l || !d)
        return nullptr;
    switch(op_){
        case bin_op::plus:
            return Builder.CreateAdd(l, d, "addtmp");
        case bin_op::minus:
            return Builder.CreateSub(l, d, "subtmp");
        case bin_op::mul:
            return Builder.CreateMul(l, d, "multmp");
        case bin_op::di:
            return Builder.CreateFDiv(l, d, "divtmp");
        case bin_op::gt: {
            return Builder.CreateICmpSGT(l, d, "gttmp");
        }
        case bin_op::lt: {
            return Builder.CreateICmpSLT(l, d, "lttmp");
        }
        case bin_op::geq: {
            return Builder.CreateICmpSGE(l, d, "geqtmp");
        }
        case bin_op::leq: {
            return Builder.CreateICmpSLE(l, d, "leqtmp");
        }
        case bin_op::eq: {
            return Builder.CreateICmpEQ(l, d, "eqtmp");
        }
        case bin_op::neq: {
            return Builder.CreateICmpNE(l, d, "neqtmp");
        }
    }
}

Value* ReturnNode::codegen() const {
    cerr << "Entered ReturnNode" << endl;
    return e_->codegen();
}

Value* BlockNode::codegen() const {
    cerr << "Entered BlockNode" << endl;
    for(unsigned i = 0; i < statements_.size() - 1; i++) {
        Value *tmp = statements_[i]->codegen();
        if (tmp == nullptr)
            return nullptr;
        }
    return statements_[statements_.size() - 1]->codegen();
}

Value* PrintNode::codegen() const {
    cerr << "Entered PrintNode" << endl;
    Value *e = e_->codegen();
    if (e == nullptr)
        return nullptr;

    vector<Value*> ArgsV;
    ArgsV.push_back(Str);
    ArgsV.push_back(e);
    Builder.CreateCall(PrintfFja, ArgsV, "printfCall");

    return e;
}

Value* EmptyNode::codegen() const {
    return ConstantInt::get(TheContext, APInt(32, 0));
}

Value* FunctionCallNode::codegen() const {
    cerr << "Entered FunctionCallNode" << endl;
    Function* F = TheModule->getFunction(id_);
    if(!F) {
        cerr << "Function is not defined: " << id_ << endl;
        exit(EXIT_FAILURE);
    }
    if(F->arg_size() != params_.size()) {
        cerr << "Wrong argument size: given " << id_ << ", expected " << F->arg_size() << endl ;
        exit(1);
    }

    vector<Value*> a;
    for(auto const& value: params_)
        a.push_back(value->codegen());

    return Builder.CreateCall(F, a, "calltmp");
}

Value* IfElseNode::codegen() const {
    cerr << "Entered IfElseNode" << endl;
    Value *Cond = cond_->codegen();
    if (!Cond)
      return nullptr;
    Value* Tmp = Builder.CreateFCmpONE(Builder.CreateSIToFP(Cond, Type::getDoubleTy(TheContext)), ConstantFP::get(TheContext, APFloat(0.0)), "ifcond");

    Function *F = Builder.GetInsertBlock()->getParent();
    BasicBlock *ThenBB = BasicBlock::Create(TheContext, "then", F);
    BasicBlock *ElseBB = BasicBlock::Create(TheContext, "else");
    BasicBlock *MergeBB = BasicBlock::Create(TheContext, "ifcont");

    Builder.CreateCondBr(Tmp, ThenBB, ElseBB);

    Builder.SetInsertPoint(ThenBB);
    Value* Then = then_->codegen();
    if (!Then)
      return nullptr;
    Builder.CreateBr(MergeBB);
    ThenBB = Builder.GetInsertBlock();

    F->getBasicBlockList().push_back(ElseBB);
    Builder.SetInsertPoint(ElseBB);
    Value* Else = else_->codegen();
    if (!Else)
      return nullptr;
    Builder.CreateBr(MergeBB);
    ElseBB = Builder.GetInsertBlock();

    F->getBasicBlockList().push_back(MergeBB);
    Builder.SetInsertPoint(MergeBB);
    PHINode* PHI = Builder.CreatePHI(Type::getInt32Ty(TheContext), 2, "iftmp");
    PHI->addIncoming(Then, ThenBB);
    PHI->addIncoming(Else, ElseBB);

    return PHI;
}


Value* ForLoopNode::codegen() const {
    cerr << "Entered ForLoopNode" << endl;
    Value* StartVal = start_->codegen();
    if (!StartVal)
      return nullptr;

    Function *F = Builder.GetInsertBlock()->getParent();
    BasicBlock *LoopBB = BasicBlock::Create(TheContext, "loop", F);

    AllocaInst* Alloca = CreateEntryBlockAlloca(F, id_);
    Builder.CreateStore(StartVal, Alloca);
    Builder.CreateBr(LoopBB);

    Builder.SetInsertPoint(LoopBB);
    AllocaInst* OldVal = NamedValues[id_];
    NamedValues[id_] = Alloca;

    Value* BodyVal = body_->codegen();
    if (!BodyVal)
      return nullptr;

    Value* IncVal = ConstantInt::get(TheContext, APInt(32, 1));
    if (!IncVal)
      return nullptr;
    Value *CurrVal = Builder.CreateLoad(Alloca);
    Value* NextVar = Builder.CreateAdd(CurrVal, IncVal, "nextvar");
    Builder.CreateStore(NextVar, Alloca);

    Value* Cond = Builder.CreateICmpSLT(CurrVal, end_->codegen(), "leqtmp");
    if (!Cond)
      return nullptr;

    Value* Tmp = Builder.CreateFCmpONE(Builder.CreateSIToFP(Cond, Type::getDoubleTy(TheContext)), ConstantFP::get(TheContext, APFloat(0.0)), "loopcond");

    BasicBlock *AfterLoopBB = BasicBlock::Create(TheContext, "afterloop", F);
    Builder.CreateCondBr(Tmp, LoopBB, AfterLoopBB);

    Builder.SetInsertPoint(AfterLoopBB);

    LoopBB = Builder.GetInsertBlock();

    if (OldVal != nullptr)
      NamedValues[id_] = OldVal;
    else
      NamedValues.erase(id_);
    return ConstantInt::get(TheContext, APInt(32, 0));
}

Function* FunctionPrototypeNode::codegen() const {
    cerr << "Entered FunctionPrototypeNode" << endl;
    vector<Type*> d(params_.size(), Type::getInt32Ty(TheContext));
    FunctionType* FT = FunctionType::get(Type::getInt32Ty(TheContext), d, false);
    Function *F = Function::Create(FT, Function::ExternalLinkage, id_, TheModule);

    unsigned i = 0;
    for (auto &Arg : F->args())
        Arg.setName(params_[i++]);

    return F;
}


Function* FunctionNode::codegen() const {
    cerr << "Entered FunctionNode" << endl;
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
    if((RetVal = body_->codegen())) {
        Builder.CreateRet(RetVal);
        verifyFunction(*F);

        return F;
    }
    else {
        F->eraseFromParent();
        return nullptr;
    }
}


void InitializeModuleAndPassManager() {
    TheModule = new llvm::Module("Module", TheContext);
    TheFPM = new llvm::legacy::FunctionPassManager(TheModule);

    TheFPM->add(createInstructionCombiningPass());
    TheFPM->add(createReassociatePass());
    TheFPM->add(createNewGVNPass());
    TheFPM->add(createCFGSimplificationPass());
    TheFPM->doInitialization();
}

AllocaInst *CreateEntryBlockAlloca(Function *TheFunction, const string &VarName) {
    IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
    return TmpB.CreateAlloca(Type::getInt32Ty(TheContext), 0, VarName.c_str());
}
