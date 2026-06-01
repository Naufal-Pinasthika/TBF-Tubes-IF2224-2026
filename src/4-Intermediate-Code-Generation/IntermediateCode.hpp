#pragma once

#include <cstddef>
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
    Opr, // execute TacOperation for non-core stack operations
    Jmp, // PC = l
    Jpc, // val=pop; if val==0 then PC=l
    Cal, // Simpan konteks (PC, BP)
    Ret, // Pulihkan konteks    
    Int, // Initiate Memory size of m
    Label // Kinda need it to define things
};

enum class TacOperation
{
    None = 0,
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

enum class TacValueType
{
    None,
    Integer,
    Real,
    Boolean,
    Char,
    String
};

enum class TacOperandKind
{
    None,       // instruction without operand (ADD, RET)
    Literal,    // operand of literal value (LIT)
    Address,    // operand of memory address (LOD, STO, INT)
    Label,      // operand of jump target/call (JMP, JPC, CAL)
    Operation   // additional operation
};

struct TacValue
{
    TacValueType type = TacValueType::None;
    string text;
};

struct TacOperand
{
    TacOperandKind kind = TacOperandKind::None;
    TacValue literal;
    int address = 0;
    string label;
    TacOperation operationCode = TacOperation::None;

    string toString() const;
};

struct IntermediateInstruction
{
    TacOpcode opcode = TacOpcode::Label;
    int level = 0;
    TacOperand operand;

    string toString() const;
};

class TacProgram
{
private:
    vector<IntermediateInstruction> instructions;

public:
    const vector<IntermediateInstruction>& getInstructions() const;
    vector<IntermediateInstruction>& getInstructions();
    void add(const IntermediateInstruction& instruction);
    size_t size() const;
    bool empty() const;
    const IntermediateInstruction& at(size_t index) const;
    void clear();
    void print(ostream& out) const;
};
