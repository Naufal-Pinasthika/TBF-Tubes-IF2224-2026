#include "Token.h"

Token::Token(const string& type, const string& lexeme) : type(type), lexeme(lexeme) {

}

string Token::getType() const {
    return type;
}

string Token::getLexeme() const {
    return lexeme;
}