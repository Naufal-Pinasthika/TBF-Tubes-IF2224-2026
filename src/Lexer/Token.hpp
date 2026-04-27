#pragma once

#include <string>

using namespace std;

class Token {
private:
    string type;
    string lexeme;
public:
    Token(const string& type, const string& lexeme);
    string getType() const;
    string getLexeme() const;
    string toString() const;
};  
