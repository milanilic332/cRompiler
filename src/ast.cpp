#include "ast.hpp"
#include <iostream>

LLVMContext TheContext;
Module* TheModule;
IRBuilder<> Builder(TheContext);
map<Function*, map<string,  AllocaInst*>> NamedValues;
llvm::legacy::FunctionPassManager *TheFPM;
extern Function *Printf;
extern Value* StringInt;
extern Value* StringDouble;

Value* VariableNode::codegen() const {
    cerr << "Entered VariableNode" << endl;
    Function *f = Builder.GetInsertBlock()->getParent();

    AllocaInst *alloca = NamedValues[f][id_];
    if (!alloca) {
        cerr << "Variable doesn't exist: " << id_ << endl;
        exit(1);
    }
    return Builder.CreateLoad(alloca);
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
    Value *val = e_->codegen();
    if (!val) {
        cerr << "AssignmentNode: nullptr" << endl;
        return nullptr;
    }
    Function *f = Builder.GetInsertBlock()->getParent();
    if(NamedValues[f][id_] == nullptr){
        AllocaInst* alloca = nullptr;
        if(val->getType() == Type::getInt32Ty(TheContext))
            alloca = CreateEntryBlockAllocaInt(f, id_);
        else
            alloca = CreateEntryBlockAllocaDouble(f, id_);

        Builder.CreateStore(val, alloca);

        NamedValues[f][id_] = alloca;
    }
    else {
        AllocaInst* alloca = NamedValues[f][id_];
        if(alloca->getAllocatedType() == Type::getInt32Ty(TheContext) and val->getType() !=  Type::getInt32Ty(TheContext))
            alloca = CreateEntryBlockAllocaDouble(f, id_);

        Builder.CreateStore(val, alloca);

        NamedValues[f][id_] = alloca;
    }
    return val;
}

Value* ArrayAssignmentNode::codegen() const {
    cerr << "Entered ArrayAssignmentNode" << endl;
    Function *f = Builder.GetInsertBlock()->getParent();
    bool is_int = true;
    for(auto &el: ve_){
        if(el->codegen()->getType() != Type::getInt32Ty(TheContext)) {
            is_int = false;
            break;
        }
    }

    AllocaInst* alloca = nullptr;
    if(is_int)
        alloca = CreateEntryBlockAllocaIntArray(f, id_, ve_.size());
    else
        alloca = CreateEntryBlockAllocaDoubleArray(f, id_, ve_.size());

    for(unsigned i = 0; i < ve_.size(); i++){
        vector<Value*> s;
        s.push_back(ConstantInt::get(TheContext, APInt(32, 0)));
        s.push_back(ConstantInt::get(TheContext, APInt(32, i)));

        Value* ptr = Builder.CreateGEP(alloca, s);

        Value* val = ve_[i]->codegen();

        if(!is_int and val->getType() == Type::getInt32Ty(TheContext))
            val = Builder.CreateSIToFP(val, Type::getDoubleTy(TheContext));
        Builder.CreateStore(val, ptr);
    }

    NamedValues[f][id_] = alloca;

    return ConstantInt::get(TheContext, APInt(32, 0));
}

Value* AccessArrayNode::codegen() const {
    cerr << "Entered AccessArrayNode" << endl;
    Function *f = Builder.GetInsertBlock()->getParent();

    Value* index = e_->codegen();
    if(!index)
        return nullptr;

    AllocaInst* alloca = NamedValues[f][id_];

    vector<Value*> s;
    s.push_back(ConstantInt::get(TheContext, APInt(32, 0)));
    s.push_back(index);

    Value* ptr = Builder.CreateGEP(alloca, s);

    return Builder.CreateLoad(ptr);
}

Value* ModifyArrayNode::codegen() const {
    cerr << "Entered AccessArrayNode" << endl;
    Function *f = Builder.GetInsertBlock()->getParent();

    Value* index = e1_->codegen();
    if(!index)
        return nullptr;

    AllocaInst* alloca = NamedValues[f][id_];

    vector<Value*> s;
    s.push_back(ConstantInt::get(TheContext, APInt(32, 0)));
    s.push_back(index);

    Value* ptr = Builder.CreateGEP(alloca, s);

    Value* nval = e2_->codegen();

    Value* oval = Builder.CreateLoad(ptr);

    if(nval->getType() == Type::getInt32Ty(TheContext) and oval->getType() == Type::getDoubleTy(TheContext))
        nval = Builder.CreateSIToFP(nval, Type::getDoubleTy(TheContext));

    Builder.CreateStore(nval, ptr);

    return nval;
}

Value* SequenceNode::codegen() const {
    cerr << "Entered SequenceNode" << endl;
    Function *f = Builder.GetInsertBlock()->getParent();

    Value* start = start_->codegen();
    if(!start)
        return nullptr;

    if(start->getType() == Type::getInt32Ty(TheContext))
        start = Builder.CreateSIToFP(start, Type::getDoubleTy(TheContext));

    Value* end = end_->codegen();
    if(!end)
        return nullptr;

    if(end->getType() == Type::getInt32Ty(TheContext))
        end = Builder.CreateSIToFP(end, Type::getDoubleTy(TheContext));

    Value* step = step_->codegen();
    if(!step)
        return nullptr;

    if(step->getType() == Type::getInt32Ty(TheContext))
        step = Builder.CreateSIToFP(step, Type::getDoubleTy(TheContext));

    Value* idx = ConstantInt::get(TheContext, APInt(32, 0));

    AllocaInst* alloca = CreateEntryBlockAllocaDoubleVector(f, id_);
    AllocaInst* allocaIdx = CreateEntryBlockAllocaInt(f, "idx");
    AllocaInst* allocaStart = CreateEntryBlockAllocaDouble(f, "start");
    AllocaInst* allocaEnd = CreateEntryBlockAllocaDouble(f, "end");
    AllocaInst* allocaStep = CreateEntryBlockAllocaDouble(f, "step");

    Builder.CreateStore(idx, allocaIdx);
    Builder.CreateStore(start, allocaStart);
    Builder.CreateStore(end, allocaEnd);
    Builder.CreateStore(step, allocaStep);

    BasicBlock *loop_BB = BasicBlock::Create(TheContext, "loop", f);

    Builder.CreateBr(loop_BB);

    Builder.SetInsertPoint(loop_BB);

    start = Builder.CreateLoad(allocaStart);
    end = Builder.CreateLoad(allocaEnd);
    step = Builder.CreateLoad(allocaStep);
    idx = Builder.CreateLoad(allocaIdx);

    vector<Value*> s;
    s.push_back(ConstantInt::get(TheContext, APInt(32, 0)));
    s.push_back(idx);

    Value* ptr = Builder.CreateGEP(alloca, s);

    Builder.CreateStore(start, ptr);

    start = Builder.CreateFAdd(start, step, "addtmp");

    idx = Builder.CreateAdd(idx, ConstantInt::get(TheContext, APInt(32, 1)), "addtmp");

    Builder.CreateStore(idx, allocaIdx);
    Builder.CreateStore(start, allocaStart);

    Value* cond = Builder.CreateFCmpULE(start, end, "leqtmp");
    cond = Builder.CreateSIToFP(cond, Type::getDoubleTy(TheContext));
    Value* tmp = Builder.CreateFCmpONE(cond, ConstantFP::get(TheContext, APFloat(0.0)), "loopcond");

    BasicBlock *after_loop_BB = BasicBlock::Create(TheContext, "afterloop", f);
    Builder.CreateCondBr(tmp, loop_BB, after_loop_BB);

    Builder.SetInsertPoint(after_loop_BB);

    loop_BB = Builder.GetInsertBlock();

    NamedValues[f][id_] = alloca;

    return ConstantFP::get(TheContext, APFloat(0.0));
}

Value* BinaryOperatorNode::codegen() const {
    cerr << "Entered BinaryOperatorNode" << endl;
    Value *l = l_->codegen();
    Value *d = r_->codegen();
    if (!l || !d) {
        cerr << "BinaryOperatorNode: nullptr" << endl;
        return nullptr;
    }
    switch(op_){
        case bin_op::or_: {
            return Builder.CreateOr(l, d, "ortmp");
        }
        case bin_op::and_: {
            return Builder.CreateOr(l, d, "ortmp");
        }
    }
    if(l->getType() == d->getType() && d->getType() == Type::getInt32Ty(TheContext)){
        switch(op_){
            case bin_op::plus: {
                return Builder.CreateAdd(l, d, "addtmp");
            }
            case bin_op::minus: {
                return Builder.CreateSub(l, d, "subtmp");
            }
            case bin_op::mul: {
                return Builder.CreateMul(l, d, "multmp");
            }
            case bin_op::di: {

                return Builder.CreateSDiv(l, d, "divtmp");
            }
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
            case bin_op::plus: {
                return Builder.CreateFAdd(l, d, "addtmp");
            }
            case bin_op::minus: {
                return Builder.CreateFSub(l, d, "subtmp");
            }
            case bin_op::mul: {
                return Builder.CreateFMul(l, d, "multmp");
            }
            case bin_op::di: {
                return Builder.CreateFDiv(l, d, "divtmp");
            }
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
    return nullptr;
}

Value* ReturnNode::codegen() const {
    cerr << "Entered ReturnNode" << endl;
    Function *f = Builder.GetInsertBlock()->getParent();
    Type* ft = f->getReturnType();

    Value* val = e_->codegen();
    if(val->getType() == Type::getInt32Ty(TheContext) and ft == Type::getDoubleTy(TheContext))
        val = Builder.CreateSIToFP(val, Type::getDoubleTy(TheContext));
    else if(val->getType() == Type::getDoubleTy(TheContext) and ft == Type::getInt32Ty(TheContext))
        val = Builder.CreateFPToSI(val, Type::getInt32Ty(TheContext));
    return val;
}

Value* BlockNode::codegen() const {
    cerr << "Entered BlockNode" << endl;
    for(unsigned i = 0; i < statements_.size() - 1; i++) {
        Value *tmp = statements_[i]->codegen();
        if (tmp == nullptr) {
            cerr << "BlockNode: nullptr" << endl;
            return nullptr;
        }
    }
    return statements_[statements_.size() - 1]->codegen();
}

Value* PrintNode::codegen() const {
    cerr << "Entered PrintNode" << endl;
    Value *e = e_->codegen();
    if (e == nullptr){
        cerr << "PrintNode: nullptr" << endl;
        return nullptr;
    }

    vector<Value*> args;
    if(e->getType() == Type::getInt32Ty(TheContext))
        args.push_back(StringInt);
    else
        args.push_back(StringDouble);
    args.push_back(e);
    Builder.CreateCall(Printf, args, "printfCall");

    return e;
}

Value* EmptyNode::codegen() const {
    return ConstantInt::get(TheContext, APInt(32, 0));
}

Value* FunctionCallNode::codegen() const {
    cerr << "Entered FunctionCallNode" << endl;
    Function* f = TheModule->getFunction(id_);
    if(!f) {
        cerr << "Function is not defined: " << id_ << endl;
        exit(1);
    }
    if(f->arg_size() != params_.size()) {
        cerr << "Wrong argument size: given " << id_ << ", expected " << f->arg_size() << endl ;
        exit(1);
    }

    vector<Value*> a;
    for(auto const& value: params_)
        a.push_back(value->codegen());

    return Builder.CreateCall(f, a, "calltmp");
}


Value* IfElseNode::codegen() const {
    cerr << "Entered IfElseNode" << endl;
    Value *cond = cond_->codegen();
    if (!cond) {
        cerr << "IfElseNode: nullptr" << endl;
        return nullptr;
    }

    Value* tmp = Builder.CreateFCmpONE(Builder.CreateSIToFP(cond, Type::getDoubleTy(TheContext)), ConstantFP::get(TheContext, APFloat(0.0)), "ifcond");

    Function *f = Builder.GetInsertBlock()->getParent();
    BasicBlock *thenBB = BasicBlock::Create(TheContext, "then", f);
    BasicBlock *elseBB = BasicBlock::Create(TheContext, "else");
    BasicBlock *mergeBB = BasicBlock::Create(TheContext, "ifcont");

    Builder.CreateCondBr(tmp, thenBB, elseBB);

    Builder.SetInsertPoint(thenBB);
    Value* then = then_->codegen();
    if (!then) {
        cerr << "IfElseNode: nullptr" << endl;
        return nullptr;
    }
    Builder.CreateBr(mergeBB);
    thenBB = Builder.GetInsertBlock();

    f->getBasicBlockList().push_back(elseBB);
    Builder.SetInsertPoint(elseBB);
    Value* Else = else_->codegen();
    if (!Else) {
        cerr << "IfElseNode: nullptr" << endl;
        return nullptr;
    }
    Builder.CreateBr(mergeBB);
    elseBB = Builder.GetInsertBlock();

    f->getBasicBlockList().push_back(mergeBB);
    Builder.SetInsertPoint(mergeBB);
    PHINode* phi = Builder.CreatePHI(Type::getInt32Ty(TheContext), 2, "iftmp");
    phi->addIncoming(then, thenBB);
    phi->addIncoming(Else, elseBB);

    return phi;
}


Value* ForLoopNode::codegen() const {
    cerr << "Entered ForLoopNode" << endl;
    Value* start_val = start_->codegen();
    if (!start_val){
        cerr << "ForLoopNode: nullptr" << endl;
        return nullptr;
    }
    Function *f = Builder.GetInsertBlock()->getParent();
    BasicBlock *loop_BB = BasicBlock::Create(TheContext, "loop", f);

    AllocaInst* alloca = CreateEntryBlockAllocaInt(f, id_);
    Builder.CreateStore(start_val, alloca);
    Builder.CreateBr(loop_BB);

    Builder.SetInsertPoint(loop_BB);
    AllocaInst* old_val = NamedValues[f][id_];
    NamedValues[f][id_] = alloca;

    Value* body_val = body_->codegen();
    if (!body_val) {
        cerr << "ForLoopNode: nullptr" << endl;
        return nullptr;
    }
    Value* inc_val = ConstantInt::get(TheContext, APInt(32, 1));
    if (!inc_val) {
        cerr << "ForLoopNode: nullptr" << endl;
        return nullptr;
    }
    Value *curr_val = Builder.CreateLoad(alloca);
    Value* next_var = Builder.CreateAdd(curr_val, inc_val, "nextvar");
    Builder.CreateStore(next_var, alloca);

    Value* end = end_->codegen();
    if(end->getType() == Type::getDoubleTy(TheContext))
        end = Builder.CreateFPToSI(end, Type::getInt32Ty(TheContext));

    Value* cond = Builder.CreateICmpSLT(curr_val, end, "leqtmp");
    if (!cond){
        cerr << "ForLoopNode: nullptr" << endl;
        return nullptr;
    }
    Value* tmp = Builder.CreateFCmpONE(Builder.CreateSIToFP(cond, Type::getDoubleTy(TheContext)), ConstantFP::get(TheContext, APFloat(0.0)), "loopcond");

    BasicBlock *after_loop_BB = BasicBlock::Create(TheContext, "afterloop", f);
    Builder.CreateCondBr(tmp, loop_BB, after_loop_BB);

    Builder.SetInsertPoint(after_loop_BB);

    loop_BB = Builder.GetInsertBlock();

    if (old_val != nullptr)
        NamedValues[f][id_] = old_val;
    else
        NamedValues[f].erase(id_);
    return ConstantInt::get(TheContext, APInt(32, 0));
}

Value* WhileNode::codegen() const {
    cerr << "Entered WhileNode" << endl;

    Function *f = Builder.GetInsertBlock()->getParent();
    BasicBlock *loop_BB = BasicBlock::Create(TheContext, "loop", f);
    Builder.CreateBr(loop_BB);

    Builder.SetInsertPoint(loop_BB);

    Value* body = body_->codegen();
    if (!body)
        return NULL;

    Value* cond = cond_->codegen();
    if (!cond)
        return NULL;

    Value* tmp = Builder.CreateFCmpONE(Builder.CreateSIToFP(cond, Type::getDoubleTy(TheContext)), ConstantFP::get(TheContext, APFloat(0.0)), "loopcond");

    BasicBlock *after_loop_BB = BasicBlock::Create(TheContext, "afterloop", f);
    Builder.CreateCondBr(tmp, loop_BB, after_loop_BB);

    Builder.SetInsertPoint(after_loop_BB);

    loop_BB = Builder.GetInsertBlock();

    return ConstantInt::get(TheContext, APInt(32, 0));
}

Function* FunctionPrototypeNode::codegen() const {
    cerr << "Entered FunctionPrototypeNode" << endl;
    vector<Type*> d;
    for(int i = 0; i < params_.size(); i++){
        if(params_[i].first == my_type::int_)
            d.push_back(Type::getInt32Ty(TheContext));
        else
            d.push_back(Type::getDoubleTy(TheContext));
    }
    FunctionType *ft = nullptr;
    if(ret_type_ == my_type::int_)
        ft = FunctionType::get(Type::getInt32Ty(TheContext), d, false);
    else
        ft = FunctionType::get(Type::getDoubleTy(TheContext), d, false);
    Function *f = Function::Create(ft, Function::ExternalLinkage, id_, TheModule);

    unsigned i = 0;
    for (auto &arg : f->args())
        arg.setName(params_[i++].second);
    return f;
}


Function* FunctionNode::codegen() const {
    cerr << "Entered FunctionNode" << endl;
    Function* f = TheModule->getFunction(prototype_.getName());
    if(!f)
        f = prototype_.codegen();

    if(!f) {
        cerr << "Can't generate code for function: " << prototype_.getName() << endl;
        exit(1);
    }

    if(!f->empty()) {
        cerr << "Can't redefine function: " << prototype_.getName() << endl;
        exit(1);
    }

    BasicBlock *BB = BasicBlock::Create(TheContext, "entry", f);
    Builder.SetInsertPoint(BB);

    StringInt = Builder.CreateGlobalStringPtr("%d\n");
    StringDouble = Builder.CreateGlobalStringPtr("%.2lf\n");

    NamedValues[f].clear();
    for(auto &arg : f->args()) {
        AllocaInst* alloca = nullptr;
        if(arg.getType() == Type::getInt32Ty(TheContext))
            alloca = CreateEntryBlockAllocaInt(f, arg.getName());
        else
            alloca = CreateEntryBlockAllocaDouble(f, arg.getName());
        NamedValues[f][arg.getName()] = alloca;
        Builder.CreateStore(&arg, alloca);
    }

    Value* ret_val;
    if((ret_val = body_->codegen())) {
        Builder.CreateRet(ret_val);
        verifyFunction(*f);

        return f;
    }
    else {
        f->eraseFromParent();
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

AllocaInst *CreateEntryBlockAllocaIntArray(Function *TheFunction, const string &VarName, unsigned size) {
    Type* I = IntegerType::getInt32Ty(TheContext);
    ArrayType* arrayType = ArrayType::get(I, size);
    IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
    return TmpB.CreateAlloca(arrayType, 0, VarName.c_str());
}

AllocaInst *CreateEntryBlockAllocaDoubleArray(Function *TheFunction, const string &VarName, unsigned size) {
    Type* I = Type::getDoubleTy(TheContext);
    ArrayType* arrayType = ArrayType::get(I, size);
    IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
    return TmpB.CreateAlloca(arrayType, 0, VarName.c_str());
}

AllocaInst *CreateEntryBlockAllocaDoubleVector(Function * TheFunction, const string &VarName) {
    Type* I = Type::getDoubleTy(TheContext);
    VectorType* vectorType = VectorType::get(I, 128);
    IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
    return TmpB.CreateAlloca(vectorType, 0, VarName.c_str());
}
