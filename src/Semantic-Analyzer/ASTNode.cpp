#include "ASTPrinter.hpp"

ProgramNode::ProgramNode(string name) : name(name) {}

ProgramNode::~ProgramNode(){
    for (DeclarationNode* declaration : declarations){
        delete declaration;
    }
    delete body;
}

void ProgramNode::print(int indent) const { printAstNode(this, indent); }

VarDeclNode::VarDeclNode(vector<string> names, TypeNode* type) : names(names), type(type) {}

VarDeclNode::~VarDeclNode(){
    delete type;
}

void VarDeclNode::print(int indent) const { printAstNode(this, indent); }

ConstDeclNode::ConstDeclNode(string name, ExpressionNode* value) : name(name), value(value) {}

ConstDeclNode::~ConstDeclNode(){
    delete value;
}

void ConstDeclNode::print(int indent) const { printAstNode(this, indent); }

TypeDeclNode::TypeDeclNode(string name, TypeNode* type) : name(name), type(type) {}

TypeDeclNode::~TypeDeclNode(){
    delete type;
}

void TypeDeclNode::print(int indent) const { printAstNode(this, indent); }

NamedTypeNode::NamedTypeNode(string name) : name(name) {}

void NamedTypeNode::print(int indent) const { printAstNode(this, indent); }

RangeTypeNode::RangeTypeNode(ExpressionNode* low, ExpressionNode* high) : low(low), high(high) {}

RangeTypeNode::~RangeTypeNode(){
    delete low;
    delete high;
}

void RangeTypeNode::print(int indent) const { printAstNode(this, indent); }

EnumeratedTypeNode::EnumeratedTypeNode(vector<string> values) : values(values){
    evalType = TypeClass::Enumerated;
}

void EnumeratedTypeNode::print(int indent) const { printAstNode(this, indent); }

ArrayTypeNode::ArrayTypeNode(TypeNode* indexType, TypeNode* elementType) : indexType(indexType), elementType(elementType){
    evalType = TypeClass::Array;
}

ArrayTypeNode::~ArrayTypeNode(){
    delete indexType;
    delete elementType;
}

void ArrayTypeNode::print(int indent) const { printAstNode(this, indent); }

RecordTypeNode::~RecordTypeNode(){
    for (VarDeclNode* field : fields){
        delete field;
    }
}

void RecordTypeNode::print(int indent) const { printAstNode(this, indent); }

ProcedureDeclNode::ProcedureDeclNode(string name) : name(name) {}

ProcedureDeclNode::~ProcedureDeclNode(){
    for (VarDeclNode* parameter : parameters){
        delete parameter;
    }
    for (DeclarationNode* declaration : declarations){
        delete declaration;
    }
    delete body;
}

void ProcedureDeclNode::print(int indent) const { printAstNode(this, indent); }

FunctionDeclNode::FunctionDeclNode(string name, TypeNode* returnType) : name(name), returnType(returnType) {}

FunctionDeclNode::~FunctionDeclNode(){
    delete returnType;
    for (VarDeclNode* parameter : parameters){
        delete parameter;
    }
    for (DeclarationNode* declaration : declarations){
        delete declaration;
    }
    delete body;
}

void FunctionDeclNode::print(int indent) const { printAstNode(this, indent); }

CompoundNode::~CompoundNode(){
    for (StatementNode* statement : statements){
        delete statement;
    }
}

void CompoundNode::print(int indent) const { printAstNode(this, indent); }

AssignNode::AssignNode(ExpressionNode* target, ExpressionNode* value) : target(target), value(value) {}

AssignNode::~AssignNode(){
    delete target;
    delete value;
}

void AssignNode::print(int indent) const { printAstNode(this, indent); }

BinOpNode::BinOpNode(string op, ExpressionNode* left, ExpressionNode* right) : op(op), left(left), right(right) {}

BinOpNode::~BinOpNode(){
    delete left;
    delete right;
}

void BinOpNode::print(int indent) const { printAstNode(this, indent); }

VarNode::VarNode(string name) : name(name) {}

void VarNode::print(int indent) const { printAstNode(this, indent); }

NumberNode::NumberNode(string value, bool isReal) : value(value), isReal(isReal){
    evalType = isReal ? TypeClass::Real : TypeClass::Integer;
}

void NumberNode::print(int indent) const { printAstNode(this, indent); }

StringNode::StringNode(string value) : value(value){
    evalType = TypeClass::String;
}

void StringNode::print(int indent) const { printAstNode(this, indent); }

IfNode::IfNode(ExpressionNode* condition, StatementNode* thenBranch, StatementNode* elseBranch) : condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {}

IfNode::~IfNode(){
    delete condition;
    delete thenBranch;
    delete elseBranch;
}

void IfNode::print(int indent) const { printAstNode(this, indent); }

WhileNode::WhileNode(ExpressionNode* condition, StatementNode* body) : condition(condition), body(body) {}

WhileNode::~WhileNode(){
    delete condition;
    delete body;
}

void WhileNode::print(int indent) const { printAstNode(this, indent); }

ForNode::ForNode(string variable, ExpressionNode* start, ExpressionNode* stop, bool isDownto, StatementNode* body) : variable(variable), start(start), stop(stop), isDownto(isDownto), body(body) {}

ForNode::~ForNode(){
    delete start;
    delete stop;
    delete body;
}

void ForNode::print(int indent) const { printAstNode(this, indent); }

RepeatNode::RepeatNode(vector<StatementNode*> statements, ExpressionNode* condition) : statements(statements), condition(condition) {}

RepeatNode::~RepeatNode(){
    for (StatementNode* statement : statements){
        delete statement;
    }
    delete condition;
}

void RepeatNode::print(int indent) const { printAstNode(this, indent); }

CaseBranch::CaseBranch(vector<ExpressionNode*> labels, StatementNode* statement) : labels(labels), statement(statement) {}

CaseBranch::~CaseBranch(){
    for (ExpressionNode* label : labels){
        delete label;
    }
    delete statement;
}

void CaseBranch::print(int indent) const { printAstCaseBranch(this, indent); }

CaseNode::CaseNode(ExpressionNode* expression, vector<CaseBranch*> branches) : expression(expression), branches(branches) {}

CaseNode::~CaseNode(){
    delete expression;
    for (CaseBranch* branch : branches){
        delete branch;
    }
}

void CaseNode::print(int indent) const { printAstNode(this, indent); }

CallNode::CallNode(string name, vector<ExpressionNode*> arguments) : name(name), arguments(arguments) {}

CallNode::~CallNode(){
    for (ExpressionNode* argument : arguments){
        delete argument;
    }
}

void CallNode::print(int indent) const { printAstNode(this, indent); }

FunctionCallNode::FunctionCallNode(string name, vector<ExpressionNode*> arguments) : name(name), arguments(arguments) {}

FunctionCallNode::~FunctionCallNode(){
    for (ExpressionNode* argument : arguments){
        delete argument;
    }
}

void FunctionCallNode::print(int indent) const { printAstNode(this, indent); }

UnaryOpNode::UnaryOpNode(string op, ExpressionNode* operand) : op(op), operand(operand) {}

UnaryOpNode::~UnaryOpNode(){
    delete operand;
}

void UnaryOpNode::print(int indent) const { printAstNode(this, indent); }

CharNode::CharNode(string value) : value(value){
    evalType = TypeClass::Char;
}

void CharNode::print(int indent) const { printAstNode(this, indent); }

BoolNode::BoolNode(bool value) : value(value){
    evalType = TypeClass::Boolean;
}

void BoolNode::print(int indent) const { printAstNode(this, indent); }

ArrayAccessNode::ArrayAccessNode(ExpressionNode* array, ExpressionNode* index) : array(array), index(index) {}

ArrayAccessNode::~ArrayAccessNode(){
    delete array;
    delete index;
}

void ArrayAccessNode::print(int indent) const { printAstNode(this, indent); }

RecordAccessNode::RecordAccessNode(ExpressionNode* record, string field) : record(record), field(field) {}

RecordAccessNode::~RecordAccessNode(){
    delete record;
}

void RecordAccessNode::print(int indent) const { printAstNode(this, indent); }
