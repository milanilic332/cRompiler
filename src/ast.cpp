#include "ast.hpp"
#include <iostream>

LLVMContext TheContext;
Module* TheModule;
IRBuilder<> Builder(TheContext);
map<Function*, map<string,  AllocaInst*>> NamedValues;
llvm::legacy::FunctionPassManager *TheFPM;
extern Function *PrintfFja;
extern Value* Str;
extern Value* Str1;

Value* VariableNode::codegen() const {
    cerr << "Entered VariableNode" << endl;
    Function *F = Builder.GetInsertBlock()->getParent();

    AllocaInst *Alloca = NamedValues[F][id_];
    if (!Alloca) {
        cerr << "Variable doesn't exist: " << id_ << endl;
        exit(1);
    }
    return Builder.CreateLoad(Alloca);
}

Value* IntNode::codegen() const {
    cerr << "Entered IntNode" << endl;
    return ConstantInt::get(TheContext, APInt(32, num_));
}

Value* DoubleNode::codegen() const {
    cerr << "Entered DoubleNode" << endl;
    return ConstantFP::get(TheContext, APFloat(num_));
}

Value* AssignmentNode::codegen() const {
    cerr << "Entered AssignmentNode" << endl;
    Value *Val = e_->codegen();
    if (!Val)
        return nullptr;
    Function *F = Builder.GetInsertBlock()->getParent();

    AllocaInst* Alloca = nullptr;
    if(Val->getType() == Type::getInt32Ty(TheContext))
        Alloca = CreateEntryBlockAllocaInt(F, id_);
    else
        Alloca = CreateEntryBlockAllocaDouble(F, id_);

    Builder.CreateStore(Val, Alloca);

    NamedValues[F][id_] = Alloca;
    return Val;
}

Value* BinaryOperatorNode::codegen() const {
    cerr << "Entered BinaryOperatorNode" << endl;
    Value *l = l_->codegen();
    Value *d = r_->codegen();
    if (!l || !d)
        return nullptr;
    if(l->getType() == d->getType() && d->getType() == Type::getInt32Ty(TheContext)){
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
    else {
        if(l->getType() == Type::getInt32Ty(TheContext))
            l = Builder.CreateSIToFP(l, Type::getDoubleTy(TheContext));
        if(d->getType() == Type::getInt32Ty(TheContext))
            d = Builder.CreateSIToFP(d, Type::getDoubleTy(TheContext));
        switch(op_){
            case bin_op::plus:
                return Builder.CreateFAdd(l, d, "addtmp");
            case bin_op::minus:
                return Builder.CreateFSub(l, d, "subtmp");
            case bin_op::mul:
                return Builder.CreateFMul(l, d, "multmp");
            case bin_op::di:
                return Builder.CreateFDiv(l, d, "divtmp");
            case bin_op::gt: {
                return Builder.CreateFCmpUGT(l, d, "gttmp");
            }
            case bin_op::lt: {
                return Builder.CreateFCmpULT(l, d, "lttmp");
            }
            case bin_op::geq: {
                return Builder.CreateFCmpUGE(l, d, "geqtmp");
            }
            case bin_op::leq: {
                return Builder.CreateFCmpULE(l, d, "leqtmp");
            }
            case bin_op::eq: {
                return Builder.CreateFCmpUEQ(l, d, "eqtmp");
            }
            case bin_op::neq: {
                return Builder.CreateFCmpUNE(l, d, "neqtmp");
            }
        }
    }
}

Value* ArrayNode::codegen() const {
    return nullptr;
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
    if(e->getType() == Type::getInt32Ty(TheContext))
        ArgsV.push_back(Str);
    else
        ArgsV.push_back(Str1);
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
    if(Cond->getType() == Type::getInt32Ty(TheContext))
        Cond = Builder.CreateSIToFP(Cond, Type::getDoubleTy(TheContext));
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
    PHINode* PHI = Builder.CreatePHI(Type::getFloatTy(TheContext), 2, "iftmp");
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

    AllocaInst* Alloca = CreateEntryBlockAllocaInt(F, id_);
    Builder.CreateStore(StartVal, Alloca);
    Builder.CreateBr(LoopBB);

    Builder.SetInsertPoint(LoopBB);
    AllocaInst* OldVal = NamedValues[F][id_];
    NamedValues[F][id_] = Alloca;

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
      NamedValues[F][id_] = OldVal;
    else
      NamedValues[F].erase(id_);
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

    Str = Builder.CreateGlobalStringPtr("%d\n");
    Str = Builder.CreateGlobalStringPtr("%lf\n");

    NamedValues[F].clear();
    for(auto &Arg : F->args()) {
        AllocaInst* Alloca = CreateEntryBlockAllocaDouble(F, Arg.getName());
        NamedValues[F][Arg.getName()] = Alloca;
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

AllocaInst *CreateEntryBlockAllocaInt(Function *TheFunction, const string &VarName) {
    IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
    return TmpB.CreateAlloca(Type::getInt32Ty(TheContext), 0, VarName.c_str());
}

AllocaInst *CreateEntryBlockAllocaDouble(Function *TheFunction, const string &VarName) {
    IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
    return TmpB.CreateAlloca(Type::getDoubleTy(TheContext), 0, VarName.c_str());
}
