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
        do save = pos;
        while (match("ident") && match("eql") && constantProd() && match("semicolon"));
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
        do int save = pos;
        while (match("ident") && match("eql") && typeProd() && match("semicolon"));
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
        do int save = pos;
        while (identifierListProd() && match("colon") && typeProd() && match("semicolon"));
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
        do int save = pos;
        while (match("comma") && match("ident"));
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

// constant + period + period + constant
bool Parser::rangeProd() {
    Node *parent = insert(array_type);
    int save = pos;

    if (constantProd() && match("period") && match("period") && constantProd()) 
        return success(parent);

    backTrack(save);
    return fails(parent);
}
// lparent + ident + (comma + ident)* + rparent
bool Parser::enumeratedProd() {
    Node *parent = insert(array_type);
    int save = pos;

    if (match("lparent") && match("ident")) {
        int save2 = pos;
        while (match("comma") && match("ident"))
            save2 = pos;
        backTrack(save2);
        if (match("rparent")) return success(parent);
    }
    
    backTrack(save);
    return fails(parent);
}
// recordsy + field-list + endsy
bool Parser::recordTypeProd() {
    Node *parent = insert(array_type);
    int save = pos;

    if (match("recordsy") && fieldListProd() && match("endsy")) 
        return success(parent);
    
    backTrack(save);
    return fails(parent);
}
// field-part + (semicolon + field-part)*
bool Parser::fieldListProd() {
    Node *parent = insert(array_type);
    int save = pos;

    if (fieldPartProd()) {
        int save2 = pos;
        while (match("semicolon") && fieldPartProd())
            save2 = pos;
        backTrack(save2);
        return success(parent);
    }
    
    backTrack(save);
    return fails(parent);
}
// identifier-list + colon + type
bool Parser::fieldPartProd() {
    Node *parent = insert(array_type);
    int save = pos;

    if (identifierListProd() && match("colon") && typeProd()) 
        return success(parent);
    
    backTrack(save);
    return fails(parent);
}
// procedure-declaration | function-declaration
bool Parser::subprogramDeclarationProd() {
    Node *parent = insert(array_type);
    int save = pos;

    if (procedureDeclarationProd() || functionDeclarationProd())
        return success(parent);

    backTrack(save);
    return fails(parent);
}
// proceduresy + ident + (formal-parameter-list)? + semicolon + block + semicolon
bool Parser::procedureDeclarationProd() {
    Node *parent = insert(array_type);
    int save = pos;

    if (match("proceduresy") && match("ident")) {
        int save2 = pos;
        if (formalParameterListProd()) {
            if (match("semicolon") && blockProd() && match("semicolon"))
                return success(parent);
        }
        backTrack(save2);
    }
    
    backTrack(save);
    return fails(parent);
}
// functionsy + ident + (formal-parameter-list)? + colon + ident + semicolon+ block + semicolon
bool Parser::functionDeclarationProd() {
    Node *parent = insert(array_type);
    int save = pos;

    if (match("functionsy") && match("ident")) {
        int save2 = pos;
        if (formalParameterListProd()) {
            if (match("colon") && match("ident") && match("semicolon") && blockProd() && match("semicolon"))
                return success(parent);
        }
        backTrack(save2);
    }
    
    backTrack(save);
    return fails(parent);
}
// declaration-part + compound-statement
bool Parser::blockProd() {
    Node *parent = insert(array_type);
    int save = pos;

    if (declarationPartProd() && compoundStatementProd()) 
        return success(parent);
    
    backTrack(save);
    return fails(parent);
}
// lparent + parameter-group + (semicolon + parameter-group)* + rparent
bool Parser::formalParameterListProd() {
    Node *parent = insert(array_type);
    int save = pos;

    if (match("lparent") && parameterGroupProd()) {
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

bool Parser::parameterGroupProd(){
    Node *parent = insert(parameter_group);
    int save = pos;

    if(identifierListProd() && match("colon") && (match("ident") || arrayTypeProd())){
        return success(parent);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::compoundStatementProd(){
    Node *parent = insert(compound_statement);
    int save = pos;

    if(match("beginsy") && statementListProd() && match("endsy")){
        return success(parent);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::statementListProd(){
    Node *parent = insert(statement_list);
    int save = pos;

    if(statementProd()){
        int save2 = pos;
        while(match("semicolon") && statementProd()){
            save2 = pos;
        }
        backTrack(save2);
        return success(parent);
    }
    backTrack(save);
    return false;
}

bool Parser::statementProd(){
    Node *parent = insert(statement);
    int save = pos;

    if((assignmentStatementProd() || ifStatementProd() || caseStatementProd || whileStatementProd || repeatStatementProd || forStatementProd() || procedureFunctionCallProd()) || true){
        return success(parent);
    }
    backTrack(save);
    return fails(parent);
}

bool Parser::assignmentStatementProd(){
    Node *parent = insert(assignment_statement);
    int save = pos;

    if(variableProd() && match("becomes") && expressionProd()){
        return success(parent);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::ifStatementProd(){
    Node *parent = insert(if_statement);
    int save = pos;

    if(match("ifsy") && expressionProd() && match("thensy") && statementProd() && ((match("elsy") && statementProd()) || true)){
        return success(parent);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::caseStatementProd(){
    Node *parent = insert(case_statement);
    int save = pos;

    if(match("casesy") && expressionProd() && match("ofsy") && caseBlockProd() && match("endsy")){
        return success(parent);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::caseBlockProd(){
    Node *parent = insert(case_block);
    int save = pos;

    if (constantProd()) {
        int save2 = pos;

        while (match("comma") && constantProd()) {
            save2 = pos;
        }
        backTrack(save2);

        if (match("colon") && statementProd()) {
            save2 = pos;

            while (match("semicolon")) {
                int afterSemicolon = pos;

                if (caseBlockProd()) {
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

bool Parser::whileStatementProd(){
    Node *parent = insert(while_statement);
    int save = pos;

    if(match("whensy") && expressionProd() &&  match("dosy") && statementProd()){
        return success(parent);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::repeatStatementProd(){
    Node *parent = insert(repeat_statement);
    int save = pos;

    if(match("repeatsy") && statementListProd() && match("untilsy") && expressionProd()){
        return success(parent);
    }

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


