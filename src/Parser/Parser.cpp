#include "Parser.hpp"

Token Parser::peek() {
    return tokens[pos];
}

Token Parser::pop() {
    return tokens[pos++];
}

bool Parser::success(Node* parent) {
    parent->children.push_back(curr);
    curr = parent;
    return true;
}

bool Parser::fails(Node* parent) {
    delete curr;
    curr = parent;
    return false;
}

void Parser::backTrack(int save) {
    pos = save;
}

Node* Parser::insert(NodeType type) {
    Node* parent = curr;
    Node* node = new Node(type);
    curr = node;
    return parent;
}

bool Parser::match(string t) {
    if(peek().getType() == t) {
        Node* node = new Node(tokens[pos]);
        pop();
        curr->children.push_back(node);
        return true;
    }

    return false;
}


bool Parser::programProd() {
    curr = new Node(program);
    int save = pos;

    if(programHeaderProd() && declarationPartProd() && compoundStatementProd() && match("period")) return true;
    
    backTrack(save);
    delete curr;
    curr = nullptr;
    return false;
}

// programsy + ident + semicolon
bool Parser::programHeaderProd() {
    Node* parent = insert(program_header);
    int save = pos;

    if(match("programsy") && match("ident") && match("semicolon")) return success(parent);

    backTrack(save);
    return fails(parent);
}

bool Parser::declarationPartProd() {
    Node* parent = insert(declaration_part);
    int save = pos;

    while(constDeclarationProd()) int save = pos;
    backTrack(save);
    while(typeDeclarationProd()) int save = pos;
    backTrack(save);
    while(varDeclarationProd()) int save = pos;
    backTrack(save);
    while(subprogramDeclarationProd()) int save = pos;
    backTrack(save);

    return success(parent);
}

bool Parser::constDeclarationProd() {
    Node* parent = insert(const_declaration);
    int save = pos;

    if(match("constsy") && match("ident") && match("eql") && constantProd() && match("semicolon")) {
        while(match("ident") && match("eql") && constantProd() && match("semicolon")) int save = pos;
        backTrack(save);
        return success(parent);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::constantProd() {
    Node* parent = insert(constant);
    int save = pos;

    if(match("charcon")) return success(parent);
    backTrack(save);
    if(match("string")) return success(parent);
    backTrack(save);
    if((match("plus") || match("minus") || true) && (match("ident") || match("intcon") || match("realcon"))) return success(parent);

    backTrack(save);
    return fails(parent);
}

bool Parser::typeDeclarationProd() {
    Node* parent = insert(type_declaration);
    int save = pos;

    if(match("typesy") && match("ident") && match("eql") && typeProd() && match("semicolon")) {
        while(match("ident") && match("eql") && typeProd() && match("semicolon")) int save = pos;
        backTrack(save);
        return success(parent);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::varDeclarationProd() {
    Node* parent = insert(var_declaration);
    int save = pos;

    if(match("varsy") && identifierListProd() && match("colon") && typeProd() && match("semicolon")) {
        while(identifierListProd() && match("colon") && typeProd() && match("semicolon")) int save = pos;
        backTrack(save);
        return success(parent);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::identifierListProd() {
    Node* parent = insert(identifier_list);
    int save = pos;

    if(match("ident")) {
        while(match("comma") && match("ident")) int save = pos;
        backTrack(save);
        return success(parent);
    }

    backTrack(save);
    return fails(parent);
}

bool Parser::typeProd() {
    
}