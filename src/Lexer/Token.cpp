#include "Token.hpp"

Token::Token(const string& type, const string& lexeme) : type(type), lexeme(lexeme), pos{0,0} {

}

Token::Token(const string& type, const string& lexeme, const int row, const int col) : type(type), lexeme(lexeme), pos{row,col} {
    pos[1] = pos[1] - lexeme.size();
}

string Token::getType() const {
    return type;
}

string Token::getLexeme() const {
    return lexeme;
}

vector<int> Token::getPos() const {
    return pos;
}

string Token::toString() const {
    string result = "";
    if (this->type == "intcon" || this->type == "realcon" || this->type == "charcon" || this->type == "ident" || this->type == "string" || this->type == "unknown" || this->type == "comment") {
        result += this->type;
        result += " (";
        if(this->type == "charcon" || this->type == "string") result += '\'';
        result += this->lexeme;
        if(this->type == "charcon" || this->type == "string") result += '\'';
        result += ")";
    }
    else {
        result += this->type;
    }

    // result += " [" + to_string(pos[0]) + "," + to_string(pos[1]) + "]";
    return result;
}
