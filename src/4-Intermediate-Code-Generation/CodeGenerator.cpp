#include "CodeGenerator.hpp"

#include <cctype>

using namespace std;

string CodeGenerator::newLabel(const string& prefix) {
    return prefix + to_string(nextLabelId++);
}

const TacProgram& CodeGenerator::generate(ProgramNode* root, SymbolTable& symbols) {
    reset();
    this->symbols = &symbols;
    buildAddressMap(root);
    emitProgram(root);
    return program;
}

const TacProgram& CodeGenerator::getProgram() const {
    return program;
}

void CodeGenerator::reset() {     
    program.clear();
    symbols = nullptr;
    nextLabelId = 0;
    currentBlockIndex = 0;
    addressByTabIndex.clear();
    addressByNameByBlock.clear();
    frameSizeByBlockIndex.clear();
    constantByTabIndex.clear();
}

void CodeGenerator::buildAddressMap(ProgramNode* root) {
    if (root == nullptr) return;

    int mainBlock = root->body != nullptr ? root->body->btabIndex : 0;
    int nextAddress = 3;

    for (DeclarationNode* declaration : root->declarations){
        if (auto varDecl = dynamic_cast<VarDeclNode*>(declaration)){
            for (size_t i = 0; i < varDecl->tabIndices.size(); ++i){
                int tabIndex = varDecl->tabIndices[i];
                addressByTabIndex[tabIndex] = nextAddress++;
                if (i < varDecl->names.size() && symbols != nullptr){
                    addressByNameByBlock[mainBlock][symbols->toUpper(varDecl->names[i])] = addressByTabIndex[tabIndex];
                }
            }
        } else if (auto constDecl = dynamic_cast<ConstDeclNode*>(declaration)){
            if (constDecl->tabIndex != -1){
                TacValue value;
                if (auto number = dynamic_cast<NumberNode*>(constDecl->value)){
                    value.type = number->isReal ? TacValueType::Real : TacValueType::Integer;
                    value.text = number->value;
                } else if (auto str = dynamic_cast<StringNode*>(constDecl->value)){
                    value.type = TacValueType::String;
                    value.text = str->value;
                } else if (auto chr = dynamic_cast<CharNode*>(constDecl->value)){
                    value.type = TacValueType::Char;
                    value.text = chr->value;
                } else if (auto boolean = dynamic_cast<BoolNode*>(constDecl->value)){
                    value.type = TacValueType::Boolean;
                    value.text = boolean->value ? "1" : "0";
                }
                constantByTabIndex[constDecl->tabIndex] = value;
            }
        }
    }
    frameSizeByBlockIndex[mainBlock] = nextAddress;
    collectDeclarations(root->declarations);
}

void CodeGenerator::collectDeclarations(const vector<DeclarationNode*>& declarations) {
    for (DeclarationNode* declaration : declarations){
        if (auto procDecl = dynamic_cast<ProcedureDeclNode*>(declaration)){
            int nextAddress = 3;
            for (VarDeclNode* parameter : procDecl->parameters){
                for (size_t i = 0; i < parameter->tabIndices.size(); ++i){
                    int tabIndex = parameter->tabIndices[i];
                    addressByTabIndex[tabIndex] = nextAddress++;
                    if (i < parameter->names.size() && symbols != nullptr){
                        addressByNameByBlock[procDecl->btabIndex][symbols->toUpper(parameter->names[i])] = addressByTabIndex[tabIndex];
                    }
                }
            }
            for (DeclarationNode* localDeclaration : procDecl->declarations){
                if (auto varDecl = dynamic_cast<VarDeclNode*>(localDeclaration)){
                    for (size_t i = 0; i < varDecl->tabIndices.size(); ++i){
                        int tabIndex = varDecl->tabIndices[i];
                        addressByTabIndex[tabIndex] = nextAddress++;
                        if (i < varDecl->names.size() && symbols != nullptr){
                            addressByNameByBlock[procDecl->btabIndex][symbols->toUpper(varDecl->names[i])] = addressByTabIndex[tabIndex];
                        }
                    }
                } else if (auto constDecl = dynamic_cast<ConstDeclNode*>(localDeclaration)){
                    if (constDecl->tabIndex != -1){
                        TacValue value;
                        if (auto number = dynamic_cast<NumberNode*>(constDecl->value)){
                            value.type = number->isReal ? TacValueType::Real : TacValueType::Integer;
                            value.text = number->value;
                        } else if (auto str = dynamic_cast<StringNode*>(constDecl->value)){
                            value.type = TacValueType::String;
                            value.text = str->value;
                        } else if (auto chr = dynamic_cast<CharNode*>(constDecl->value)){
                            value.type = TacValueType::Char;
                            value.text = chr->value;
                        } else if (auto boolean = dynamic_cast<BoolNode*>(constDecl->value)){
                            value.type = TacValueType::Boolean;
                            value.text = boolean->value ? "1" : "0";
                        }
                        constantByTabIndex[constDecl->tabIndex] = value;
                    }
                }
            }
            frameSizeByBlockIndex[procDecl->btabIndex] = nextAddress;
            collectDeclarations(procDecl->declarations);
        } else if (auto funcDecl = dynamic_cast<FunctionDeclNode*>(declaration)){
            int nextAddress = 3;
            for (VarDeclNode* parameter : funcDecl->parameters){
                for (size_t i = 0; i < parameter->tabIndices.size(); ++i){
                    int tabIndex = parameter->tabIndices[i];
                    addressByTabIndex[tabIndex] = nextAddress++;
                    if (i < parameter->names.size() && symbols != nullptr){
                        addressByNameByBlock[funcDecl->btabIndex][symbols->toUpper(parameter->names[i])] = addressByTabIndex[tabIndex];
                    }
                }
            }
            for (DeclarationNode* localDeclaration : funcDecl->declarations){
                if (auto varDecl = dynamic_cast<VarDeclNode*>(localDeclaration)){
                    for (size_t i = 0; i < varDecl->tabIndices.size(); ++i){
                        int tabIndex = varDecl->tabIndices[i];
                        addressByTabIndex[tabIndex] = nextAddress++;
                        if (i < varDecl->names.size() && symbols != nullptr) {
                            addressByNameByBlock[funcDecl->btabIndex][symbols->toUpper(varDecl->names[i])] = addressByTabIndex[tabIndex];
                        }
                    }
                }
            }
            frameSizeByBlockIndex[funcDecl->btabIndex] = nextAddress;
            collectDeclarations(funcDecl->declarations);
        }
    }
}

void CodeGenerator::collectBlock(StatementNode* body, int blockIndex) {
    int currentSize = frameSizeOf(blockIndex);
    if (currentSize < 3) currentSize = 3;
    frameSizeByBlockIndex[blockIndex] = currentSize;
    (void) body;
}

int CodeGenerator::addressOf(const ASTNode* node) const {
    if (node == nullptr) return 0;
    return addressOf(node->tabIndex);
}

int CodeGenerator::addressOf(int tabIndex) const {
    auto found = addressByTabIndex.find(tabIndex);
    if (found != addressByTabIndex.end()) return found->second;

    if (symbols != nullptr){
        TabEntry* entry = symbols->getTab(tabIndex);
        if (entry != nullptr) return entry->adr;
    }
    return 0;
}

int CodeGenerator::addressOfName(const string& name) const {
    if (symbols == nullptr) return 0;

    string upperName = symbols->toUpper(name);
    auto blockFound = addressByNameByBlock.find(currentBlockIndex);
    if (blockFound != addressByNameByBlock.end()){
        auto nameFound = blockFound->second.find(upperName);
        if (nameFound != blockFound->second.end()){
            return nameFound->second;
        }
    }

    for (const auto& entry : addressByTabIndex){
        TabEntry* tabEntry = symbols->getTab(entry.first);
        if (tabEntry != nullptr && symbols->toUpper(tabEntry->identifiers) == upperName){
            return entry.second;
        }
    }
    return 0;
}

int CodeGenerator::frameSizeOf(int blockIndex) const {
    auto found = frameSizeByBlockIndex.find(blockIndex);
    if (found != frameSizeByBlockIndex.end()){
        return found->second;
    }
    return 3;
}

void CodeGenerator::emitExpression(ExpressionNode* node) {
    if (node == nullptr) return;

    if (auto number = dynamic_cast<NumberNode*>(node)) {
        IntermediateInstruction instruction;
        instruction.opcode = TacOpcode::Lit;
        instruction.operand.kind = TacOperandKind::Literal;
        instruction.operand.literal.type = number->isReal ? TacValueType::Real : TacValueType::Integer;
        instruction.operand.literal.text = number->value;
        program.add(instruction);
    } else if (auto str = dynamic_cast<StringNode*>(node)){
        IntermediateInstruction instruction;
        instruction.opcode = TacOpcode::Lit;
        instruction.operand.kind = TacOperandKind::Literal;
        instruction.operand.literal.type = TacValueType::String;
        instruction.operand.literal.text = str->value;
        program.add(instruction);
    } else if (auto chr = dynamic_cast<CharNode*>(node)){
        IntermediateInstruction instruction;
        instruction.opcode = TacOpcode::Lit;
        instruction.operand.kind = TacOperandKind::Literal;
        instruction.operand.literal.type = TacValueType::Char;
        instruction.operand.literal.text = chr->value;
        program.add(instruction);
    } else if (auto boolean = dynamic_cast<BoolNode*>(node)){
        IntermediateInstruction instruction;
        instruction.opcode = TacOpcode::Lit;
        instruction.operand.kind = TacOperandKind::Literal;
        instruction.operand.literal.type = TacValueType::Boolean;
        instruction.operand.literal.text = boolean->value ? "1" : "0";
        program.add(instruction);
    } else if (auto var = dynamic_cast<VarNode*>(node)){
        auto constant = constantByTabIndex.find(var->tabIndex);
        if (constant != constantByTabIndex.end()){
            IntermediateInstruction instruction;
            instruction.opcode = TacOpcode::Lit;
            instruction.operand.kind = TacOperandKind::Literal;
            instruction.operand.literal = constant->second;
            program.add(instruction);
        } else if (symbols != nullptr){
            TabEntry* entry = symbols->getTab(var->tabIndex);
            if (entry != nullptr && entry->obj == ObjClass::Constant){
                IntermediateInstruction instruction;
                instruction.opcode = TacOpcode::Lit;
                instruction.operand.kind = TacOperandKind::Literal;

                if (entry->type == TypeClass::Boolean){
                    instruction.operand.literal.type = TacValueType::Boolean;
                    instruction.operand.literal.text = entry->adr == 0 ? "0" : "1";
                    program.add(instruction);
                } else if (entry->type == TypeClass::Integer){
                    instruction.operand.literal.type = TacValueType::Integer;
                    instruction.operand.literal.text = to_string(entry->adr);
                    program.add(instruction);
                } else if (entry->type == TypeClass::Char){
                    instruction.operand.literal.type = TacValueType::Char;
                    instruction.operand.literal.text = string(1, static_cast<char>(entry->adr));
                    program.add(instruction);
                }
            } else {
                IntermediateInstruction instruction;
                instruction.opcode = TacOpcode::Lod;
                instruction.level = 0;
                instruction.operand.kind = TacOperandKind::Address;
                instruction.operand.address = addressOf(var);
                program.add(instruction);
            }
        } else {
            IntermediateInstruction instruction;
            instruction.opcode = TacOpcode::Lod;
            instruction.level = 0;
            instruction.operand.kind = TacOperandKind::Address;
            instruction.operand.address = addressOf(var);
            program.add(instruction);
        }
    } else if (auto binary = dynamic_cast<BinOpNode*>(node)){
        emitBinary(binary);
    } else if (auto unary = dynamic_cast<UnaryOpNode*>(node)){
        emitUnary(unary);
    } else if (auto call = dynamic_cast<ProcedureFunctionCallNode*>(node)){
        emitCallExpr(call);
    }
}

void CodeGenerator::emitLValue(ExpressionNode* node) {
    if (auto var = dynamic_cast<VarNode*>(node)){
        IntermediateInstruction instruction;
        instruction.opcode = TacOpcode::Lit;
        instruction.operand.kind = TacOperandKind::Address;
        instruction.operand.address = addressOf(var);
        program.add(instruction);
    }
}

void CodeGenerator::emitOperation(TacOperation operation) {
    IntermediateInstruction instruction;
    instruction.opcode = TacOpcode::Opr;
    instruction.level = 0;
    instruction.operand.kind = TacOperandKind::Operation;
    instruction.operand.operationCode = operation;
    program.add(instruction);
}

void CodeGenerator::emitStatement(StatementNode* node) {
    if (node == nullptr) return;

    if (auto compound = dynamic_cast<CompoundNode*>(node)) emitCompound(compound);
    else if (auto assign = dynamic_cast<AssignNode*>(node)) emitAssign(assign);
    else if (auto ifNode = dynamic_cast<IfNode*>(node)) emitIf(ifNode);
    else if (auto whileNode = dynamic_cast<WhileNode*>(node)) emitWhile(whileNode);
    else if (auto forNode = dynamic_cast<ForNode*>(node)) emitFor(forNode);
    else if (auto repeatNode = dynamic_cast<RepeatNode*>(node)) emitRepeat(repeatNode);
    else if (auto caseNode = dynamic_cast<CaseNode*>(node)) emitCase(caseNode);
    else if (auto call = dynamic_cast<CallNode*>(node)) emitCall(call);
}

void CodeGenerator::emitDeclaration(DeclarationNode* node) {
    if (auto procDecl = dynamic_cast<ProcedureDeclNode*>(node)){
        int previousBlock = currentBlockIndex;
        currentBlockIndex = procDecl->btabIndex;

        IntermediateInstruction label;
        label.opcode = TacOpcode::Label;
        label.operand.kind = TacOperandKind::Label;
        label.operand.label = "P_" + to_string(procDecl->tabIndex);
        program.add(label);

        IntermediateInstruction init;
        init.opcode = TacOpcode::Int;
        init.level = 0;
        init.operand.kind = TacOperandKind::Address;
        init.operand.address = frameSizeOf(procDecl->btabIndex);
        program.add(init);

        emitStatement(procDecl->body);

        IntermediateInstruction ret;
        ret.opcode = TacOpcode::Ret;
        program.add(ret);

        currentBlockIndex = previousBlock;
    } else if (auto funcDecl = dynamic_cast<FunctionDeclNode*>(node)){
        int previousBlock = currentBlockIndex;
        currentBlockIndex = funcDecl->btabIndex;

        IntermediateInstruction label;
        label.opcode = TacOpcode::Label;
        label.operand.kind = TacOperandKind::Label;
        label.operand.label = "F_" + to_string(funcDecl->tabIndex);
        program.add(label);

        IntermediateInstruction init;
        init.opcode = TacOpcode::Int;
        init.level = 0;
        init.operand.kind = TacOperandKind::Address;
        init.operand.address = frameSizeOf(funcDecl->btabIndex);
        program.add(init);

        emitStatement(funcDecl->body);

        IntermediateInstruction ret;
        ret.opcode = TacOpcode::Ret;
        program.add(ret);

        currentBlockIndex = previousBlock;
    }
}

void CodeGenerator::emitProgram(ProgramNode* node) {
    if (node == nullptr) return;

    bool hasSubprogram = false;
    for (DeclarationNode* declaration : node->declarations){
        if (dynamic_cast<ProcedureDeclNode*>(declaration) != nullptr || dynamic_cast<FunctionDeclNode*>(declaration) != nullptr){
            hasSubprogram = true;
        }
    }

    string mainLabel = "MAIN";
    if (hasSubprogram){
        IntermediateInstruction jumpMain;
        jumpMain.opcode = TacOpcode::Jmp;
        jumpMain.level = 0;
        jumpMain.operand.kind = TacOperandKind::Label;
        jumpMain.operand.label = mainLabel;
        program.add(jumpMain);
    }

    for (DeclarationNode* declaration : node->declarations){
        emitDeclaration(declaration);
    }

    if (hasSubprogram){
        IntermediateInstruction label;
        label.opcode = TacOpcode::Label;
        label.operand.kind = TacOperandKind::Label;
        label.operand.label = mainLabel;
        program.add(label);
    }

    currentBlockIndex = node->body != nullptr ? node->body->btabIndex : 0;

    IntermediateInstruction init;
    init.opcode = TacOpcode::Int;
    init.level = 0;
    init.operand.kind = TacOperandKind::Address;
    init.operand.address = frameSizeOf(node->body != nullptr ? node->body->btabIndex : 0);
    program.add(init);

    emitStatement(node->body);

    IntermediateInstruction ret;
    ret.opcode = TacOpcode::Ret;
    program.add(ret);
}

void CodeGenerator::emitCompound(CompoundNode* node) {
    for (StatementNode* statement : node->statements){
        emitStatement(statement);
    }
}

void CodeGenerator::emitAssign(AssignNode* node) {
    emitExpression(node->value);

    if (auto var = dynamic_cast<VarNode*>(node->target)){
        IntermediateInstruction instruction;
        instruction.opcode = TacOpcode::Sto;
        instruction.level = 0;
        instruction.operand.kind = TacOperandKind::Address;
        instruction.operand.address = addressOf(var);
        program.add(instruction);
    }
}

void CodeGenerator::emitIf(IfNode* node) {
    string elseLabel = newLabel("L_ELSE_");
    string endLabel = newLabel("L_END_");

    emitExpression(node->condition);

    IntermediateInstruction jumpElse;
    jumpElse.opcode = TacOpcode::Jpc;
    jumpElse.level = 0;
    jumpElse.operand.kind = TacOperandKind::Label;
    jumpElse.operand.label = node->elseBranch != nullptr ? elseLabel : endLabel;
    program.add(jumpElse);

    emitStatement(node->thenBranch);

    if (node->elseBranch != nullptr){
        IntermediateInstruction jumpEnd;
        jumpEnd.opcode = TacOpcode::Jmp;
        jumpEnd.level = 0;
        jumpEnd.operand.kind = TacOperandKind::Label;
        jumpEnd.operand.label = endLabel;
        program.add(jumpEnd);

        IntermediateInstruction elseInstruction;
        elseInstruction.opcode = TacOpcode::Label;
        elseInstruction.operand.kind = TacOperandKind::Label;
        elseInstruction.operand.label = elseLabel;
        program.add(elseInstruction);

        emitStatement(node->elseBranch);
    }

    IntermediateInstruction endInstruction;
    endInstruction.opcode = TacOpcode::Label;
    endInstruction.operand.kind = TacOperandKind::Label;
    endInstruction.operand.label = endLabel;
    program.add(endInstruction);
}

void CodeGenerator::emitWhile(WhileNode* node) {
    string startLabel = newLabel("L_WHILE_");
    string endLabel = newLabel("L_END_");

    IntermediateInstruction startInstruction;
    startInstruction.opcode = TacOpcode::Label;
    startInstruction.operand.kind = TacOperandKind::Label;
    startInstruction.operand.label = startLabel;
    program.add(startInstruction);

    emitExpression(node->condition);

    IntermediateInstruction jumpEnd;
    jumpEnd.opcode = TacOpcode::Jpc;
    jumpEnd.level = 0;
    jumpEnd.operand.kind = TacOperandKind::Label;
    jumpEnd.operand.label = endLabel;
    program.add(jumpEnd);

    emitStatement(node->body);

    IntermediateInstruction jumpStart;
    jumpStart.opcode = TacOpcode::Jmp;
    jumpStart.level = 0;
    jumpStart.operand.kind = TacOperandKind::Label;
    jumpStart.operand.label = startLabel;
    program.add(jumpStart);

    IntermediateInstruction endInstruction;
    endInstruction.opcode = TacOpcode::Label;
    endInstruction.operand.kind = TacOperandKind::Label;
    endInstruction.operand.label = endLabel;
    program.add(endInstruction);
}

void CodeGenerator::emitFor(ForNode* node) {
    int loopAddress = addressOfName(node->variable);
    string startLabel = newLabel("L_FOR_");
    string endLabel = newLabel("L_END_");

    emitExpression(node->start);

    IntermediateInstruction storeStart;
    storeStart.opcode = TacOpcode::Sto;
    storeStart.level = 0;
    storeStart.operand.kind = TacOperandKind::Address;
    storeStart.operand.address = loopAddress;
    program.add(storeStart);

    IntermediateInstruction startInstruction;
    startInstruction.opcode = TacOpcode::Label;
    startInstruction.operand.kind = TacOperandKind::Label;
    startInstruction.operand.label = startLabel;
    program.add(startInstruction);

    IntermediateInstruction loadLoop;
    loadLoop.opcode = TacOpcode::Lod;
    loadLoop.level = 0;
    loadLoop.operand.kind = TacOperandKind::Address;
    loadLoop.operand.address = loopAddress;
    program.add(loadLoop);

    emitExpression(node->stop);
    emitOperation(node->isDownto ? TacOperation::Geq : TacOperation::Leq);

    IntermediateInstruction jumpEnd;
    jumpEnd.opcode = TacOpcode::Jpc;
    jumpEnd.level = 0;
    jumpEnd.operand.kind = TacOperandKind::Label;
    jumpEnd.operand.label = endLabel;
    program.add(jumpEnd);

    emitStatement(node->body);

    IntermediateInstruction reloadLoop;
    reloadLoop.opcode = TacOpcode::Lod;
    reloadLoop.level = 0;
    reloadLoop.operand.kind = TacOperandKind::Address;
    reloadLoop.operand.address = loopAddress;
    program.add(reloadLoop);

    IntermediateInstruction one;
    one.opcode = TacOpcode::Lit;
    one.operand.kind = TacOperandKind::Literal;
    one.operand.literal.type = TacValueType::Integer;
    one.operand.literal.text = "1";
    program.add(one);

    IntermediateInstruction step;
    step.opcode = node->isDownto ? TacOpcode::Sub : TacOpcode::Add;
    program.add(step);

    IntermediateInstruction storeStep;
    storeStep.opcode = TacOpcode::Sto;
    storeStep.level = 0;
    storeStep.operand.kind = TacOperandKind::Address;
    storeStep.operand.address = loopAddress;
    program.add(storeStep);

    IntermediateInstruction jumpStart;
    jumpStart.opcode = TacOpcode::Jmp;
    jumpStart.level = 0;
    jumpStart.operand.kind = TacOperandKind::Label;
    jumpStart.operand.label = startLabel;
    program.add(jumpStart);

    IntermediateInstruction endInstruction;
    endInstruction.opcode = TacOpcode::Label;
    endInstruction.operand.kind = TacOperandKind::Label;
    endInstruction.operand.label = endLabel;
    program.add(endInstruction);
}

void CodeGenerator::emitRepeat(RepeatNode* node) {
    string startLabel = newLabel("L_REPEAT_");

    IntermediateInstruction startInstruction;
    startInstruction.opcode = TacOpcode::Label;
    startInstruction.operand.kind = TacOperandKind::Label;
    startInstruction.operand.label = startLabel;
    program.add(startInstruction);

    for (StatementNode* statement : node->statements){
        emitStatement(statement);
    }

    emitExpression(node->condition);

    IntermediateInstruction jumpStart;
    jumpStart.opcode = TacOpcode::Jpc;
    jumpStart.level = 0;
    jumpStart.operand.kind = TacOperandKind::Label;
    jumpStart.operand.label = startLabel;
    program.add(jumpStart);
}

void CodeGenerator::emitCase(CaseNode* node) {
    string endLabel = newLabel("L_CASE_END_");

    for (CaseBranch* branch : node->branches){
        string nextLabel = newLabel("L_CASE_NEXT_");

        if (branch->labels.empty()){
            emitStatement(branch->statement);
            continue;
        }

        emitExpression(node->expression);
        emitExpression(branch->labels.front());
        emitOperation(TacOperation::Eql);

        IntermediateInstruction jumpNext;
        jumpNext.opcode = TacOpcode::Jpc;
        jumpNext.level = 0;
        jumpNext.operand.kind = TacOperandKind::Label;
        jumpNext.operand.label = nextLabel;
        program.add(jumpNext);

        emitStatement(branch->statement);

        IntermediateInstruction jumpEnd;
        jumpEnd.opcode = TacOpcode::Jmp;
        jumpEnd.level = 0;
        jumpEnd.operand.kind = TacOperandKind::Label;
        jumpEnd.operand.label = endLabel;
        program.add(jumpEnd);

        IntermediateInstruction nextInstruction;
        nextInstruction.opcode = TacOpcode::Label;
        nextInstruction.operand.kind = TacOperandKind::Label;
        nextInstruction.operand.label = nextLabel;
        program.add(nextInstruction);
    }

    IntermediateInstruction endInstruction;
    endInstruction.opcode = TacOpcode::Label;
    endInstruction.operand.kind = TacOperandKind::Label;
    endInstruction.operand.label = endLabel;
    program.add(endInstruction);
}

void CodeGenerator::emitCall(CallNode* node) {
    if (isWriteCall(node->name)){
        emitWriteCall(node);
        return;
    }

    for (ExpressionNode* argument : node->arguments){
        emitExpression(argument);
    }

    IntermediateInstruction instruction;
    instruction.opcode = TacOpcode::Cal;
    instruction.level = 0;
    instruction.operand.kind = TacOperandKind::Label;
    instruction.operand.label = "P_" + to_string(node->tabIndex);
    program.add(instruction);
}

void CodeGenerator::emitWriteCall(CallNode* node) {
    for (size_t i = 0; i < node->arguments.size(); ++i){
        emitExpression(node->arguments[i]);
        bool lastArgument = i + 1 == node->arguments.size();
        bool newline = symbols != nullptr && symbols->toUpper(node->name) == "WRITELN" && lastArgument;
        emitOperation(newline ? TacOperation::Wrtln : TacOperation::Wrt);
    }

    if (node->arguments.empty() && symbols != nullptr && symbols->toUpper(node->name) == "WRITELN"){
        emitOperation(TacOperation::Wrtln);
    }
}

void CodeGenerator::emitBinary(BinOpNode* node) {
    emitExpression(node->left);
    emitExpression(node->right);

    string op = node->op;
    if (symbols != nullptr) op = symbols->toUpper(op);

    if (op == "+") {
        IntermediateInstruction instruction;
        instruction.opcode = TacOpcode::Add;
        program.add(instruction);
    } else if (op == "-") {
        IntermediateInstruction instruction;
        instruction.opcode = TacOpcode::Sub;
        program.add(instruction);
    } else if (op == "*") {
        IntermediateInstruction instruction;
        instruction.opcode = TacOpcode::Mul;
        program.add(instruction);
    } else if (op == "/" || op == "DIV") {
        IntermediateInstruction instruction;
        instruction.opcode = TacOpcode::Div;
        program.add(instruction);
    } else if (op == "MOD") {
        emitOperation(TacOperation::Mod);
    } else if (op == "==" || op == "=") {
        emitOperation(TacOperation::Eql);
    } else if (op == "<>") {
        emitOperation(TacOperation::Neq);
    } else if (op == "<") {
        emitOperation(TacOperation::Lss);
    } else if (op == ">=") {
        emitOperation(TacOperation::Geq);
    } else if (op == ">") {
        emitOperation(TacOperation::Gtr);
    } else if (op == "<=") {
        emitOperation(TacOperation::Leq);
    } else if (op == "AND") {
        IntermediateInstruction instruction;
        instruction.opcode = TacOpcode::Mul;
        program.add(instruction);
    } else if (op == "OR") {
        IntermediateInstruction instruction;
        instruction.opcode = TacOpcode::Add;
        program.add(instruction);
    }
}

void CodeGenerator::emitUnary(UnaryOpNode* node) {
    string op = node->op;
    if (symbols != nullptr) op = symbols->toUpper(op);

    emitExpression(node->operand);

    if (op == "-") {
        emitOperation(TacOperation::Neg);
    } else if (op == "NOT") {
        IntermediateInstruction zero;
        zero.opcode = TacOpcode::Lit;
        zero.operand.kind = TacOperandKind::Literal;
        zero.operand.literal.type = TacValueType::Integer;
        zero.operand.literal.text = "0";
        program.add(zero);
        emitOperation(TacOperation::Eql);
    }
}

void CodeGenerator::emitCallExpr(ProcedureFunctionCallNode* node) {
    for (ExpressionNode* argument : node->arguments){
        emitExpression(argument);
    }

    IntermediateInstruction instruction;
    instruction.opcode = TacOpcode::Cal;
    instruction.level = 0;
    instruction.operand.kind = TacOperandKind::Label;
    instruction.operand.label = "F_" + to_string(node->tabIndex);
    program.add(instruction);
}

bool CodeGenerator::isWriteCall(const string& name) const {
    if (symbols == nullptr) return false;
    string upperName = symbols->toUpper(name);
    return upperName == "WRITE" || upperName == "WRITELN";
}
