#include "Lexer.hpp"

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
    {"not", "notsy"}};

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
    {":", "colon"}};

Lexer::Lexer(ifstream &input) : input(input), readRow(1), readCol(1) {}

int Lexer::get()
{
    int c = input.get();
    if (c == '\n')
    {
        readRow += 1;
        readCol = 1;
    }
    else
        readCol += 1;
    return c;
}

int Lexer::peek() const
{
    return input.peek();
}

vector<Token> Lexer::runLexer()
{
    vector<Token> tokens;

    while (peek() != EOF)
    {

        // Ignore whitespace
        while (peek() != EOF && isspace(peek()))
        {
            get();
        }

        // Check for EOF after consuming whitespace
        if (peek() == EOF)
        {
            break;
        }

        // Grab char and check the current state its in
        char ch = static_cast<char>(peek());

        if (isalpha(ch))
        {
            Token result = scanIndentOrKeyword();
            tokens.push_back(result);
        }
        else if (isdigit(ch))
        {
            Token result = scanNumber();
            tokens.push_back(result);
        }
        else if (ch == '\'')
        {
            Token result = scanString();
            tokens.push_back(result);
        }
        else if (ch == '(')
        {
            tokens.push_back(scanCommentParen());
        }
        else if (ch == ':' || ch == ';' || ch == '+' || ch == '<' || ch == '>' || ch == '*' || ch == '/' ||
                 ch == '=' || ch == '-' || ch == '.' || ch == ',' || ch == ')' || ch == '[' || ch == ']')
        {
            Token result = scanSymbol();
            tokens.push_back(result);
        }
        else if (ch == '{')
        {
            tokens.push_back(scanCommentCurly());
        }
        else
        {
            // any symbol that are not recongnized as valid token will be pass as "unknown" token
            string unknownSymbol = string(1, ch);
            Token result = Token("unknown", unknownSymbol, readRow, readCol);
            tokens.push_back(result);
        }
    }

    return tokens;
}

Token Lexer::scanSymbol()
{
    char ch = static_cast<char>(get());
    if (ch == '<')
    {

        if (peek() == '=')
        {
            get();
            return Token("leq", "<=", readRow, readCol);
        }

        if (peek() == '>')
        {
            get();
            return Token("neq", "<>", readRow, readCol);
        }

        return Token("lss", "<", readRow, readCol);
    }
    else if (ch == '=')
    {

        if (peek() == '=')
        {
            get();
            return Token("eql", "==", readRow, readCol);
        }

        // = is treated as unknown symbol
        return Token("unknown", "=", readRow, readCol);
    }
    else if (ch == '>')
    {

        if (peek() == '=')
        {
            get();
            return Token("geq", ">=", readRow, readCol);
        }

        return Token("gtr", ">", readRow, readCol);
    }
    else if (ch == ':')
    {

        if (peek() == '=')
        {
            get();
            return Token("becomes", ":=", readRow, readCol);
        }

        return Token("colon", ":", readRow, readCol);
    }

    string symbol = string(1, ch);
    string tokenName = SYMBOLS.at(symbol);
    return Token(tokenName, symbol, readRow, readCol);
}

Token Lexer::scanNumber()
{

    string tokenName = "";

    char ch = static_cast<char>(get());

    char next_ch = static_cast<char>(peek());

    tokenName += ch;

    while (isdigit(next_ch))
    {
        ch = static_cast<char>(get());
        next_ch = static_cast<char>(peek());
        tokenName += ch;
    }

    if (next_ch == '.')
    {
        ch = static_cast<char>(get());
        next_ch = static_cast<char>(peek());

        tokenName += ch;

        if (isdigit(next_ch))
        {
            while (isdigit(peek()))
            {
                ch = static_cast<char>(get());
                next_ch = static_cast<char>(peek());

                tokenName += ch;
            }
            return Token("realcon", tokenName, readRow, readCol);
        }

        input.unget();
        tokenName.pop_back();
    }

    return Token("intcon", tokenName, readRow, readCol);
}

Token Lexer::scanIndentOrKeyword()
{
    string tokenName = "";

    tokenName += static_cast<char>(get());

    while (peek() != EOF && isalnum(static_cast<unsigned char>(peek())))
    {
        tokenName += static_cast<char>(get());
    }

    string temp = tokenName;
    transform(temp.begin(), temp.end(), temp.begin(), [](unsigned char c)
              { return tolower(c); });

    auto it = KEYWORDS.find(temp);

    if (it != KEYWORDS.end())
    {
        string token = it->second;
        return Token(token, tokenName, readRow, readCol);
    }

    return Token("ident", tokenName, readRow, readCol);
}

Token Lexer::scanString()
{
    string tokenName = "";
    get(); // consume opening '

    while (peek() != EOF)
    {
        char ch = static_cast<char>(get());

        if (ch == '\'')
        {
            if (peek() == '\'')
            {
                get();
                tokenName += '\'';
            }
            else
            {
                if (tokenName.length() == 1)
                    return Token("charcon", tokenName, readRow, readCol);
                else
                    return Token("string", tokenName, readRow, readCol);
            }
        }
        else
        {
            tokenName += ch;
        }
    }

    return Token("unknown", '\'' + tokenName, readRow, readCol);
}

Token Lexer::scanCommentCurly()
{
    string tokenName = "";
    char ch = static_cast<char>(get());
    while (peek() != EOF && peek() != '}')
    {
        ch = static_cast<char>(get());
        tokenName += ch;
    }

    if (peek() == '}')
    {
        get();
        return Token("comment", tokenName, readRow, readCol);
    }
    return Token("unknown", "{" + tokenName, readRow, readCol);
}

Token Lexer::scanCommentParen()
{
    string tokenName = "";

    // Consume '('
    if (get() != '(')
    {
        return Token("unknown", "(", readRow, readCol);
    }

    // Consume '*'
    if (peek() != '*')
    {
        return Token("lparent", "(", readRow, readCol);
    }
    get();

    bool isClosed = false;

    // Read until the closing delimiter "*)"
    while (peek() != EOF)
    {
        char ch = static_cast<char>(get());

        if (ch == '*' && peek() == ')')
        {
            get(); // consume ')'
            isClosed = true;
            break;
        }

        tokenName += ch;
    }

    if (!isClosed)
    {
        return Token("unknown", tokenName, readRow, readCol);
    }

    return Token("comment", tokenName, readRow, readCol);
}
