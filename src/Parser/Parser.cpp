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

bool Parser::failed(Node* parent) {
    delete curr;
    curr = parent;
    return false;
}

Node* Parser::backTrack(int save) {
    pos = save;
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
    Node* parent = curr;
    int save = pos;

    Node* node = new Node(program_header);
    curr = node;

    if(match("programsy") && match("ident") && match("semicolon")) return success(parent);

    backTrack(save);
    return failed(parent);
}

bool Parser::declarationPartProd() {
    Node* parent = curr;
    int save = pos;

    Node* node = new Node(declaration_part);
    curr = node;

    while(constDeclarationProd());
    while(typeDeclarationProd());
    while(varDeclarationProd());
    while(subprogramDeclarationProd());

    return success(parent);
}

bool Parser::constDeclarationProd() {
    Node* parent = curr;
    int save = pos;

    Node* node = new Node(const_declaration);
    curr = node;

    if(match("constsy") && match("ident") && match("eql") && constantProd() && match("semicolon")) {
        while(match("ident") && match("eql") && constantProd() && match("semicolon")) save = pos;
        backTrack(save);
        return success(parent);
    }

    backTrack(save);
    return failed(parent);
}

// charcon | string | [(plus | minus)? + (ident | intcon | realcon)]
bool Parser::constantProd() {
    Node* parent = curr;
    int save = pos;

    Node* node = new Node(constant);
    curr = node;

    if(match("charcon")) return success(parent);
    backTrack(save);
    if(match("string")) return success(parent);
    backTrack(save);
    if((match("plus") || match("minus") || true) && (match("ident") || match("intcon") || match("realcon"))) return success(parent);

    backTrack(save);
    return failed(parent);
}

// typesy + (ident + eql + type + semicolon)+
bool Parser::typeDeclarationProd() {
    Node* parent = curr;
    int save = pos;

    Node* node = new Node(type_declaration);
    curr = node;

    if(match("typesy") && match("ident") && match("eql") && typeProd() && match("semicolon")) {
        while(match("ident") && match("eql") && typeProd() && match("semicolon")) save = pos;
        backTrack(save);
        return success(parent);
    }

    backTrack(save);
    return failed(parent);
}

bool Parser::varDeclarationProd() {
    Node* parent = curr;
    int save = pos;

    Node* node = new Node(var_declaration);
    curr = node;



}