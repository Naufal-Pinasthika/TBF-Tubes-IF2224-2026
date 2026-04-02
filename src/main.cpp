#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "Lexer.h"

using namespace std;

/*int main() {
    string path = "test/input/";
    string filename;
    
    cout << "Input .txt name";
    cin >> filename;
    
    string fullpath = path + filename;
    
    ifstream file(fullpath);
    
    if (!file.is_open()) {
        cerr << "Invalid file" << endl;
        return 1;
    }

    Lexer lexer(file);
    vector <Token> tokenize = lexer.runLexer();

    return 0;
}*/

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

        filepath = "test/output/" + s + "_tokenize";
        ofstream outputFile(filepath);
        
        for (auto token : tokenize) {
            string out = token.toString();
            outputFile << out << endl;
        }
        return 0;
    }
    else{
        cout << "argc != 2";
        return 0;
    }
}
