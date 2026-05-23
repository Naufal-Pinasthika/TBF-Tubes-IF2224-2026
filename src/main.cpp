#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include "Lexer/Lexer.hpp"
#include "Parser/Parser.hpp"
#include "Semantic-Analyzer/ASTBuilder.hpp"
#include "Semantic-Analyzer/Semantic.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    std::filesystem::create_directories("test/milestone-1");
    std::filesystem::create_directories("test/milestone-2");
    std::filesystem::create_directories("test/milestone-3");

    string mode = "P"; // default parser
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
        }
        else
        {
            cerr << "Unknown flag: " << flag << endl;
            cerr << "Usage: " << argv[0] << " [ -L | -P ] <file>" << endl;
            return 1;
        }
    }
    else
    {
        cerr << "Usage: " << argv[0] << " [ -L | -P ] <file>" << endl;
        return 1;
    }

    bool isTokenFile = s.size() >= 9 && s.substr(s.size() - 9) == "_tokenize";
    vector<Token> tokens;
    if (isTokenFile)
    {
        if (mode == "L")
        {
            cerr << "Cannot lexer a tokenized file\n";
            return 1;
        }

        string filepath = "test/" + s;
        tokens = Lexer::readTokensFromFile(filepath);
    }
    else
    {
        string filepath = "test/input/" + s;
        ifstream file(filepath);

        if (!file.is_open())
        {
            cerr << "Invalid file" << endl;
            return 1;
        }

        Lexer lexer(file);
        tokens = lexer.runLexer();

        filepath = "test/milestone-1/" + s + "_tokenize";
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
    }

    Parser parser(tokens);

    if (!parser.parse())
    {
        Token token = parser.getTokens().at(parser.getHighestPos());
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

    string outputName = s;
    if (isTokenFile)
    {
        outputName = s.substr(0, s.size() - 9);
    }

    parser.getRoot()->printTreeToFile(s + "_parsed");

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
    }

    ofstream astOutput("test/milestone-3/" + outputName + "_ast");
    streambuf* oldCout = cout.rdbuf(astOutput.rdbuf());

    semantic.getSymbolTable().print(cout);
    cout << "\nDecorated AST:\n";
    ast->print();

    cout.rdbuf(oldCout);

    delete ast;
    return 0;
}
