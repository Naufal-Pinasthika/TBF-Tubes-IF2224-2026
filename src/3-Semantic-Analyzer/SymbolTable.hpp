#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>

using namespace std;

enum class ObjClass
{
    None        = 0,
    Program     = 1,
    Constant    = 2,
    Variable    = 3,
    Type        = 4,
    Procedure   = 5,
    Function    = 6
};

enum class TypeClass
{
    None        = 0,
    Integer     = 1,
    Real        = 2,
    Char        = 3,
    Boolean     = 4,
    String      = 5,
    Array       = 6,
    Record      = 7,
    Subrange    = 8,
    Enumerated  = 9
};

// tab: identifier table
struct TabEntry
{
    string identifiers;    
    int link;
    ObjClass obj;
    TypeClass type;
    int ref;
    int nrm;
    int lev;
    int adr;
};

// atab: array table
struct AtabEntry
{
    int arrays;
    int xtype;
    int etype;
    int eref;
    int low;
    int high;
    int elsz;
    int size;
};

// btab: block table (procedure/func/records)
struct BtabEntry
{
    int blocks;
    int last;
    int lpar;
    int psze;
    int vsze;
};

class SymbolTable
{
private:
    vector<TabEntry> tab;
    vector<AtabEntry> atab;
    vector<BtabEntry> btab;
    int currentLevel;
    int currentBlock;
    vector<int> display;
public:
    int RESERVED_COUNT;
    SymbolTable() : RESERVED_COUNT(39)
    {
        // init tab for reserved keywords
        tab.push_back({"NULL",      0, ObjClass::None, TypeClass::None,    0, 0, 0, 0});
        tab.push_back({"AND",       0, ObjClass::None, TypeClass::None,    0, 0, 0, 0});
        tab.push_back({"ARRAY",     0, ObjClass::None, TypeClass::None,    0, 0, 0, 0});
        tab.push_back({"BEGIN",     0, ObjClass::None, TypeClass::None,    0, 0, 0, 0});                
        tab.push_back({"CASE",      0, ObjClass::None, TypeClass::None,    0, 0, 0, 0});
        tab.push_back({"CONST",     0, ObjClass::None, TypeClass::None,    0, 0, 0, 0});
        tab.push_back({"DIV",       0, ObjClass::None, TypeClass::None,    0, 0, 0, 0});
        tab.push_back({"DOWNTO",    0, ObjClass::None, TypeClass::None,    0, 0, 0, 0});
        tab.push_back({"DO",        0, ObjClass::None, TypeClass::None,    0, 0, 0, 0});
        tab.push_back({"ELSE",      0, ObjClass::None, TypeClass::None,    0, 0, 0, 0});
        tab.push_back({"END",       0, ObjClass::None, TypeClass::None,    0, 0, 0, 0});
        tab.push_back({"FOR",       0, ObjClass::None, TypeClass::None,    0, 0, 0, 0});
        tab.push_back({"FUNCTION",  0, ObjClass::None, TypeClass::None,    0, 0, 0, 0});
        tab.push_back({"IF",        0, ObjClass::None, TypeClass::None,    0, 0, 0, 0});
        tab.push_back({"MOD",       0, ObjClass::None, TypeClass::None,    0, 0, 0, 0});
        tab.push_back({"NOT",       0, ObjClass::None, TypeClass::None,    0, 0, 0, 0});                
        tab.push_back({"OF",        0, ObjClass::None, TypeClass::None,    0, 0, 0, 0});
        tab.push_back({"OR",        0, ObjClass::None, TypeClass::None,    0, 0, 0, 0});
        tab.push_back({"PROCEDURE", 0, ObjClass::None, TypeClass::None,    0, 0, 0, 0});
        tab.push_back({"PROGRAM",   0, ObjClass::None, TypeClass::None,    0, 0, 0, 0});
        tab.push_back({"RECORD",    0, ObjClass::None, TypeClass::None,    0, 0, 0, 0});
        tab.push_back({"REPEAT",    0, ObjClass::None, TypeClass::None,    0, 0, 0, 0});

        tab.push_back({"INTEGER",   0, ObjClass::Type, TypeClass::Integer, 0, 1, 0, 0});
        tab.push_back({"REAL",      0, ObjClass::Type, TypeClass::Real,    0, 1, 0, 0});
        tab.push_back({"BOOLEAN",   0, ObjClass::Type, TypeClass::Boolean, 0, 1, 0, 0});
        tab.push_back({"CHAR",      0, ObjClass::Type, TypeClass::Char,    0, 1, 0, 0});
        tab.push_back({"STRING",    0, ObjClass::Type, TypeClass::String,  0, 1, 0, 0});

        tab.push_back({"THEN",      0, ObjClass::None, TypeClass::None,    0, 0, 0, 0});
        tab.push_back({"TO",        0, ObjClass::None, TypeClass::None,    0, 0, 0, 0});        
        tab.push_back({"TYPE",      0, ObjClass::None, TypeClass::None,    0, 0, 0, 0});
        tab.push_back({"UNTIL",     0, ObjClass::None, TypeClass::None,    0, 0, 0, 0});
        tab.push_back({"VAR",       0, ObjClass::None, TypeClass::None,    0, 0, 0, 0});
        tab.push_back({"WHILE",     0, ObjClass::None, TypeClass::None,    0, 0, 0, 0});  

        // init predefined identifier thats not reserved keyword
        tab.push_back({"TRUE",      0, ObjClass::Constant,      TypeClass::Boolean, 0, 1, 0, 1});
        tab.push_back({"FALSE",     0, ObjClass::Constant,      TypeClass::Boolean, 0, 1, 0, 0});
        tab.push_back({"WRITE",     0, ObjClass::Procedure,     TypeClass::None,    0, 1, 0, 0});
        tab.push_back({"WRITELN",   0, ObjClass::Procedure,     TypeClass::None,    0, 1, 0, 0});
        tab.push_back({"READ",      0, ObjClass::Procedure,     TypeClass::None,    0, 1, 0, 0});
        tab.push_back({"READLN",    0, ObjClass::Procedure,     TypeClass::None,    0, 1, 0, 0});

        btab.push_back({0, 0, 0, 0, 0});
        currentLevel = 0;
        currentBlock = 0;

        display.push_back(0);

        int last = 0;
        for (int i = 0; i < static_cast<int>(tab.size()); i++){
            if (
                tab[i].obj == ObjClass::Type || 
                tab[i].obj == ObjClass::Constant || 
                tab[i].obj == ObjClass::Procedure ||
                tab[i].obj == ObjClass::Function
            ) {
                tab[i].link = last;
                last = i;
            }
        }
        btab[0].last = last;
    }

    int enterBlock();
    void exitBlock();
    int insertAtab(int xtype, int etype, int eref, int low, int high, int elsz);
    int insertTab(const string& name, ObjClass obj, TypeClass type, int ref, int nrm, int adr);
    int lookupCurrentBlock(const string& name, int blockIdx);
    int lookup(const string& name);
    static SymbolTable buildFromAstDumpLines(const vector<string>& lines, vector<string>& errors);
    void print(ostream& out) const;
    string toUpper(const string& name);
    TabEntry* getTab(int idx);
    AtabEntry* getAtab(int idx);
    BtabEntry* getBtab(int idx);

};
