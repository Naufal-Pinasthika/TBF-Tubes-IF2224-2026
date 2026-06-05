#include "ASTPrinter.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

namespace
{
struct AstDumpAnnotation
{
    TypeClass type = TypeClass::None;
    int tabIndex = -1;
    int btabIndex = -1;
    int atabIndex = -1;
    int level = -1;
};

struct AstDumpLabel
{
    string text;
    AstDumpAnnotation annotation;
};

struct AstDumpTreeLine
{
    int depth = 0;
    string label;
};

struct AstDumpNode
{
    string label;
    vector<AstDumpNode*> children;

    ~AstDumpNode()
    {
        for (AstDumpNode* child : children) delete child;
    }
};

string trimAstText(const string& text)
{
    size_t start = text.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";

    size_t end = text.find_last_not_of(" \t\r\n");
    return text.substr(start, end - start + 1);
}

string lowerAstText(string text)
{
    transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(tolower(ch));
    });
    return text;
}

bool startsWithAstText(const string& text, const string& prefix)
{
    return text.compare(0, prefix.size(), prefix) == 0;
}

bool consumeAstPrefix(const string& text, size_t& pos, const string& prefix)
{
    if (text.compare(pos, prefix.size(), prefix) != 0) return false;
    pos += prefix.size();
    return true;
}

vector<string> splitAstText(const string& text, char separator)
{
    vector<string> result;
    string item;
    stringstream stream(text);
    while (getline(stream, item, separator)) {
        string trimmed = trimAstText(item);
        if (!trimmed.empty()) result.push_back(trimmed);
    }
    return result;
}

TypeClass typeClassFromAstText(const string& value)
{
    string lowered = lowerAstText(trimAstText(value));
    if (lowered == "integer") return TypeClass::Integer;
    if (lowered == "real") return TypeClass::Real;
    if (lowered == "char") return TypeClass::Char;
    if (lowered == "boolean") return TypeClass::Boolean;
    if (lowered == "string") return TypeClass::String;
    if (lowered == "array") return TypeClass::Array;
    if (lowered == "record") return TypeClass::Record;
    if (lowered == "subrange") return TypeClass::Subrange;
    if (lowered == "enumerated") return TypeClass::Enumerated;
    return TypeClass::None;
}

string typeNameFromClass(TypeClass type)
{
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
    case TypeClass::None:
    default: return "none";
    }
}

AstDumpLabel parseAstDumpLabel(const string& label)
{
    AstDumpLabel parsed;
    size_t arrow = label.rfind(" -> ");
    parsed.text = arrow == string::npos ? trimAstText(label) : trimAstText(label.substr(0, arrow));

    if (arrow == string::npos) return parsed;

    string annotationText = trimAstText(label.substr(arrow + 4));
    for (const string& part : splitAstText(annotationText, ',')) {
        if (part == "predefined") continue;

        size_t colon = part.find(':');
        if (colon == string::npos) continue;

        string key = trimAstText(part.substr(0, colon));
        string value = trimAstText(part.substr(colon + 1));

        if (key == "type") parsed.annotation.type = typeClassFromAstText(value);
        else if (key == "tab_index") parsed.annotation.tabIndex = atoi(value.c_str());
        else if (key == "block_index") parsed.annotation.btabIndex = atoi(value.c_str());
        else if (key == "array_index") parsed.annotation.atabIndex = atoi(value.c_str());
        else if (key == "lev") parsed.annotation.level = atoi(value.c_str());
    }

    return parsed;
}

void applyAstAnnotation(ASTNode* node, const AstDumpAnnotation& annotation, SymbolTable* symbols)
{
    if (node == nullptr) return;

    if (annotation.type != TypeClass::None) node->evalType = annotation.type;
    if (annotation.tabIndex != -1) node->tabIndex = annotation.tabIndex;
    if (annotation.btabIndex != -1) node->btabIndex = annotation.btabIndex;
    if (annotation.atabIndex != -1) node->atabIndex = annotation.atabIndex;
    if (annotation.level != -1) node->level = annotation.level;

    if (symbols != nullptr && annotation.tabIndex != -1) {
        TabEntry* entry = symbols->getTab(annotation.tabIndex);
        if (entry != nullptr) {
            if (node->evalType == TypeClass::None) node->evalType = entry->type;
            if (node->level == -1) node->level = entry->lev;
            if (entry->type == TypeClass::Array && node->atabIndex == -1) node->atabIndex = entry->ref;
            if (entry->type == TypeClass::Record && node->btabIndex == -1) node->btabIndex = entry->ref;
        }
    }
}

string upperAstText(string text)
{
    transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(toupper(ch));
    });
    return text;
}

int findSymbolByName(SymbolTable* symbols, const string& name, int preferredLevel = -1)
{
    if (symbols == nullptr || name.empty()) return 0;

    string target = upperAstText(name);
    int fallback = 0;
    for (int index = 0; symbols->getTab(index) != nullptr; ++index) {
        TabEntry* entry = symbols->getTab(index);
        if (entry != nullptr && upperAstText(entry->identifiers) == target) {
            fallback = index;
            if (preferredLevel != -1 && entry->lev == preferredLevel) return index;
        }
    }

    return fallback;
}

void decorateNodeFromSymbol(ASTNode* node, SymbolTable* symbols, int tabIndex)
{
    if (node == nullptr || symbols == nullptr || tabIndex <= 0) return;

    TabEntry* entry = symbols->getTab(tabIndex);
    if (entry == nullptr) return;

    node->tabIndex = tabIndex;
    node->evalType = entry->type;
    node->level = entry->lev;
    if (entry->type == TypeClass::Array) node->atabIndex = entry->ref;
    if (entry->type == TypeClass::Record) node->btabIndex = entry->ref;
}

string stripExpressionRole(const string& text)
{
    vector<string> roles = {"target ", "value ", "condition ", "argument ", "selector ", "start ", "stop ", "until "};
    for (const string& role : roles) {
        if (startsWithAstText(text, role)) return trimAstText(text.substr(role.size()));
    }
    return text;
}

bool extractFirstQuoted(const string& text, string& value, size_t* endPos = nullptr)
{
    size_t first = text.find('\'');
    if (first == string::npos) return false;
    size_t second = text.find('\'', first + 1);
    if (second == string::npos) return false;

    value = text.substr(first + 1, second - first - 1);
    if (endPos != nullptr) *endPos = second + 1;
    return true;
}

string contentInOuterParentheses(const string& text)
{
    size_t open = text.find('(');
    size_t close = text.rfind(')');
    if (open == string::npos || close == string::npos || close <= open) return "";
    return text.substr(open + 1, close - open - 1);
}

AstDumpTreeLine parseAstTreeLine(const string& line)
{
    AstDumpTreeLine parsed;
    if (trimAstText(line).empty()) return parsed;

    size_t pos = 0;
    while (consumeAstPrefix(line, pos, "│   ") ||
           consumeAstPrefix(line, pos, "|   ") ||
           consumeAstPrefix(line, pos, "    ")) {
        parsed.depth++;
    }

    if (consumeAstPrefix(line, pos, "├── ") ||
        consumeAstPrefix(line, pos, "└── ") ||
        consumeAstPrefix(line, pos, "+-- ") ||
        consumeAstPrefix(line, pos, "`-- ")) {
        parsed.depth++;
    }

    parsed.label = trimAstText(line.substr(pos));
    return parsed;
}

AstDumpNode* parseAstTree(const vector<string>& lines, size_t startLine)
{
    AstDumpNode* root = nullptr;
    vector<pair<int, AstDumpNode*>> nodeStack;

    for (size_t i = startLine; i < lines.size(); ++i) {
        AstDumpTreeLine treeLine = parseAstTreeLine(lines[i]);
        if (treeLine.label.empty()) continue;

        AstDumpNode* node = new AstDumpNode;
        node->label = treeLine.label;

        while (!nodeStack.empty() && nodeStack.back().first >= treeLine.depth) {
            nodeStack.pop_back();
        }

        if (nodeStack.empty()) {
            if (root != nullptr) delete root;
            root = node;
        } else {
            nodeStack.back().second->children.push_back(node);
        }

        nodeStack.push_back({treeLine.depth, node});
    }

    return root;
}

ExpressionNode* buildAstExpression(const AstDumpNode* node, SymbolTable* symbols);
StatementNode* buildAstStatement(const AstDumpNode* node, SymbolTable* symbols);
DeclarationNode* buildAstDeclaration(const AstDumpNode* node, SymbolTable* symbols);
TypeNode* buildAstType(const AstDumpNode* node, SymbolTable* symbols);

ExpressionNode* buildScalarAstExpression(const string& text, const AstDumpAnnotation& annotation, SymbolTable* symbols)
{
    string value;
    size_t quotedEnd = 0;
    if (extractFirstQuoted(text, value, &quotedEnd)) {
        string suffix = trimAstText(text.substr(quotedEnd));
        if (startsWithAstText(suffix, "[")) {
            size_t close = suffix.rfind(']');
            string indexText = close == string::npos ? "" : suffix.substr(1, close - 1);

            VarNode* array = new VarNode(value);
            decorateNodeFromSymbol(array, symbols, findSymbolByName(symbols, value));

            AstDumpNode indexNode;
            indexNode.label = trimAstText(indexText);
            ExpressionNode* index = buildAstExpression(&indexNode, symbols);

            ArrayAccessNode* access = new ArrayAccessNode(array, index);
            applyAstAnnotation(access, annotation, symbols);
            return access;
        }

        if (startsWithAstText(suffix, ".")) {
            string field = trimAstText(suffix.substr(1));

            VarNode* record = new VarNode(value);
            decorateNodeFromSymbol(record, symbols, findSymbolByName(symbols, value));

            RecordAccessNode* access = new RecordAccessNode(record, field);
            applyAstAnnotation(access, annotation, symbols);
            return access;
        }

        int symbolIndex = annotation.tabIndex != -1 ? annotation.tabIndex : findSymbolByName(symbols, value);
        if (symbolIndex > 0 && annotation.type == TypeClass::None) {
            VarNode* variable = new VarNode(value);
            decorateNodeFromSymbol(variable, symbols, symbolIndex);
            return variable;
        }

        if (annotation.tabIndex != -1) {
            VarNode* variable = new VarNode(value);
            applyAstAnnotation(variable, annotation, symbols);
            return variable;
        }

        if (annotation.type == TypeClass::Char || (annotation.type == TypeClass::None && value.size() == 1)) {
            CharNode* character = new CharNode(value);
            applyAstAnnotation(character, annotation, symbols);
            return character;
        }

        if (annotation.type == TypeClass::Boolean) {
            string lowered = lowerAstText(value);
            BoolNode* boolean = new BoolNode(lowered == "true");
            applyAstAnnotation(boolean, annotation, symbols);
            return boolean;
        }

        StringNode* stringNode = new StringNode(value);
        applyAstAnnotation(stringNode, annotation, symbols);
        return stringNode;
    }

    string lowered = lowerAstText(text);
    if (lowered == "true" || lowered == "false") {
        BoolNode* boolean = new BoolNode(lowered == "true");
        applyAstAnnotation(boolean, annotation, symbols);
        return boolean;
    }

    bool numeric = !text.empty();
    bool hasDigit = false;
    bool hasDot = false;
    for (size_t i = 0; i < text.size(); ++i) {
        unsigned char ch = static_cast<unsigned char>(text[i]);
        if (isdigit(ch)) {
            hasDigit = true;
        } else if (text[i] == '.') {
            hasDot = true;
        } else if ((text[i] == '+' || text[i] == '-') && i == 0) {
            continue;
        } else {
            numeric = false;
            break;
        }
    }

    if (numeric && hasDigit) {
        NumberNode* number = new NumberNode(text, hasDot || annotation.type == TypeClass::Real);
        applyAstAnnotation(number, annotation, symbols);
        return number;
    }

    VarNode* variable = new VarNode(text);
    if (annotation.tabIndex != -1) applyAstAnnotation(variable, annotation, symbols);
    else decorateNodeFromSymbol(variable, symbols, findSymbolByName(symbols, text, annotation.level));
    return variable;
}

ExpressionNode* buildAstExpression(const AstDumpNode* node, SymbolTable* symbols)
{
    if (node == nullptr) return nullptr;

    AstDumpLabel parsed = parseAstDumpLabel(node->label);
    string text = stripExpressionRole(parsed.text);

    if (startsWithAstText(text, "BinOp '")) {
        string op;
        extractFirstQuoted(text, op);

        ExpressionNode* left = node->children.size() > 0 ? buildAstExpression(node->children[0], symbols) : nullptr;
        ExpressionNode* right = node->children.size() > 1 ? buildAstExpression(node->children[1], symbols) : nullptr;
        BinOpNode* binOp = new BinOpNode(op, left, right);
        applyAstAnnotation(binOp, parsed.annotation, symbols);
        return binOp;
    }

    if (startsWithAstText(text, "UnaryOp '")) {
        string op;
        extractFirstQuoted(text, op);
        ExpressionNode* operand = node->children.empty() ? nullptr : buildAstExpression(node->children[0], symbols);
        UnaryOpNode* unaryOp = new UnaryOpNode(op, operand);
        applyAstAnnotation(unaryOp, parsed.annotation, symbols);
        return unaryOp;
    }

    if (startsWithAstText(text, "ArrayAccess(")) {
        ExpressionNode* array = node->children.size() > 0 ? buildAstExpression(node->children[0], symbols) : nullptr;
        ExpressionNode* index = node->children.size() > 1 ? buildAstExpression(node->children[1], symbols) : nullptr;

        if (array == nullptr || index == nullptr) {
            AstDumpNode summaryNode;
            summaryNode.label = contentInOuterParentheses(text);
            delete array;
            delete index;
            return buildScalarAstExpression(summaryNode.label, parsed.annotation, symbols);
        }

        ArrayAccessNode* access = new ArrayAccessNode(array, index);
        applyAstAnnotation(access, parsed.annotation, symbols);
        return access;
    }

    if (startsWithAstText(text, "RecordAccess(")) {
        string summary = contentInOuterParentheses(text);
        string field;
        size_t dot = summary.rfind('.');
        if (dot != string::npos) field = trimAstText(summary.substr(dot + 1));

        ExpressionNode* record = node->children.empty() ? nullptr : buildAstExpression(node->children[0], symbols);
        if (record == nullptr) return buildScalarAstExpression(summary, parsed.annotation, symbols);

        RecordAccessNode* access = new RecordAccessNode(record, field);
        applyAstAnnotation(access, parsed.annotation, symbols);
        return access;
    }

    if (text.size() >= 5 && text.find("(...)") != string::npos) {
        string name = trimAstText(text.substr(0, text.find("(...)")));
        vector<ExpressionNode*> arguments;
        for (const AstDumpNode* child : node->children) {
            arguments.push_back(buildAstExpression(child, symbols));
        }

        ProcedureFunctionCallNode* call = new ProcedureFunctionCallNode(name, arguments);
        applyAstAnnotation(call, parsed.annotation, symbols);
        return call;
    }

    return buildScalarAstExpression(text, parsed.annotation, symbols);
}

TypeNode* buildAstType(const AstDumpNode* node, SymbolTable* symbols)
{
    if (node == nullptr) return nullptr;

    AstDumpLabel parsed = parseAstDumpLabel(node->label);
    string text = parsed.text;

    if (startsWithAstText(text, "NamedType(")) {
        string name;
        extractFirstQuoted(text, name);
        NamedTypeNode* type = new NamedTypeNode(name);
        applyAstAnnotation(type, parsed.annotation, symbols);
        return type;
    }

    if (startsWithAstText(text, "RangeType(")) {
        string content = contentInOuterParentheses(text);
        size_t dots = content.find("..");
        AstDumpNode lowNode;
        AstDumpNode highNode;
        lowNode.label = dots == string::npos ? content : trimAstText(content.substr(0, dots));
        highNode.label = dots == string::npos ? content : trimAstText(content.substr(dots + 2));

        RangeTypeNode* type = new RangeTypeNode(
            buildAstExpression(&lowNode, symbols),
            buildAstExpression(&highNode, symbols)
        );
        applyAstAnnotation(type, parsed.annotation, symbols);
        return type;
    }

    if (startsWithAstText(text, "EnumeratedType(")) {
        vector<string> values = splitAstText(contentInOuterParentheses(text), ',');
        EnumeratedTypeNode* type = new EnumeratedTypeNode(values);
        applyAstAnnotation(type, parsed.annotation, symbols);
        return type;
    }

    if (startsWithAstText(text, "ArrayType")) {
        TypeNode* indexType = nullptr;
        TypeNode* elementType = nullptr;
        for (const AstDumpNode* child : node->children) {
            AstDumpLabel childLabel = parseAstDumpLabel(child->label);
            if (childLabel.text == "IndexType" && !child->children.empty()) {
                indexType = buildAstType(child->children[0], symbols);
            } else if (childLabel.text == "ElementType" && !child->children.empty()) {
                elementType = buildAstType(child->children[0], symbols);
            }
        }

        ArrayTypeNode* type = new ArrayTypeNode(indexType, elementType);
        applyAstAnnotation(type, parsed.annotation, symbols);
        return type;
    }

    if (startsWithAstText(text, "RecordType")) {
        RecordTypeNode* type = new RecordTypeNode;
        for (const AstDumpNode* child : node->children) {
            if (VarDeclNode* field = dynamic_cast<VarDeclNode*>(buildAstDeclaration(child, symbols))) {
                type->fields.push_back(field);
            }
        }
        applyAstAnnotation(type, parsed.annotation, symbols);
        return type;
    }

    NamedTypeNode* fallback = new NamedTypeNode(text);
    applyAstAnnotation(fallback, parsed.annotation, symbols);
    return fallback;
}

DeclarationNode* buildAstDeclaration(const AstDumpNode* node, SymbolTable* symbols)
{
    if (node == nullptr) return nullptr;

    AstDumpLabel parsed = parseAstDumpLabel(node->label);
    string text = parsed.text;

    if (startsWithAstText(text, "VarDecl(")) {
        string name;
        extractFirstQuoted(text, name);
        VarDeclNode* declaration = new VarDeclNode({name}, nullptr);
        if (parsed.annotation.tabIndex != -1) declaration->tabIndices.push_back(parsed.annotation.tabIndex);
        declaration->isParameter = text.find("parameter") != string::npos;
        declaration->isVarParameter = text.find(", var") != string::npos;
        applyAstAnnotation(declaration, parsed.annotation, symbols);
        return declaration;
    }

    if (startsWithAstText(text, "ConstDecl(")) {
        string name;
        size_t nameEnd = 0;
        extractFirstQuoted(text, name, &nameEnd);

        string valueText;
        size_t equals = text.find(" = ", nameEnd);
        size_t close = text.rfind(')');
        if (equals != string::npos && close != string::npos && close > equals + 3) {
            valueText = trimAstText(text.substr(equals + 3, close - equals - 3));
        }

        AstDumpNode valueNode;
        valueNode.label = valueText + " -> type:" + typeNameFromClass(parsed.annotation.type);

        ConstDeclNode* declaration = new ConstDeclNode(name, buildAstExpression(&valueNode, symbols));
        applyAstAnnotation(declaration, parsed.annotation, symbols);
        return declaration;
    }

    if (startsWithAstText(text, "TypeDecl(")) {
        string name;
        extractFirstQuoted(text, name);
        TypeNode* type = node->children.empty() ? nullptr : buildAstType(node->children[0], symbols);

        TypeDeclNode* declaration = new TypeDeclNode(name, type);
        applyAstAnnotation(declaration, parsed.annotation, symbols);
        return declaration;
    }

    if (startsWithAstText(text, "ProcedureDecl(")) {
        string name;
        extractFirstQuoted(text, name);
        ProcedureDeclNode* declaration = new ProcedureDeclNode(name);
        applyAstAnnotation(declaration, parsed.annotation, symbols);

        for (const AstDumpNode* child : node->children) {
            AstDumpLabel childLabel = parseAstDumpLabel(child->label);
            if (childLabel.text == "Parameters") {
                for (const AstDumpNode* parameterNode : child->children) {
                    if (VarDeclNode* parameter = dynamic_cast<VarDeclNode*>(buildAstDeclaration(parameterNode, symbols))) {
                        parameter->isParameter = true;
                        declaration->parameters.push_back(parameter);
                    }
                }
            } else if (childLabel.text == "Declarations") {
                for (const AstDumpNode* declNode : child->children) {
                    DeclarationNode* local = buildAstDeclaration(declNode, symbols);
                    if (local != nullptr) declaration->declarations.push_back(local);
                }
            } else if (startsWithAstText(childLabel.text, "Block")) {
                declaration->body = buildAstStatement(child, symbols);
            }
        }

        return declaration;
    }

    if (startsWithAstText(text, "FunctionDecl(")) {
        string name;
        extractFirstQuoted(text, name);
        FunctionDeclNode* declaration = new FunctionDeclNode(name, nullptr);
        applyAstAnnotation(declaration, parsed.annotation, symbols);

        for (const AstDumpNode* child : node->children) {
            AstDumpLabel childLabel = parseAstDumpLabel(child->label);
            if (childLabel.text == "ReturnType" && !child->children.empty()) {
                declaration->returnType = buildAstType(child->children[0], symbols);
            } else if (childLabel.text == "Parameters") {
                for (const AstDumpNode* parameterNode : child->children) {
                    if (VarDeclNode* parameter = dynamic_cast<VarDeclNode*>(buildAstDeclaration(parameterNode, symbols))) {
                        parameter->isParameter = true;
                        declaration->parameters.push_back(parameter);
                    }
                }
            } else if (childLabel.text == "Declarations") {
                for (const AstDumpNode* declNode : child->children) {
                    DeclarationNode* local = buildAstDeclaration(declNode, symbols);
                    if (local != nullptr) declaration->declarations.push_back(local);
                }
            } else if (startsWithAstText(childLabel.text, "Block")) {
                declaration->body = buildAstStatement(child, symbols);
            }
        }

        return declaration;
    }

    return nullptr;
}

CaseBranch* buildAstCaseBranch(const AstDumpNode* node, SymbolTable* symbols)
{
    AstDumpLabel parsed = parseAstDumpLabel(node->label);
    vector<ExpressionNode*> labels;

    string content = contentInOuterParentheses(parsed.text);
    for (const string& labelText : splitAstText(content, ',')) {
        AstDumpNode labelNode;
        labelNode.label = labelText;
        labels.push_back(buildAstExpression(&labelNode, symbols));
    }

    StatementNode* statement = nullptr;
    if (!node->children.empty()) statement = buildAstStatement(node->children[0], symbols);
    return new CaseBranch(labels, statement);
}

StatementNode* buildAstStatement(const AstDumpNode* node, SymbolTable* symbols)
{
    if (node == nullptr) return nullptr;

    AstDumpLabel parsed = parseAstDumpLabel(node->label);
    string text = parsed.text;

    if (startsWithAstText(text, "Block")) {
        CompoundNode* block = new CompoundNode;
        applyAstAnnotation(block, parsed.annotation, symbols);
        for (const AstDumpNode* child : node->children) {
            AstDumpLabel childLabel = parseAstDumpLabel(child->label);
            if (childLabel.text == "(empty)") continue;

            StatementNode* statement = buildAstStatement(child, symbols);
            if (statement != nullptr) block->statements.push_back(statement);
        }
        return block;
    }

    if (startsWithAstText(text, "Assign(")) {
        ExpressionNode* target = nullptr;
        ExpressionNode* value = nullptr;
        for (const AstDumpNode* child : node->children) {
            AstDumpLabel childLabel = parseAstDumpLabel(child->label);
            if (startsWithAstText(childLabel.text, "target ")) target = buildAstExpression(child, symbols);
            else if (startsWithAstText(childLabel.text, "value ")) value = buildAstExpression(child, symbols);
        }

        AssignNode* assign = new AssignNode(target, value);
        applyAstAnnotation(assign, parsed.annotation, symbols);
        return assign;
    }

    if (startsWithAstText(text, "If")) {
        ExpressionNode* condition = nullptr;
        StatementNode* thenBranch = nullptr;
        StatementNode* elseBranch = nullptr;

        for (const AstDumpNode* child : node->children) {
            AstDumpLabel childLabel = parseAstDumpLabel(child->label);
            if (startsWithAstText(childLabel.text, "condition ")) {
                condition = buildAstExpression(child, symbols);
            } else if (childLabel.text == "then" && !child->children.empty()) {
                thenBranch = buildAstStatement(child->children[0], symbols);
            } else if (childLabel.text == "else" && !child->children.empty()) {
                elseBranch = buildAstStatement(child->children[0], symbols);
            }
        }

        IfNode* ifNode = new IfNode(condition, thenBranch, elseBranch);
        applyAstAnnotation(ifNode, parsed.annotation, symbols);
        return ifNode;
    }

    if (startsWithAstText(text, "While")) {
        ExpressionNode* condition = nullptr;
        StatementNode* body = nullptr;

        for (const AstDumpNode* child : node->children) {
            AstDumpLabel childLabel = parseAstDumpLabel(child->label);
            if (startsWithAstText(childLabel.text, "condition ")) condition = buildAstExpression(child, symbols);
            else if (body == nullptr) body = buildAstStatement(child, symbols);
        }

        WhileNode* whileNode = new WhileNode(condition, body);
        applyAstAnnotation(whileNode, parsed.annotation, symbols);
        return whileNode;
    }

    if (startsWithAstText(text, "For(")) {
        string variable;
        extractFirstQuoted(text, variable);
        bool isDownto = text.find("downto") != string::npos;
        ExpressionNode* start = nullptr;
        ExpressionNode* stop = nullptr;
        StatementNode* body = nullptr;

        for (const AstDumpNode* child : node->children) {
            AstDumpLabel childLabel = parseAstDumpLabel(child->label);
            if (startsWithAstText(childLabel.text, "start ")) start = buildAstExpression(child, symbols);
            else if (startsWithAstText(childLabel.text, "stop ")) stop = buildAstExpression(child, symbols);
            else if (body == nullptr) body = buildAstStatement(child, symbols);
        }

        ForNode* forNode = new ForNode(variable, start, stop, isDownto, body);
        applyAstAnnotation(forNode, parsed.annotation, symbols);
        return forNode;
    }

    if (startsWithAstText(text, "Repeat")) {
        vector<StatementNode*> statements;
        ExpressionNode* condition = nullptr;
        for (const AstDumpNode* child : node->children) {
            AstDumpLabel childLabel = parseAstDumpLabel(child->label);
            if (startsWithAstText(childLabel.text, "until ")) condition = buildAstExpression(child, symbols);
            else {
                StatementNode* statement = buildAstStatement(child, symbols);
                if (statement != nullptr) statements.push_back(statement);
            }
        }

        RepeatNode* repeatNode = new RepeatNode(statements, condition);
        applyAstAnnotation(repeatNode, parsed.annotation, symbols);
        return repeatNode;
    }

    if (startsWithAstText(text, "Case")) {
        ExpressionNode* expression = nullptr;
        vector<CaseBranch*> branches;
        for (const AstDumpNode* child : node->children) {
            AstDumpLabel childLabel = parseAstDumpLabel(child->label);
            if (startsWithAstText(childLabel.text, "selector ")) expression = buildAstExpression(child, symbols);
            else if (startsWithAstText(childLabel.text, "CaseBranch")) branches.push_back(buildAstCaseBranch(child, symbols));
        }

        CaseNode* caseNode = new CaseNode(expression, branches);
        applyAstAnnotation(caseNode, parsed.annotation, symbols);
        return caseNode;
    }

    if (text.find("(...)") != string::npos) {
        string name = trimAstText(text.substr(0, text.find("(...)")));
        vector<ExpressionNode*> arguments;
        for (const AstDumpNode* child : node->children) {
            arguments.push_back(buildAstExpression(child, symbols));
        }

        CallNode* call = new CallNode(name, arguments);
        applyAstAnnotation(call, parsed.annotation, symbols);
        return call;
    }

    return nullptr;
}

ProgramNode* buildAstProgram(const AstDumpNode* root, SymbolTable* symbols)
{
    if (root == nullptr) return nullptr;

    AstDumpLabel parsed = parseAstDumpLabel(root->label);
    if (!startsWithAstText(parsed.text, "ProgramNode(")) return nullptr;

    string name;
    extractFirstQuoted(parsed.text, name);
    ProgramNode* program = new ProgramNode(name);
    applyAstAnnotation(program, parsed.annotation, symbols);

    for (const AstDumpNode* child : root->children) {
        AstDumpLabel childLabel = parseAstDumpLabel(child->label);
        if (childLabel.text == "Declarations") {
            for (const AstDumpNode* declNode : child->children) {
                DeclarationNode* declaration = buildAstDeclaration(declNode, symbols);
                if (declaration != nullptr) program->declarations.push_back(declaration);
            }
        } else if (startsWithAstText(childLabel.text, "Block")) {
            program->body = buildAstStatement(child, symbols);
        }
    }

    return program;
}
}

ASTFileReadResult ASTNode::buildFromAstFile(const string& filepath)
{
    ASTFileReadResult result;
    ifstream file(filepath);
    if (!file.is_open()) {
        result.errors.push_back("Invalid AST file: " + filepath);
        return result;
    }

    vector<string> lines;
    string line;
    while (getline(file, line)) {
        lines.push_back(line);
    }

    result.symbolTable = SymbolTable::buildFromAstDumpLines(lines, result.errors);

    size_t astStart = lines.size();
    for (size_t i = 0; i < lines.size(); ++i) {
        if (trimAstText(lines[i]) == "Decorated AST:") {
            astStart = i + 1;
            break;
        }
    }

    if (astStart >= lines.size()) {
        result.errors.push_back("AST dump does not contain Decorated AST section");
        return result;
    }

    AstDumpNode* root = parseAstTree(lines, astStart);
    if (root == nullptr) {
        result.errors.push_back("Decorated AST section is empty");
        return result;
    }

    result.root = buildAstProgram(root, &result.symbolTable);
    delete root;

    if (result.root == nullptr) {
        result.errors.push_back("Failed to rebuild ProgramNode from AST dump");
    }

    return result;
}

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

ProcedureFunctionCallNode::ProcedureFunctionCallNode(string name, vector<ExpressionNode*> arguments) : name(name), arguments(arguments) {}

ProcedureFunctionCallNode::~ProcedureFunctionCallNode(){
    for (ExpressionNode* argument : arguments){
        delete argument;
    }
}

void ProcedureFunctionCallNode::print(int indent) const { printAstNode(this, indent); }

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
