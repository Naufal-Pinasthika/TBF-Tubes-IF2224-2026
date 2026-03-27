#pragma once

#include <fstream>

using namespace std;

class Lexer {
private:
    ifstream& input;

public:
    Lexer(ifstream& input);
    void runLexer();
    ~Lexer();
};