#include "ASTPrinter.hpp"

#include <algorithm>
#include <cctype>

struct PrintNode{
    string text;
    vector<PrintNode> children;
};

static vector<bool>& treeLastByDepth(){
    static vector<bool> lastByDepth(1, true);
    return lastByDepth;
}

static void markTreeDepth(int depth, bool isLast){
    vector<bool>& lastByDepth = treeLastByDepth();
    if (lastByDepth.size() <= static_cast<size_t>(depth)){
        lastByDepth.resize(depth + 1, true);
    }
    lastByDepth[depth] = isLast;
}

static bool isTreeDepthLast(int depth){
    vector<bool>& lastByDepth = treeLastByDepth();
    if (lastByDepth.size() <= static_cast<size_t>(depth)){
        lastByDepth.resize(depth + 1, true);
    }
    return lastByDepth[depth];
}

static void printTreePrefix(int indent){
    vector<bool>& lastByDepth = treeLastByDepth();
    if (indent == 0) lastByDepth.assign(1, true);
    if (lastByDepth.size() <= static_cast<size_t>(indent)) lastByDepth.resize(indent + 1, true);

    for (int depth = 1; depth <= indent; ++depth){
        if (depth == indent){
            cout << (lastByDepth[depth] ? "└── " : "├── ");
        } else{
            cout << (lastByDepth[depth] ? "    " : "│   ");
        }
    }
}

static void printRenderedNode(const PrintNode& node, int indent){
    printTreePrefix(indent);
    cout << node.text << "\n";

    for (size_t i = 0; i < node.children.size(); ++i){
        markTreeDepth(indent + 1, i + 1 == node.children.size());
        printRenderedNode(node.children[i], indent + 1);
    }
}

static void printRenderedNodes(const vector<PrintNode>& nodes, int indent){
    bool originalIsLast = isTreeDepthLast(indent);
    for (size_t i = 0; i < nodes.size(); ++i){
        markTreeDepth(indent, i + 1 == nodes.size() && originalIsLast);
        printRenderedNode(nodes[i], indent);
    }
}

static PrintNode leaf(const string& text){
    return {text, {}};
}

static PrintNode branch(const string& text, vector<PrintNode> children){
    return {text, children};
}

static void appendNodes(vector<PrintNode>& destination, vector<PrintNode> nodes){
    destination.insert(destination.end(), nodes.begin(), nodes.end());
}

static string typeClassToString(TypeClass type){
    switch (type){
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

static string joinParts(const vector<string>& parts){
    string result;
    for (size_t i = 0; i < parts.size(); ++i){
        if (i > 0){
            result += ", ";
        }
        result += parts[i];
    }
    return result;
}

static string annotationFromValues(TypeClass evalType, int tabIndex, int btabIndex, int atabIndex, int level, bool includeType, bool forceVoid){
    vector<string> parts;

    if (forceVoid) parts.push_back("type:void");
    if (tabIndex != -1) parts.push_back("tab_index:" + to_string(tabIndex));
    if (includeType && evalType != TypeClass::None) parts.push_back("type:" + typeClassToString(evalType));
    if (btabIndex != -1) parts.push_back("block_index:" + to_string(btabIndex));
    if (atabIndex != -1) parts.push_back("array_index:" + to_string(atabIndex));
    if (level != -1) parts.push_back("lev:" + to_string(level));

    return parts.empty() ? "" : " -> " + joinParts(parts);
}

static string annotation(const ASTNode& node, bool includeType = true, bool forceVoid = false){
    return annotationFromValues(node.evalType, node.tabIndex, node.btabIndex, node.atabIndex, node.level, includeType, forceVoid);
}

static string upperString(string value){
    transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(toupper(ch));
    });
    return value;
}

static bool isPredefinedProcedure(const string& name){
    string upper = upperString(name);
    return upper == "WRITE" || upper == "WRITELN" || upper == "READ" || upper == "READLN";
}

static string expressionSummary(const ExpressionNode* node){
    if (node == nullptr) return "<empty>";
    if (auto var = dynamic_cast<const VarNode*>(node)) return "'" + var->name + "'";
    if (auto number = dynamic_cast<const NumberNode*>(node)) return number->value;
    if (auto str = dynamic_cast<const StringNode*>(node)) return "'" + str->value + "'";
    if (auto chr = dynamic_cast<const CharNode*>(node)) return "'" + chr->value + "'";
    if (auto boolean = dynamic_cast<const BoolNode*>(node)) return boolean->value ? "true" : "false";
    if (auto bin = dynamic_cast<const BinOpNode*>(node)) return expressionSummary(bin->left) + " " + bin->op + " " + expressionSummary(bin->right);
    if (auto unary = dynamic_cast<const UnaryOpNode*>(node)) return unary->op + " " + expressionSummary(unary->operand);
    if (auto access = dynamic_cast<const ArrayAccessNode*>(node)) return expressionSummary(access->array) + "[" + expressionSummary(access->index) + "]";
    if (auto access = dynamic_cast<const RecordAccessNode*>(node)) return expressionSummary(access->record) + "." + access->field;
    if (auto call = dynamic_cast<const FunctionCallNode*>(node)) return call->name + "(...)";
    return "<expr>";
}

static vector<PrintNode> renderAst(const ASTNode* node);
static PrintNode renderExpressionWithRole(const string& role, const ExpressionNode* node);
static PrintNode renderCaseBranch(const CaseBranch& branchNode);

template <typename T>
static vector<PrintNode> renderAstList(const vector<T*>& nodes){
    vector<PrintNode> rendered;
    for (const T* node : nodes){
        appendNodes(rendered, renderAst(node));
    }
    return rendered;
}

static PrintNode renderSection(const string& label, vector<PrintNode> children){
    return branch(label, children);
}

static string varDeclAnnotation(const VarDeclNode& node, size_t index){
    int tabIndex = -1;
    if (index < node.tabIndices.size()){
        tabIndex = node.tabIndices[index];
    } else if (node.tabIndex != -1 && node.names.size() > 0){
        tabIndex = node.tabIndex - static_cast<int>(node.names.size()) + static_cast<int>(index) + 1;
    }

    return annotationFromValues(node.evalType, tabIndex, node.btabIndex, node.atabIndex, node.level, true, false);
}

static string enumeratedValues(const EnumeratedTypeNode& node){
    string result;
    for (size_t i = 0; i < node.values.size(); ++i){
        if (i > 0){
            result += ", ";
        }
        result += node.values[i];
    }
    return result;
}

static string caseLabels(const CaseBranch& node){
    string result;
    for (size_t i = 0; i < node.labels.size(); ++i){
        if (i > 0){
            result += ", ";
        }
        result += expressionSummary(node.labels[i]);
    }
    return result;
}

static PrintNode renderExpression(const ExpressionNode* node){
    if (node == nullptr){
        return leaf("<empty>");
    }
    if (auto bin = dynamic_cast<const BinOpNode*>(node)){
        vector<PrintNode> children;
        appendNodes(children, renderAst(bin->left));
        appendNodes(children, renderAst(bin->right));
        return branch("BinOp '" + bin->op + "'" + annotation(*bin), children);
    }
    if (auto var = dynamic_cast<const VarNode*>(node)){
        return leaf("'" + var->name + "'" + annotation(*var));
    }
    if (auto number = dynamic_cast<const NumberNode*>(node)){
        return leaf(number->value + annotation(*number));
    }
    if (auto str = dynamic_cast<const StringNode*>(node)){
        return leaf("'" + str->value + "'" + annotation(*str));
    }
    if (auto unary = dynamic_cast<const UnaryOpNode*>(node)){
        return branch("UnaryOp '" + unary->op + "'" + annotation(*unary), renderAst(unary->operand));
    }
    if (auto chr = dynamic_cast<const CharNode*>(node)){
        return leaf("'" + chr->value + "'" + annotation(*chr));
    }
    if (auto boolean = dynamic_cast<const BoolNode*>(node)){
        return leaf(string(boolean->value ? "true" : "false") + annotation(*boolean));
    }
    if (auto access = dynamic_cast<const ArrayAccessNode*>(node)){
        vector<PrintNode> children;
        appendNodes(children, renderAst(access->array));
        appendNodes(children, renderAst(access->index));
        return branch("ArrayAccess(" + expressionSummary(access) + ")" + annotation(*access), children);
    }
    if (auto access = dynamic_cast<const RecordAccessNode*>(node)){
        return branch("RecordAccess(" + expressionSummary(access) + ")" + annotation(*access), renderAst(access->record));
    }
    if (auto call = dynamic_cast<const FunctionCallNode*>(node)){
        vector<PrintNode> children;
        for (const ExpressionNode* argument : call->arguments)
        {
            if (argument != nullptr){
                children.push_back(renderExpressionWithRole("argument", argument));
            }
        }
        return branch(call->name + "(...)" + annotation(*call), children);
    }
    return leaf(expressionSummary(node) + annotation(*node));
}

static PrintNode renderExpressionWithRole(const string& role, const ExpressionNode* node){
    if (auto bin = dynamic_cast<const BinOpNode*>(node)){
        vector<PrintNode> children;
        appendNodes(children, renderAst(bin->left));
        appendNodes(children, renderAst(bin->right));
        return branch(role + " BinOp '" + bin->op + "'" + annotation(*bin), children);
    }
    if (auto unary = dynamic_cast<const UnaryOpNode*>(node)){
        return branch(role + " UnaryOp '" + unary->op + "'" + annotation(*unary), renderAst(unary->operand));
    }
    return leaf(role + " " + expressionSummary(node) + annotation(*node));
}

static vector<PrintNode> renderVarDecl(const VarDeclNode& node){
    vector<PrintNode> rendered;
    for (size_t i = 0; i < node.names.size(); ++i){
        string text = "VarDecl('" + node.names[i] + "')" + varDeclAnnotation(node, i);
        if (node.isParameter){
            text += ", parameter";
            if (node.isVarParameter){
                text += ", var";
            }
        }
        rendered.push_back(leaf(text));
    }
    return rendered;
}

static PrintNode renderCaseBranch(const CaseBranch& node){
    string text = "CaseBranch";
    if (!node.labels.empty()){
        text += "(" + caseLabels(node) + ")";
    }

    vector<PrintNode> children;
    appendNodes(children, renderAst(node.statement));
    return branch(text, children);
}

static vector<PrintNode> renderAst(const ASTNode* node){
    if (node == nullptr) return {};

    if (auto program = dynamic_cast<const ProgramNode*>(node)){
        vector<PrintNode> children;
        if (!program->declarations.empty()) children.push_back(renderSection("Declarations", renderAstList(program->declarations)));
        appendNodes(children, renderAst(program->body));
        return {branch("ProgramNode(name: '" + program->name + "')" + annotation(*program, false), children)};
    }
    if (auto varDecl = dynamic_cast<const VarDeclNode*>(node)){
        return renderVarDecl(*varDecl);
    }
    if (auto constDecl = dynamic_cast<const ConstDeclNode*>(node)){
        return {leaf("ConstDecl('" + constDecl->name + "' = " + expressionSummary(constDecl->value) + ")" + annotation(*constDecl))};
    }
    if (auto typeDecl = dynamic_cast<const TypeDeclNode*>(node)){
        return {branch("TypeDecl('" + typeDecl->name + "')" + annotation(*typeDecl), renderAst(typeDecl->type))};
    }
    if (auto namedType = dynamic_cast<const NamedTypeNode*>(node)){
        return {leaf("NamedType('" + namedType->name + "')" + annotation(*namedType))};
    }
    if (auto rangeType = dynamic_cast<const RangeTypeNode*>(node)){
        return {leaf("RangeType(" + expressionSummary(rangeType->low) + ".." + expressionSummary(rangeType->high) + ")" + annotation(*rangeType))};
    }
    if (auto enumerated = dynamic_cast<const EnumeratedTypeNode*>(node)){
        return {leaf("EnumeratedType(" + enumeratedValues(*enumerated) + ")" + annotation(*enumerated))};
    }
    if (auto arrayType = dynamic_cast<const ArrayTypeNode*>(node)){
        vector<PrintNode> children;
        if (arrayType->indexType != nullptr) children.push_back(renderSection("IndexType", renderAst(arrayType->indexType)));
        if (arrayType->elementType != nullptr) children.push_back(renderSection("ElementType", renderAst(arrayType->elementType)));
        return {branch("ArrayType" + annotation(*arrayType), children)};
    }
    if (auto recordType = dynamic_cast<const RecordTypeNode*>(node)){
        return {branch("RecordType" + annotation(*recordType), renderAstList(recordType->fields))};
    }
    if (auto procedure = dynamic_cast<const ProcedureDeclNode*>(node)){
        vector<PrintNode> children;
        if (!procedure->parameters.empty()) children.push_back(renderSection("Parameters", renderAstList(procedure->parameters)));
        if (!procedure->declarations.empty()) children.push_back(renderSection("Declarations", renderAstList(procedure->declarations)));
        appendNodes(children, renderAst(procedure->body));
        return {branch("ProcedureDecl('" + procedure->name + "')" + annotation(*procedure, false), children)};
    }
    if (auto function = dynamic_cast<const FunctionDeclNode*>(node)){
        vector<PrintNode> children;
        if (function->returnType != nullptr) children.push_back(renderSection("ReturnType", renderAst(function->returnType)));
        if (!function->parameters.empty()) children.push_back(renderSection("Parameters", renderAstList(function->parameters)));
        if (!function->declarations.empty()) children.push_back(renderSection("Declarations", renderAstList(function->declarations)));
        appendNodes(children, renderAst(function->body));
        return {branch("FunctionDecl('" + function->name + "')" + annotation(*function), children)};
    }
    if (auto compound = dynamic_cast<const CompoundNode*>(node)){
        vector<PrintNode> children = renderAstList(compound->statements);
        if (children.empty()) children.push_back(leaf("(empty)"));
        return {branch("Block" + annotation(*compound, false), children)};
    }
    if (auto assign = dynamic_cast<const AssignNode*>(node)){
        vector<PrintNode> children;
        if (assign->target != nullptr) children.push_back(renderExpressionWithRole("target", assign->target));
        if (assign->value != nullptr) children.push_back(renderExpressionWithRole("value", assign->value));
        return {branch("Assign(" + expressionSummary(assign->target) + " := " + expressionSummary(assign->value) + ")" + annotation(*assign, false, true), children)};
    }
    if (auto expression = dynamic_cast<const ExpressionNode*>(node)){
        return {renderExpression(expression)};
    }
    if (auto ifNode = dynamic_cast<const IfNode*>(node)){
        vector<PrintNode> children;
        if (ifNode->condition != nullptr) children.push_back(renderExpressionWithRole("condition", ifNode->condition));
        if (ifNode->thenBranch != nullptr) children.push_back(renderSection("then", renderAst(ifNode->thenBranch)));
        if (ifNode->elseBranch != nullptr) children.push_back(renderSection("else", renderAst(ifNode->elseBranch)));
        return {branch("If" + annotation(*ifNode, false, true), children)};
    }
    if (auto whileNode = dynamic_cast<const WhileNode*>(node)){
        vector<PrintNode> children;
        if (whileNode->condition != nullptr) children.push_back(renderExpressionWithRole("condition", whileNode->condition));
        appendNodes(children, renderAst(whileNode->body));
        return {branch("While" + annotation(*whileNode, false, true), children)};
    }
    if (auto forNode = dynamic_cast<const ForNode*>(node)){
        vector<PrintNode> children;
        if (forNode->start != nullptr) children.push_back(renderExpressionWithRole("start", forNode->start));
        if (forNode->stop != nullptr) children.push_back(renderExpressionWithRole("stop", forNode->stop));
        appendNodes(children, renderAst(forNode->body));
        return {branch("For('" + forNode->variable + "', " + (forNode->isDownto ? "downto" : "to") + ")" + annotation(*forNode, false, true), children)};
    }
    if (auto repeatNode = dynamic_cast<const RepeatNode*>(node)){
        vector<PrintNode> children = renderAstList(repeatNode->statements);
        if (repeatNode->condition != nullptr) children.push_back(renderExpressionWithRole("until", repeatNode->condition));
        return {branch("Repeat" + annotation(*repeatNode, false, true), children)};
    }
    if (auto caseNode = dynamic_cast<const CaseNode*>(node)){
        vector<PrintNode> children;
        if (caseNode->expression != nullptr) children.push_back(renderExpressionWithRole("selector", caseNode->expression));
        for (const CaseBranch* caseBranch : caseNode->branches){
            if (caseBranch != nullptr) children.push_back(renderCaseBranch(*caseBranch));
        }
        return {branch("Case" + annotation(*caseNode, false, true), children)};
    }
    if (auto call = dynamic_cast<const CallNode*>(node)){
        string text = call->name + "(...)";
        if (isPredefinedProcedure(call->name) && call->tabIndex != -1){
            text += " -> predefined, tab_index:" + to_string(call->tabIndex);
            if (call->level != -1){
                text += ", lev:" + to_string(call->level);
            }
        } else {
            text += annotation(*call, false);
        }

        vector<PrintNode> children;
        for (const ExpressionNode* argument : call->arguments){
            if (argument != nullptr) children.push_back(renderExpressionWithRole("argument", argument));
        }
        return {branch(text, children)};
    }
    return {};
}

void printAstNode(const ASTNode* node, int indent){
    printRenderedNodes(renderAst(node), indent);
}

void printAstCaseBranch(const CaseBranch* node, int indent){
    if (node != nullptr) printRenderedNodes({renderCaseBranch(*node)}, indent);
    
}
