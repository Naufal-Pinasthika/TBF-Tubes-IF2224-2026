#include "Node.hpp"

void Node::printTreeToFile(std::string fileName) {
    ofstream file("test/Parser/" + fileName);

    printTree(&file, 0);

    file.close();
}

void Node::printTree(std::ostream* stream, int depth) {
    for(int i = 0; i < depth; i++) {
        if(i+1 == depth) {
            *stream << "├── ";
        }
        else {
            *stream << "│   ";
        }
    }
    *stream << toString() << "\n";
    // cout << toString();
    for(Node* child : children) {
        child->printTree(stream, depth+1);
    }
}