#include "Node.hpp"

void Node::printTreeToFile(std::string fileName) {
    ofstream file("test/Lexer" + fileName);

    printTree(&file, this, 0);

    file.close();
}

void Node::printTree(std::ostream* stream, Node* root, int depth) {
    for(int i = 0; i < depth; i++) {
        if(i+1 == depth) {
            *stream << "├── ";
        }
        else {
            if(i == 0) {
                *stream << "│   ";
            }
            else {
                *stream << "    ";
            }
        }
    }
    *stream << toString();
    for(Node* child : children) {
        printTree(stream, child, depth+1);
    }
}