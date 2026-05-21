#include "Semantic.hpp"

#include <cstdlib>

void Semantic::addError(const string& message) {
    errors.push_back("Semantic error:" + message);
}

bool Semantic::hasErrors() const {
    return !errors.empty();
}

const vector<string>& Semantic::getErrors() const {
    return errors;
}

SymbolTable& Semantic::getSymbolTable() {
    return symbolTable;
}

int Semantic::lookupName(const string& name) {
    int idx = symbolTable.lookup(name);
    if (idx == 0) idx = symbolTable.lookup(symbolTable.toUpper(name));
    return idx;
}

bool Semantic::isRelationalOp(const string& op) const
{
    return op == "==" || op == "<>" || op == "<" || op == ">" || op == "<=" || op == ">=";
}

void Semantic::decorate(ASTNode* node, int tabIndex) {
    if (node == nullptr || tabIndex == 0) return;

    TabEntry* entry = symbolTable.getTab(tabIndex);
    if (entry == nullptr) return;

    node->tabIndex = tabIndex;
    node->evalType = entry->type;
    node->level = entry->lev;

    if (entry->type == TypeClass::Array) node->atabIndex = entry->ref;
    if (entry->type == TypeClass::Record) node->btabIndex = entry->ref;
}

void Semantic::analyze(ProgramNode* root) {
    errors.clear();

    if (root == nullptr) {
        addError(to_string(root->line) + ":" + to_string(root->column) + ":" + " empty AST");
        return;
    }

    int idx = symbolTable.insertTab(root->name, ObjClass::Program, TypeClass::None, 0, 1, 0);
    if (idx == 0) addError(to_string(root->line) + ":" + to_string(root->column) + ":" + " program redeclared: " + root->name);
    decorate(root, idx);

    for (DeclarationNode* decl : root->declarations) analyzeDeclaration(decl);

    int blockIdx = symbolTable.enterBlock();
    if (root->body != nullptr) {
        root->body->btabIndex = blockIdx;
        root->body->level = root->level + 1;
    }
    analyzeStatement(root->body);
    symbolTable.exitBlock();
}

void Semantic::analyzeDeclaration(DeclarationNode* node) {
    if (node == nullptr) return;

    if (auto varDecl = dynamic_cast<VarDeclNode*>(node)) analyzeVarDecl(varDecl);
    else if (auto constDecl = dynamic_cast<ConstDeclNode*>(node)) analyzeConstDecl(constDecl);
    else if (auto typeDecl = dynamic_cast<TypeDeclNode*>(node)) analyzeTypeDecl(typeDecl);
    else if (auto procDecl = dynamic_cast<ProcedureDeclNode*>(node)) analyzeProcedureDecl(procDecl);
    else if (auto funcDecl = dynamic_cast<FunctionDeclNode*>(node)) analyzeFunctionDecl(funcDecl);
}

void Semantic::analyzeVarDecl(VarDeclNode* node) {
    int ref = 0;
    TypeClass type = resolveType(node->type, ref);
    node->evalType = type;
    node->tabIndices.clear();

    for (const string& name : node->names) {
        int nrm = node->isVarParameter ? 0 : 1;
        int idx = symbolTable.insertTab(name, ObjClass::Variable, type, ref, nrm, 0);
        node->tabIndices.push_back(idx);

        if (idx == 0) {
            addError(to_string(node->line) + ":" + to_string(node->column) + ":" + " redeclaration of variable: " + name);
        } else {
            decorate(node, idx);
        }
    }
}

void Semantic::analyzeConstDecl(ConstDeclNode* node) {
    TypeClass type = analyzeExpression(node->value);
    node->evalType = type;

    int idx = symbolTable.insertTab(
        node->name,
        ObjClass::Constant,
        type,
        0,
        1,
        constantAddress(node->value)
    );

    if (idx == 0) addError(to_string(node->line) + ":" + to_string(node->column) + ":" + " redeclaration of constant: " + node->name);
    else decorate(node, idx);
}

void Semantic::analyzeTypeDecl(TypeDeclNode* node) {
    int ref = 0;
    TypeClass type = resolveType(node->type, ref);
    node->evalType = type;

    int idx = symbolTable.insertTab(node->name, ObjClass::Type, type, ref, 1, 0);
    if (idx == 0) addError(to_string(node->line) + ":" + to_string(node->column) + ":" + " redeclaration of type: " + node->name);
    else decorate(node, idx);
}

void Semantic::analyzeProcedureDecl(ProcedureDeclNode* node) {
    int idx = symbolTable.insertTab(node->name, ObjClass::Procedure, TypeClass::None, 0, 1, 0);
    if (idx == 0) addError(to_string(node->line) + ":" + to_string(node->column) + ":" + " redeclaration of procedure: " + node->name);

    int blockIdx = symbolTable.enterBlock();
    node->btabIndex = blockIdx;
    decorate(node, idx);

    TabEntry* entry = symbolTable.getTab(idx);
    if (entry != nullptr) entry->ref = blockIdx;

    for (VarDeclNode* param : node->parameters) {
        param->isParameter = true;
        analyzeVarDecl(param);
    }

    for (DeclarationNode* decl : node->declarations) analyzeDeclaration(decl);
    if (node->body != nullptr) {
        node->body->btabIndex = blockIdx;
        node->body->level = node->level + 1;
    }
    analyzeStatement(node->body);

    symbolTable.exitBlock();
}

void Semantic::analyzeFunctionDecl(FunctionDeclNode* node) {
    int ref = 0;
    TypeClass returnType = resolveType(node->returnType, ref);

    int idx = symbolTable.insertTab(node->name, ObjClass::Function, returnType, ref, 1, 0);
    if (idx == 0) addError(to_string(node->line) + ":" + to_string(node->column) + ":" + " redeclaration of function: " + node->name);

    int blockIdx = symbolTable.enterBlock();
    node->btabIndex = blockIdx;
    decorate(node, idx);

    TabEntry* entry = symbolTable.getTab(idx);
    if (entry != nullptr) entry->ref = blockIdx;

    for (VarDeclNode* param : node->parameters) {
        param->isParameter = true;
        analyzeVarDecl(param);
    }

    for (DeclarationNode* decl : node->declarations) analyzeDeclaration(decl);
    if (node->body != nullptr) {
        node->body->btabIndex = blockIdx;
        node->body->level = node->level + 1;
    }
    analyzeStatement(node->body);

    symbolTable.exitBlock();
}

void Semantic::analyzeStatement(StatementNode* node) {
    if (node == nullptr) return;

    if (auto compound = dynamic_cast<CompoundNode*>(node)) {
        for (StatementNode* stmt : compound->statements) analyzeStatement(stmt);
    }
    else if (auto assign = dynamic_cast<AssignNode*>(node)) {
        TypeClass target = analyzeExpression(assign->target);
        TypeClass value = analyzeExpression(assign->value);

        if (!isAssignable(target, value)) {
            addError(to_string(node->line) + ":" + to_string(node->column) + ":" + " cannot assign " + typeName(value) + " to " + typeName(target));
        }
    }
    else if (auto ifNode = dynamic_cast<IfNode*>(node)) {
        if (analyzeExpression(ifNode->condition) != TypeClass::Boolean)
            addError(to_string(node->line) + ":" + to_string(node->column) + ":" + " if condition must be boolean");

        analyzeStatement(ifNode->thenBranch);
        analyzeStatement(ifNode->elseBranch);
    }
    else if (auto whileNode = dynamic_cast<WhileNode*>(node)) {
        if (analyzeExpression(whileNode->condition) != TypeClass::Boolean)
            addError(to_string(node->line) + ":" + to_string(node->column) + ":" + " while condition must be boolean");

        analyzeStatement(whileNode->body);
    }
    else if (auto forNode = dynamic_cast<ForNode*>(node)) {
        int idx = lookupName(forNode->variable);
        if (idx == 0) addError(to_string(node->line) + ":" + to_string(node->column) + ":" + " undeclared loop variable: " + forNode->variable);

        TypeClass startType = analyzeExpression(forNode->start);
        TypeClass stopType = analyzeExpression(forNode->stop);

        if (startType != TypeClass::Integer || stopType != TypeClass::Integer)
            addError(to_string(node->line) + ":" + to_string(node->column) + ":" + " for bounds must be integer");

        analyzeStatement(forNode->body);
    }
    else if (auto repeatNode = dynamic_cast<RepeatNode*>(node)) {
        for (StatementNode* stmt : repeatNode->statements) analyzeStatement(stmt);

        if (analyzeExpression(repeatNode->condition) != TypeClass::Boolean)
            addError(to_string(node->line) + ":" + to_string(node->column) + ":" + " repeat-until condition must be boolean");
    }
    else if (auto caseNode = dynamic_cast<CaseNode*>(node)) {
        TypeClass selectorType = analyzeExpression(caseNode->expression);

        for (CaseBranch* branch : caseNode->branches) {
            for (ExpressionNode* label : branch->labels) {
                TypeClass labelType = analyzeExpression(label);
                if (!isCompatible(selectorType, labelType))
                    addError(to_string(node->line) + ":" + to_string(node->column) + ":" + " case label type mismatch");
            }
            analyzeStatement(branch->statement);
        }
    }
    else if (auto call = dynamic_cast<CallNode*>(node)) {
        int idx = lookupName(call->name);
        if (idx == 0) addError(to_string(node->line) + ":" + to_string(node->column) + ":" + " undeclared procedure/function: " + call->name);
        else decorate(call, idx);

        for (ExpressionNode* arg : call->arguments) analyzeExpression(arg);
    }
}

TypeClass Semantic::analyzeExpression(ExpressionNode* node) {
    if (node == nullptr) return TypeClass::None;

    if (auto var = dynamic_cast<VarNode*>(node)) {
        int idx = lookupName(var->name);
        if (idx == 0) {
            addError(to_string(node->line) + ":" + to_string(node->column) + ":" + " undeclared identifier: " + var->name);
            return TypeClass::None;
        }

        decorate(var, idx);
        return var->evalType;
    }

    if (auto call = dynamic_cast<FunctionCallNode*>(node)) {
        int idx = lookupName(call->name);
        if (idx == 0) {
            addError(to_string(node->line) + ":" + to_string(node->column) + ":" + " undeclared function: " + call->name);
            return TypeClass::None;
        }

        decorate(call, idx);
        for (ExpressionNode* arg : call->arguments) analyzeExpression(arg);
        return call->evalType;
    }

    if (auto number = dynamic_cast<NumberNode*>(node)) return number->evalType;
    if (auto str = dynamic_cast<StringNode*>(node)) return str->evalType;
    if (auto chr = dynamic_cast<CharNode*>(node)) return chr->evalType;
    if (auto boolean = dynamic_cast<BoolNode*>(node)) return boolean->evalType;

    if (auto unary = dynamic_cast<UnaryOpNode*>(node)) {
        TypeClass operand = analyzeExpression(unary->operand);

        if (unary->op == "not") {
            if (operand != TypeClass::Boolean) addError(to_string(node->line) + ":" + to_string(node->column) + ":" + " not operand must be boolean");
            unary->evalType = TypeClass::Boolean;
        } else {
            if (!isNumeric(operand)) addError(to_string(node->line) + ":" + to_string(node->column) + ":" + " unary +/- operand must be numeric");
            unary->evalType = operand;
        }

        return unary->evalType;
    }

    if (auto bin = dynamic_cast<BinOpNode*>(node)) {
        TypeClass left = analyzeExpression(bin->left);
        TypeClass right = analyzeExpression(bin->right);

        if (isRelationalOp(bin->op)) {
            if (!isCompatible(left, right)) addError(to_string(node->line) + ":" + to_string(node->column) + ":" + " relational operand type mismatch");
            bin->evalType = TypeClass::Boolean;
        }
        else if (bin->op == "AND" || bin->op == "OR") {
            if (left != TypeClass::Boolean || right != TypeClass::Boolean)
                addError(to_string(node->line) + ":" + to_string(node->column) + ":" + " logical operands must be boolean");
            bin->evalType = TypeClass::Boolean;
        }
        else {
            if (!isNumeric(left) || !isNumeric(right))
                addError(to_string(node->line) + ":" + to_string(node->column) + ":" + " arithmetic operands must be numeric");

            if (bin->op == "/" || left == TypeClass::Real || right == TypeClass::Real)
                bin->evalType = TypeClass::Real;
            else
                bin->evalType = TypeClass::Integer;
        }

        return bin->evalType;
    }

    if (auto access = dynamic_cast<ArrayAccessNode*>(node)) {
        TypeClass arrayType = analyzeExpression(access->array);
        TypeClass indexType = analyzeExpression(access->index);

        if (arrayType != TypeClass::Array)
            addError(to_string(node->line) + ":" + to_string(node->column) + ":" + " indexed value is not array");

        if (indexType == TypeClass::Real || indexType == TypeClass::String)
            addError(to_string(node->line) + ":" + to_string(node->column) + ":" + " array index must be simple non-real type");

        if (access->array != nullptr && access->array->atabIndex != -1) {
            AtabEntry* entry = symbolTable.getAtab(access->array->atabIndex);
            if (entry != nullptr) access->evalType = static_cast<TypeClass>(entry->etype);
        }

        return access->evalType;
    }

    if (auto access = dynamic_cast<RecordAccessNode*>(node)) {
        TypeClass recordType = analyzeExpression(access->record);
        if (recordType != TypeClass::Record)
            addError(to_string(node->line) + ":" + to_string(node->column) + ":" + " field access target is not record");

        return access->evalType;
    }

    return TypeClass::None;
}

TypeClass Semantic::resolveType(TypeNode* node, int& ref) {
    ref = 0;
    if (node == nullptr) return TypeClass::None;

    if (auto named = dynamic_cast<NamedTypeNode*>(node)) {
        int idx = lookupName(named->name);
        if (idx == 0) {
            addError(to_string(node->line) + ":" + to_string(node->column) + ":" + " unknown type: " + named->name);
            return TypeClass::None;
        }

        decorate(named, idx);
        TabEntry* entry = symbolTable.getTab(idx);
        if (entry != nullptr) ref = entry->ref;

        return named->evalType;
    }

    if (auto range = dynamic_cast<RangeTypeNode*>(node)) {
        TypeClass low = analyzeExpression(range->low);
        TypeClass high = analyzeExpression(range->high);

        if (low != high) addError(to_string(node->line) + ":" + to_string(node->column) + ":" + " range bounds must have same type");
        if (low == TypeClass::Real) addError(to_string(node->line) + ":" + to_string(node->column) + ":" + " range bounds cannot be real");

        range->evalType = TypeClass::Subrange;
        return TypeClass::Subrange;
    }

    if (auto enumerated = dynamic_cast<EnumeratedTypeNode*>(node)) {
        for (const string& value : enumerated->values) {
            int idx = symbolTable.insertTab(value, ObjClass::Constant, TypeClass::Enumerated, 0, 1, 0);
            if (idx == 0) addError(to_string(node->line) + ":" + to_string(node->column) + ":" + " redeclaration of enumerated value: " + value);
        }

        enumerated->evalType = TypeClass::Enumerated;
        return TypeClass::Enumerated;
    }

    if (auto array = dynamic_cast<ArrayTypeNode*>(node)) {
        int indexRef = 0;
        int elementRef = 0;

        TypeClass indexType = resolveType(array->indexType, indexRef);
        TypeClass elementType = resolveType(array->elementType, elementRef);

        int atabIdx = symbolTable.insertAtab(
            static_cast<int>(indexType),
            static_cast<int>(elementType),
            elementRef,
            0,
            0,
            1
        );

        array->evalType = TypeClass::Array;
        array->atabIndex = atabIdx;
        ref = atabIdx;

        return TypeClass::Array;
    }

    if (auto record = dynamic_cast<RecordTypeNode*>(node)) {
        int blockIdx = symbolTable.enterBlock();
        record->btabIndex = blockIdx;

        for (VarDeclNode* field : record->fields) analyzeVarDecl(field);

        symbolTable.exitBlock();

        record->evalType = TypeClass::Record;
        ref = blockIdx;
        return TypeClass::Record;
    }

    return TypeClass::None;
}

bool Semantic::isNumeric(TypeClass type) const {
    return type == TypeClass::Integer || type == TypeClass::Real;
}

bool Semantic::isCompatible(TypeClass left, TypeClass right) const {
    if (left == right) return true;
    if (left == TypeClass::Subrange && right == TypeClass::Integer) return true;
    if (left == TypeClass::Integer && right == TypeClass::Subrange) return true;
    return false;
}

bool Semantic::isAssignable(TypeClass target, TypeClass value) const {
    if (target == value) return true;
    if (target == TypeClass::Real && value == TypeClass::Integer) return true;
    if (target == TypeClass::Subrange && value == TypeClass::Integer) return true;
    return false;
}

int Semantic::constantAddress(ExpressionNode* node) const {
    if (auto number = dynamic_cast<NumberNode*>(node)) return atoi(number->value.c_str());
    if (auto boolean = dynamic_cast<BoolNode*>(node)) return boolean->value ? 1 : 0;
    if (auto chr = dynamic_cast<CharNode*>(node)) return chr->value.empty() ? 0 : chr->value[0];
    return 0;
}

string Semantic::typeName(TypeClass type) const {
    switch (type) {
        case TypeClass::Integer: return "integer";
        case TypeClass::Real: return "real";
        case TypeClass::Char: return "char";
        case TypeClass::Boolean: return "boolean";
        case TypeClass::String: return "string";
        case TypeClass::Array: return "array";
        case TypeClass::Record: return "record";
        case TypeClass::Subrange: return "subrange";
        case TypeClass::Enumerated: return "enumerated";
        default: return "none";
    }
}
