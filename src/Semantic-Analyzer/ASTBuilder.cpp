#include "ASTBuilder.hpp"

static bool hasToken(Node* node, const string& type)
{
    if (node == nullptr)
    {
        return false;
    }

    for (Node* child : node->children)
    {
        if (child != nullptr && child->type == terminal && child->token.getType() == type)
        {
            return true;
        }
    }

    return false;
}

static bool hasTokenRecursive(Node* node, const string& type)
{
    if (node == nullptr)
    {
        return false;
    }

    if (node->type == terminal && node->token.getType() == type)
    {
        return true;
    }

    for (Node* child : node->children)
    {
        if (hasTokenRecursive(child, type))
        {
            return true;
        }
    }

    return false;
}

static string operatorFromToken(const string& type, const string& lexeme)
{
    // Canonicalize by token type first.
    // This keeps source mode and *_tokenize mode consistent.
    if (type == "plus")  return "+";
    if (type == "minus") return "-";
    if (type == "times") return "*";
    if (type == "rdiv")  return "/";
    if (type == "idiv")  return "div";
    if (type == "imod")  return "mod";
    if (type == "andsy") return "and";
    if (type == "orsy")  return "or";
    if (type == "notsy") return "not";

    if (type == "eql") return "==";
    if (type == "neq") return "<>";
    if (type == "gtr") return ">";
    if (type == "geq") return ">=";
    if (type == "lss") return "<";
    if (type == "leq") return "<=";

    return !lexeme.empty() ? lexeme : type;
}

//helper function
bool ASTBuilder::isTerminal(Node* node) const {
    return node != nullptr && node->type == terminal;
}

bool ASTBuilder::isToken(Node* node, const string& type) const {
    return isTerminal(node) && node->token.getType() == type;
}

string ASTBuilder::lexeme(Node* node) const {
    if (!isTerminal(node)) return "";
    return node->token.getLexeme();
}

Node* ASTBuilder::findChild(Node* node, NodeType type) const {
    if (node == nullptr) return nullptr;

    for (Node* child : node->children) {
        if (child != nullptr && child->type == type) {
            return child;
        }
    }

    return nullptr;
}

vector<Node*> ASTBuilder::findChildren(Node* node, NodeType type) const {
    vector<Node*> result;
    if (node == nullptr) return result;

    for (Node* child : node->children) {
        if (child != nullptr && child->type == type) {
            result.push_back(child);
        }
    }

    return result;
}

void ASTBuilder::addError(const string& message) {
    errors.push_back("ASTBuilder: " + message);
}

bool ASTBuilder::hasErrors() const {
    return !errors.empty();
}

const vector<string>& ASTBuilder::getErrors() const {
    return errors;
}

string ASTBuilder::getFirstIdent(Node* node) const {
    if (node == nullptr) {
        return "";
    }

    if (isToken(node, "ident")) {
        return lexeme(node);
    }

    for (Node* child : node->children) {
        string found = getFirstIdent(child);
        if (!found.empty()) {
            return found;
        }
    }

    return "";
}

Node* ASTBuilder::getFirstTerminal(Node* node) const {
    if (node == nullptr) return nullptr;
    if (isTerminal(node)) return node;
    if (node->children.empty()) return nullptr;

    return getFirstTerminal(node->children[0]);
}

string ASTBuilder::getOperator(Node* node) const {
    if (node == nullptr) {
        return "";
    }

    if (isTerminal(node)) {
        return operatorFromToken(node->token.getType(), node->token.getLexeme());
    }

    for (Node* child : node->children) {
        if (child != nullptr && child->type == terminal) {
            return operatorFromToken(child->token.getType(), child->token.getLexeme());
        }
    }

    return "";
}

void ASTBuilder::assignPosition(ASTNode* astNode, Node* parseNode) {
    Node* firstTerminal = getFirstTerminal(parseNode);
    if (firstTerminal == nullptr || astNode == nullptr) {
        return;
    }
    astNode->line = firstTerminal->token.getRow();
    astNode->column = firstTerminal->token.getCol();
}

// main implementation
ProgramNode* ASTBuilder::build(Node* parseTree) {
    errors.clear();

    if (parseTree == nullptr) {
        addError("parse tree is null");
        return nullptr;
    }

    if (parseTree->type != program) {
        addError("root node is not <program>");
        return nullptr;
    }

    return buildProgram(parseTree);
}

ProgramNode* ASTBuilder::buildProgram(Node* node) {
    Node* headerNode = findChild(node, program_header);
    Node* declNode = findChild(node, declaration_part);
    Node* bodyNode = findChild(node, compound_statement);

    string name = getFirstIdent(headerNode);
    ProgramNode* programNode = new ProgramNode(name);

    if (declNode != nullptr) {
        programNode->declarations = buildDeclarationPart(declNode);
    }

    if (bodyNode != nullptr) {
        programNode->body = buildCompoundStatement(bodyNode);
    }

    assignPosition(programNode, node);

    return programNode;
}

vector<DeclarationNode*> ASTBuilder::buildDeclarationPart(Node* node) {
    vector<DeclarationNode*> result;
    if (node == nullptr) {
        return result;
    }

    for (Node* child : node->children) {
        vector<DeclarationNode*> declarations;

        if (child->type == const_declaration) {
            declarations = buildConstDeclaration(child);
        } else if (child->type == type_declaration) {
            declarations = buildTypeDeclaration(child);
        } else if (child->type == var_declaration) {
            declarations = buildVarDeclaration(child);
        } else if (child->type == subprogram_declaration ||
                   hasTokenRecursive(child, "proceduresy") ||
                   hasTokenRecursive(child, "functionsy")) {
            DeclarationNode* declaration = buildSubprogramDeclaration(child);
            if (declaration != nullptr) {
                result.push_back(declaration);
            }
        }

        result.insert(result.end(), declarations.begin(), declarations.end());
    }

    return result;
}

vector<DeclarationNode*> ASTBuilder::buildConstDeclaration(Node* node) {
    vector<DeclarationNode*> result;
    string name = "";

    for (Node* child : node->children) {
        if (isToken(child, "ident")) {
            name = lexeme(child);
        } else if (child->type == constant && !name.empty()) {
            result.push_back(new ConstDeclNode(name, buildConstant(child)));
            name = "";
        }
    }

    return result;
}

vector<DeclarationNode*> ASTBuilder::buildTypeDeclaration(Node* node) {
    vector<DeclarationNode*> result;
    string name = "";

    for (Node* child : node->children) {
        if (isToken(child, "ident")) {
            name = lexeme(child);
        } else if (child->type == type && !name.empty()) {
            result.push_back(new TypeDeclNode(name, buildType(child)));
            name = "";
        }
    }

    return result;
}

vector<DeclarationNode*> ASTBuilder::buildVarDeclaration(Node* node) {
    vector<DeclarationNode*> result;
    vector<string> names;

    for (Node* child : node->children) {
        if (child->type == identifier_list) {
            names = buildIdentifierList(child);
        } else if (child->type == type && !names.empty()) {
            result.push_back(new VarDeclNode(names, buildType(child)));
            names.clear();
        }
    }

    return result;
}

DeclarationNode* ASTBuilder::buildSubprogramDeclaration(Node* node) {
    if (node == nullptr) {
        return nullptr;
    }

    if (hasToken(node, "proceduresy")) {
        return buildProcedureDeclaration(node);
    }
    if (hasToken(node, "functionsy")) {
        return buildFunctionDeclaration(node);
    }

    for (Node* child : node->children) {
        if (child != nullptr && child->type != terminal &&
            (hasTokenRecursive(child, "proceduresy") || hasTokenRecursive(child, "functionsy"))) {
            return buildSubprogramDeclaration(child);
        }
    }

    addError("failed to build subprogram declaration");
    return nullptr;
}

ProcedureDeclNode* ASTBuilder::buildProcedureDeclaration(Node* node) {
    ProcedureDeclNode* procedure = new ProcedureDeclNode(getFirstIdent(node));
    Node* parameterNode = nullptr;
    Node* blockNode = nullptr;

    for (Node* child : node->children) {
        if (child == nullptr || child->type == terminal) {
            continue;
        }

        if (parameterNode == nullptr &&
            (child->type == formal_parameter_list || hasToken(child, "lparent"))) {
            parameterNode = child;
        } else if (blockNode == nullptr &&
                   (child->type == block ||
                    (findChild(child, declaration_part) != nullptr &&
                     findChild(child, compound_statement) != nullptr))) {
            blockNode = child;
        }
    }

    if (parameterNode != nullptr) {
        procedure->parameters = buildFormalParameterList(parameterNode);
    }

    if (blockNode != nullptr) {
        procedure->declarations = buildDeclarationPart(findChild(blockNode, declaration_part));
        procedure->body = buildCompoundStatement(findChild(blockNode, compound_statement));
    }

    assignPosition(procedure, node);

    return procedure;
}

FunctionDeclNode* ASTBuilder::buildFunctionDeclaration(Node* node) {
    string name;
    TypeNode* returnType = nullptr;
    bool afterColon = false;
    Node* parameterNode = nullptr;
    Node* blockNode = nullptr;

    for (Node* child : node->children) {
        if (isToken(child, "ident")) {
            if (name.empty()) {
                name = lexeme(child);
            } else if (afterColon && returnType == nullptr) {
                returnType = new NamedTypeNode(lexeme(child));
            }
        } else if (isToken(child, "colon")) {
            afterColon = true;
        } else if (child != nullptr && child->type != terminal) {
            if (parameterNode == nullptr &&
                (child->type == formal_parameter_list || hasToken(child, "lparent"))) {
                parameterNode = child;
            } else if (blockNode == nullptr &&
                       (child->type == block ||
                        (findChild(child, declaration_part) != nullptr &&
                         findChild(child, compound_statement) != nullptr))) {
                blockNode = child;
            }
        }
    }

    FunctionDeclNode* function = new FunctionDeclNode(name, returnType);

    if (parameterNode != nullptr) {
        function->parameters = buildFormalParameterList(parameterNode);
    }

    if (blockNode != nullptr) {
        function->declarations = buildDeclarationPart(findChild(blockNode, declaration_part));
        function->body = buildCompoundStatement(findChild(blockNode, compound_statement));
    }

    assignPosition(function, node);

    return function;
}

vector<string> ASTBuilder::buildIdentifierList(Node* node) {
    vector<string> result;
    if (node == nullptr) {
        return result;
    }

    for (Node* child : node->children) {
        if (isToken(child, "ident")) {
            result.push_back(lexeme(child));
        }
    }

    return result;
}

ExpressionNode* ASTBuilder::buildConstant(Node* node) {
    ExpressionNode* result = nullptr;
    string sign = "";

    for (Node* child : node->children) {
        if (isToken(child, "minus")) {
            sign = "-";
        } else if (isToken(child, "plus")) {
            sign = "";
        } else if (isToken(child, "intcon")) {
            result = new NumberNode(sign + lexeme(child), false);
        } else if (isToken(child, "realcon")) {
            result = new NumberNode(sign + lexeme(child), true);
        } else if (isToken(child, "charcon")) {
            result = new CharNode(lexeme(child));
        } else if (isToken(child, "string")) {
            result = new StringNode(lexeme(child));
        } else if (isToken(child, "ident")) {
            string name = lexeme(child);
            if (name == "true" || name == "TRUE") {
                result = new BoolNode(true);
            }
            if (name == "false" || name == "FALSE") {
                result = new BoolNode(false);
            }
            result = new VarNode(name);
        }
    }

    assignPosition(result, node);

    if (result == nullptr) {
        addError("failed to build constant");
    }
    
    return result;
}

TypeNode* ASTBuilder::buildType(Node* node) {
    if (node == nullptr) {
        return nullptr;
    }
    
    if (hasToken(node, "arraysy")) {
        return buildArrayType(node);
    }
    if (hasToken(node, "recordsy")) {
        return buildRecordType(node);
    }
    if (hasToken(node, "lparent")) {
        return buildEnumeratedType(node);
    }
    if (hasToken(node, "period")) {
        return buildRangeType(node);
    }
    
    for (Node* child : node->children) {
        if (isToken(child, "ident")) {
            TypeNode* result = new NamedTypeNode(lexeme(child));
            assignPosition(result, node);
            return result;
        }
    }
    for (Node* child : node->children) {
        if (child->type == array_type || child->type == range ||
            child->type == enumerated || child->type == record_type) {
            return buildType(child);
        }
    }

    addError("failed to build type");
    return nullptr;
}

TypeNode* ASTBuilder::buildArrayType(Node* node) {
    TypeNode* indexType = nullptr;
    TypeNode* elementType = nullptr;

    for (Node* child : node->children) {
        if (isToken(child, "ident") && indexType == nullptr) {
            indexType = new NamedTypeNode(lexeme(child));
            assignPosition(indexType, child);
        } else if ((child->type == array_type || child->type == range) && indexType == nullptr) {
            indexType = buildType(child);
        } else if (child->type == type) {
            elementType = buildType(child);
        }
    }

    TypeNode* result = new ArrayTypeNode(indexType, elementType);
    assignPosition(result, node);
    return result;
}

TypeNode* ASTBuilder::buildRangeType(Node* node) {
    vector<Node*> constants = findChildren(node, constant);
    ExpressionNode* low = constants.size() > 0 ? buildConstant(constants[0]) : nullptr;
    ExpressionNode* high = constants.size() > 1 ? buildConstant(constants[1]) : nullptr;

    TypeNode* result = new RangeTypeNode(low, high);
    assignPosition(result, node);
    return result;
}

TypeNode* ASTBuilder::buildEnumeratedType(Node* node) {
    vector<string> values;

    for (Node* child : node->children) {
        if (isToken(child, "ident")) {
            values.push_back(lexeme(child));
        }
    }

    TypeNode* result = new EnumeratedTypeNode(values);
    assignPosition(result, node);
    return result;
}

TypeNode* ASTBuilder::buildRecordType(Node* node) {
    RecordTypeNode* record = new RecordTypeNode();
    vector<Node*> stack;
    stack.push_back(node);

    while (!stack.empty()) {
        Node* current = stack.back();
        stack.pop_back();

        Node* ids = findChild(current, identifier_list);
        Node* typeNode = findChild(current, type);
        if (ids != nullptr && typeNode != nullptr) {
            VarDeclNode* varDecl = new VarDeclNode(buildIdentifierList(ids), buildType(typeNode));
            assignPosition(varDecl, current);
            record->fields.push_back(varDecl);
        }

        for (Node* child : current->children) {
            if (child != nullptr && child->type != terminal) {
                stack.push_back(child);
            }
        }
    }

    assignPosition(record, node);

    return record;
}

StatementNode* ASTBuilder::buildCompoundStatement(Node* node) {
    CompoundNode* compound = new CompoundNode();
    Node* statements = findChild(node, statement_list);

    if (statements != nullptr) {
        compound->statements = buildStatementList(statements);
    }

    assignPosition(compound, node);

    return compound;
}

vector<StatementNode*> ASTBuilder::buildStatementList(Node* node) {
    vector<StatementNode*> result;

    for (Node* child : node->children) {
        if (child->type == statement) {
            StatementNode* statementNode = buildStatement(child);
            if (statementNode != nullptr) {
                result.push_back(statementNode);
            }
        }
    }

    return result;
}

StatementNode* ASTBuilder::buildStatement(Node* node) {
    if (node == nullptr) {
        return nullptr;
    }

    for (Node* child : node->children) {
        if (child->type == assignment_statement) {
            return buildAssignment(child);
        }
        if (child->type == if_statement) {
            return buildIf(child);
        }
        if (child->type == case_statement) {
            return buildCase(child);
        }
        if (child->type == while_statement) {
            return buildWhile(child);
        }
        if (child->type == repeat_statement) {
            return buildRepeat(child);
        }
        if (child->type == for_statement) {
            return buildFor(child);
        }
        if (child->type == procedure_function_call) {
            return buildCallStatement(child);
        }
    }

    return nullptr;
}

StatementNode* ASTBuilder::buildAssignment(Node* node) {
    StatementNode* result = new AssignNode(buildVariable(findChild(node, variable)),
                          buildExpression(findChild(node, expression)));
    assignPosition(result, node);
    return result;
}

StatementNode* ASTBuilder::buildIf(Node* node) {
    vector<Node*> statements = findChildren(node, statement);

    StatementNode* thenBranch = statements.size() > 0 ? buildStatement(statements[0]) : nullptr;
    StatementNode* elseBranch = statements.size() > 1 ? buildStatement(statements[1]) : nullptr;

    StatementNode* result = new IfNode(buildExpression(findChild(node, expression)), thenBranch, elseBranch);
    assignPosition(result, node);
    return result;
}

StatementNode* ASTBuilder::buildWhile(Node* node) {
    StatementNode* result = new WhileNode(buildExpression(findChild(node, expression)),
                         buildCompoundStatement(findChild(node, compound_statement)));
    assignPosition(result, node);
    return result;
}

StatementNode* ASTBuilder::buildFor(Node* node) {
    vector<Node*> expressions = findChildren(node, expression);
    string variableName = getFirstIdent(node);
    bool isDownto = hasToken(node, "downtosy");

    ExpressionNode* start = expressions.size() > 0 ? buildExpression(expressions[0]) : nullptr;
    ExpressionNode* stop = expressions.size() > 1 ? buildExpression(expressions[1]) : nullptr;

    StatementNode* result = new ForNode(variableName, start, stop, isDownto,
                       buildCompoundStatement(findChild(node, compound_statement)));
    assignPosition(result, node);
    return result;
}

StatementNode* ASTBuilder::buildRepeat(Node* node) {
    vector<StatementNode*> statements = buildStatementList(findChild(node, statement_list));
    ExpressionNode* condition = buildExpression(findChild(node, expression));

    StatementNode* result = new RepeatNode(statements, condition);
    assignPosition(result, node);
    return result;
}

StatementNode* ASTBuilder::buildCase(Node* node) {
    ExpressionNode* selector = buildExpression(findChild(node, expression));
    vector<CaseBranch*> branches = buildCaseBlock(findChild(node, case_block));

    StatementNode* result = new CaseNode(selector, branches);
    assignPosition(result, node);
    return result;
}

vector<CaseBranch*> ASTBuilder::buildCaseBlock(Node* node) {
    vector<CaseBranch*> result;
    if (node == nullptr) {
        return result;
    }

    vector<ExpressionNode*> labels;
    StatementNode* branchStatement = nullptr;

    for (Node* child : node->children) {
        if (child->type == constant && branchStatement == nullptr) {
            labels.push_back(buildConstant(child));
        } else if (child->type == statement && branchStatement == nullptr) {
            branchStatement = buildStatement(child);
        } else if (child->type == case_block) {
            vector<CaseBranch*> nextBranches = buildCaseBlock(child);
            result.insert(result.end(), nextBranches.begin(), nextBranches.end());
        }
    }

    if (!labels.empty() || branchStatement != nullptr) {
        result.insert(result.begin(), new CaseBranch(labels, branchStatement));
    }

    return result;
}

StatementNode* ASTBuilder::buildCallStatement(Node* node) {
    StatementNode* result = new CallNode(getFirstIdent(node), buildParameterList(findChild(node, parameter_list)));
    assignPosition(result, node);
    return result;
}

vector<VarDeclNode*> ASTBuilder::buildFormalParameterList(Node* node) {
    vector<VarDeclNode*> result;
    if (node == nullptr) {
        return result;
    }

    for (Node* child : node->children) {
        if (child->type == parameter_group) {
            vector<VarDeclNode*> group = buildParameterGroup(child);
            result.insert(result.end(), group.begin(), group.end());
        }
    }

    return result;
}

vector<VarDeclNode*> ASTBuilder::buildParameterGroup(Node* node) {
    vector<VarDeclNode*> result;
    Node* identifiers = findChild(node, identifier_list);

    if (identifiers == nullptr) {
        return result;
    }

    TypeNode* parameterType = nullptr;

    for (Node* child : node->children) {
        if (child->type == type) {
            parameterType = buildType(child);
        } else if (child->type == array_type) {
            parameterType = buildArrayType(child);
        } else if (isToken(child, "ident")) {
            parameterType = new NamedTypeNode(lexeme(child));
            assignPosition(parameterType, child);
        }
    }

    VarDeclNode* parameter = new VarDeclNode(buildIdentifierList(identifiers), parameterType);
    assignPosition(parameter, node);
    parameter->isParameter = true;
    result.push_back(parameter);

    return result;
}

// L-attributed grammar
ExpressionNode* ASTBuilder::buildExpression(Node* node) {
    vector<Node*> simples = findChildren(node, simple_expression);

    if (simples.empty()) return nullptr;

    ExpressionNode* left = buildSimpleExpression(simples[0]);

    if (simples.size() == 1) {
        return left;
    }

    Node* opNode = findChild(node, relational_operator);
    string op = getOperator(opNode);

    ExpressionNode* right = buildSimpleExpression(simples[1]);
    ExpressionNode* result = new BinOpNode(op, left, right);
    assignPosition(result, node);
    return result;
}

ExpressionNode* ASTBuilder::buildSimpleExpression(Node* node) {
    vector<ExpressionNode*> terms;
    vector<string> ops;

    bool unaryMinus = false;

    for (Node* child : node->children) {
        if (isToken(child, "minus")) {
            if (terms.empty()) unaryMinus = true;
        } else if (child->type == term) {
            terms.push_back(buildTerm(child));
        } else if (child->type == additive_operator) {
            ops.push_back(getOperator(child));
        }
    }

    if (terms.empty()) return nullptr;

    ExpressionNode* result = terms[0];

    if (unaryMinus) {
        result = new UnaryOpNode("-", result);
    }

    for (size_t i = 1; i < terms.size(); i++) {
        result = new BinOpNode(ops[i - 1], result, terms[i]);
    }

    assignPosition(result, node);
    return result;
}

ExpressionNode* ASTBuilder::buildTerm(Node* node) {
    vector<ExpressionNode*> factors;
    vector<string> ops;

    for (Node* child : node->children) {
        if (child->type == factor) {
            factors.push_back(buildFactor(child));
        } else if (child->type == multiplicative_operator) {
            ops.push_back(getOperator(child));
        }
    }

    if (factors.empty()) return nullptr;

    ExpressionNode* result = factors[0];

    for (size_t i = 1; i < factors.size(); i++) {
        result = new BinOpNode(ops[i - 1], result, factors[i]);
    }

    assignPosition(result, node);
    return result;
}

ExpressionNode* ASTBuilder::buildFactor(Node* node) {
    ExpressionNode* result = nullptr;
    for (Node* child : node->children) {
        if (isToken(child, "intcon")) {
            result = new NumberNode(lexeme(child), false);
        }

        if (isToken(child, "realcon")) {
            result = new NumberNode(lexeme(child), true);
        }

        if (isToken(child, "string")) {
            result = new StringNode(lexeme(child));
        }

        if (isToken(child, "charcon")) {
            result = new CharNode(lexeme(child));
        }

        if (isToken(child, "ident")) {
            string name = lexeme(child);
            if (name == "true" || name == "TRUE") return new BoolNode(true);
            if (name == "false" || name == "FALSE") return new BoolNode(false);
            result = new VarNode(name);
        }

        if (result != nullptr) {
            assignPosition(result, child);
            return result;
        }

        if (child->type == expression) {
            return buildExpression(child);
        }

        if (child->type == procedure_function_call) {
            return buildCallExpression(child);
        }

        if (child->type == variable) {
            return buildVariable(child);
        }

        if (child->type == factor) {
            result = new UnaryOpNode("not", buildFactor(child));
            assignPosition(result, child);
            return result;
        }
    }

    addError("failed to build factor");
    return nullptr;
}

ExpressionNode* ASTBuilder::buildVariable(Node* node) {
    if (node == nullptr) {
        return nullptr;
    }

    ExpressionNode* result = nullptr;

    for (Node* child : node->children) {
        if (isToken(child, "ident") && result == nullptr) {
            result = new VarNode(lexeme(child));
        } else if (child->type == component_variable && result != nullptr) {
            Node* indexList = findChild(child, index_list);
            if (indexList != nullptr) {
                vector<ExpressionNode*> indexes = buildIndexList(indexList);
                for (ExpressionNode* index : indexes) {
                    result = new ArrayAccessNode(result, index);
                }
            } else {
                string field = getFirstIdent(child);
                if (!field.empty()) {
                    result = new RecordAccessNode(result, field);
                }
            }
        }
    }

    assignPosition(result, node);
    return result;
}

FunctionCallNode* ASTBuilder::buildCallExpression(Node* node) {
    FunctionCallNode* result = new FunctionCallNode(getFirstIdent(node), buildParameterList(findChild(node, parameter_list)));
    assignPosition(result, node);
    return result;
}

vector<ExpressionNode*> ASTBuilder::buildParameterList(Node* node) {
    vector<ExpressionNode*> result;
    if (node == nullptr) {
        return result;
    }

    for (Node* child : node->children) {
        if (child->type == expression) {
            result.push_back(buildExpression(child));
        }
    }

    return result;
}

vector<ExpressionNode*> ASTBuilder::buildIndexList(Node* node) {
    vector<ExpressionNode*> result;
    if (node == nullptr) {
        return result;
    }

    for (Node* child : node->children) {
        if (isToken(child, "intcon")) {
            NumberNode* numberNode = new NumberNode(lexeme(child), false);
            assignPosition(numberNode, child);
            result.push_back(numberNode);
        } else if (isToken(child, "charcon")) {
            CharNode* charNode = new CharNode(lexeme(child));
            assignPosition(charNode, child);
            result.push_back(charNode);
        } else if (isToken(child, "ident")) {
            VarNode* varNode = new VarNode(lexeme(child));
            assignPosition(varNode, child);
            result.push_back(varNode);
        } else if (child->type == index_list) {
            vector<ExpressionNode*> nested = buildIndexList(child);
            result.insert(result.end(), nested.begin(), nested.end());
        }
    }

    return result;
}
