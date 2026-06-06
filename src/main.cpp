#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "1-Lexer/Lexer.hpp"
#include "2-Parser/Parser.hpp"
#include "3-Semantic-Analyzer/ASTBuilder.hpp"
#include "3-Semantic-Analyzer/Semantic.hpp"
#include "4-Intermediate-Code-Generation/CodeGenerator.hpp"
#include "4-Intermediate-Code-Generation/Interpreter.hpp"

using namespace std;

enum class InputStage
{
    Source,
    Tokens,
    Parsed,
    Ast
};

enum class TargetStage
{
    Run,
    Tokens,
    Parsed,
    Ast,
    Intermediate
};

static string getBaseName(const string& path)
{
    size_t p = path.find_last_of('/');
    if (p == string::npos) return path;
    return path.substr(p + 1);
}

static bool hasSuffix(const string& text, const string& suffix)
{
    return text.size() >= suffix.size() && text.substr(text.size() - suffix.size()) == suffix;
}

static string trimText(const string& text)
{
    size_t start = text.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";

    size_t end = text.find_last_not_of(" \t\r\n");
    return text.substr(start, end - start + 1);
}

static bool startsWith(const string& text, const string& prefix)
{
    return text.compare(0, prefix.size(), prefix) == 0;
}

static string resolveInputPath(const string& path)
{
    if (std::filesystem::exists(path)) return path;

    string testPath = "test/" + path;
    if (std::filesystem::exists(testPath)) return testPath;

    string baseName = getBaseName(path);
    if (hasSuffix(baseName, "_tokenize")) return "test/milestone-1/" + baseName;
    if (hasSuffix(baseName, "_parsed")) return "test/milestone-2/" + baseName;
    if (hasSuffix(baseName, "_ast")) return "test/milestone-3/" + baseName;
    return "test/input/" + baseName;
}

static bool readInput(const string& path, string& content, string& resolvedPath)
{
    ostringstream buffer;

    if (path.empty()) {
        buffer << cin.rdbuf();
        content = buffer.str();
        resolvedPath = "";
        return true;
    }

    resolvedPath = resolveInputPath(path);
    ifstream file(resolvedPath);
    if (!file.is_open()) return false;

    buffer << file.rdbuf();
    content = buffer.str();
    return true;
}

static string firstNonEmptyLine(const string& content)
{
    istringstream input(content);
    string line;
    while (getline(input, line)) {
        string trimmed = trimText(line);
        if (!trimmed.empty()) return trimmed;
    }
    return "";
}

static bool isKnownTokenType(const string& type)
{
    static const set<string> tokenTypes = {
        "programsy", "varsy", "beginsy", "endsy", "ifsy", "elsesy", "whilesy",
        "forsy", "functionsy", "proceduresy", "arraysy", "recordsy", "constsy",
        "typesy", "casesy", "repeatsy", "untilsy", "ofsy", "dosy", "tosy",
        "downtosy", "thensy", "idiv", "imod", "andsy", "orsy", "notsy",
        "plus", "minus", "times", "rdiv", "eql", "lss", "gtr", "leq", "geq",
        "neq", "lparent", "rparent", "lbrack", "rbrack", "comma", "semicolon",
        "period", "colon", "becomes", "intcon", "realcon", "charcon", "ident",
        "string", "unknown", "comment"
    };
    return tokenTypes.find(type) != tokenTypes.end();
}

static bool looksLikeTokenLine(const string& line)
{
    if (line.empty()) return false;

    size_t detailPos = line.find(" (");
    string type = detailPos == string::npos ? line : line.substr(0, detailPos);
    if (!isKnownTokenType(type)) return false;

    if (detailPos == string::npos) return true;
    return line.size() >= 3 && line.back() == ')';
}

static bool looksLikeParsedTreeLine(const string& line)
{
    string label = line;
    size_t branch = label.find("── ");
    if (branch != string::npos) label = trimText(label.substr(branch + 4));
    return label == "<program>";
}

static InputStage inferInputStage(const string& path, const string& content)
{
    string baseName = getBaseName(path);
    if (hasSuffix(baseName, "_tokenize")) return InputStage::Tokens;
    if (hasSuffix(baseName, "_parsed")) return InputStage::Parsed;
    if (hasSuffix(baseName, "_ast")) return InputStage::Ast;

    if (content.find("Decorated AST:") != string::npos) return InputStage::Ast;

    string firstLine = firstNonEmptyLine(content);
    if (looksLikeParsedTreeLine(firstLine)) return InputStage::Parsed;
    if (looksLikeTokenLine(firstLine)) return InputStage::Tokens;
    return InputStage::Source;
}

static int stageRank(InputStage stage)
{
    switch (stage) {
    case InputStage::Source: return 0;
    case InputStage::Tokens: return 1;
    case InputStage::Parsed: return 2;
    case InputStage::Ast: return 3;
    }
    return 0;
}

static int targetRank(TargetStage stage)
{
    switch (stage) {
    case TargetStage::Run: return 4;
    case TargetStage::Tokens: return 1;
    case TargetStage::Parsed: return 2;
    case TargetStage::Ast: return 3;
    case TargetStage::Intermediate: return 4;
    }
    return 4;
}

static bool parseFlag(const string& flag, TargetStage& target)
{
    if (flag == "-L") {
        target = TargetStage::Tokens;
        return true;
    }
    if (flag == "-P") {
        target = TargetStage::Parsed;
        return true;
    }
    if (flag == "-S") {
        target = TargetStage::Ast;
        return true;
    }
    if (flag == "-I") {
        target = TargetStage::Intermediate;
        return true;
    }
    return false;
}

static void printUsage(const string& programName)
{
    cerr << "Usage: " << programName << " [ -L | -P | -S | -I ] [file]" << endl;
}

static void printTokens(const vector<Token>& tokens, ostream& out)
{
    for (const Token& token : tokens) {
        out << token.toString() << '\n';
    }
}

static void printParseTree(Node* root, ostream& out)
{
    if (root == nullptr) return;

    vector<bool> isAboveLeaf;
    root->printTree(&out, 0, true, isAboveLeaf);
}

static void printAst(ProgramNode* ast, SymbolTable& symbolTable)
{
    symbolTable.print(cout);
    cout << "\nDecorated AST:\n";
    if (ast != nullptr) ast->print();
}

static void printSyntaxError(Parser& parser)
{
    vector<Token> parserTokens = parser.getTokens();
    int highestPos = parser.getHighestPos();
    Token token = (highestPos >= 0 && static_cast<size_t>(highestPos) < parserTokens.size())
        ? parserTokens[highestPos]
        : Token("eof", "");
    set<string> expected = parser.getExpected();
    vector<int> pos = token.getPos();

    cerr << "Syntax error:" << pos[0] << ":" << pos[1] << ": unexpected token "
         << token.getType() << " (\"" << token.getLexeme() << "\"), expected ";

    bool first = true;
    for (const string& exp : expected) {
        if (!first) cerr << "|";
        cerr << exp;
        first = false;
    }
    cerr << "\n";
}

int main(int argc, char* argv[])
{
    TargetStage target = TargetStage::Run;
    bool hasFlag = false;
    string inputPath;

    if (argc == 1) {
        inputPath = "";
    } else if (argc == 2) {
        string arg = argv[1];
        if (startsWith(arg, "-")) {
            if (!parseFlag(arg, target)) {
                cerr << "Unknown flag: " << arg << endl;
                printUsage(argv[0]);
                return 1;
            }
            hasFlag = true;
        } else {
            inputPath = arg;
        }
    } else if (argc == 3) {
        string flag = argv[1];
        if (!parseFlag(flag, target)) {
            cerr << "Unknown flag: " << flag << endl;
            printUsage(argv[0]);
            return 1;
        }
        hasFlag = true;
        inputPath = argv[2];
    } else {
        printUsage(argv[0]);
        return 1;
    }

    string content;
    string resolvedPath;
    if (!readInput(inputPath, content, resolvedPath)) {
        cerr << "Invalid file" << endl;
        return 1;
    }

    InputStage inputStage = inferInputStage(inputPath, content);
    if (hasFlag && targetRank(target) < stageRank(inputStage)) {
        cerr << "Cannot produce an earlier compiler stage from this input" << endl;
        return 1;
    }

    vector<Token> tokens;
    Parser parser = Parser(vector<Token>{});
    bool loadedParsedTree = false;

    ProgramNode* ast = nullptr;
    SymbolTable* symbolTable = nullptr;
    SymbolTable loadedSymbolTable;
    Semantic semantic;
    ASTFileReadResult astInput;

    if (inputStage == InputStage::Ast) {
        istringstream input(content);
        astInput = ASTNode::buildFromAstStream(input);
        if (!astInput.errors.empty() || astInput.root == nullptr) {
            for (const string& error : astInput.errors) cerr << error << endl;
            delete astInput.root;
            return 1;
        }

        ast = astInput.root;
        loadedSymbolTable = astInput.symbolTable;
        symbolTable = &loadedSymbolTable;
    } else if (inputStage == InputStage::Parsed) {
        istringstream input(content);
        parser = Parser::buildFromParsedStream(input);
        loadedParsedTree = true;
        tokens = parser.getTokens();
    } else if (inputStage == InputStage::Tokens) {
        istringstream input(content);
        tokens = Lexer::readTokensFromStream(input);
        parser = Parser(tokens);
    } else {
        istringstream input(content);
        Lexer lexer(input);
        tokens = lexer.runLexer();
        parser = Parser(tokens);
    }

    if (target == TargetStage::Tokens) {
        printTokens(tokens, cout);
        return 0;
    }

    if (inputStage != InputStage::Ast) {
        if (!loadedParsedTree && !parser.parse()) {
            printSyntaxError(parser);
            return 1;
        }

        if (parser.getRoot() == nullptr) {
            cerr << "Invalid parsed file" << endl;
            return 1;
        }

        if (target == TargetStage::Parsed) {
            printParseTree(parser.getRoot(), cout);
            return 0;
        }

        ASTBuilder astBuilder;
        ast = astBuilder.build(parser.getRoot());
        if (astBuilder.hasErrors()) {
            for (const string& error : astBuilder.getErrors()) cerr << error << endl;
            delete ast;
            return 1;
        }

        semantic.analyze(ast);
        symbolTable = &semantic.getSymbolTable();

        if (semantic.hasErrors()) {
            for (const string& error : semantic.getErrors()) cerr << error << endl;

            if (target == TargetStage::Intermediate || target == TargetStage::Run) {
                delete ast;
                return 1;
            }
        }
    }

    if (target == TargetStage::Ast) {
        printAst(ast, *symbolTable);
        delete ast;
        return 0;
    }

    CodeGenerator generator;
    const TacProgram& program = generator.generate(ast, *symbolTable);

    if (target == TargetStage::Intermediate) {
        program.print(cout);
        delete ast;
        return 0;
    }

    Interpreter interpreter;
    try {
        interpreter.load(program);
        interpreter.run();
    } catch (const exception& error) {
        cerr << "Runtime error: " << error.what() << endl;
        delete ast;
        return 1;
    }

    delete ast;
    return 0;
}
