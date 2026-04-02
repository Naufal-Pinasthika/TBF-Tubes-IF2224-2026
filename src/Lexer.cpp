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
    // {"==", "eql"},
    // {"<>", "neq"},
    // {">", "gtr"},
    // {">=", "geq"},
    // {"<", "lss"},
    // {"<=", "leq"},
    {"(", "lparent"},
    {")", "rparent"},
    {"[", "lbrack"},
    {"]", "rbrack"},
    {",", "comma"},
    {";", "semicolon"},
    {".", "period"},
    // {":", "colon"},
    // {":=", "becomes"}
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
            Token result = scanIndentOrKeyword();

        } else if (isdigit(ch)){
            Token result = scanNumber();
            tokens.push_back(result);

        } else if (ch == ':' || ch == ';' || ch == '+' || ch == '<' || ch == '>' || ch == '*' || ch == '/' ||
                   ch == '=' || ch == '-' || ch == '.' || ch == ',' || ch == '(' || ch == ')' || ch == '[' || ch == ']') {
            Token result = scanSymbol();
            tokens.push_back(result);
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

        return Token("<", "lss");

    } else if (ch == '=') {
        
        if (input.peek() == '=') {
            input.get();
            return Token("==", "eql");
        }    

        // for now this is special case where = need to throw
        
    } else if (ch == '>') {

        if (input.peek() == '=') {
            input.get();
            return Token(">=", "geq");
        }    
        
        return Token(">", "gtr");      
        
    } else if (ch == ':') {

        if (input.peek() == '=') {
            input.get();
            return Token(":=", "becomes");
        }

        return Token(":", "colon");
    } 

    string symbol = string(1,ch);
    string tokenName = SYMBOLS.at(symbol);
    return Token(symbol, tokenName);
    // please continue this, i wanna sleep
}

Token Lexer::scanNumber() {

    string tokenName = "";

    char ch = static_cast <char>(input.get());
    
    char next_ch = static_cast<char> (input.peek());

    tokenName += ch;

    while (isdigit(next_ch)) {
        ch = static_cast <char>(input.get());
        next_ch = static_cast<char> (input.peek());
        tokenName += ch;
    }

    if (next_ch == '.') {        
        ch = static_cast <char>(input.get());
        next_ch = static_cast<char> (input.peek());

        tokenName += ch;

        if (isdigit(next_ch)) {
            while (isdigit(input.peek())){
                ch = static_cast <char>(input.get());
                next_ch = static_cast<char> (input.peek());

                tokenName += ch;
            }
            return Token("realcon", tokenName);
            
        }

        input.unget();
        tokenName.pop_back();

    } 

    return Token("intcon", tokenName);
    
}

Token Lexer::scanIndentOrKeyword() {
    
}