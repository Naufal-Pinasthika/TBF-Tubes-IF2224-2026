#include "Parser.hpp"

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
    if (peek().getType() == t)
    {
        Node *node = new Node(tokens[pos]);
        pop();
        curr->children.push_back(node);
        return true;
    }

    return false;
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
        int save = pos;
    backTrack(save);
    while (typeDeclarationProd())
        int save = pos;
    backTrack(save);
    while (varDeclarationProd())
        int save = pos;
    backTrack(save);
    while (subprogramDeclarationProd())
        int save = pos;
    backTrack(save);

    return success(parent);
}

bool Parser::constDeclarationProd()
{
    Node *parent = insert(const_declaration);
    int save = pos;

    if (match("constsy") && match("ident") && match("eql") && constantProd() && match("semicolon"))
    {
        while (match("ident") && match("eql") && constantProd() && match("semicolon"))
            int save = pos;
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
    if ((match("plus") || match("minus") || true) && (match("ident") || match("intcon") || match("realcon")))
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
        while (match("ident") && match("eql") && typeProd() && match("semicolon"))
            int save = pos;
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
        while (identifierListProd() && match("colon") && typeProd() && match("semicolon"))
            int save = pos;
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
        while (match("comma") && match("ident"))
            int save = pos;
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

bool Parser::forStatementProd()
{
    Node *parent = insert(for_statement);
    int save = pos;

    if (match("forsy") && match("ident") && match("becomes") && expressionProd() && (match("tosy") || match("downtosy")) && expressionProd() && match("dosy") && statementProd()) {
        return success(parent);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::procedureFunctionCallProd() 
{
    Node *parent = insert(procedure_function_call);
    int save = pos;

    if (match("ident") && ((match("lparent") && parameterListProd() && match("rparent")) || true )) {
        return success(parent);
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
            int save = pos;
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
        if ((relationalOperatorProd() && simpleExpressionProd()) || true ) {
            int save = pos;
            backTrack(save);
            return success(parent);
        }
        backTrack(save);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::simpleExpressionProd() 
{ 
    Node *parent = insert(simple_expression);
    int save = pos;

    if ((match("plus") || match("minus")) || true) {
        int save = pos;
        if (termProd())
        {
            while (additiveOperatorProd() && termProd())
                int save = pos;
            backTrack(save);
            return success(parent);
        }
        backTrack(save);
    }

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
            int save = pos;
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

    if (match("ident"))
        return success(parent);
    backTrack(save);
    if (match("intcon"))
        return success(parent);
    backTrack(save);
    if (match("charcon"))
        return success(parent);
    backTrack(save);
    if (match("string"))
        return success(parent);
    backTrack(save); 

    if (match("lparent") && expressionProd() && match("rparent"))
        return success(parent);
    backTrack(save);     
    
    if (match("notsy") && factorProd())
        return success(parent);
    backTrack(save);     
    if (procedureFunctionCallProd()) 
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
    if (match("neql"))
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


