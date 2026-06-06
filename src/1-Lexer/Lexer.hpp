#pragma once

#include <fstream>
#include <istream>
#include <string>
#include "Token.hpp"
#include <unordered_map>
#include <vector>
#include <cctype>
#include <algorithm>

using namespace std;

class Lexer {
private:
    istream& input;
    int readRow;
    int readCol;
public:
    Lexer(istream& input);
    int get();
    int peek() const;
    vector<Token> runLexer();
    Token scanSymbol();
    Token scanIndentOrKeyword();
    Token scanNumber();
    Token scanString();
    Token scanCommentCurly();
    Token scanCommentParen();
    static vector<Token> readTokensFromStream(istream& input);
    static vector<Token> readTokensFromFile(const string& filepath);
};
