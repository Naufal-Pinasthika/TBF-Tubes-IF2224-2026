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
    if (this->type == "intcon" || this->type == "realcon" || this->type == "charcon" || this->type == "ident" || this->type == "string") {
        result += this->type;
        result += " (";
        result += this->lexeme;
        result += ")";
        return result;
    }
    else if(this->type == "comment"){
        result += this->lexeme;
        return result;
    }
    else {
        result += this->type;
        return result;
    }
}
