#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
using namespace std;

#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"

using namespace llvm;
using namespace llvm::legacy;


enum class bin_op {
	plus,
	minus,
	mul,
	di,
	gt,
	lt,
	geq,
	leq,
	eq,
	neq,
	or_,
	and_
};


enum class my_type {
	int_,
	double_
};


class ExpressionNode {
public:
	virtual ~ExpressionNode() {}
	virtual Value* codegen() const = 0;
};


class VariableNode: public ExpressionNode {
public:
	VariableNode(string id)
		: id_(id)
	{}
    Value* codegen() const;
private:
	string id_;
};


class IntNode: public ExpressionNode {
public:
    IntNode(int num)
		: num_(num)
	{}
	Value* codegen() const;
private:
	int num_;
};

class DoubleNode: public ExpressionNode {
public:
    DoubleNode(float num)
		: num_(num)
	{}
	Value* codegen() const;
private:
 	double num_;
};


class AssignmentNode: public ExpressionNode {
public:
    AssignmentNode(string id, ExpressionNode* e)
        : id_(id), e_(e)
    {}
	Value* codegen() const;
private:
	string id_;
    ExpressionNode* e_;
};


class ArrayAssignmentNode: public ExpressionNode {
public:
    ArrayAssignmentNode(string id, vector<ExpressionNode*> ve)
        : id_(id), ve_(ve)
    {}
	Value* codegen() const;
private:
	string id_;
    vector<ExpressionNode*> ve_;
};


class AccessArrayNode: public ExpressionNode {
public:
    AccessArrayNode(string id, ExpressionNode* e)
        : id_(id), e_(e)
    {}
	Value* codegen() const;
private:
	string id_;
    ExpressionNode* e_;
};


class ModifyArrayNode: public ExpressionNode {
public:
    ModifyArrayNode(string id, ExpressionNode* e1, ExpressionNode* e2)
        : id_(id), e1_(e1), e2_(e2)
    {}
	Value* codegen() const;
private:
	string id_;
	ExpressionNode* e1_;
	ExpressionNode* e2_;
};


class BinaryOperatorNode: public ExpressionNode {
public:
    BinaryOperatorNode(bin_op op, ExpressionNode* l, ExpressionNode* r)
        : op_(op), l_(l), r_(r)
    {}
	Value* codegen() const;
private:
    bin_op op_;
    ExpressionNode* l_;
    ExpressionNode* r_;
};


class ReturnNode: public ExpressionNode {
public:
	ReturnNode(ExpressionNode* e)
		: e_(e)
	{}
	Value* codegen() const;
private:
	ExpressionNode* e_;
};


class BlockNode: public ExpressionNode {
public:
	BlockNode(vector<ExpressionNode*> statements)
		: statements_(statements)
	{}
	// ~BlockNode();
	Value* codegen() const;
private:
	vector<ExpressionNode*> statements_;
};


class PrintNode: public ExpressionNode {
public:
	PrintNode(ExpressionNode* e)
		: e_(e)
	{}
	Value* codegen() const;
private:
	ExpressionNode *e_;
};


class EmptyNode: public ExpressionNode {
public:
	EmptyNode() {}
	Value* codegen() const;
};


class FunctionCallNode: public ExpressionNode {
public:
	FunctionCallNode(string id, vector<ExpressionNode*> params)
		: id_(id), params_(params)
	{}
	// ~FunctionCallNode();
	Value* codegen() const;
private:
	string id_;
	vector<ExpressionNode*> params_;
};


class SequenceNode: public ExpressionNode {
public:
	SequenceNode(string id, ExpressionNode* e1, ExpressionNode* e2, ExpressionNode* e3)
		: id_(id), start_(e1), end_(e2), step_(e3)
	{}
	Value* codegen() const;
private:
	string id_;
	ExpressionNode* start_;
	ExpressionNode* end_;
	ExpressionNode* step_;
};


class IfElseNode: public ExpressionNode {
public:
   IfElseNode(ExpressionNode* cond, ExpressionNode* then, ExpressionNode* els)
       : cond_(cond), then_(then), else_(els)
   {}
   // ~IfElseNode();
   Value* codegen() const;
private:
   ExpressionNode *cond_;
   ExpressionNode *then_;
   ExpressionNode *else_;
};


class ForLoopNode: public ExpressionNode {
public:
	ForLoopNode(string id, ExpressionNode* start, ExpressionNode* end, ExpressionNode* body)
		: id_(id), start_(start), end_(end), body_(body)
	{}
	// ~ForLoopNode();
	Value* codegen() const;
private:
	string id_;
	ExpressionNode *start_;
    ExpressionNode *end_;
    ExpressionNode *body_;
};


class WhileNode: public ExpressionNode {
public:
	WhileNode(ExpressionNode* cond, ExpressionNode* body)
		: cond_(cond), body_(body)
	{}
	// ~ForLoopNode();
	Value* codegen() const;
private:
	string id_;
	ExpressionNode *cond_;
    ExpressionNode *body_;
};


class FunctionPrototypeNode {
public:
    FunctionPrototypeNode(string id, vector<pair<my_type, string>> params, my_type m)
        : id_(id), params_(params), m_(m)
    {}
	Function* codegen() const;
	string getName() const {
    return id_;
  	}
private:
    string id_;
    vector<pair<my_type, string>> params_;
	my_type m_;
};


class FunctionNode {
public:
	FunctionNode(FunctionPrototypeNode prototype, ExpressionNode* body)
		: prototype_(prototype), body_(body)
	{}
	// ~FunctionNode();
	Function* codegen() const;
private:
	FunctionPrototypeNode prototype_;
	ExpressionNode* body_;
};

void InitializeModuleAndPassManager();
AllocaInst *CreateEntryBlockAllocaInt(Function *TheFunction, const string &VarName);
AllocaInst *CreateEntryBlockAllocaDouble(Function *TheFunction, const string &VarName);
AllocaInst *CreateEntryBlockAllocaIntArray(Function *TheFunction, const string &VarName, unsigned size);
AllocaInst *CreateEntryBlockAllocaDoubleArray(Function *TheFunction, const string &VarName, unsigned size);
AllocaInst *CreateEntryBlockAllocaDoubleVector(Function * TheFunction, const string &VarName);
