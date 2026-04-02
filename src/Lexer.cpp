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

        }  else if (ch == '(') {
            tokens.push_back(scanCommentParen()); 
        }   else if (ch == ':' || ch == ';' || ch == '+' || ch == '<' || ch == '>' || ch == '*' || ch == '/' ||
                   ch == '=' || ch == '-' || ch == '.' || ch == ',' || ch == ')' || ch == '[' || ch == ']') {
            Token result = scanSymbol();
            tokens.push_back(result);
        } else if (ch == '{'){
            tokens.push_back(scanCommentCurly());
        }
        else {
            // any symbol that are not recongnized as valid token will be pass as "unknown" token
            string unknownSymbol = string(1, ch);
            Token result = Token("unknown", unknownSymbol);
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
            return Token("leq", "<=");
        }

        if (input.peek() == '>') {
            input.get();
            return Token("neq", "<>");
        }

        return Token("lss", "<");

    } else if (ch == '=') {
        
        if (input.peek() == '=') {
            input.get();
            return Token("eql", "==");
        }    

        // = is treated as unknown symbol
        return Token("unknown", "=");        

    } else if (ch == '>') {

        if (input.peek() == '=') {
            input.get();
            return Token("geq", ">=");
        }    
        
        return Token("gtr", ">");    
        
    } else if (ch == ':') {

        if (input.peek() == '=') {
            input.get();
            return Token("becomes", ":=");
        }

        return Token("colon", ":");
    } 

    string symbol = string(1,ch);
    string tokenName = SYMBOLS.at(symbol);
    return Token(tokenName, symbol);
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
    string tokenName = "";

    tokenName += static_cast<char>(input.get());
    
    while (input.peek() != EOF && isalnum(static_cast<unsigned char>(input.peek()))) {
        tokenName += static_cast<char>(input.get());

    }
    
    string temp = tokenName;
    transform(temp.begin(), temp.end(), temp.begin(), [](unsigned char c){return tolower(c);});

    auto it = KEYWORDS.find(temp);

    if (it != KEYWORDS.end()) {
        string token = it->second;
        return Token(token, tokenName); 
    }    

    return Token("ident", tokenName);

}

Token Lexer::scanString() {
    string tokenName = "";
    input.get(); // consume opening '

    while (input.peek() != EOF) {
        char ch = static_cast<char>(input.get());

        if (ch == '\'') {
            if (input.peek() == '\'') {
                input.get();
                tokenName += '\'';
            } 
            else {
                if (tokenName.length() == 1)
                    return Token("charcon", tokenName);
                else
                    return Token("string", tokenName);
            }
        } 
        else {
            tokenName += ch;
        }
    }

    return Token("unknown", '\'' + tokenName);
}

Token Lexer::scanCommentCurly(){
    string tokenName = "";
    char ch = static_cast<char>(input.get());
    while(input.peek() != EOF && input.peek() != '}'){
        ch = static_cast<char>(input.get());
        tokenName += ch;
    }

    if(input.peek() == '}'){
        input.get();
        return Token("comment", tokenName);
    }
    return Token("unknown", "{" + tokenName);
}



Token Lexer::scanCommentParen(){
    string tokenName = "";

    // Consume '('
    if (input.get() != '(') {
        return Token("unknown", "(");
    }

    // Consume '*'
    if (input.peek() != '*') {
        return Token("lparent", "(");
    }
    input.get();

    bool isClosed = false;

    // Read until the closing delimiter "*)"
    while (input.peek() != EOF) {
        char ch = static_cast<char>(input.get());

        if (ch == '*' && input.peek() == ')') {
            input.get();  // consume ')'
            isClosed = true;
            break;
        }

        tokenName += ch;
    }

    if (!isClosed) {
        return Token("unknown", tokenName);
    }

    return Token("comment", tokenName);
}
