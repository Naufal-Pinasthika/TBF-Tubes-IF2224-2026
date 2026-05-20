#include "SymbolTable.hpp"

#include <iomanip>

static string objClassToString(ObjClass obj) {
    switch (obj) {
        case ObjClass::Program: return "program";
        case ObjClass::Constant: return "constant";
        case ObjClass::Variable: return "variable";
        case ObjClass::Type: return "type";
        case ObjClass::Procedure: return "procedure";
        case ObjClass::Function: return "function";
        case ObjClass::None:
        default: return "none";
    }
}

int SymbolTable::enterBlock() {
    currentLevel++;
    btab.push_back({static_cast<int>(btab.size()), 0, 0, 0, 0});
    display.push_back(btab.size() - 1);
    currentBlock = display.back();
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
    currentBlock = display.empty() ? 0 : display.back();
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

void SymbolTable::print(ostream& out) const {
    out << "tab (hanya sebagian yang relevan):\n";
    out << left
        << setw(5) << "idx"
        << setw(14) << "id"
        << setw(12) << "obj"
        << setw(7) << "type"
        << setw(6) << "ref"
        << setw(6) << "nrm"
        << setw(6) << "lev"
        << setw(6) << "adr"
        << "link\n";
    out << "-------------------------------------------------------------\n";
    out << "...  reserved words / keyword entries omitted\n";

    for (size_t i = 0; i < tab.size(); ++i) {
        const TabEntry& entry = tab[i];
        if (entry.obj == ObjClass::None) {
            continue;
        }

        out << left
            << setw(5) << i
            << setw(14) << entry.identifiers
            << setw(12) << objClassToString(entry.obj)
            << setw(7) << static_cast<int>(entry.type)
            << setw(6) << entry.ref
            << setw(6) << entry.nrm
            << setw(6) << entry.lev
            << setw(6) << entry.adr
            << entry.link << "\n";
    }

    out << "\nbtab:\n";
    out << left
        << setw(5) << "idx"
        << setw(7) << "last"
        << setw(7) << "lpar"
        << setw(7) << "psze"
        << "vsze\n";
    out << "-----------------------------\n";
    for (size_t i = 0; i < btab.size(); ++i) {
        const BtabEntry& entry = btab[i];
        out << left
            << setw(5) << i
            << setw(7) << entry.last
            << setw(7) << entry.lpar
            << setw(7) << entry.psze
            << entry.vsze << "\n";
    }

    out << "\natab:\n";
    if (atab.empty()) {
        out << "(kosong karena tidak ada array)\n";
        return;
    }

    out << left
        << setw(5) << "idx"
        << setw(7) << "xtyp"
        << setw(7) << "etyp"
        << setw(7) << "eref"
        << setw(7) << "low"
        << setw(7) << "high"
        << setw(7) << "elsz"
        << "size\n";
    out << "------------------------------------------------\n";
    for (const AtabEntry& entry : atab) {
        out << left
            << setw(5) << entry.arrays
            << setw(7) << entry.xtype
            << setw(7) << entry.etype
            << setw(7) << entry.eref
            << setw(7) << entry.low
            << setw(7) << entry.high
            << setw(7) << entry.elsz
            << entry.size << "\n";
    }
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
