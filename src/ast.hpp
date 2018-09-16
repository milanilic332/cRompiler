#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
using namespace std;


enum class bin_op {
	plus,
	minus,
	mul,
	div,
	gt,
	lt,
	geq,
	leq,
	eq,
	neq
};


class ExpressionNode {
public:
	virtual ~ExpressionNode() {}
	virtual Value* codegen() const = 0;
};


class VariableNode: public ExpressionNode {
public:
	VariableNode(const string &id)
		: id_(id)
	{}
    Value* codegen() const;
private:
	std::string id_;
};


class ConstantNode: public ExpressionNode {
public:
    ConstantNode(const int num)
		: num_(num)
	{}
	Value* codegen() const;
private:
	int num_;
};


class AssignmentNode: public ExpressionNode {
public:
    AssignmentNode(const string &id, ExpressionNode* e)
        : id_(id), e_(e)
    {}
	Value* codegen() const;
private:
	string id_;
    ExpressionNode* e_;
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
	~BlockNode();
	Value* codegen() const;
private:
	vector<ExpressionNode*> statements_;
};


class FunctionCallNode: public ExpressionNode {
public:
	FunctionCallNode(const string &id, const vector<ExpressionNode*> &params)
		: id_(id), params_(params)
	{}
	~FunctionCallNode();
	Value* codegen() const;
private:
	string id_;
	vector<ExpressionNode*> params_;
};


class IfElseNode: public ExpressionNode {
public:
   IfElseNode(ExpressionNode* cond, ExpressionNode* then, ExpressionNode* els)
       : cond_(cond), then_(then), else_(els)
   {}
   ~IfElseNode();
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
	~ForLoopNode();
	Value* codegen() const;
private:
	string id_;
	ExpressionNode *start_;
    ExpressionNode *end_;
    ExpressionNode *body_;
};


class FunctionPrototypeNode {
public:
    FunctionPrototypeNode(const std::string &id, const vector<string> &params)
        : id_(id), params_(params)
    {}
	Value* codegen() const;
private:
    string id_;
    vector<string> params_;
};


class FunctionNode {
public:
	FunctionNode(const FunctionPrototypeNode &prototype, ExpressionNode* body)
		: prototype_(prototype), body_(body)
	{}
	~FunctionNode();
	Value* codegen() const;
private:
	FunctionPrototypeNode prototype_;
	ExpressionNode* body_;
};
