#pragma once

#include <string>
#include <vector>

using namespace std;

class Token {
private:
    string type;
    string lexeme;
    vector<int> pos;
public:
    Token(const string& type, const string& lexeme);
    Token(const string& type, const string& lexeme, const int row, const int col);
    vector<int> getPos() const;
    string getType() const;
    string getLexeme() const;
    string toString() const;
};  
