#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "Lexer/Lexer.hpp"
#include "Parser/Parser.hpp"

using namespace std;

int main(int argc,char* argv[]){
    if(argc == 2){
        string s(argv[1]);
        string filepath = "test/input/" + s;

        ifstream file(filepath);

        if (!file.is_open()) {
            cerr << "Invalid file" << endl;
            return 1;
        }

        Lexer lexer(file);
        vector <Token> tokenize = lexer.runLexer();

        filepath = "test/Lexer/" + s + "_tokenize";
        ofstream outputFile(filepath);
        
        for (auto token : tokenize) {
            string out = token.toString();

            outputFile << out << endl;
            if (out == "semicolon") {
                outputFile << "\n";
            }
        }

        Parser parser(tokenize);

        if(!parser.parse()) {
            Token token = parser.getTokens().at(parser.getHighestPos());
            vector<string> expected = parser.getExpected();
            vector<int> pos = token.getPos();
            cout << "Syntax error:" << pos[0] << ":" << pos[1] << ": unexpected token " + token.getType() + " (\"" + token.getLexeme() + "\"), expected ";
            bool first = true;
            for(string s : expected) {
                if(!first) cout << "|";
                cout << s;
                first = false;
            }
            cout << "\n";
            return 1;
        }

        parser.getRoot()->printTreeToFile(s + "_parsed");

        return 0;
    }
    else{
        cout << "argc != 2";
        return 0;
    }
}
