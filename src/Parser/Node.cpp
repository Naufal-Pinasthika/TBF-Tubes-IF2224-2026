#include "Node.hpp"

void Node::printTreeToFile(std::string fileName) {
    ofstream file("test/Parser/" + fileName);

    vector<bool> isAboveLeaf;
    printTree(&file, 0, true, isAboveLeaf);

    file.close();
}

void Node::printTree(std::ostream* stream, int depth, bool isLast, vector<bool> isAboveLeaf) {
    while (isAboveLeaf.size() < (size_t) depth) isAboveLeaf.push_back(isLast);
    for(int i = 0; i < depth; i++) {
        if(i+1 == depth) {
            if(!isLast) *stream << "├── ";
            else *stream << "└── ";
        }
        else {
            if(!isAboveLeaf.at(i)) *stream << "│   ";
            else *stream << "    ";
        }
    }
    *stream << toString() << "\n";
    // cout << toString();
    for(size_t i = 0; i < children.size(); i++) {
        if(i == children.size() - 1) isLast = true;
        else isLast = false;
        children.at(i)->printTree(stream, depth+1, isLast, isAboveLeaf);
    }
}