#pragma once

#include <ostream>
#include <string>
#include <vector>

using namespace std;

// refer to this https://docs.google.com/document/d/1pAy0sLZaSylTLXS1yBc7Snu4dHsAE3fJXbY24DkHeic/edit?tab=t.0
enum class TacOpcode
{
    Lit, // push(v)
    Lod, // push(mem[a])
    Sto, // pop(val); mem[a] = val
    Add, // v2=pop; v1=pop; push(v1+v2)
    Sub, // v2=pop; v1=pop; push(v1-v2) 
    Mul, // v2=pop; v1=pop; push(v1*v2)
    Div, // v2=pop; v1=pop; push(v1/v2)
    Jmp, // PC = l
    Jpc, // val=pop; if val==0 then PC=l
    Cal, // Simpan konteks (PC, BP)
    Ret, // Pulihkan konteks    
    Int, // Initiate Memory size of m
    Label // Kinda need it to define things
};

enum class TacOperation
{
    Neg = 1,
    Add = 2,
    Sub = 3,
    Mul = 4,
    Div = 5,
    Mod = 6,
    Eql = 7,
    Neq = 8,
    Lss = 9,
    Geq = 10,
    Gtr = 11,
    Leq = 12,
    Wrt = 13,
    Wrtln = 14
};

struct TacOperand
{
    bool hasValue = false;
    int value = 0;
    string text;

    TacOperand() = default;
    TacOperand(int value);
    TacOperand(string text);

    string toString() const;
};

struct TacInstruction
{
    TacOpcode opcode = TacOpcode::Label;
    int level = 0;
    TacOperand operand;
    string label;

    TacInstruction() = default;
    TacInstruction(TacOpcode opcode, int level = 0, TacOperand operand = TacOperand(), string label = "");

    string toString() const;
};

class TacProgram
{
private:
    vector<TacInstruction> instructions;

public:
    const vector<TacInstruction>& getInstructions() const;
    vector<TacInstruction>& getInstructions();
    void add(const TacInstruction& instruction);
    size_t size() const;
    bool empty() const;
    const TacInstruction& at(size_t index) const;
    void clear();
    void print(ostream& out) const;
};