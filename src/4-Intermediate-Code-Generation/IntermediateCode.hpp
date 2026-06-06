#pragma once

#include <cstddef>
#include <ostream>
#include <string>
#include <vector>

using namespace std;

// refer to this https://docs.google.com/document/d/1pAy0sLZaSylTLXS1yBc7Snu4dHsAE3fJXbY24DkHeic/edit?tab=t.0
enum class ICOpCode
{
    Lit, // push(v)
    Lda, // push(a)
    Lod, // push(mem[a])
    Ldi, // addr=pop; push(mem[addr])
    Sto, // pop(val); mem[a] = val
    Sti, // val=pop; addr=pop; mem[addr] = val
    Add, // v2=pop; v1=pop; push(v1+v2)
    Sub, // v2=pop; v1=pop; push(v1-v2) 
    Mul, // v2=pop; v1=pop; push(v1*v2)
    Div, // v2=pop; v1=pop; push(v1/v2)
    Opr, // execute ICOperation for non-core stack operations
    Jmp, // PC = l
    Jpc, // val=pop; if val==0 then PC=l
    Cal, // Simpan konteks (PC, BP)
    Ret, // Pulihkan konteks    
    Int, // Initiate Memory size of m
    Label // Kinda need it to define things
};

enum class ICOperation
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

enum class ICValueType
{
    None,
    Integer,
    Real,
    Boolean,
    Char,
    String
};

enum class ICOperandKind
{
    None,       // instruction without operand (ADD, RET)
    Literal,    // operand of literal value (LIT)
    Address,    // operand of memory address (LOD, STO, INT)
    Label,      // operand of jump target/call (JMP, JPC, CAL)
    Operation   // additional operation
};

struct ICValue
{
    ICValueType type = ICValueType::None;
    string text;
};

struct ICOperand
{
    ICOperandKind kind = ICOperandKind::None;
    ICValue literal;
    int address = 0;
    string label;
    ICOperation operationCode = ICOperation::None;

    string toString() const;
};

struct IntermediateInstruction
{
    ICOpCode opcode = ICOpCode::Label;
    int level = 0;
    ICOperand operand;

    string toString() const;
};

class ICProgram
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
