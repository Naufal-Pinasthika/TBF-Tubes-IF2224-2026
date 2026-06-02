#include "IntermediateCode.hpp"

#include <string>

using namespace std;

string TacOperand::toString() const
{
    switch (kind)
    {
        case TacOperandKind::None:
            return "";
        case TacOperandKind::Literal:
            return literal.text;
        case TacOperandKind::Address:
            return to_string(address);
        case TacOperandKind::Label:
            return label;
        case TacOperandKind::Operation:
            return to_string(static_cast<int>(operationCode));
    }

    return "";
}

string IntermediateInstruction::toString() const {
    string text;
    switch (opcode)
    {
        case TacOpcode::Lit:
            text = "LIT " + to_string(level) + " " + operand.toString();
            break;
        case TacOpcode::Lda:
            text = "LDA " + to_string(level) + " " + operand.toString();
            break;
        case TacOpcode::Lod:
            text = "LOD " + to_string(level) + " " + operand.toString();
            break;
        case TacOpcode::Ldi:
            text = "LDI";
            break;
        case TacOpcode::Sto:
            text = "STO " + to_string(level) + " " + operand.toString();
            break;
        case TacOpcode::Sti:
            text = "STI";
            break;
        case TacOpcode::Add:
            text = "ADD";
            break;
        case TacOpcode::Sub:
            text = "SUB";
            break;
        case TacOpcode::Mul:
            text = "MUL";
            break;
        case TacOpcode::Div:
            text = "DIV";
            break;
        case TacOpcode::Opr:
            text = "OPR " + to_string(level) + " " + operand.toString();
            break;
        case TacOpcode::Jmp:
            text = "JMP " + to_string(level) + " " + operand.toString();
            break;
        case TacOpcode::Jpc:
            text = "JPC " + to_string(level) + " " + operand.toString();
            break;
        case TacOpcode::Cal:
            text = "CAL " + to_string(level) + " " + operand.toString();
            break;
        case TacOpcode::Ret:
            text = "RET";
            break;
        case TacOpcode::Int:
            text = "INT " + to_string(level) + " " + operand.toString();
            break;
        case TacOpcode::Label:
            if (operand.toString().empty()){
                text = "LABEL";
            } else { 
                text = operand.toString() + ":";
            }
            break;
    }

    return text;
}

const vector<IntermediateInstruction>& TacProgram::getInstructions() const {
    return instructions;
}

vector<IntermediateInstruction>& TacProgram::getInstructions() {
    return instructions;
}

void TacProgram::add(const IntermediateInstruction& instruction) {
    instructions.push_back(instruction);
}

size_t TacProgram::size() const {
    return instructions.size();
}

bool TacProgram::empty() const {
    return instructions.empty();
}

const IntermediateInstruction& TacProgram::at(size_t index) const {
    return instructions.at(index);
}

void TacProgram::clear() {
    instructions.clear();
}

void TacProgram::print(ostream& out) const {
    for (size_t i = 0; i < instructions.size(); ++i){
        out << i << " " << instructions[i].toString() << '\n';
    }
}
