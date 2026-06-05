#include "SymbolTable.hpp"

#include <iomanip>
#include <sstream>

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

static string trimText(const string& text) {
    size_t start = text.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";

    size_t end = text.find_last_not_of(" \t\r\n");
    return text.substr(start, end - start + 1);
}

static string lowerText(string text) {
    transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(tolower(ch));
    });
    return text;
}

static bool startsWithText(const string& text, const string& prefix) {
    return text.compare(0, prefix.size(), prefix) == 0;
}

static ObjClass objClassFromString(const string& value) {
    if (value == "program") return ObjClass::Program;
    if (value == "constant") return ObjClass::Constant;
    if (value == "variable") return ObjClass::Variable;
    if (value == "type") return ObjClass::Type;
    if (value == "procedure") return ObjClass::Procedure;
    if (value == "function") return ObjClass::Function;
    return ObjClass::None;
}

static bool parseTabDumpLine(const string& line, TabEntry& entry, int& index) {
    string trimmed = trimText(line);
    if (trimmed.empty()) return false;

    size_t pos = 0;
    while (pos < trimmed.size() && isdigit(static_cast<unsigned char>(trimmed[pos]))) pos++;
    if (pos == 0) return false;

    index = atoi(trimmed.substr(0, pos).c_str());
    string rest = trimText(trimmed.substr(pos));
    string lowered = lowerText(rest);
    vector<string> objNames = {"procedure", "function", "constant", "variable", "program", "type", "none"};

    for (const string& objName : objNames) {
        size_t searchPos = 0;
        while (searchPos < lowered.size()) {
            size_t objPos = lowered.find(objName, searchPos);
            if (objPos == string::npos) break;

            string numberPart = trimText(rest.substr(objPos + objName.size()));
            stringstream numbers(numberPart);
            int type = 0;
            int ref = 0;
            int nrm = 0;
            int lev = 0;
            int adr = 0;
            int link = 0;
            if (numbers >> type >> ref >> nrm >> lev >> adr >> link) {
                string leftover;
                getline(numbers, leftover);
                if (trimText(leftover).empty()) {
                    entry.identifiers = trimText(rest.substr(0, objPos));
                    entry.obj = objClassFromString(objName);
                    entry.type = static_cast<TypeClass>(type);
                    entry.ref = ref;
                    entry.nrm = nrm;
                    entry.lev = lev;
                    entry.adr = adr;
                    entry.link = link;
                    return true;
                }
            }

            searchPos = objPos + 1;
        }
    }

    return false;
}

static bool parseBtabDumpLine(const string& line, BtabEntry& entry, int& index) {
    stringstream stream(line);
    int last = 0;
    int lpar = 0;
    int psze = 0;
    int vsze = 0;
    if (!(stream >> index >> last >> lpar >> psze >> vsze)) return false;

    entry.blocks = index;
    entry.last = last;
    entry.lpar = lpar;
    entry.psze = psze;
    entry.vsze = vsze;
    return true;
}

static bool parseAtabDumpLine(const string& line, AtabEntry& entry) {
    stringstream stream(line);
    if (!(stream >> entry.arrays >> entry.xtype >> entry.etype >> entry.eref >> entry.low >> entry.high >> entry.elsz >> entry.size)) {
        return false;
    }
    return true;
}

SymbolTable SymbolTable::buildFromAstDumpLines(const vector<string>& lines, vector<string>& errors) {
    SymbolTable result;
    result.tab.clear();
    result.atab.clear();
    result.btab.clear();
    result.display.clear();
    result.currentLevel = 0;
    result.currentBlock = 0;

    enum class DumpSection { None, Tab, Btab, Atab };
    DumpSection section = DumpSection::None;

    for (const string& line : lines) {
        string trimmed = trimText(line);
        if (trimmed == "Decorated AST:") break;
        if (trimmed == "tab:") {
            section = DumpSection::Tab;
            continue;
        }
        if (trimmed == "btab:") {
            section = DumpSection::Btab;
            continue;
        }
        if (trimmed == "atab:") {
            section = DumpSection::Atab;
            continue;
        }

        if (trimmed.empty() ||
            startsWithText(trimmed, "idx") ||
            startsWithText(trimmed, "---") ||
            startsWithText(trimmed, "(kosong")) {
            continue;
        }

        if (section == DumpSection::Tab) {
            TabEntry entry;
            int index = 0;
            if (!parseTabDumpLine(line, entry, index)) {
                errors.push_back("Failed to parse tab entry: " + line);
                continue;
            }
            if (index < 0) {
                errors.push_back("Invalid negative tab index: " + line);
                continue;
            }
            if (static_cast<int>(result.tab.size()) <= index) {
                result.tab.resize(index + 1);
            }
            result.tab[index] = entry;
        } else if (section == DumpSection::Btab) {
            BtabEntry entry;
            int index = 0;
            if (!parseBtabDumpLine(line, entry, index)) {
                errors.push_back("Failed to parse btab entry: " + line);
                continue;
            }
            if (index < 0) {
                errors.push_back("Invalid negative btab index: " + line);
                continue;
            }
            if (static_cast<int>(result.btab.size()) <= index) {
                result.btab.resize(index + 1);
            }
            result.btab[index] = entry;
        } else if (section == DumpSection::Atab) {
            AtabEntry entry;
            if (!parseAtabDumpLine(line, entry)) {
                errors.push_back("Failed to parse atab entry: " + line);
                continue;
            }
            if (entry.arrays <= 0) {
                errors.push_back("Invalid atab index: " + line);
                continue;
            }
            if (static_cast<int>(result.atab.size()) < entry.arrays) {
                result.atab.resize(entry.arrays);
            }
            result.atab[entry.arrays - 1] = entry;
        }
    }

    if (result.tab.empty()) {
        errors.push_back("AST dump does not contain tab entries");
    }
    if (result.btab.empty()) {
        result.btab.push_back({0, 0, 0, 0, 0});
    }

    result.display.push_back(0);
    return result;
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
    string nameUpper = toUpper(name);
    while (curr > 0 && curr < static_cast<int>(tab.size())){
        if (toUpper(tab[curr].identifiers) == nameUpper){
            return curr;
        }
        curr = tab[curr].link;
    }
    return 0;
    
}

int SymbolTable::lookup(const string& name) {
    int currBlockIdx, lastInBlock;
 
    string nameUpper = toUpper(name);
    for (int lev = currentLevel; lev >= 0; --lev) {
        currBlockIdx = display[lev];
        lastInBlock = btab[currBlockIdx].last;

        while (lastInBlock > 0 && lastInBlock < static_cast<int>(tab.size())){

            if (toUpper(tab[lastInBlock].identifiers) == nameUpper) return lastInBlock;


            lastInBlock = tab[lastInBlock].link;
        }
        
    }
    return 0;
}

void SymbolTable::print(ostream& out) const {
    out << "tab:\n";
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
    // out << "...  reserved words / keyword entries omitted\n";

    for (size_t i = 0; i < tab.size(); ++i) {
        const TabEntry& entry = tab[i];
        // if (entry.obj == ObjClass::None) continue;

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
