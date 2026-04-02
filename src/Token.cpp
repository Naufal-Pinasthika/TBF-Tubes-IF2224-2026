#include "Token.h"

Token::Token(const string& type, const string& lexeme) : type(type), lexeme(lexeme) {

}

string Token::getType() const {
    return type;
}

string Token::getLexeme() const {
    return lexeme;
}

string Token::toString() const {
    string result = "";
    if (this->lexeme == "intcon" || this->lexeme == "realcon" || this->lexeme == "charcon" || this->lexeme == "string" || this->lexeme == "ident" || this->lexeme == "comment") {
        result += this->type;
        result += " (";
        result += this->lexeme;
        result += ")";
        return result;
    }
    else {
        return this->type;
    }
}
