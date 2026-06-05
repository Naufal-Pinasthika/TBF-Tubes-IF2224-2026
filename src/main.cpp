#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <exception>
#include "1-Lexer/Lexer.hpp"
#include "2-Parser/Parser.hpp"
#include "3-Semantic-Analyzer/ASTBuilder.hpp"
#include "3-Semantic-Analyzer/Semantic.hpp"
#include "4-Intermediate-Code-Generation/CodeGenerator.hpp"
#include "4-Intermediate-Code-Generation/Interpreter.hpp"

using namespace std;

static string getBaseName(const string &path)
{
    size_t p = path.find_last_of('/');
    if (p == string::npos)
        return path;
    return path.substr(p + 1);
}

static string resolveInputPath(const string &path, const string &fallbackDir)
{
    if (std::filesystem::exists(path))
        return path;

    string testPath = "test/" + path;
    if (std::filesystem::exists(testPath))
        return testPath;

    return "test/" + fallbackDir + "/" + getBaseName(path);
}

int main(int argc, char *argv[])
{
    std::filesystem::create_directories("test/milestone-1");
    std::filesystem::create_directories("test/milestone-2");
    std::filesystem::create_directories("test/milestone-3");
    std::filesystem::create_directories("test/milestone-4");

    string mode = "I"; // default interpreter
    string s;

    if (argc == 2)
    {
        s = argv[1];
    }
    else if (argc == 3)
    {
        string flag = argv[1];
        s = argv[2];

        if (flag == "-L")
        {
            mode = "L";
        }
        else if (flag == "-P")
        {
            mode = "P";
        } else if (flag == "-S")
        {
            mode = "S";
        } else if (flag == "-I")
        {
            mode = "I";
        }
        else
        {
            cerr << "Unknown flag: " << flag << endl;
            cerr << "Usage: " << argv[0] << " [ -L | -P | -S | -I ] <file>" << endl;
            return 1;
        }
    }
    else
    {
        cerr << "Usage: " << argv[0] << " [ -L | -P | -S | -I ] <file>" << endl;
        return 1;
    }

    bool isTokenFile = s.size() >= 9 && s.substr(s.size() - 9) == "_tokenize";
    bool isParsedFile = s.size() >= 7 && s.substr(s.size() - 7) == "_parsed";
    bool isAstFile = s.size() >= 4 && s.substr(s.size() - 4) == "_ast";
    bool loadedParsedTree = false;
    vector<Token> tokens;
    Parser parser = Parser(vector<Token>{});
    string outputName = s;
    string baseName;
    if (isAstFile)
    {
        if (mode != "I")
        {
            cerr << "Cannot run lexer/parser/semantic mode on an AST file\n";
            return 1;
        }

        string filepath = resolveInputPath(s, "milestone-3");
        ASTFileReadResult astInput = ASTNode::buildFromAstFile(filepath);
        if (!astInput.errors.empty() || astInput.root == nullptr)
        {
            for (const string& error : astInput.errors)
                cerr << error << endl;

            delete astInput.root;
            return 1;
        }

        baseName = getBaseName(s);
        outputName = (baseName.size() > 4) ? baseName.substr(0, baseName.size() - 4) : baseName;

        CodeGenerator generator;
        const TacProgram& program = generator.generate(astInput.root, astInput.symbolTable);

        ofstream icOutput("test/milestone-4/" + outputName + "_ic");
        if (!icOutput.is_open())
        {
            cerr << "Failed to open intermediate code output file" << endl;
            delete astInput.root;
            return 1;
        }

        program.print(icOutput);

        Interpreter interpreter;
        try
        {
            interpreter.load(program);
            interpreter.run();
        }
        catch (const exception& error)
        {
            cerr << "Runtime error: " << error.what() << endl;
            delete astInput.root;
            return 1;
        }

        delete astInput.root;
        return 0;
    }
    else if (isTokenFile)
    {
        if (mode == "L")
        {
            cerr << "Cannot lexer a tokenized file\n";
            return 1;
        }

        string filepath = resolveInputPath(s, "milestone-1");
        tokens = Lexer::readTokensFromFile(filepath);
        parser = Parser(tokens);

        baseName = getBaseName(s);
        outputName = (baseName.size() > 9) ? baseName.substr(0, baseName.size() - 9) : baseName;
    }
    else if (isParsedFile)
    {
        if (mode == "L")
        {
            cerr << "Cannot lexer a parsed file\n";
            return 1;
        }

        string filepath = resolveInputPath(s, "milestone-2");
        parser = Parser::buildFromParsedFile(filepath);
        loadedParsedTree = true;
        baseName = getBaseName(s);
        outputName = (baseName.size() > 7) ? baseName.substr(0, baseName.size() - 7) : baseName;
    }
    else
    {
        baseName = getBaseName(s);
        outputName = baseName;

        string filepath = resolveInputPath(s, "input");
        ifstream file(filepath);

        if (!file.is_open())
        {
            cerr << "Invalid file" << endl;
            return 1;
        }

        Lexer lexer(file);
        tokens = lexer.runLexer();

        filepath = "test/milestone-1/" + outputName + "_tokenize";
        ofstream outputFile(filepath);

        if (!outputFile.is_open())
        {
            cerr << "Failed to open lexer output file" << endl;
            return 1;
        }

        for (const auto &token : tokens)
        {
            outputFile << token.toString() << endl;
        }
        outputFile.close();

        if (mode == "L")
        {
            return 0;
        }

        parser = Parser(tokens);
    }

    if (!loadedParsedTree && !parser.parse())
    {
        vector<Token> parserTokens = parser.getTokens();
        int highestPos = parser.getHighestPos();
        Token token = (highestPos >= 0 && static_cast<size_t>(highestPos) < parserTokens.size())
            ? parserTokens[highestPos]
            : Token("eof", "");
        set<string> expected = parser.getExpected();
        vector<int> pos = token.getPos();

        cout << "Syntax error:" << pos[0] << ":" << pos[1] << ": unexpected token " << token.getType() << " (\"" << token.getLexeme() << "\"), expected ";

        bool first = true;
        for (const string &exp : expected)
        {
            if (!first)
                cout << "|";
            cout << exp;
            first = false;
        }
        cout << "\n";
        return 1;
    }
    else if (loadedParsedTree && parser.getRoot() == nullptr)
    {
        cerr << "Invalid parsed file" << endl;
        return 1;
    }

    if (parser.getRoot())
        parser.getRoot()->printTreeToFile(outputName + "_parsed");

    if (mode == "P")
        return 0;

    ASTBuilder astBuilder;
    ProgramNode* ast = astBuilder.build(parser.getRoot());

    if (astBuilder.hasErrors())
    {
        for (const string& error : astBuilder.getErrors())
            cout << error << endl;

        delete ast;
        return 1;
    }

    Semantic semantic;
    semantic.analyze(ast);

    if (semantic.hasErrors())
    {
        for (const string& error : semantic.getErrors())
            cout << error << endl;

        if (mode == "I")
        {
            delete ast;
            return 1;
        }
    }

    ofstream astOutput("test/milestone-3/" + outputName + "_ast");
    streambuf* oldCout = cout.rdbuf(astOutput.rdbuf());

    semantic.getSymbolTable().print(cout);
    cout << "\nDecorated AST:\n";
    ast->print();

    cout.rdbuf(oldCout);

    if (mode == "I")
    {
        CodeGenerator generator;
        const TacProgram& program = generator.generate(ast, semantic.getSymbolTable());

        ofstream icOutput("test/milestone-4/" + outputName + "_ic");
        if (!icOutput.is_open())
        {
            cerr << "Failed to open intermediate code output file" << endl;
            delete ast;
            return 1;
        }

        program.print(icOutput);

        Interpreter interpreter;
        try
        {
            interpreter.load(program);
            interpreter.run();
        }
        catch (const exception& error)
        {
            cerr << "Runtime error: " << error.what() << endl;
            delete ast;
            return 1;
        }
    }

    delete ast;
    return 0;
}
