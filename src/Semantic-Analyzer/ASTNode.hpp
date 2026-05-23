#pragma once

#include "SymbolTable.hpp"

#include <iostream>
#include <string>
#include <vector>

using namespace std;

class ASTNode 
{
public:
    TypeClass evalType = TypeClass::None;
    int tabIndex = -1;
    int btabIndex = -1;
    int atabIndex = -1;
    int level = -1;
    int line = 0;
    int column = 0;

    virtual ~ASTNode() = default;
    virtual void print(int indent = 0) const = 0;
};

// base category node (the ones that will not be visit/analyze further)
class ExpressionNode : public ASTNode {};
class StatementNode : public ASTNode {};
class DeclarationNode : public ASTNode {};
class TypeNode : public ASTNode {};

class ProgramNode : public ASTNode {
public:
    string name;
    vector<DeclarationNode*> declarations;
    StatementNode* body = nullptr;

    ProgramNode(string name);
    ~ProgramNode();
    void print(int indent = 0) const override;
};

class VarDeclNode : public DeclarationNode {
public:
    vector<string> names;
    vector<int> tabIndices;
    TypeNode* type = nullptr;
    bool isParameter = false;
    bool isVarParameter = false;

    VarDeclNode(vector<string> names, TypeNode* type);
    ~VarDeclNode();
    void print(int indent = 0) const override;
};

class ConstDeclNode : public DeclarationNode {
public:
    string name;
    ExpressionNode* value = nullptr;

    ConstDeclNode(string name, ExpressionNode* value);
    ~ConstDeclNode();
    void print(int indent = 0) const override;
};

class TypeDeclNode : public DeclarationNode {
public:
    string name;
    TypeNode* type = nullptr;

    TypeDeclNode(string name, TypeNode* type);
    ~TypeDeclNode();
    void print(int indent = 0) const override;
};

class NamedTypeNode : public TypeNode {
public:
    string name;
    NamedTypeNode(string name);
    void print(int indent = 0) const override;
};

class RangeTypeNode : public TypeNode {
public:
    ExpressionNode* low = nullptr;
    ExpressionNode* high = nullptr;
    RangeTypeNode(ExpressionNode* low, ExpressionNode* high);
    ~RangeTypeNode();
    void print(int indent = 0) const override;
};

class EnumeratedTypeNode : public TypeNode {
public:
    vector<string> values;

    EnumeratedTypeNode(vector<string> values);
    void print(int indent = 0) const override;
};

class ArrayTypeNode : public TypeNode {
public:
    TypeNode* indexType = nullptr;
    TypeNode* elementType = nullptr;
    ArrayTypeNode(TypeNode* indexType, TypeNode* elementType);
    ~ArrayTypeNode();
    void print(int indent = 0) const override;
};

class RecordTypeNode : public TypeNode {
public:
    vector<VarDeclNode*> fields;
    ~RecordTypeNode();
    void print(int indent = 0) const override;
};

class ProcedureDeclNode : public DeclarationNode {
public:
    string name;
    vector<VarDeclNode*> parameters;
    vector<DeclarationNode*> declarations;
    StatementNode* body = nullptr;

    ProcedureDeclNode(string name);
    ~ProcedureDeclNode();
    void print(int indent = 0) const override;
};

class FunctionDeclNode : public DeclarationNode {
public:
    string name;
    TypeNode* returnType = nullptr;
    vector<VarDeclNode*> parameters;
    vector<DeclarationNode*> declarations;
    StatementNode* body = nullptr;

    FunctionDeclNode(string name, TypeNode* returnType);
    ~FunctionDeclNode();
    void print(int indent = 0) const override;
};

// statement & expresion node
class CompoundNode : public StatementNode {
public:
    vector<StatementNode*> statements;

    ~CompoundNode();
    void print(int indent = 0) const override;
};

class AssignNode : public StatementNode {
public:
    ExpressionNode* target = nullptr;
    ExpressionNode* value = nullptr;

    AssignNode(ExpressionNode* target, ExpressionNode* value);
    ~AssignNode();
    void print(int indent = 0) const override;
};

class BinOpNode : public ExpressionNode {
public:
    string op;
    ExpressionNode* left = nullptr;
    ExpressionNode* right = nullptr;

    BinOpNode(string op, ExpressionNode* left, ExpressionNode* right);
    ~BinOpNode();
    void print(int indent = 0) const override;
};

class VarNode : public ExpressionNode {
public:
    string name;

    VarNode(string name);
    void print(int indent = 0) const override;
};

class NumberNode : public ExpressionNode {
public:
    string value;
    bool isReal = false;

    NumberNode(string value, bool isReal);
    void print(int indent = 0) const override;
};

class StringNode : public ExpressionNode {
public:
    string value;

    StringNode(string value);
    void print(int indent = 0) const override;
};

class IfNode : public StatementNode {
public:
    ExpressionNode* condition = nullptr;
    StatementNode* thenBranch = nullptr;
    StatementNode* elseBranch = nullptr;

    IfNode(ExpressionNode* condition, StatementNode* thenBranch, StatementNode* elseBranch = nullptr);
    ~IfNode();
    void print(int indent = 0) const override;
};

class WhileNode : public StatementNode {
public:
    ExpressionNode* condition = nullptr;
    StatementNode* body = nullptr;

    WhileNode(ExpressionNode* condition, StatementNode* body);
    ~WhileNode();
    void print(int indent = 0) const override;
};

class ForNode : public StatementNode {
public:
    string variable;
    ExpressionNode* start = nullptr;
    ExpressionNode* stop = nullptr;
    bool isDownto = false;
    StatementNode* body = nullptr;

    ForNode(string variable, ExpressionNode* start, ExpressionNode* stop, bool isDownto, StatementNode* body);
    ~ForNode();
    void print(int indent = 0) const override;
};

class RepeatNode : public StatementNode {
public:
    vector<StatementNode*> statements;
    ExpressionNode* condition = nullptr;

    RepeatNode(vector<StatementNode*> statements, ExpressionNode* condition);
    ~RepeatNode();
    void print(int indent = 0) const override;
};

class CaseBranch {
public:
    vector<ExpressionNode*> labels;
    StatementNode* statement = nullptr;

    CaseBranch(vector<ExpressionNode*> labels, StatementNode* statement);
    ~CaseBranch();
    void print(int indent = 0) const;
};

class CaseNode : public StatementNode {
public:
    ExpressionNode* expression = nullptr;
    vector<CaseBranch*> branches;

    CaseNode(ExpressionNode* expression, vector<CaseBranch*> branches);
    ~CaseNode();
    void print(int indent = 0) const override;
};

class CallNode : public StatementNode {
public:
    string name;
    vector<ExpressionNode*> arguments;

    CallNode(string name, vector<ExpressionNode*> arguments);
    ~CallNode();
    void print(int indent = 0) const override;
};

class ProcedureFunctionCallNode : public ExpressionNode {
public:
    string name;
    vector<ExpressionNode*> arguments;

    ProcedureFunctionCallNode(string name, vector<ExpressionNode*> arguments);
    ~ProcedureFunctionCallNode();
    void print(int indent = 0) const override;
};

class UnaryOpNode : public ExpressionNode {
public:
    string op;
    ExpressionNode* operand = nullptr;

    UnaryOpNode(string op, ExpressionNode* operand);
    ~UnaryOpNode();
    void print(int indent = 0) const override;
};

class CharNode : public ExpressionNode {
public:
    string value;
    CharNode(string value);
    void print(int indent = 0) const override;
};

class BoolNode : public ExpressionNode {
public:
    bool value;
    BoolNode(bool value);
    void print(int indent = 0) const override;
};

class ArrayAccessNode : public ExpressionNode {
public:
    ExpressionNode* array = nullptr;
    ExpressionNode* index = nullptr;

    ArrayAccessNode(ExpressionNode* array, ExpressionNode* index);
    ~ArrayAccessNode();
    void print(int indent = 0) const override;
};

class RecordAccessNode : public ExpressionNode {
public:
    ExpressionNode* record = nullptr;
    string field;

    RecordAccessNode(ExpressionNode* record, string field);
    ~RecordAccessNode();
    void print(int indent = 0) const override;
};
