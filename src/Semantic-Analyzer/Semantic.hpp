#pragma once

#include "ASTNode.hpp"
#include "SymbolTable.hpp"

#include <string>
#include <vector>

using namespace std;

class Semantic 
{
private:
    SymbolTable symbolTable;
    vector<string> errors;

    void addError(const string& message);
    int lookupName(const string& name);
    bool isRelationalOp(const string& op) const;
    void decorate(ASTNode* node, int tabIndex);

    void analyzeDeclaration(DeclarationNode* node);
    void analyzeStatement(StatementNode* node);
    TypeClass analyzeExpression(ExpressionNode* node);
    TypeClass resolveType(TypeNode* node, int& ref);

    void analyzeVarDecl(VarDeclNode* node);
    void analyzeConstDecl(ConstDeclNode* node);
    void analyzeTypeDecl(TypeDeclNode* node);
    void analyzeProcedureDecl(ProcedureDeclNode* node);
    void analyzeFunctionDecl(FunctionDeclNode* node);

    bool isNumeric(TypeClass type) const;
    bool isCompatible(TypeClass left, TypeClass right) const;
    bool isAssignable(TypeClass target, TypeClass value) const;
    int constantAddress(ExpressionNode* node) const;
    string typeName(TypeClass type) const;

public:
    Semantic() = default;

    void analyze(ProgramNode* root);

    bool hasErrors() const;
    const vector<string>& getErrors() const;
    SymbolTable& getSymbolTable();
};