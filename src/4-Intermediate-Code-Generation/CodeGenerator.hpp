#pragma once

#include "IntermediateCode.hpp"
#include "../3-Semantic-Analyzer/ASTNode.hpp"
#include "../3-Semantic-Analyzer/SymbolTable.hpp"

#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class CodeGenerator
{
private:
    TacProgram program;
    SymbolTable* symbols = nullptr;
    int nextLabelId = 0;
    unordered_map<int, int> addressByTabIndex;
    unordered_map<int, int> frameSizeByBlockIndex;

    string newLabel(const string& prefix = "L");

    void buildAddressMap(ProgramNode* root);
    void collectDeclarations(const vector<DeclarationNode*>& declarations);
    void collectBlock(StatementNode* body, int blockIndex);

    int addressOf(const ASTNode* node) const;
    int addressOf(int tabIndex) const;
    int frameSizeOf(int blockIndex) const;

    void emitExpression(ExpressionNode* node);
    void emitLValue(ExpressionNode* node);
    void emitOperation(TacOperation operation);
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
    void emitWriteCall(CallNode* node);

    void emitBinary(BinOpNode* node);
    void emitUnary(UnaryOpNode* node);
    void emitCallExpr(ProcedureFunctionCallNode* node);

    bool isWriteCall(const string& name) const;

public:
    CodeGenerator() = default;

    const TacProgram& generate(ProgramNode* root, SymbolTable& symbols);
    const TacProgram& getProgram() const;
    void reset();
};
