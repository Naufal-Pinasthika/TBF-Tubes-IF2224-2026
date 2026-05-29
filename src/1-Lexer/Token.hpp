#pragma once

#include <string>
#include <vector>

using namespace std;

class Token {
private:
    string type;
    string lexeme;
    int row;
    int col;
    vector<int> pos;
public:
    Token(const string& type, const string& lexeme);
    Token(const string& type, const string& lexeme, const int row, const int col);
    string getType() const;
    string getLexeme() const;
    string toString() const;
    vector<int> getPos() const;
    int getRow() const;
    int getCol() const;
};  
