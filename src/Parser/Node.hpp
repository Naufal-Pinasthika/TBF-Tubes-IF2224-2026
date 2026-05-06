#pragma once
#include "../Lexer/Lexer.hpp"
#include <iostream>

enum NodeType
{
    program,
    program_header,
    declaration_part,
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
    variable,
    component_variable,
    index_list,
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

const std::string nodeTypeStr[] = {
    "<program>",
    "<program-header>",
    "<declaration-part>",
    "<const-declaration>",
    "<constant>",
    "<type-declaration>",
    "<var-declaration>",
    "<identifier-list>",
    "<type>",
    "<array-type>",
    "<range>",
    "<enumerated>",
    "<record-type>",
    "<field-list>",
    "<field-part>",
    "<subprogram-declaration>",
    "<procedure-declaration>",
    "<function-declaration>",
    "block",
    "<formal-parameter-list>",
    "<parameter-group>",
    "<compound-statement>",
    "<statement-list>",
    "<statement>",
    "<variable>",
    "<component_variable>",
    "<index_list>",
    "<assignment-statement>",
    "<if-statement>",
    "<case-statement>",
    "<case-block>",
    "<while-statement>",
    "<repeat-statement>",
    "<for-statement>",
    "<procedure/function-call>",
    "<parameter-list>",
    "<expression>",
    "<simple-expression>",
    "<term>",
    "<factor>",
    "<relational-operator>",
    "<additive-operator>",
    "<multiplicative-operator>",
    "<terminal>"};

class Node
{
public:
    NodeType type;
    Token token;
    vector<Node *> children;

    Node(NodeType type) : type(type), token(Token("", "")) {}
    Node(Token token) : type(terminal), token(token) {}
    virtual ~Node()
    {
        for (Node *child : children)
        {
            delete child;
        }
    }
    string toString()
    {
        if (type == terminal)
            return token.toString();
        else
            return nodeTypeStr[type];
    }

    void printTreeToFile(std::string fileName);
    void printTree(std::ostream *stream, int depth, bool isLast, vector<bool> isAboveLeaf);
};