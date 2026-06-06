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
    ICProgram program;
    SymbolTable* symbols = nullptr;
    int nextLabelId = 0;
    int currentBlockIndex = 0;
    unordered_map<int, int> addressByTabIndex;
    unordered_map<int, unordered_map<string, int>> addressByNameByBlock;
    unordered_map<int, int> frameSizeByBlockIndex;
    unordered_map<int, ICValue> constantByTabIndex;

    string newLabel(const string& prefix = "L");

    void buildAddressMap(ProgramNode* root);
    void collectDeclarations(const vector<DeclarationNode*>& declarations);
    void collectBlock(StatementNode* body, int blockIndex);

    int addressOf(const ASTNode* node) const;
    int addressOf(int tabIndex) const;
    int addressOfName(const string& name) const;
    int frameSizeOf(int blockIndex) const;
    int typeSize(TypeClass type, int ref) const;
    int arraySize(int atabIndex) const;
    int recordFieldOffset(int recordBlockIndex, const string& field) const;

    void emitAddress(ExpressionNode* node);
    void emitExpression(ExpressionNode* node);
    void emitLValue(ExpressionNode* node);
    void emitOperation(ICOperation operation);
    void emitStatement(StatementNode* node);
    void emitDeclaration(DeclarationNode* node);
    void emitProgram(ProgramNode* node);
    void emitParameterStores(const vector<VarDeclNode*>& parameters);

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

    const ICProgram& generate(ProgramNode* root, SymbolTable& symbols);
    const ICProgram& getProgram() const;
    void reset();
};
