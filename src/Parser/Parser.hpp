#pragma once
#include "Node.hpp"

class Parser
{
private:
    vector<Token> tokens;
    vector<string> expected;
    int pos;
    int highestPos;
    Node *root;
    Node *curr;

    // Main function

    // Return token at pos value
    Token peek();
    // "Consume" token by incerementing pos and return token before incrementing
    Token pop();
    // Called when a production succeded, appending the curr node to the parent node and replace curr with parent, always returns true
    bool success(Node *parent);
    // Called when a production fails, replaces curr with parent, always return false
    bool fails(Node *parent);
    // Called when part of a production fails, replaces the pos value to save
    void backTrack(int save);
    // Called at the beginning of a production, creating a node with type specified in the argument, replaces curr with node created, and returns the previous curr
    Node *insert(NodeType type);
    // To check whether the token being read (at pos) have the same name as t
    bool match(string t);

    // Production function

    // program-header + declaration-part + compound-statement + period
    bool programProd();
    // programsy + ident + semicolon
    bool programHeaderProd();
    // (const-declaration)* + (type-declaration)* + (var-declaration)* + (subprogram-declaration)*
    bool declarationPartProd();
    // constsy + (ident + eql + constant + semicolon)+
    bool constDeclarationProd();
    // charcon | string | [(plus | minus)? + (ident | intcon | realcon)]
    bool constantProd();
    // typesy + (ident + eql + type + semicolon)+
    bool typeDeclarationProd();
    // varsy + (identifier-list + colon + type + semicolon)+
    bool varDeclarationProd();
    // ident (comma + ident)*
    bool identifierListProd();
    // ident | array-type | range | enumerated | record-type
    bool typeProd();
    // arraysy + lbrack + (range | ident) + rbrack + ofsy + type
    bool arrayTypeProd();
    // constant + period + period + constant
    bool rangeProd();
    // lparent + ident + (comma + ident)* + rparent
    bool enumeratedProd();
    // recordsy + field-list + endsy
    bool recordTypeProd();
    // field-part + (semicolon + field-part)*
    bool fieldListProd();
    // identifier-list + colon + type
    bool fieldPartProd();
    // procedure-declaration | function-declaration
    bool subprogramDeclarationProd();
    // proceduresy + ident + (formal-parameter-list)? + semicolon + block + semicolon
    bool procedureDeclarationProd();
    // functionsy + ident + (formal-parameter-list)? + colon + ident + semicolon+ block + semicolon
    bool functionDeclarationProd();
    // declaration-part + compound-statement
    bool blockProd();
    // lparent + parameter-group + (semicolon + parameter-group)* + rparent
    bool formalParameterListProd();
    // identifier-list + colon + (ident | array-type)
    bool parameterGroupProd();
    // beginsy + statement-list + endsy
    bool compoundStatementProd();
    // statement (semicolon + statement)*
    bool statementListProd();
    // (assignment-statement | if-statement | case-statement | while-statement | repeat-statement | for-statement | procedure/function-call)?
    bool statementProd();
    // ident + (component-variable)*
    bool variableProd();
    // (lbrack + index-list + rbrack) | (period + ident) 
    bool componentVariableProd();
    // (intcon | charcon | ident) + (comma + index-list)*
    bool indexListProd();
    // variable + becomes + expression
    bool assignmentStatementProd();
    // ifsy + expression + thensy + statement + (elsy + statement)?
    bool ifStatementProd();
    // casesy + expression + ofsy + case-block + endsy
    bool caseStatementProd();
    // constant + (comma + constant)* + colon + statement + (semicolon + case-block?)*
    bool caseBlockProd();
    // whilesy + expression + dosy + statement
    bool whileStatementProd();
    // repeatsy + statement-list + untilsy + expression
    bool repeatStatementProd();
    // forsy + ident + becomes + expression + ( tosy | downtosy) + expression + dosy + statement
    bool forStatementProd();
    // ident + (lparent + parameter-list + rparent)?
    bool procedureFunctionCallProd();
    // expression (comma + expression)*
    bool parameterListProd();
    // simple-expression (relational-operator + simple-expression)?
    bool expressionProd();
    // (plus | minus)? term (additive-operator + term)*
    bool simpleExpressionProd();
    // factor (multiplicative-operator + factor)*
    bool termProd();
    // ident | intcon | charcon | string | (lparent + expression + rparent) | (notsy + factor) | procedure/function-call | variable
    bool factorProd();
    // eql | neq | gtr | geq | lss | leq
    bool relationalOperatorProd();
    // plus | minus | orsy
    bool additiveOperatorProd();
    // times | rdiv | idiv | imod | andsy
    bool multiplicativeOperatorProd();

public:
    Parser(vector<Token> tokens) : tokens(tokens), pos(0), highestPos(0) {}

    bool parse();

    Node *getRoot() const { return root; }
    vector<Token> getTokens() const { return tokens; }
    vector<string> getExpected() const { return expected; }
    int getPos() const { return pos; }
    int getHighestPos() const { return highestPos; }
};