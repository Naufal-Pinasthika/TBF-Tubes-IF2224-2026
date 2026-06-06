#include "IntermediateCode.hpp"

#include <string>

using namespace std;

string ICOperand::toString() const
{
    switch (kind)
    {
        case ICOperandKind::None:
            return "";
        case ICOperandKind::Literal:
            return literal.text;
        case ICOperandKind::Address:
            return to_string(address);
        case ICOperandKind::Label:
            return label;
        case ICOperandKind::Operation:
            return to_string(static_cast<int>(operationCode));
    }

    return "";
}

string IntermediateInstruction::toString() const {
    string text;
    switch (opcode)
    {
        case ICOpCode::Lit:
            text = "LIT " + to_string(level) + " " + operand.toString();
            break;
        case ICOpCode::Lda:
            text = "LDA " + to_string(level) + " " + operand.toString();
            break;
        case ICOpCode::Lod:
            text = "LOD " + to_string(level) + " " + operand.toString();
            break;
        case ICOpCode::Ldi:
            text = "LDI";
            break;
        case ICOpCode::Sto:
            text = "STO " + to_string(level) + " " + operand.toString();
            break;
        case ICOpCode::Sti:
            text = "STI";
            break;
        case ICOpCode::Add:
            text = "ADD";
            break;
        case ICOpCode::Sub:
            text = "SUB";
            break;
        case ICOpCode::Mul:
            text = "MUL";
            break;
        case ICOpCode::Div:
            text = "DIV";
            break;
        case ICOpCode::Opr:
            text = "OPR " + to_string(level) + " " + operand.toString();
            break;
        case ICOpCode::Jmp:
            text = "JMP " + to_string(level) + " " + operand.toString();
            break;
        case ICOpCode::Jpc:
            text = "JPC " + to_string(level) + " " + operand.toString();
            break;
        case ICOpCode::Cal:
            text = "CAL " + to_string(level) + " " + operand.toString();
            break;
        case ICOpCode::Ret:
            text = "RET";
            break;
        case ICOpCode::Int:
            text = "INT " + to_string(level) + " " + operand.toString();
            break;
        case ICOpCode::Label:
            if (operand.toString().empty()){
                text = "LABEL";
            } else { 
                text = operand.toString() + ":";
            }
            break;
    }

    return text;
}

const vector<IntermediateInstruction>& ICProgram::getInstructions() const {
    return instructions;
}

vector<IntermediateInstruction>& ICProgram::getInstructions() {
    return instructions;
}

void ICProgram::add(const IntermediateInstruction& instruction) {
    instructions.push_back(instruction);
}

size_t ICProgram::size() const {
    return instructions.size();
}

bool ICProgram::empty() const {
    return instructions.empty();
}

const IntermediateInstruction& ICProgram::at(size_t index) const {
    return instructions.at(index);
}

void ICProgram::clear() {
    instructions.clear();
}

void ICProgram::print(ostream& out) const {
    for (size_t i = 0; i < instructions.size(); ++i){
        out << i << " " << instructions[i].toString() << '\n';
    }
}
