#pragma once

#include "ASTNode.hpp"
#include "../Parser/Node.hpp"

#include <string>
#include <vector>

using namespace std;

class ASTBuilder {
private:
    vector<string> errors;

    bool isTerminal(Node* node) const;
    bool isToken(Node* node, const string& type) const;
    string lexeme(Node* node) const;

    Node* findChild(Node* node, NodeType type) const;
    vector<Node*> findChildren(Node* node, NodeType type) const;

    string getFirstIdent(Node* node) const;
    string getOperator(Node* node) const;

    ProgramNode* buildProgram(Node* node);

    vector<DeclarationNode*> buildDeclarationPart(Node* node);
    vector<DeclarationNode*> buildConstDeclaration(Node* node);
    vector<DeclarationNode*> buildTypeDeclaration(Node* node);
    vector<DeclarationNode*> buildVarDeclaration(Node* node);
    DeclarationNode* buildSubprogramDeclaration(Node* node);
    ProcedureDeclNode* buildProcedureDeclaration(Node* node);
    FunctionDeclNode* buildFunctionDeclaration(Node* node);

    vector<string> buildIdentifierList(Node* node);
    ExpressionNode* buildConstant(Node* node);

    TypeNode* buildType(Node* node);
    TypeNode* buildArrayType(Node* node);
    TypeNode* buildRangeType(Node* node);
    TypeNode* buildEnumeratedType(Node* node);
    TypeNode* buildRecordType(Node* node);

    StatementNode* buildCompoundStatement(Node* node);
    vector<StatementNode*> buildStatementList(Node* node);
    StatementNode* buildStatement(Node* node);

    StatementNode* buildAssignment(Node* node);
    StatementNode* buildIf(Node* node);
    StatementNode* buildWhile(Node* node);
    StatementNode* buildFor(Node* node);
    StatementNode* buildRepeat(Node* node);
    StatementNode* buildCase(Node* node);
    vector<CaseBranch*> buildCaseBlock(Node* node);
    StatementNode* buildCallStatement(Node* node);

    ExpressionNode* buildExpression(Node* node);
    ExpressionNode* buildSimpleExpression(Node* node);
    ExpressionNode* buildTerm(Node* node);
    ExpressionNode* buildFactor(Node* node);
    ExpressionNode* buildVariable(Node* node);
    FunctionCallNode* buildCallExpression(Node* node);

    vector<VarDeclNode*> buildFormalParameterList(Node* node);
    vector<VarDeclNode*> buildParameterGroup(Node* node);
    vector<ExpressionNode*> buildParameterList(Node* node);
    vector<ExpressionNode*> buildIndexList(Node* node);

    void addError(const string& message);

public:
    ASTBuilder() = default;

    ProgramNode* build(Node* parseTree);

    bool hasErrors() const;
    const vector<string>& getErrors() const;
};
