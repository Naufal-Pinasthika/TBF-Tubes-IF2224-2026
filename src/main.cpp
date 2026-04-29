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

        // filepath = "test/output/" + s + "_tokenize";
        // ofstream outputFile(filepath);
        
        // for (auto token : tokenize) {
        //     string out = token.toString();

        //     outputFile << out << endl;
        //     if (out == "semicolon") {
        //         outputFile << "\n";
        //     }
        // }

        Parser parser(tokenize);
        parser.parse();

        parser.getRoot()->printTreeToFile(s + "_parsed");
        
        return 0;
    }
    else{
        cout << "argc != 2";
        return 0;
    }
}
