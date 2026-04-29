#pragma once
#include "../Lexer/Lexer.hpp"

const enum NodeType {
    program,
    program_header,
    declaration_part,
    compound_statement,
    const_declaration,
    constant,
    type_declaration,
    var_declaration,
    identifier_list,
    type,
    array_type,
    range,
    enumerated,
    record_type,
    field_list,
    field_part,
    subprogram_declaration,
    procedure_declaration,
    function_declaration,
    block,
    formal_parameter_list,
    parameter_group,
    compound_statement,
    statement_list,
    statement,
    assignment_statement,
    if_statement,
    case_statement,
    case_block,
    while_statement,
    repeat_statement,
    for_statement,
    procedure_function_call,
    parameter_list,
    expression,
    simple_expression,
    term,
    factor,
    relational_operator,
    additive_operator,
    multiplicative_operator,
    terminal
};

vector<Token> tokens;
Token next;

class Node {
public:
    NodeType type;
    Token token;
    vector<Node*> children;

    Node(NodeType type): type(type), token(Token("","")) {}
    Node(Token token): type(terminal), token(token) {}
    virtual ~Node() {
        for(Node* child : children) {
            delete child;
        }
    }
};