#pragma once

#include <fstream>
#include <string>
#include "Token.h"
#include <unordered_map>
#include <vector>
#include <cctype>
#include <algorithm>

using namespace std;

class Lexer {
private:
    ifstream& input;

public:
    Lexer(ifstream& input);
    vector<Token> runLexer();
    Token scanSymbol();
    Token scanIndentOrKeyword();
    Token scanNumber();
    Token scanString();

};