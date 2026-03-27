#include "Lexer.h"

/* List of token special keyword*/
static const unordered_map<string, string> KEYWORDS = {

    // Reserved keyword (for defining if-else, loop, etc.)
    {"program", "programsy"},
    {"var", "varsy"},
    {"begin", "beginsy"},
    {"end", "endsy"},

    {"if", "ifsy"},
    {"else", "elsesy"},
    {"while", "whilesy"},
    {"for", "forsy"},
    {"function", "functionsy"},
    {"procedure", "proceduresy"},
    {"array", "arraysy"},
    {"record", "recordsy"},
    {"const", "constsy"},
    {"type", "typesy"},
    {"case", "casesy"},
    {"repeat", "repeatsy"},
    {"until", "untilsy"},
    {"of", "ofsy"},
    {"do", "dosy"},
    {"to", "tosy"},
    {"downto", "downtosy"},
    {"then", "thensy"},

    // Word operator
    {"div", "idiv"},
    {"mod", "imod"},
    {"and", "andsy"},
    {"or", "orsy"},
    {"not", "notsy"}
};

Lexer::Lexer(ifstream& input) : input(input) {}

vector<Token> Lexer::runLexer() {
    vector <Token> tokens;
    char ch;
    string lexeme;
    while (input.get(ch)) {
        while (ch != ' ' || ch != '\t' || ch != '\n') {
            ch = tolower(ch);
            lexeme += ch;   
        }

        auto keyword = KEYWORDS.find(lexeme);
        if (keyword != KEYWORDS.end()) {
            string type = KEYWORDS.at(lexeme);
            Token token = Token(type, lexeme);
            tokens.push_back(token);
            
        } else {
            Token token = Token("ident", lexeme);
            tokens.push_back(token);            
        }

        lexeme = "";
    }

    return tokens;
}
Lexer::~Lexer()
{
}
