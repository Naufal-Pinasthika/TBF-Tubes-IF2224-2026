#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include "Lexer/Lexer.hpp"
#include "Parser/Parser.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    string mode = "P"; // default parser
    string s;

    if (argc == 2) {
        s = argv[1];
    }
    else if (argc == 3) {
        string flag = argv[1];
        s = argv[2];

        if (flag == "-L") {
            mode = "L";
        }
        else if (flag == "-P") {
            mode = "P";
        }
        else {
            cerr << "Unknown flag: " << flag << endl;
            cerr << "Usage: " << argv[0] << " [ -L | -P ] <file>" << endl;
            return 1;
        }
    }
    else {
        cerr << "Usage: " << argv[0] << " [ -L | -P ] <file>" << endl;
        return 1;
    }

    string filepath = "test/input/" + s;
    ifstream file(filepath);

    if (!file.is_open()) {
        cerr << "Invalid file" << endl;
        return 1;
    }

    std::filesystem::create_directories("test/Lexer");
    std::filesystem::create_directories("test/Parser");

    Lexer lexer(file);
    vector<Token> tokenize = lexer.runLexer();

    filepath = "test/Lexer/" + s + "_tokenize";
    ofstream outputFile(filepath);

    if (!outputFile.is_open()) {
        cerr << "Failed to open lexer output file" << endl;
        return 1;
    }

    for (const auto& token : tokenize) {
        outputFile << token.toString() << endl;
    }
    outputFile.close();

    if (mode == "L") {
        return 0;
    }

    Parser parser(tokenize);

    if (!parser.parse()) {
        Token token = parser.getTokens().at(parser.getHighestPos());
        vector<string> expected = parser.getExpected();
        vector<int> pos = token.getPos();

        cout << "Syntax error:" << pos[0] << ":" << pos[1] << ": unexpected token " << token.getType() << " (\"" << token.getLexeme() << "\"), expected ";

        bool first = true;
        for (const string& exp : expected) {
            if (!first) cout << "|";
            cout << exp;
            first = false;
        }
        cout << "\n";
        return 1;
    }

    parser.getRoot()->printTreeToFile(s + "_parsed");
    return 0;
}
