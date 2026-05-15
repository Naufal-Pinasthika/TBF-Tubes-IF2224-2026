#pragma once

#include <iostream>
#include <vector>
#include <string>

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
    SymbolTable()
    {
        // push until 33 entry
        for (int i = 0; i < 33; i++) {
            tab.push_back({"", 0, ObjClass::None, TypeClass::None, 0, 0, 0, 0});
        }

        btab.push_back({0, 0, 0, 0, 0});
        currentLevel = 0;
        currentBlock = 0;

        display.push_back(0);
    }

    int enterBlock();
    int exitBlock();
    int insertTab(const string& name, ObjClass obj, TypeClass type, int ref, int nrm, int adr);
    int lookupCurrentBlock(const string& name);
    int lookup(const string& name);
    TabEntry* getTab(int idx);
    AtabEntry* getAtab(int idx);
    BtabEntry* getBtab(int idx);

};

