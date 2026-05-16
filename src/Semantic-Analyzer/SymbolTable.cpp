#include "SymbolTable.hpp"

int SymbolTable::enterBlock() {
    currentLevel++;
    btab.push_back({0, 0, 0, 0});
    display.push_back(btab.size() - 1);
    return btab.size() - 1;
}

void SymbolTable::exitBlock() {
    if (currentLevel == 0){
        return;
    }

    if (!display.empty()){
        display.pop_back();
    }
    currentLevel--;
}

int SymbolTable::insertAtab(int xtype, int etype, int eref, int low, int high, int elsz) {
    int size = (high - low + 1) * elsz;

    atab.push_back({
        static_cast<int>(atab.size()) + 1,
        xtype,
        etype,
        eref,
        low,
        high,
        elsz,
        size
    });

    return atab.size(); 
}

int SymbolTable::insertTab(const string& name, ObjClass obj, TypeClass type, int ref, int nrm, int adr) {
    int currBlockIdx = display[currentLevel];
    int lastInBlock = btab[currBlockIdx].last;

    TabEntry newEntry = {
        name, 
        lastInBlock,
        obj, 
        type, 
        ref, 
        nrm, 
        currentLevel, 
        adr 
    };

    // edge case: redeclaration variables
    if (lookupCurrentBlock(name, currBlockIdx) != 0){
        return 0;
    }

    tab.push_back(newEntry);
    int newIdx = tab.size() - 1;

    btab[currBlockIdx].last = newIdx;

    if (obj == ObjClass::Variable) {
        btab[currBlockIdx].vsze += 1;
    }
    return newIdx;
}

int SymbolTable::lookupCurrentBlock(const string& name, int blockIdx) {
    if (blockIdx < 0 || blockIdx >= static_cast<int>(btab.size())){
        return 0;
    }

    int curr = btab[blockIdx].last;
    while (curr > 0 && curr < static_cast<int>(tab.size())){
        if (tab[curr].identifiers == name){
            return curr;
        }
        curr = tab[curr].link;
    }
    return 0;
    
}

int SymbolTable::lookup(const string& name) {
    int currBlockIdx, lastInBlock;
 
    for (int lev = currentLevel; lev >= 0; --lev) {
        currBlockIdx = display[lev];
        lastInBlock = btab[currBlockIdx].last;

        while (lastInBlock > 0 && lastInBlock < static_cast<int>(tab.size())){

            if (tab[lastInBlock].identifiers == name) return lastInBlock;


            lastInBlock = tab[lastInBlock].link;
        }
        
    }
    return 0;
}

// helper func
string SymbolTable::toUpper(const string& name) {
    string upperName = name;
    std::transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);
    return upperName;
}
TabEntry* SymbolTable::getTab(int idx) {
    if (idx >= 0 && idx < static_cast<int>(tab.size())){
        return &tab[idx];
    }
    return nullptr;
}
AtabEntry* SymbolTable::getAtab(int idx) {
    int realIdx = idx - 1;
    if (realIdx >= 0 && realIdx < static_cast<int>(atab.size())){
        return &atab[realIdx];
    }    
    return nullptr;
}
BtabEntry* SymbolTable::getBtab(int idx) {
    if (idx >= 0 && idx < static_cast<int>(btab.size())){
        return &btab[idx];
    }
    return nullptr;
}