#include "ASTNode.hpp"

#include <algorithm>
#include <cctype>

static vector<bool>& treeLastByDepth()
{
    static vector<bool> lastByDepth(1, true);
    return lastByDepth;
}

static void markTreeDepth(int depth, bool isLast)
{
    vector<bool>& lastByDepth = treeLastByDepth();
    if (lastByDepth.size() <= static_cast<size_t>(depth))
    {
        lastByDepth.resize(depth + 1, true);
    }
    lastByDepth[depth] = isLast;
}

static bool isTreeDepthLast(int depth)
{
    vector<bool>& lastByDepth = treeLastByDepth();
    if (lastByDepth.size() <= static_cast<size_t>(depth))
    {
        lastByDepth.resize(depth + 1, true);
    }
    return lastByDepth[depth];
}

template <typename T>
static size_t countPresent(const vector<T*>& nodes)
{
    size_t count = 0;
    for (const T* node : nodes)
    {
        if (node != nullptr)
        {
            ++count;
        }
    }
    return count;
}

static void printTreePrefix(int indent)
{
    vector<bool>& lastByDepth = treeLastByDepth();
    if (indent == 0)
    {
        lastByDepth.assign(1, true);
    }
    if (lastByDepth.size() <= static_cast<size_t>(indent))
    {
        lastByDepth.resize(indent + 1, true);
    }

    for (int depth = 1; depth <= indent; ++depth)
    {
        if (depth == indent)
        {
            cout << (lastByDepth[depth] ? "└── " : "├── ");
        }
        else
        {
            cout << (lastByDepth[depth] ? "    " : "│   ");
        }
    }
}

template <typename T>
static void printNodeListSection(const string& label, const vector<T*>& nodes, int indent, size_t& childIndex,size_t childCount)
{
    markTreeDepth(indent + 1, ++childIndex == childCount);
    printTreePrefix(indent + 1);
    cout << label << "\n";

    size_t nodeCount = countPresent(nodes);
    size_t nodeIndex = 0;

    for (T* node : nodes)
    {
        if (node != nullptr)
        {
            markTreeDepth(indent + 2, ++nodeIndex == nodeCount);
            node->print(indent + 2);
        }
    }
}

static string typeClassToString(TypeClass type)
{
    switch (type)
    {
    case TypeClass::Integer:
        return "integer";
    case TypeClass::Real:
        return "real";
    case TypeClass::Char:
        return "char";
    case TypeClass::Boolean:
        return "boolean";
    case TypeClass::String:
        return "string";
    case TypeClass::Array:
        return "array";
    case TypeClass::Record:
        return "record";
    case TypeClass::Subrange:
        return "subrange";
    case TypeClass::Enumerated:
        return "enumerated";
    case TypeClass::None:
    default:
        return "none";
    }
}

static void addPart(vector<string>& parts, const string& part)
{
    parts.push_back(part);
}

static string joinParts(const vector<string>& parts)
{
    string result;
    for (size_t i = 0; i < parts.size(); ++i)
    {
        if (i > 0)
        {
            result += ", ";
        }
        result += parts[i];
    }
    return result;
}

static string annotationFromValues(TypeClass evalType, int tabIndex, int btabIndex, int atabIndex, int level, bool includeType, bool forceVoid)
{
    vector<string> parts;

    if (forceVoid)
    {
        addPart(parts, "type:void");
    }
    if (tabIndex != -1)
    {
        addPart(parts, "tab_index:" + to_string(tabIndex));
    }
    if (includeType && evalType != TypeClass::None)
    {
        addPart(parts, "type:" + typeClassToString(evalType));
    }
    if (btabIndex != -1)
    {
        addPart(parts, "block_index:" + to_string(btabIndex));
    }
    if (atabIndex != -1)
    {
        addPart(parts, "array_index:" + to_string(atabIndex));
    }
    if (level != -1)
    {
        addPart(parts, "lev:" + to_string(level));
    }

    if (parts.empty())
    {
        return "";
    }
    return " -> " + joinParts(parts);
}

static string annotation(const ASTNode& node, bool includeType = true, bool forceVoid = false)
{
    return annotationFromValues(node.evalType, node.tabIndex, node.btabIndex, node.atabIndex, node.level, includeType, forceVoid);
}

static string upperString(string value)
{
    transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(toupper(ch));
    });
    return value;
}

static bool isPredefinedProcedure(const string& name)
{
    string upper = upperString(name);
    return upper == "WRITE" || upper == "WRITELN" || upper == "READ" || upper == "READLN";
}

static string expressionSummary(const ExpressionNode* node)
{
    if (node == nullptr)
    {
        return "<empty>";
    }
    if (auto var = dynamic_cast<const VarNode*>(node))
    {
        return "'" + var->name + "'";
    }
    if (auto number = dynamic_cast<const NumberNode*>(node))
    {
        return number->value;
    }
    if (auto str = dynamic_cast<const StringNode*>(node))
    {
        return "'" + str->value + "'";
    }
    if (auto chr = dynamic_cast<const CharNode*>(node))
    {
        return "'" + chr->value + "'";
    }
    if (auto boolean = dynamic_cast<const BoolNode*>(node))
    {
        return boolean->value ? "true" : "false";
    }
    if (auto bin = dynamic_cast<const BinOpNode*>(node))
    {
        return expressionSummary(bin->left) + " " + bin->op + " " + expressionSummary(bin->right);
    }
    if (auto unary = dynamic_cast<const UnaryOpNode*>(node))
    {
        return unary->op + " " + expressionSummary(unary->operand);
    }
    if (auto access = dynamic_cast<const ArrayAccessNode*>(node))
    {
        return expressionSummary(access->array) + "[" + expressionSummary(access->index) + "]";
    }
    if (auto access = dynamic_cast<const RecordAccessNode*>(node))
    {
        return expressionSummary(access->record) + "." + access->field;
    }
    if (auto call = dynamic_cast<const FunctionCallNode*>(node))
    {
        return call->name + "(...)";
    }
    return "<expr>";
}

static void printExpressionWithRole(const string& role, const ExpressionNode* node, int indent)
{
    if (node == nullptr)
    {
        return;
    }

    printTreePrefix(indent);
    cout << role << " ";

    if (auto bin = dynamic_cast<const BinOpNode*>(node))
    {
        cout << "BinOp '" << bin->op << "'" << annotation(*bin) << "\n";
        size_t childCount = (bin->left != nullptr ? 1 : 0) + (bin->right != nullptr ? 1 : 0);
        size_t childIndex = 0;
        if (bin->left != nullptr)
        {
            markTreeDepth(indent + 1, ++childIndex == childCount);
            bin->left->print(indent + 1);
        }
        if (bin->right != nullptr)
        {
            markTreeDepth(indent + 1, ++childIndex == childCount);
            bin->right->print(indent + 1);
        }
        return;
    }

    if (auto unary = dynamic_cast<const UnaryOpNode*>(node))
    {
        cout << "UnaryOp '" << unary->op << "'" << annotation(*unary) << "\n";
        if (unary->operand != nullptr)
        {
            markTreeDepth(indent + 1, true);
            unary->operand->print(indent + 1);
        }
        return;
    }

    cout << expressionSummary(node) << annotation(*node) << "\n";
}

static string varDeclAnnotation(const VarDeclNode& node, size_t index)
{
    int tabIndex = -1;
    if (index < node.tabIndices.size())
    {
        tabIndex = node.tabIndices[index];
    }
    else if (node.tabIndex != -1 && node.names.size() > 0)
    {
        tabIndex = node.tabIndex - static_cast<int>(node.names.size()) + static_cast<int>(index) + 1;
    }

    return annotationFromValues(node.evalType, tabIndex, node.btabIndex, node.atabIndex, node.level, true, false);
}

ProgramNode::ProgramNode(string name) : name(name) {}

ProgramNode::~ProgramNode()
{
    for (DeclarationNode* declaration : declarations)
    {
        delete declaration;
    }
    delete body;
}

void ProgramNode::print(int indent) const
{
    printTreePrefix(indent);
    cout << "ProgramNode(name: '" << name << "')" << annotation(*this, false) << "\n";

    size_t childCount = (!declarations.empty() ? 1 : 0) + (body != nullptr ? 1 : 0);
    size_t childIndex = 0;

    if (!declarations.empty()){
        printNodeListSection("Declarations", declarations, indent, childIndex, childCount);
    }

    if (body != nullptr){
        markTreeDepth(indent + 1, ++childIndex == childCount);
        body->print(indent + 1);
    }
}

VarDeclNode::VarDeclNode(vector<string> names, TypeNode* type)
    : names(names), type(type) {}

VarDeclNode::~VarDeclNode()
{
    delete type;
}

void VarDeclNode::print(int indent) const
{
    bool originalIsLast = isTreeDepthLast(indent);
    for (size_t i = 0; i < names.size(); ++i)
    {
        markTreeDepth(indent, i + 1 == names.size() && originalIsLast);
        printTreePrefix(indent);
        cout << "VarDecl('" << names[i] << "')" << varDeclAnnotation(*this, i);
        if (isParameter)
        {
            cout << ", parameter";
            if (isVarParameter)
            {
                cout << ", var";
            }
        }
        cout << "\n";
    }
}

ConstDeclNode::ConstDeclNode(string name, ExpressionNode* value)
    : name(name), value(value) {}

ConstDeclNode::~ConstDeclNode()
{
    delete value;
}

void ConstDeclNode::print(int indent) const
{
    printTreePrefix(indent);
    cout << "ConstDecl('" << name << "' = " << expressionSummary(value) << ")" << annotation(*this) << "\n";
}

TypeDeclNode::TypeDeclNode(string name, TypeNode* type)
    : name(name), type(type) {}

TypeDeclNode::~TypeDeclNode()
{
    delete type;
}

void TypeDeclNode::print(int indent) const
{
    printTreePrefix(indent);
    cout << "TypeDecl('" << name << "')" << annotation(*this) << "\n";
    if (type != nullptr)
    {
        markTreeDepth(indent + 1, true);
        type->print(indent + 1);
    }
}

NamedTypeNode::NamedTypeNode(string name) : name(name) {}

void NamedTypeNode::print(int indent) const
{
    printTreePrefix(indent);
    cout << "NamedType('" << name << "')" << annotation(*this) << "\n";
}

RangeTypeNode::RangeTypeNode(ExpressionNode* low, ExpressionNode* high)
    : low(low), high(high) {}

RangeTypeNode::~RangeTypeNode()
{
    delete low;
    delete high;
}

void RangeTypeNode::print(int indent) const
{
    printTreePrefix(indent);
    cout << "RangeType(" << expressionSummary(low) << ".." << expressionSummary(high) << ")" << annotation(*this) << "\n";
}

EnumeratedTypeNode::EnumeratedTypeNode(vector<string> values)
    : values(values)
{
    evalType = TypeClass::Enumerated;
}

void EnumeratedTypeNode::print(int indent) const
{
    printTreePrefix(indent);
    cout << "EnumeratedType(";
    for (size_t i = 0; i < values.size(); ++i)
    {
        if (i > 0)
        {
            cout << ", ";
        }
        cout << values[i];
    }
    cout << ")" << annotation(*this) << "\n";
}

ArrayTypeNode::ArrayTypeNode(TypeNode* indexType, TypeNode* elementType)
    : indexType(indexType), elementType(elementType)
{
    evalType = TypeClass::Array;
}

ArrayTypeNode::~ArrayTypeNode()
{
    delete indexType;
    delete elementType;
}

void ArrayTypeNode::print(int indent) const
{
    printTreePrefix(indent);
    cout << "ArrayType" << annotation(*this) << "\n";

    size_t childCount = (indexType != nullptr ? 1 : 0) + (elementType != nullptr ? 1 : 0);
    size_t childIndex = 0;

    if (indexType != nullptr)
    {
        markTreeDepth(indent + 1, ++childIndex == childCount);
        printTreePrefix(indent + 1);
        cout << "IndexType\n";
        markTreeDepth(indent + 2, true);
        indexType->print(indent + 2);
    }
    if (elementType != nullptr)
    {
        markTreeDepth(indent + 1, ++childIndex == childCount);
        printTreePrefix(indent + 1);
        cout << "ElementType\n";
        markTreeDepth(indent + 2, true);
        elementType->print(indent + 2);
    }
}

RecordTypeNode::~RecordTypeNode()
{
    for (VarDeclNode* field : fields)
    {
        delete field;
    }
}

void RecordTypeNode::print(int indent) const
{
    printTreePrefix(indent);
    cout << "RecordType" << annotation(*this) << "\n";

    size_t fieldCount = countPresent(fields);
    size_t fieldIndex = 0;
    for (VarDeclNode* field : fields)
    {
        if (field != nullptr)
        {
            markTreeDepth(indent + 1, ++fieldIndex == fieldCount);
            field->print(indent + 1);
        }
    }
}

ProcedureDeclNode::ProcedureDeclNode(string name) : name(name) {}

ProcedureDeclNode::~ProcedureDeclNode()
{
    for (VarDeclNode* parameter : parameters)
    {
        delete parameter;
    }
    for (DeclarationNode* declaration : declarations)
    {
        delete declaration;
    }
    delete body;
}

void ProcedureDeclNode::print(int indent) const
{
    printTreePrefix(indent);
    cout << "ProcedureDecl('" << name << "')" << annotation(*this, false) << "\n";

    size_t childCount = (!parameters.empty() ? 1 : 0) + (!declarations.empty() ? 1 : 0) + (body != nullptr ? 1 : 0);
    size_t childIndex = 0;

    if (!parameters.empty()){
        printNodeListSection("Parameters", parameters, indent, childIndex, childCount);
    }
    if (!declarations.empty()){
        printNodeListSection("Declarations", declarations, indent, childIndex, childCount);
    }
    if (body != nullptr){
        markTreeDepth(indent + 1, ++childIndex == childCount);
        body->print(indent + 1);
    }
}

FunctionDeclNode::FunctionDeclNode(string name, TypeNode* returnType)
    : name(name), returnType(returnType) {}

FunctionDeclNode::~FunctionDeclNode()
{
    delete returnType;
    for (VarDeclNode* parameter : parameters)
    {
        delete parameter;
    }
    for (DeclarationNode* declaration : declarations)
    {
        delete declaration;
    }
    delete body;
}

void FunctionDeclNode::print(int indent) const
{
    printTreePrefix(indent);
    cout << "FunctionDecl('" << name << "')" << annotation(*this) << "\n";

    size_t childCount = (returnType != nullptr ? 1 : 0) + (!parameters.empty() ? 1 : 0) + (!declarations.empty() ? 1 : 0) + (body != nullptr ? 1 : 0);
    size_t childIndex = 0;

    if (returnType != nullptr)
    {
        markTreeDepth(indent + 1, ++childIndex == childCount);
        printTreePrefix(indent + 1);
        cout << "ReturnType\n";
        markTreeDepth(indent + 2, true);
        returnType->print(indent + 2);
    }
    if (!parameters.empty()){
        printNodeListSection("Parameters", parameters, indent, childIndex, childCount);
    }
    if (!declarations.empty()){
        printNodeListSection("Declarations", declarations, indent, childIndex, childCount);
    }
    if (body != nullptr)
    {
        markTreeDepth(indent + 1, ++childIndex == childCount);
        body->print(indent + 1);
    }
}

CompoundNode::~CompoundNode()
{
    for (StatementNode* statement : statements)
    {
        delete statement;
    }
}

void CompoundNode::print(int indent) const
{
    printTreePrefix(indent);
    cout << "Block" << annotation(*this, false) << "\n";

    size_t statementCount = countPresent(statements);
    if (statementCount == 0)
    {
        markTreeDepth(indent + 1, true);
        printTreePrefix(indent + 1);
        cout << "(empty)\n";
        return;
    }

    size_t statementIndex = 0;
    for (StatementNode* statement : statements)
    {
        if (statement != nullptr)
        {
            markTreeDepth(indent + 1, ++statementIndex == statementCount);
            statement->print(indent + 1);
        }
    }
}

AssignNode::AssignNode(ExpressionNode* target, ExpressionNode* value)
    : target(target), value(value) {}

AssignNode::~AssignNode()
{
    delete target;
    delete value;
}

void AssignNode::print(int indent) const
{
    printTreePrefix(indent);
    cout << "Assign(" << expressionSummary(target) << " := " << expressionSummary(value) << ")" << annotation(*this, false, true) << "\n";

    size_t childCount = (target != nullptr ? 1 : 0) + (value != nullptr ? 1 : 0);
    size_t childIndex = 0;

    if (target != nullptr)
    {
        markTreeDepth(indent + 1, ++childIndex == childCount);
        printExpressionWithRole("target", target, indent + 1);
    }
    if (value != nullptr)
    {
        markTreeDepth(indent + 1, ++childIndex == childCount);
        printExpressionWithRole("value", value, indent + 1);
    }
}

BinOpNode::BinOpNode(string op, ExpressionNode* left, ExpressionNode* right)
    : op(op), left(left), right(right) {}

BinOpNode::~BinOpNode()
{
    delete left;
    delete right;
}

void BinOpNode::print(int indent) const
{
    printTreePrefix(indent);
    cout << "BinOp '" << op << "'" << annotation(*this) << "\n";

    size_t childCount = (left != nullptr ? 1 : 0) + (right != nullptr ? 1 : 0);
    size_t childIndex = 0;

    if (left != nullptr)
    {
        markTreeDepth(indent + 1, ++childIndex == childCount);
        left->print(indent + 1);
    }
    if (right != nullptr)
    {
        markTreeDepth(indent + 1, ++childIndex == childCount);
        right->print(indent + 1);
    }
}

VarNode::VarNode(string name) : name(name) {}

void VarNode::print(int indent) const
{
    printTreePrefix(indent);
    cout << "'" << name << "'" << annotation(*this) << "\n";
}

NumberNode::NumberNode(string value, bool isReal)
    : value(value), isReal(isReal)
{
    evalType = isReal ? TypeClass::Real : TypeClass::Integer;
}

void NumberNode::print(int indent) const
{
    printTreePrefix(indent);
    cout << value << annotation(*this) << "\n";
}

StringNode::StringNode(string value) : value(value)
{
    evalType = TypeClass::String;
}

void StringNode::print(int indent) const
{
    printTreePrefix(indent);
    cout << "'" << value << "'" << annotation(*this) << "\n";
}

IfNode::IfNode(ExpressionNode* condition, StatementNode* thenBranch, StatementNode* elseBranch)
    : condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {}

IfNode::~IfNode()
{
    delete condition;
    delete thenBranch;
    delete elseBranch;
}

void IfNode::print(int indent) const
{
    printTreePrefix(indent);
    cout << "If" << annotation(*this, false, true) << "\n";

    size_t childCount = (condition != nullptr ? 1 : 0) + (thenBranch != nullptr ? 1 : 0) + (elseBranch != nullptr ? 1 : 0);
    size_t childIndex = 0;

    if (condition != nullptr)
    {
        markTreeDepth(indent + 1, ++childIndex == childCount);
        printExpressionWithRole("condition", condition, indent + 1);
    }
    if (thenBranch != nullptr)
    {
        markTreeDepth(indent + 1, ++childIndex == childCount);
        printTreePrefix(indent + 1);
        cout << "then\n";
        markTreeDepth(indent + 2, true);
        thenBranch->print(indent + 2);
    }
    if (elseBranch != nullptr)
    {
        markTreeDepth(indent + 1, ++childIndex == childCount);
        printTreePrefix(indent + 1);
        cout << "else\n";
        markTreeDepth(indent + 2, true);
        elseBranch->print(indent + 2);
    }
}

WhileNode::WhileNode(ExpressionNode* condition, StatementNode* body)
    : condition(condition), body(body) {}

WhileNode::~WhileNode()
{
    delete condition;
    delete body;
}

void WhileNode::print(int indent) const
{
    printTreePrefix(indent);
    cout << "While" << annotation(*this, false, true) << "\n";

    size_t childCount = (condition != nullptr ? 1 : 0) + (body != nullptr ? 1 : 0);
    size_t childIndex = 0;

    if (condition != nullptr)
    {
        markTreeDepth(indent + 1, ++childIndex == childCount);
        printExpressionWithRole("condition", condition, indent + 1);
    }
    if (body != nullptr)
    {
        markTreeDepth(indent + 1, ++childIndex == childCount);
        body->print(indent + 1);
    }
}

ForNode::ForNode(string variable, ExpressionNode* start, ExpressionNode* stop, bool isDownto, StatementNode* body)
    : variable(variable), start(start), stop(stop), isDownto(isDownto), body(body) {}

ForNode::~ForNode()
{
    delete start;
    delete stop;
    delete body;
}

void ForNode::print(int indent) const
{
    printTreePrefix(indent);
    cout << "For('" << variable << "', " << (isDownto ? "downto" : "to") << ")" << annotation(*this, false, true) << "\n";

    size_t childCount = (start != nullptr ? 1 : 0) + (stop != nullptr ? 1 : 0) + (body != nullptr ? 1 : 0);
    size_t childIndex = 0;

    if (start != nullptr)
    {
        markTreeDepth(indent + 1, ++childIndex == childCount);
        printExpressionWithRole("start", start, indent + 1);
    }
    if (stop != nullptr)
    {
        markTreeDepth(indent + 1, ++childIndex == childCount);
        printExpressionWithRole("stop", stop, indent + 1);
    }
    if (body != nullptr)
    {
        markTreeDepth(indent + 1, ++childIndex == childCount);
        body->print(indent + 1);
    }
}

RepeatNode::RepeatNode(vector<StatementNode*> statements, ExpressionNode* condition)
    : statements(statements), condition(condition) {}

RepeatNode::~RepeatNode()
{
    for (StatementNode* statement : statements)
    {
        delete statement;
    }
    delete condition;
}

void RepeatNode::print(int indent) const
{
    printTreePrefix(indent);
    cout << "Repeat" << annotation(*this, false, true) << "\n";

    size_t childCount = countPresent(statements) + (condition != nullptr ? 1 : 0);
    size_t childIndex = 0;

    for (StatementNode* statement : statements)
    {
        if (statement != nullptr)
        {
            markTreeDepth(indent + 1, ++childIndex == childCount);
            statement->print(indent + 1);
        }
    }
    if (condition != nullptr)
    {
        markTreeDepth(indent + 1, ++childIndex == childCount);
        printExpressionWithRole("until", condition, indent + 1);
    }
}

CaseBranch::CaseBranch(vector<ExpressionNode*> labels, StatementNode* statement)
    : labels(labels), statement(statement) {}

CaseBranch::~CaseBranch()
{
    for (ExpressionNode* label : labels)
    {
        delete label;
    }
    delete statement;
}

void CaseBranch::print(int indent) const
{
    printTreePrefix(indent);
    cout << "CaseBranch";
    if (!labels.empty())
    {
        cout << "(";
        for (size_t i = 0; i < labels.size(); ++i)
        {
            if (i > 0)
            {
                cout << ", ";
            }
            cout << expressionSummary(labels[i]);
        }
        cout << ")";
    }
    cout << "\n";
    if (statement != nullptr)
    {
        markTreeDepth(indent + 1, true);
        statement->print(indent + 1);
    }
}

CaseNode::CaseNode(ExpressionNode* expression, vector<CaseBranch*> branches)
    : expression(expression), branches(branches) {}

CaseNode::~CaseNode()
{
    delete expression;
    for (CaseBranch* branch : branches)
    {
        delete branch;
    }
}

void CaseNode::print(int indent) const
{
    printTreePrefix(indent);
    cout << "Case" << annotation(*this, false, true) << "\n";

    size_t childCount = (expression != nullptr ? 1 : 0) + countPresent(branches);
    size_t childIndex = 0;

    if (expression != nullptr)
    {
        markTreeDepth(indent + 1, ++childIndex == childCount);
        printExpressionWithRole("selector", expression, indent + 1);
    }
    for (CaseBranch* branch : branches)
    {
        if (branch != nullptr)
        {
            markTreeDepth(indent + 1, ++childIndex == childCount);
            branch->print(indent + 1);
        }
    }
}

CallNode::CallNode(string name, vector<ExpressionNode*> arguments)
    : name(name), arguments(arguments) {}

CallNode::~CallNode()
{
    for (ExpressionNode* argument : arguments)
    {
        delete argument;
    }
}

void CallNode::print(int indent) const
{
    printTreePrefix(indent);
    cout << name << "(...)";
    if (isPredefinedProcedure(name) && tabIndex != -1)
    {
        cout << " -> predefined, tab_index:" << tabIndex;
        if (level != -1)
        {
            cout << ", lev:" << level;
        }
        cout << "\n";
    }
    else
    {
        cout << annotation(*this, false) << "\n";
    }

    size_t argumentCount = countPresent(arguments);
    size_t argumentIndex = 0;
    for (ExpressionNode* argument : arguments)
    {
        if (argument != nullptr)
        {
            markTreeDepth(indent + 1, ++argumentIndex == argumentCount);
            printExpressionWithRole("argument", argument, indent + 1);
        }
    }
}

FunctionCallNode::FunctionCallNode(string name, vector<ExpressionNode*> arguments)
    : name(name), arguments(arguments) {}

FunctionCallNode::~FunctionCallNode()
{
    for (ExpressionNode* argument : arguments)
    {
        delete argument;
    }
}

void FunctionCallNode::print(int indent) const
{
    printTreePrefix(indent);
    cout << name << "(...)" << annotation(*this) << "\n";

    size_t argumentCount = countPresent(arguments);
    size_t argumentIndex = 0;
    for (ExpressionNode* argument : arguments)
    {
        if (argument != nullptr)
        {
            markTreeDepth(indent + 1, ++argumentIndex == argumentCount);
            printExpressionWithRole("argument", argument, indent + 1);
        }
    }
}

UnaryOpNode::UnaryOpNode(string op, ExpressionNode* operand)
    : op(op), operand(operand) {}

UnaryOpNode::~UnaryOpNode()
{
    delete operand;
}

void UnaryOpNode::print(int indent) const
{
    printTreePrefix(indent);
    cout << "UnaryOp '" << op << "'" << annotation(*this) << "\n";
    if (operand != nullptr)
    {
        markTreeDepth(indent + 1, true);
        operand->print(indent + 1);
    }
}

CharNode::CharNode(string value) : value(value)
{
    evalType = TypeClass::Char;
}

void CharNode::print(int indent) const
{
    printTreePrefix(indent);
    cout << "'" << value << "'" << annotation(*this) << "\n";
}

BoolNode::BoolNode(bool value) : value(value)
{
    evalType = TypeClass::Boolean;
}

void BoolNode::print(int indent) const
{
    printTreePrefix(indent);
    cout << (value ? "true" : "false") << annotation(*this) << "\n";
}

ArrayAccessNode::ArrayAccessNode(ExpressionNode* array, ExpressionNode* index)
    : array(array), index(index) {}

ArrayAccessNode::~ArrayAccessNode()
{
    delete array;
    delete index;
}

void ArrayAccessNode::print(int indent) const
{
    printTreePrefix(indent);
    cout << "ArrayAccess(" << expressionSummary(this) << ")" << annotation(*this) << "\n";

    size_t childCount = (array != nullptr ? 1 : 0) + (index != nullptr ? 1 : 0);
    size_t childIndex = 0;

    if (array != nullptr)
    {
        markTreeDepth(indent + 1, ++childIndex == childCount);
        array->print(indent + 1);
    }
    if (index != nullptr)
    {
        markTreeDepth(indent + 1, ++childIndex == childCount);
        index->print(indent + 1);
    }
}

RecordAccessNode::RecordAccessNode(ExpressionNode* record, string field)
    : record(record), field(field) {}

RecordAccessNode::~RecordAccessNode()
{
    delete record;
}

void RecordAccessNode::print(int indent) const
{
    printTreePrefix(indent);
    cout << "RecordAccess(" << expressionSummary(this) << ")" << annotation(*this) << "\n";
    if (record != nullptr)
    {
        markTreeDepth(indent + 1, true);
        record->print(indent + 1);
    }
}
