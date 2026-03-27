#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "Lexer.h"

using namespace std;

int main() {
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
}
