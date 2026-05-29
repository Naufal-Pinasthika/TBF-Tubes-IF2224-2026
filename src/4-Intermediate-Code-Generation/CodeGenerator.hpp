#pragma once

#include "IntermediateCode.hpp"
#include "../3-Semantic-Analyzer/ASTNode.hpp"

#include <map>
#include <string>

using namespace std;

class CodeGenerator
{
private:
    TacProgram program;
    int nextTempId = 0;
    int nextLabelId = 0;

    string newTemp();
    string newLabel(const string& prefix = "L");

    string emitExpression(ExpressionNode* node);
    void emitStatement(StatementNode* node);
    void emitDeclaration(DeclarationNode* node);
    void emitProgram(ProgramNode* node);

    void emitCompound(CompoundNode* node);
    void emitAssign(AssignNode* node);
    void emitIf(IfNode* node);
    void emitWhile(WhileNode* node);
    void emitFor(ForNode* node);
    void emitRepeat(RepeatNode* node);
    void emitCase(CaseNode* node);
    void emitCall(CallNode* node);

    string emitBinary(BinOpNode* node);
    string emitUnary(UnaryOpNode* node);
    string emitVar(VarNode* node);
    string emitLiteral(NumberNode* node);
    string emitLiteral(StringNode* node);
    string emitLiteral(CharNode* node);
    string emitLiteral(BoolNode* node);
    string emitCallExpr(ProcedureFunctionCallNode* node);

public:
    CodeGenerator() = default;

    const TacProgram& generate(ProgramNode* root);
    const TacProgram& getProgram() const;
    void reset();
};