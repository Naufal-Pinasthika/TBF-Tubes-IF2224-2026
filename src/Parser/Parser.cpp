#include "Parser.hpp"

#include <fstream>
#include <stdexcept>

Token Parser::peek()
{
    return tokens[pos];
}

Token Parser::pop()
{
    return tokens[pos++];
}

bool Parser::success(Node *parent)
{
    parent->children.push_back(curr);
    curr = parent;
    return true;
}

bool Parser::fails(Node *parent)
{
    delete curr;
    curr = parent;
    return false;
}

void Parser::backTrack(int save)
{
    if (highestPos < pos)
        highestPos = pos;
    pos = save;
}

Node *Parser::insert(NodeType type)
{
    Node *parent = curr;
    Node *node = new Node(type);
    curr = node;
    return parent;
}

bool Parser::match(string t)
{
    while (peek().getType() == "comment")
        pop();
    if (peek().getType() == t)
    {
        Node *node = new Node(tokens[pos]);
        pop();
        curr->children.push_back(node);
        return true;
    }

    if (pos > highestPos)
    {
        expected.clear();
        expected.insert(t);
    }
    else if (pos == highestPos)
    {
        expected.insert(t);
    }

    return false;
}

bool Parser::parse()
{
    root = nullptr;
    bool result = programProd();
    root = curr;
    return result;
}

bool Parser::programProd()
{
    curr = new Node(program);
    int save = pos;

    if (programHeaderProd() && declarationPartProd() && compoundStatementProd() && match("period"))
        return true;

    backTrack(save);
    delete curr;
    curr = nullptr;
    return false;
}

bool Parser::programHeaderProd()
{
    Node *parent = insert(program_header);
    int save = pos;

    if (match("programsy") && match("ident") && match("semicolon"))
        return success(parent);

    backTrack(save);
    return fails(parent);
}

bool Parser::declarationPartProd()
{
    Node *parent = insert(declaration_part);
    int save = pos;

    while (constDeclarationProd())
        save = pos;
    backTrack(save);
    while (typeDeclarationProd())
        save = pos;
    backTrack(save);
    while (varDeclarationProd())
        save = pos;
    backTrack(save);
    while (subprogramDeclarationProd())
        save = pos;
    backTrack(save);

    return success(parent);
}

bool Parser::constDeclarationProd()
{
    Node *parent = insert(const_declaration);
    int save = pos;

    if (match("constsy") && match("ident") && match("eql") && constantProd() && match("semicolon"))
    {
        int save = pos;
        while (match("ident") && match("eql") && constantProd() && match("semicolon"))
            save = pos;
        backTrack(save);
        return success(parent);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::constantProd()
{
    Node *parent = insert(constant);
    int save = pos;

    if (match("charcon"))
        return success(parent);
    backTrack(save);
    if (match("string"))
        return success(parent);
    backTrack(save);
    if (!(match("plus") || match("minus")))
        backTrack(save);
    save = pos;
    if ((match("ident") || match("intcon") || match("realcon")))
        return success(parent);

    backTrack(save);
    return fails(parent);
}

bool Parser::typeDeclarationProd()
{
    Node *parent = insert(type_declaration);
    int save = pos;

    if (match("typesy") && match("ident") && match("eql") && typeProd() && match("semicolon"))
    {

        int save = pos;
        while (match("ident") && match("eql") && typeProd() && match("semicolon"))
            save = pos;
        backTrack(save);
        return success(parent);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::varDeclarationProd()
{
    Node *parent = insert(var_declaration);
    int save = pos;

    if (match("varsy") && identifierListProd() && match("colon") && typeProd() && match("semicolon"))
    {
        int save = pos;
        while (identifierListProd() && match("colon") && typeProd() && match("semicolon"))
            save = pos;
        backTrack(save);
        return success(parent);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::identifierListProd()
{
    Node *parent = insert(identifier_list);
    int save = pos;

    if (match("ident"))
    {
        int save = pos;
        while (match("comma") && match("ident"))
            save = pos;
        backTrack(save);
        return success(parent);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::typeProd()
{
    Node *parent = insert(type);
    int save = pos;

    if (match("ident"))
        return success(parent);
    backTrack(save);
    if (arrayTypeProd())
        return success(parent);
    backTrack(save);
    if (rangeProd())
        return success(parent);
    backTrack(save);
    if (enumeratedProd())
        return success(parent);
    backTrack(save);
    if (recordTypeProd())
        return success(parent);

    backTrack(save);
    return fails(parent);
}

bool Parser::arrayTypeProd()
{
    Node *parent = insert(array_type);
    int save = pos;

    if (match("arraysy") && match("lbrack") && (match("ident") || rangeProd()) && match("rbrack") && match("ofsy") && typeProd())
        return success(parent);

    backTrack(save);
    return fails(parent);
}

bool Parser::rangeProd()
{
    Node *parent = insert(range);
    int save = pos;

    if (constantProd() && match("period") && match("period") && constantProd())
        return success(parent);

    backTrack(save);
    return fails(parent);
}

bool Parser::enumeratedProd()
{
    Node *parent = insert(enumerated);
    int save = pos;

    if (match("lparent") && match("ident"))
    {
        int save2 = pos;
        while (match("comma") && match("ident"))
            save2 = pos;
        backTrack(save2);
        if (match("rparent"))
            return success(parent);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::recordTypeProd()
{
    Node *parent = insert(record_type);
    int save = pos;

    if (match("recordsy") && fieldListProd() && match("endsy"))
        return success(parent);

    backTrack(save);
    return fails(parent);
}

bool Parser::fieldListProd()
{
    Node *parent = insert(field_list);
    int save = pos;

    if (fieldPartProd())
    {
        int save2 = pos;
        while (match("semicolon") && fieldPartProd())
            save2 = pos;
        backTrack(save2);
        return success(parent);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::fieldPartProd()
{
    Node *parent = insert(field_part);
    int save = pos;

    if (identifierListProd() && match("colon") && typeProd())
        return success(parent);

    backTrack(save);
    return fails(parent);
}

bool Parser::subprogramDeclarationProd()
{
    Node *parent = insert(subprogram_declaration);
    int save = pos;

    if (procedureDeclarationProd())
        return success(parent);
    backTrack(save);
    if (functionDeclarationProd())
        return success(parent);

    backTrack(save);
    return fails(parent);
}

bool Parser::procedureDeclarationProd()
{
    Node *parent = insert(procedure_declaration);
    int save = pos;

    if (match("proceduresy") && match("ident"))
    {
        int save2 = pos;
        if (formalParameterListProd())
        {
            if (match("semicolon") && blockProd() && match("semicolon"))
                return success(parent);
        }
        backTrack(save2);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::functionDeclarationProd()
{
    Node *parent = insert(function_declaration);
    int save = pos;

    if (match("functionsy") && match("ident"))
    {
        int save2 = pos;
        if (formalParameterListProd())
        {
            if (match("colon") && match("ident") && match("semicolon") && blockProd() && match("semicolon"))
                return success(parent);
        }
        backTrack(save2);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::blockProd()
{
    Node *parent = insert(block);
    int save = pos;

    if (declarationPartProd() && compoundStatementProd())
        return success(parent);

    backTrack(save);
    return fails(parent);
}

bool Parser::formalParameterListProd()
{
    Node *parent = insert(formal_parameter_list);
    int save = pos;

    if (match("lparent") && parameterGroupProd())
    {
        int save2 = pos;
        while (match("semicolon") && parameterGroupProd())
            save2 = pos;
        backTrack(save2);

        if (match("rparent"))
            return success(parent);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::parameterGroupProd()
{
    Node *parent = insert(parameter_group);
    int save = pos;

    if (identifierListProd() && match("colon") && (match("ident") || arrayTypeProd()))
    {
        return success(parent);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::compoundStatementProd()
{
    Node *parent = insert(compound_statement);
    int save = pos;

    if (match("beginsy") && statementListProd() && match("endsy"))
    {
        return success(parent);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::statementListProd()
{
    Node *parent = insert(statement_list);
    int save = pos;

    if (statementProd())
    {
        int save2 = pos;
        while (match("semicolon") && statementProd())
        {
            save2 = pos;
        }
        backTrack(save2);
        return success(parent);
    }
    backTrack(save);
    return false;
}

bool Parser::statementProd()
{
    Node *parent = insert(statement);
    int save = pos;

    if (assignmentStatementProd())
        return success(parent);
    backTrack(save);
    if (ifStatementProd())
        return success(parent);
    backTrack(save);
    if (caseStatementProd())
        return success(parent);
    backTrack(save);
    if (whileStatementProd())
        return success(parent);
    backTrack(save);
    if (repeatStatementProd())
        return success(parent);
    backTrack(save);
    if (forStatementProd())
        return success(parent);
    backTrack(save);
    if (procedureFunctionCallProd())
        return success(parent);
    backTrack(save);
    if (true)
        return success(parent);

    backTrack(save);
    return fails(parent);
}

bool Parser::variableProd()
{
    Node *parent = insert(variable);
    int save = pos;

    if (match("ident"))
    {
        int save = pos;
        while (componentVariableProd())
            save = pos;
        backTrack(save);
        return success(parent);
    }
    backTrack(save);
    return fails(parent);
}

bool Parser::componentVariableProd()
{
    Node *parent = insert(component_variable);
    int save = pos;

    if (match("lbrack") && indexListProd() && match("rbrack"))
        return success(parent);
    backTrack(save);
    if (match("period") && match("ident"))
        return success(parent);
    backTrack(save);

    return fails(parent);
}

bool Parser::indexListProd()
{
    Node *parent = insert(index_list);
    int save = pos;

    if (match("intcon") || match("charcon") || match("ident"))
    {
        int save = pos;
        while (match("comma") && indexListProd())
            save = pos;
        backTrack(save);

        return success(parent);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::assignmentStatementProd()
{
    Node *parent = insert(assignment_statement);
    int save = pos;

    if (variableProd() && match("becomes") && expressionProd())
    {
        return success(parent);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::ifStatementProd()
{
    Node *parent = insert(if_statement);
    int save = pos;

    if (match("ifsy") && expressionProd() && match("thensy") && statementProd())
    {
        int save = pos;
        if (!(match("elsesy") && statementProd()))
            backTrack(save);
        return success(parent);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::caseStatementProd()
{
    Node *parent = insert(case_statement);
    int save = pos;

    if (match("casesy") && expressionProd() && match("ofsy") && caseBlockProd() && match("endsy"))
    {
        return success(parent);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::caseBlockProd()
{
    Node *parent = insert(case_block);
    int save = pos;

    if (constantProd())
    {
        int save2 = pos;

        while (match("comma") && constantProd())
        {
            save2 = pos;
        }
        backTrack(save2);

        if (match("colon") && statementProd())
        {
            save2 = pos;

            while (match("semicolon"))
            {
                int afterSemicolon = pos;

                if (caseBlockProd())
                {
                    save2 = pos;
                    continue;
                }

                backTrack(afterSemicolon);
                break;
            }

            backTrack(save2);
            return success(parent);
        }
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::whileStatementProd()
{
    Node *parent = insert(while_statement);
    int save = pos;

    if (match("whilesy") && expressionProd() && match("dosy") && compoundStatementProd())
    {
        return success(parent);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::repeatStatementProd()
{
    Node *parent = insert(repeat_statement);
    int save = pos;

    if (match("repeatsy") && statementListProd() && match("untilsy") && expressionProd())
    {
        return success(parent);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::forStatementProd()
{
    Node *parent = insert(for_statement);
    int save = pos;

    if (match("forsy") && match("ident") && match("becomes") && expressionProd() && (match("tosy") || match("downtosy")) && expressionProd() && match("dosy") && compoundStatementProd())
    {
        return success(parent);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::procedureFunctionCallProd()
{
    Node *parent = insert(procedure_function_call);
    int save = pos;

    if (match("ident") && match("lparent"))
    {
        int save = pos;
        if (!parameterListProd())
            backTrack(save);
        if (match("rparent"))
            return success(parent);
        backTrack(save);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::parameterListProd()
{
    Node *parent = insert(parameter_list);
    int save = pos;

    if (expressionProd())
    {
        int save = pos;
        while (match("comma") && expressionProd())
            save = pos;
        backTrack(save);
        return success(parent);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::expressionProd()
{
    Node *parent = insert(expression);
    int save = pos;

    if (simpleExpressionProd())
    {
        int save = pos;
        if (!(relationalOperatorProd() && simpleExpressionProd()))
            backTrack(save);
        return success(parent);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::simpleExpressionProd()
{
    Node *parent = insert(simple_expression);
    int save = pos;

    if (!(match("plus") || match("minus")))
        backTrack(save);
    save = pos;
    if (termProd())
    {
        int save = pos;
        while (additiveOperatorProd() && termProd())
            save = pos;
        backTrack(save);
        return success(parent);
    }
    backTrack(save);

    backTrack(save);
    return fails(parent);
}

bool Parser::termProd()
{
    Node *parent = insert(term);
    int save = pos;

    if (factorProd())
    {
        int save = pos;
        while (multiplicativeOperatorProd() && factorProd())
            save = pos;
        backTrack(save);
        return success(parent);
    }
    backTrack(save);
    return fails(parent);
}

bool Parser::factorProd()
{
    Node *parent = insert(factor);
    int save = pos;

    if (match("intcon") || match("realcon") || match("charcon") || match("string") || (match("lparent") && expressionProd() && match("rparent")))
        return success(parent);
    backTrack(save);
    if ((match("notsy") && factorProd()))
        return success(parent);
    backTrack(save);
    if (procedureFunctionCallProd())
        return success(parent);
    backTrack(save);
    if (variableProd())
        return success(parent);
    backTrack(save);
    if (match("ident"))
        return success(parent);
    backTrack(save);
    return fails(parent);
}

bool Parser::relationalOperatorProd()
{
    Node *parent = insert(relational_operator);
    int save = pos;

    if (match("eql"))
        return success(parent);
    backTrack(save);
    if (match("neq"))
        return success(parent);
    backTrack(save);
    if (match("gtr"))
        return success(parent);
    backTrack(save);
    if (match("geq"))
        return success(parent);
    backTrack(save);
    if (match("lss"))
        return success(parent);
    backTrack(save);
    if (match("leq"))
        return success(parent);
    backTrack(save);
    return fails(parent);
}

bool Parser::additiveOperatorProd()
{
    Node *parent = insert(additive_operator);
    int save = pos;

    if (match("plus"))
        return success(parent);
    backTrack(save);
    if (match("minus"))
        return success(parent);
    backTrack(save);
    if (match("orsy"))
        return success(parent);
    backTrack(save);
    return fails(parent);
}

bool Parser::multiplicativeOperatorProd()
{
    Node *parent = insert(multiplicative_operator);
    int save = pos;

    if (match("times"))
        return success(parent);
    backTrack(save);
    if (match("rdiv"))
        return success(parent);
    backTrack(save);
    if (match("idiv"))
        return success(parent);
    backTrack(save);
    if (match("imod"))
        return success(parent);
    backTrack(save);
    if (match("andsy"))
        return success(parent);
    backTrack(save);
    return fails(parent);
}

namespace
{
struct ParsedTreeLine
{
    int depth;
    string label;
};

struct ParsedTree
{
    Node *root = nullptr;
    vector<Token> tokens;
};

string trim(const string &text)
{
    size_t start = text.find_first_not_of(" \t\r\n");
    if (start == string::npos)
        return "";

    size_t end = text.find_last_not_of(" \t\r\n");
    return text.substr(start, end - start + 1);
}

bool consumePrefix(const string &text, size_t &pos, const string &prefix)
{
    if (text.compare(pos, prefix.size(), prefix) != 0)
        return false;

    pos += prefix.size();
    return true;
}

ParsedTreeLine parseTreeLine(const string &line)
{
    ParsedTreeLine parsed{0, ""};

    if (trim(line).empty())
        return parsed;

    size_t pos = 0;
    while (consumePrefix(line, pos, "│   ") ||
           consumePrefix(line, pos, "|   ") ||
           consumePrefix(line, pos, "    "))
    {
        parsed.depth++;
    }

    if (consumePrefix(line, pos, "├── ") ||
        consumePrefix(line, pos, "└── ") ||
        consumePrefix(line, pos, "+-- ") ||
        consumePrefix(line, pos, "`-- "))
    {
        parsed.depth++;
    }

    parsed.label = trim(line.substr(pos));
    return parsed;
}

bool tryNodeTypeFromLabel(const string &label, NodeType &type)
{
    for (int i = 0; i <= terminal; i++)
    {
        if (label == nodeTypeStr[i])
        {
            type = static_cast<NodeType>(i);
            return true;
        }
    }

    return false;
}

bool isNodeLabel(const string &label)
{
    NodeType type;
    return tryNodeTypeFromLabel(label, type);
}

NodeType nodeTypeFromLabel(const string &label)
{
    NodeType type;
    if (tryNodeTypeFromLabel(label, type))
        return type;

    throw runtime_error("Unknown node label: " + label);
}

Token tokenFromLabel(const string &label)
{
    if (label.empty())
        return Token("", "");

    size_t open = label.find(" (");
    if (open == string::npos || label.back() != ')')
        return Token(label, "");

    string type = label.substr(0, open);
    string lexeme = label.substr(open + 2, label.size() - open - 3);

    if (lexeme.size() >= 2 && lexeme.front() == '\'' && lexeme.back() == '\'')
        lexeme = lexeme.substr(1, lexeme.size() - 2);

    return Token(type, lexeme);
}

ParsedTree parseParsedTreeFile(const string &filepath)
{
    ifstream file(filepath);
    ParsedTree parsed;

    if (!file.is_open())
        return parsed;

    vector<pair<int, Node *>> nodeStack;

    string line;
    while (getline(file, line))
    {
        ParsedTreeLine treeLine = parseTreeLine(line);
        if (treeLine.label.empty())
            continue;

        Node *node = nullptr;
        if (isNodeLabel(treeLine.label))
        {
            node = new Node(nodeTypeFromLabel(treeLine.label));
        }
        else
        {
            Token token = tokenFromLabel(treeLine.label);
            parsed.tokens.push_back(token);
            node = new Node(token);
        }

        while (!nodeStack.empty() && nodeStack.back().first >= treeLine.depth)
            nodeStack.pop_back();

        if (nodeStack.empty())
        {
            if (parsed.root != nullptr)
                delete parsed.root;
            parsed.root = node;
        }
        else
        {
            nodeStack.back().second->children.push_back(node);
        }

        nodeStack.push_back({treeLine.depth, node});
    }

    return parsed;
}
}

Parser Parser::buildFromParsedFile(const string& filepath) {
    ParsedTree parsedTree = parseParsedTreeFile(filepath);

    Parser parser(parsedTree.tokens);
    parser.root = parsedTree.root;
    parser.curr = parsedTree.root;
    return parser;
}

vector<Token> Parser::readTokensFromParsedFile(const string& filepath)
{
    ParsedTree parsedTree = parseParsedTreeFile(filepath);
    delete parsedTree.root;

    return parsedTree.tokens;
}
