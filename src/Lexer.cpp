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

static const unordered_map<string, string> SYMBOLS = {
    {"+", "plus"},
    {"-", "minus"},
    {"*", "times"},
    {"/", "rdiv"},
    {"==", "eql"},
    // {"<>", "neq"},
    {">", "gtr"},
    {">=", "geq"},
    // {"<", "lss"},
    // {"<=", "leq"},
    {"(", "lparent"},
    {")", "rparent"},
    {"[", "lbrackm"},
    {"]", "rbrack"},
    {",", "comma"},
    {";", "semicolon"},
    {".", "period"},
    {":", "colon"},
    {":=", "becomes"}
};


Lexer::Lexer(ifstream& input) : input(input) {}

vector<Token> Lexer::runLexer() {
    vector <Token> tokens;
    char ch;
    string lexeme;

    while (input.peek() != EOF) {

        // Ignore whitespace
        while (input.peek() != EOF && isspace(input.peek())) {
            input.get();
        }

        // Grab char and check the current state its in
        char ch = static_cast<char> (input.peek());

        if (isalpha(ch)){

        } else if (isdigit(ch)){

        } else if (ch == ':' || ch == ';' || ch == '+' || ch == '<' || ch == '>' || ch == '*' || ch == '/') {
            Token result = scanSymbol();
        }
        
    }
    


    return tokens;
}

Token Lexer::scanSymbol() {
    char ch = static_cast <char>(input.get());
    if (ch == '<') {

        if (input.peek() == '=') {
            input.get();
            return Token("<=", "leq");
        }

        if (input.peek() == '>') {
            input.get();
            return Token("<>", "neq");
        }

        if (isspace(input.peek())) {
            return Token("<", "lss");
        }

    } else if (ch == '=') {
        if (input.peek() == '=') {
            input.get();
            return Token("==", "eql");
        }        
    } else if (ch == '>')

    // please continue this, i wanna sleep
}
