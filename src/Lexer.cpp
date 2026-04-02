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
    {"=", "eql"},
    {"<", "lss"},
    {">", "gtr"},
    {"(", "lparent"},
    {")", "rparent"},
    {"[", "lbrack"},
    {"]", "rbrack"},
    {",", "comma"},
    {";", "semicolon"},
    {".", "period"},
    {":", "colon"}
};


Lexer::Lexer(ifstream& input) : input(input) {}

vector<Token> Lexer::runLexer() {
    vector <Token> tokens;

    while (input.peek() != EOF) {

        // Ignore whitespace
        while (input.peek() != EOF && isspace(input.peek())) {
            input.get();
        }

        // Check for EOF after consuming whitespace
        if (input.peek() == EOF) {
            break;
        }

        // Grab char and check the current state its in
        char ch = static_cast<char> (input.peek());

        if (isalpha(ch)){
            Token result = scanIndentOrKeyword();
            tokens.push_back(result);

        } else if (isdigit(ch)){
            Token result = scanNumber();
            tokens.push_back(result);

        } else if (ch == '\''){
            Token result = scanString();
            tokens.push_back(result);

        } else if (ch == ':' || ch == ';' || ch == '+' || ch == '<' || ch == '>' || ch == '*' || ch == '/' ||
                   ch == '=' || ch == '-' || ch == '.' || ch == ',' || ch == '(' || ch == ')' || ch == '[' || ch == ']') {
            Token result = scanSymbol();
            tokens.push_back(result);
        } else {
            // Skip unknown characters to prevent infinite loop
            input.get();
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

        // Standalone = is a single token
        return Token("=", "eql");
        
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
            return Token(tokenName, "realcon");
            
        }

        input.unget();
        tokenName.pop_back();

    } 

    return Token(tokenName, "intcon");
    
}

Token Lexer::scanIndentOrKeyword() {
    string tokenName = "";
    char ch = static_cast <char>(input.get());
    
    char next_ch = static_cast<char> (input.peek());

    tokenName += ch;

    // we search until the longest KEYWORD (procedure, len = 9)
    // is properly validated 
    while (tokenName.size() < 10 && isalnum(next_ch)) {
        auto it = KEYWORDS.find(tokenName);

        if (it != KEYWORDS.end()) {
            string token = it->second;
            return Token(tokenName, token);
        }

        ch = static_cast <char>(input.get());
        
        next_ch = static_cast<char> (input.peek());

        tokenName += ch;        
    }

    // Check one final time after loop exits
    auto it = KEYWORDS.find(tokenName);
    if (it != KEYWORDS.end()) {
        return Token(tokenName, it->second);
    }

    return Token(tokenName, "ident");
}

Token Lexer::scanString() {
    string tokenName = "";
    char ch = static_cast<char>(input.get());  // consume opening quote
    
    // Continue until closing quote or EOF
    while (input.peek() != EOF && input.peek() != '\'') {
        ch = static_cast<char>(input.get());
        tokenName += ch;
    }
    
    // Consume closing quote if present
    if (input.peek() == '\'') {
        input.get();
    }
    
    return Token(tokenName, "string");
}