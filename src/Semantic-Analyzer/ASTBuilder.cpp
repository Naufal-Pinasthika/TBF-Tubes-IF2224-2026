#include "ASTBuilder.hpp"

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

// main implementation