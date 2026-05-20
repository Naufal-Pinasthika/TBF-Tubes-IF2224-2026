#include "ASTNode.hpp"

static void printIndent(int indent)
{
    for (int i = 0; i < indent; i++)
    {
        cout << "  ";
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

static void printDecorations(const ASTNode& node)
{
    bool hasDecoration = node.evalType != TypeClass::None ||
                         node.tabIndex != -1 ||
                         node.btabIndex != -1 ||
                         node.atabIndex != -1 ||
                         node.level != -1 ||
                         node.line != 0 ||
                         node.column != 0;

    if (!hasDecoration)
    {
        return;
    }

    bool first = true;
    cout << " [";

    if (node.evalType != TypeClass::None)
    {
        cout << "type: " << typeClassToString(node.evalType);
        first = false;
    }
    if (node.tabIndex != -1)
    {
        if (!first)
            cout << ", ";
        cout << "tab_index: " << node.tabIndex;
        first = false;
    }
    if (node.btabIndex != -1)
    {
        if (!first)
            cout << ", ";
        cout << "btab_index: " << node.btabIndex;
        first = false;
    }
    if (node.atabIndex != -1)
    {
        if (!first)
            cout << ", ";
        cout << "atab_index: " << node.atabIndex;
        first = false;
    }
    if (node.level != -1)
    {
        if (!first)
            cout << ", ";
        cout << "level: " << node.level;
        first = false;
    }
    if (node.line != 0 || node.column != 0)
    {
        if (!first)
            cout << ", ";
        cout << "pos: " << node.line << ":" << node.column;
    }

    cout << "]";
}

static void printStringList(const vector<string>& values)
{
    cout << "[";
    for (size_t i = 0; i < values.size(); i++)
    {
        if (i > 0)
        {
            cout << ", ";
        }
        cout << "'" << values[i] << "'";
    }
    cout << "]";
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
    printIndent(indent);
    cout << "ProgramNode(name: '" << name << "')";
    printDecorations(*this);
    cout << "\n";

    if (!declarations.empty())
    {
        printIndent(indent + 1);
        cout << "Declarations:\n";
        for (DeclarationNode* declaration : declarations)
        {
            if (declaration != nullptr)
            {
                declaration->print(indent + 2);
            }
        }
    }

    if (body != nullptr)
    {
        printIndent(indent + 1);
        cout << "Body:\n";
        body->print(indent + 2);
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
    printIndent(indent);
    cout << "VarDecl(names: ";
    printStringList(names);
    if (isParameter)
    {
        cout << ", parameter";
        if (isVarParameter)
        {
            cout << ", var";
        }
    }
    cout << ")";
    printDecorations(*this);
    cout << "\n";

    if (type != nullptr)
    {
        printIndent(indent + 1);
        cout << "Type:\n";
        type->print(indent + 2);
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
    printIndent(indent);
    cout << "ConstDecl(name: '" << name << "')";
    printDecorations(*this);
    cout << "\n";

    if (value != nullptr)
    {
        printIndent(indent + 1);
        cout << "Value:\n";
        value->print(indent + 2);
    }
}

TypeDeclNode::TypeDeclNode(string name, TypeNode* type)
    : name(name), type(type) {}

TypeDeclNode::~TypeDeclNode()
{
    delete type;
}

void TypeDeclNode::print(int indent) const
{
    printIndent(indent);
    cout << "TypeDecl(name: '" << name << "')";
    printDecorations(*this);
    cout << "\n";

    if (type != nullptr)
    {
        printIndent(indent + 1);
        cout << "Type:\n";
        type->print(indent + 2);
    }
}

NamedTypeNode::NamedTypeNode(string name) : name(name) {}

void NamedTypeNode::print(int indent) const
{
    printIndent(indent);
    cout << "NamedType(name: '" << name << "')";
    printDecorations(*this);
    cout << "\n";
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
    printIndent(indent);
    cout << "RangeType";
    printDecorations(*this);
    cout << "\n";

    if (low != nullptr)
    {
        printIndent(indent + 1);
        cout << "Low:\n";
        low->print(indent + 2);
    }
    if (high != nullptr)
    {
        printIndent(indent + 1);
        cout << "High:\n";
        high->print(indent + 2);
    }
}

EnumeratedTypeNode::EnumeratedTypeNode(vector<string> values)
    : values(values)
{
    evalType = TypeClass::Enumerated;
}

void EnumeratedTypeNode::print(int indent) const
{
    printIndent(indent);
    cout << "EnumeratedType(values: ";
    printStringList(values);
    cout << ")";
    printDecorations(*this);
    cout << "\n";
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
    printIndent(indent);
    cout << "ArrayType";
    printDecorations(*this);
    cout << "\n";

    if (indexType != nullptr)
    {
        printIndent(indent + 1);
        cout << "IndexType:\n";
        indexType->print(indent + 2);
    }
    if (elementType != nullptr)
    {
        printIndent(indent + 1);
        cout << "ElementType:\n";
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
    printIndent(indent);
    cout << "RecordType";
    printDecorations(*this);
    cout << "\n";

    if (!fields.empty())
    {
        printIndent(indent + 1);
        cout << "Fields:\n";
        for (VarDeclNode* field : fields)
        {
            if (field != nullptr)
            {
                field->print(indent + 2);
            }
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
    printIndent(indent);
    cout << "ProcedureDecl(name: '" << name << "')";
    printDecorations(*this);
    cout << "\n";

    if (!parameters.empty())
    {
        printIndent(indent + 1);
        cout << "Parameters:\n";
        for (VarDeclNode* parameter : parameters)
        {
            if (parameter != nullptr)
            {
                parameter->print(indent + 2);
            }
        }
    }
    if (!declarations.empty())
    {
        printIndent(indent + 1);
        cout << "Declarations:\n";
        for (DeclarationNode* declaration : declarations)
        {
            if (declaration != nullptr)
            {
                declaration->print(indent + 2);
            }
        }
    }
    if (body != nullptr)
    {
        printIndent(indent + 1);
        cout << "Body:\n";
        body->print(indent + 2);
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
    printIndent(indent);
    cout << "FunctionDecl(name: '" << name << "')";
    printDecorations(*this);
    cout << "\n";

    if (returnType != nullptr)
    {
        printIndent(indent + 1);
        cout << "ReturnType:\n";
        returnType->print(indent + 2);
    }
    if (!parameters.empty())
    {
        printIndent(indent + 1);
        cout << "Parameters:\n";
        for (VarDeclNode* parameter : parameters)
        {
            if (parameter != nullptr)
            {
                parameter->print(indent + 2);
            }
        }
    }
    if (!declarations.empty())
    {
        printIndent(indent + 1);
        cout << "Declarations:\n";
        for (DeclarationNode* declaration : declarations)
        {
            if (declaration != nullptr)
            {
                declaration->print(indent + 2);
            }
        }
    }
    if (body != nullptr)
    {
        printIndent(indent + 1);
        cout << "Body:\n";
        body->print(indent + 2);
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
    printIndent(indent);
    cout << "CompoundStatement";
    printDecorations(*this);
    cout << "\n";

    if (statements.empty())
    {
        printIndent(indent + 1);
        cout << "(empty)\n";
        return;
    }

    for (StatementNode* statement : statements)
    {
        if (statement != nullptr)
        {
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
    printIndent(indent);
    cout << "Assign";
    printDecorations(*this);
    cout << "\n";

    if (target != nullptr)
    {
        printIndent(indent + 1);
        cout << "Target:\n";
        target->print(indent + 2);
    }
    if (value != nullptr)
    {
        printIndent(indent + 1);
        cout << "Value:\n";
        value->print(indent + 2);
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
    printIndent(indent);
    cout << "BinOp(op: '" << op << "')";
    printDecorations(*this);
    cout << "\n";

    if (left != nullptr)
    {
        printIndent(indent + 1);
        cout << "Left:\n";
        left->print(indent + 2);
    }
    if (right != nullptr)
    {
        printIndent(indent + 1);
        cout << "Right:\n";
        right->print(indent + 2);
    }
}

VarNode::VarNode(string name) : name(name) {}

void VarNode::print(int indent) const
{
    printIndent(indent);
    cout << "Var(name: '" << name << "')";
    printDecorations(*this);
    cout << "\n";
}

NumberNode::NumberNode(string value, bool isReal)
    : value(value), isReal(isReal)
{
    evalType = isReal ? TypeClass::Real : TypeClass::Integer;
}

void NumberNode::print(int indent) const
{
    printIndent(indent);
    cout << "Number(" << (isReal ? "real" : "int") << ": " << value << ")";
    printDecorations(*this);
    cout << "\n";
}

StringNode::StringNode(string value) : value(value)
{
    evalType = TypeClass::String;
}

void StringNode::print(int indent) const
{
    printIndent(indent);
    cout << "String(value: '" << value << "')";
    printDecorations(*this);
    cout << "\n";
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
    printIndent(indent);
    cout << "If";
    printDecorations(*this);
    cout << "\n";

    if (condition != nullptr)
    {
        printIndent(indent + 1);
        cout << "Condition:\n";
        condition->print(indent + 2);
    }
    if (thenBranch != nullptr)
    {
        printIndent(indent + 1);
        cout << "Then:\n";
        thenBranch->print(indent + 2);
    }
    if (elseBranch != nullptr)
    {
        printIndent(indent + 1);
        cout << "Else:\n";
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
    printIndent(indent);
    cout << "While";
    printDecorations(*this);
    cout << "\n";

    if (condition != nullptr)
    {
        printIndent(indent + 1);
        cout << "Condition:\n";
        condition->print(indent + 2);
    }
    if (body != nullptr)
    {
        printIndent(indent + 1);
        cout << "Body:\n";
        body->print(indent + 2);
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
    printIndent(indent);
    cout << "For(variable: '" << variable << "', direction: " << (isDownto ? "downto" : "to") << ")";
    printDecorations(*this);
    cout << "\n";

    if (start != nullptr)
    {
        printIndent(indent + 1);
        cout << "Start:\n";
        start->print(indent + 2);
    }
    if (stop != nullptr)
    {
        printIndent(indent + 1);
        cout << "Stop:\n";
        stop->print(indent + 2);
    }
    if (body != nullptr)
    {
        printIndent(indent + 1);
        cout << "Body:\n";
        body->print(indent + 2);
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
    printIndent(indent);
    cout << "Repeat";
    printDecorations(*this);
    cout << "\n";

    if (!statements.empty())
    {
        printIndent(indent + 1);
        cout << "Body:\n";
        for (StatementNode* statement : statements)
        {
            if (statement != nullptr)
            {
                statement->print(indent + 2);
            }
        }
    }
    if (condition != nullptr)
    {
        printIndent(indent + 1);
        cout << "Until:\n";
        condition->print(indent + 2);
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
    printIndent(indent);
    cout << "CaseBranch\n";

    if (!labels.empty())
    {
        printIndent(indent + 1);
        cout << "Labels:\n";
        for (ExpressionNode* label : labels)
        {
            if (label != nullptr)
            {
                label->print(indent + 2);
            }
        }
    }

    if (statement != nullptr)
    {
        printIndent(indent + 1);
        cout << "Statement:\n";
        statement->print(indent + 2);
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
    printIndent(indent);
    cout << "Case";
    printDecorations(*this);
    cout << "\n";

    if (expression != nullptr)
    {
        printIndent(indent + 1);
        cout << "Expression:\n";
        expression->print(indent + 2);
    }

    if (!branches.empty())
    {
        printIndent(indent + 1);
        cout << "Branches:\n";
        for (CaseBranch* branch : branches)
        {
            if (branch != nullptr)
            {
                branch->print(indent + 2);
            }
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
    printIndent(indent);
    cout << "Call(name: '" << name << "')";
    printDecorations(*this);
    cout << "\n";

    if (!arguments.empty())
    {
        printIndent(indent + 1);
        cout << "Arguments:\n";
        for (ExpressionNode* argument : arguments)
        {
            if (argument != nullptr)
            {
                argument->print(indent + 2);
            }
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
    printIndent(indent);
    cout << "UnaryOp(op: '" << op << "')";
    printDecorations(*this);
    cout << "\n";

    if (operand != nullptr)
    {
        printIndent(indent + 1);
        cout << "Operand:\n";
        operand->print(indent + 2);
    }
}

CharNode::CharNode(string value) : value(value)
{
    evalType = TypeClass::Char;
}

void CharNode::print(int indent) const
{
    printIndent(indent);
    cout << "Char(value: '" << value << "')";
    printDecorations(*this);
    cout << "\n";
}

BoolNode::BoolNode(bool value) : value(value)
{
    evalType = TypeClass::Boolean;
}

void BoolNode::print(int indent) const
{
    printIndent(indent);
    cout << "Bool(value: " << (value ? "true" : "false") << ")";
    printDecorations(*this);
    cout << "\n";
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
    printIndent(indent);
    cout << "ArrayAccess";
    printDecorations(*this);
    cout << "\n";

    if (array != nullptr)
    {
        printIndent(indent + 1);
        cout << "Array:\n";
        array->print(indent + 2);
    }
    if (index != nullptr)
    {
        printIndent(indent + 1);
        cout << "Index:\n";
        index->print(indent + 2);
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
    printIndent(indent);
    cout << "RecordAccess(field: '" << field << "')";
    printDecorations(*this);
    cout << "\n";

    if (record != nullptr)
    {
        printIndent(indent + 1);
        cout << "Record:\n";
        record->print(indent + 2);
    }
}
