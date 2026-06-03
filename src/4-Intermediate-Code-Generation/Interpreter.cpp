#include "Interpreter.hpp"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace
{
    TacValue defaultValue()
    {
        TacValue value;
        value.type = TacValueType::Integer;
        value.text = "0";
        return value;
    }

    string formatReal(double value)
    {
        if (value == 0.0) return "0";

        ostringstream out;
        out << setprecision(12) << value;
        string text = out.str();

        size_t dot = text.find('.');
        if (dot != string::npos && text.find('e') == string::npos && text.find('E') == string::npos)
        {
            while (!text.empty() && text.back() == '0') text.pop_back();
            if (!text.empty() && text.back() == '.') text.pop_back();
        }

        return text == "-0" ? "0" : text;
    }

    TacValue makeInteger(long long number)
    {
        TacValue value;
        value.type = TacValueType::Integer;
        value.text = to_string(number);
        return value;
    }

    TacValue makeReal(double number)
    {
        TacValue value;
        value.type = TacValueType::Real;
        value.text = formatReal(number);
        return value;
    }

    TacValue makeBoolean(bool boolean)
    {
        TacValue value;
        value.type = TacValueType::Boolean;
        value.text = boolean ? "1" : "0";
        return value;
    }

    bool isNumericValue(const TacValue& value)
    {
        return value.type == TacValueType::Integer ||
               value.type == TacValueType::Real ||
               value.type == TacValueType::Boolean;
    }

    bool isRealValue(const TacValue& value)
    {
        return value.type == TacValueType::Real;
    }

    double toReal(const TacValue& value)
    {
        if (value.text.empty()) return 0.0;

        try
        {
            return stod(value.text);
        }
        catch (const exception&)
        {
            throw runtime_error("expected numeric value, got '" + value.text + "'");
        }
    }

    long long toInteger(const TacValue& value)
    {
        if (value.text.empty()) return 0;

        try
        {
            return stoll(value.text);
        }
        catch (const exception&)
        {
            throw runtime_error("expected integer value, got '" + value.text + "'");
        }
    }

    size_t toAddress(const TacValue& value)
    {
        long long address = toInteger(value);
        if (address < 0)
        {
            throw runtime_error("negative memory address: " + to_string(address));
        }

        return static_cast<size_t>(address);
    }

    string outputText(const TacValue& value)
    {
        if (value.type == TacValueType::Boolean)
        {
            return toInteger(value) == 0 ? "false" : "true";
        }

        return value.text;
    }

    size_t frameIndexForLevel(const vector<StackFrame>& callStack, int level)
    {
        if (callStack.empty())
        {
            throw runtime_error("no active stack frame");
        }

        if (level < 0)
        {
            throw runtime_error("negative lexical level");
        }

        size_t index = callStack.size() - 1;
        for (int i = 0; i < level; ++i)
        {
            if (index == 0)
            {
                throw runtime_error("lexical level out of range: " + to_string(level));
            }
            --index;
        }

        return index;
    }
}

void Interpreter::load(const TacProgram& program)
{
    reset();
    this->program = &program;
    buildLabelMap();
}

void Interpreter::run()
{
    if (program == nullptr)
    {
        throw runtime_error("no intermediate code loaded");
    }

    while (pc < program->size())
    {
        const IntermediateInstruction& instruction = program->at(pc);
        ++pc;

        switch (instruction.opcode)
        {
            case TacOpcode::Lit:
                executeLiteral(instruction);
                break;
            case TacOpcode::Lda:
                executeLoadAddress(instruction);
                break;
            case TacOpcode::Lod:
                executeLoad(instruction);
                break;
            case TacOpcode::Ldi:
                executeIndirectLoad();
                break;
            case TacOpcode::Sto:
                executeStore(instruction);
                break;
            case TacOpcode::Sti:
                executeIndirectStore();
                break;
            case TacOpcode::Add:
            case TacOpcode::Sub:
            case TacOpcode::Mul:
            case TacOpcode::Div:
                executeArithmetic(instruction);
                break;
            case TacOpcode::Opr:
                executeOperation(instruction);
                break;
            case TacOpcode::Jmp:
                executeJump(instruction);
                break;
            case TacOpcode::Jpc:
                executeConditionalJump(instruction);
                break;
            case TacOpcode::Cal:
                executeCall(instruction);
                break;
            case TacOpcode::Ret:
                executeReturn(instruction);
                break;
            case TacOpcode::Int:
                executeInt(instruction);
                break;
            case TacOpcode::Label:
                break;
        }
    }
}

void Interpreter::reset()
{
    program = nullptr;
    memory.clear();
    callStack.clear();
    evalStack.clear();
    labelMap.clear();
    pc = 0;
    bp = 0;
    sp = 0;
}

const vector<TacValue>& Interpreter::getMemory() const
{
    return memory;
}

const vector<StackFrame>& Interpreter::getCallStack() const
{
    return callStack;
}

const vector<TacValue>& Interpreter::getEvalStack() const
{
    return evalStack;
}

size_t Interpreter::getProgramCounter() const
{
    return pc;
}

size_t Interpreter::getBasePointer() const
{
    return bp;
}

size_t Interpreter::getStackPointer() const
{
    return sp;
}

void Interpreter::buildLabelMap()
{
    if (program == nullptr) return;

    for (size_t i = 0; i < program->size(); ++i)
    {
        const IntermediateInstruction& instruction = program->at(i);
        if (instruction.opcode != TacOpcode::Label) continue;
        if (instruction.operand.kind != TacOperandKind::Label || instruction.operand.label.empty()) continue;

        const string& label = instruction.operand.label;
        if (labelMap.find(label) != labelMap.end())
        {
            throw runtime_error("duplicate label: " + label);
        }
        labelMap[label] = i;
    }
}

void Interpreter::pushFrame(int level, int frameSize, int returnAddress, int staticLink, int dynamicLink)
{
    if (callStack.size() >= maxCallDepth)
    {
        throw runtime_error("stack overflow: maximum call depth exceeded");
    }

    if (frameSize < 0)
    {
        throw runtime_error("negative frame size: " + to_string(frameSize));
    }

    StackFrame frame;
    frame.level = level;
    frame.returnAddress = returnAddress;
    frame.staticLink = staticLink;
    frame.dynamicLink = dynamicLink;
    frame.basePointer = memory.size();
    frame.frameSize = static_cast<size_t>(frameSize);

    callStack.push_back(frame);

    if (frameSize > 0)
    {
        memory.resize(frame.basePointer + frame.frameSize, defaultValue());
        if (frame.frameSize > 0) memory[frame.basePointer] = makeInteger(staticLink);
        if (frame.frameSize > 1) memory[frame.basePointer + 1] = makeInteger(dynamicLink);
        if (frame.frameSize > 2) memory[frame.basePointer + 2] = makeInteger(returnAddress);
    }

    bp = frame.basePointer;
    sp = evalStack.size();
}

void Interpreter::popFrame()
{
    if (callStack.empty())
    {
        throw runtime_error("stack underflow: no frame to return from");
    }

    StackFrame frame = callStack.back();
    callStack.pop_back();

    if (frame.frameSize > 0)
    {
        if (frame.basePointer > memory.size())
        {
            throw runtime_error("stack corruption: invalid frame base pointer");
        }
        memory.resize(frame.basePointer);
    }

    bp = callStack.empty() ? 0 : callStack.back().basePointer;
    sp = evalStack.size();
}

TacValue Interpreter::popValue()
{
    if (evalStack.empty())
    {
        throw runtime_error("stack underflow: operand stack is empty");
    }

    TacValue value = evalStack.back();
    evalStack.pop_back();
    sp = evalStack.size();
    return value;
}

void Interpreter::pushValue(const TacValue& value)
{
    if (evalStack.size() >= maxEvalStackSize)
    {
        throw runtime_error("stack overflow: operand stack limit exceeded");
    }

    evalStack.push_back(value);
    sp = evalStack.size();
}

TacValue Interpreter::readOperand(const TacOperand& operand) const
{
    switch (operand.kind)
    {
        case TacOperandKind::Literal:
            return operand.literal;
        case TacOperandKind::Address:
            return makeInteger(operand.address);
        case TacOperandKind::Operation:
            return makeInteger(static_cast<int>(operand.operationCode));
        case TacOperandKind::Label:
        {
            TacValue value;
            value.type = TacValueType::String;
            value.text = operand.label;
            return value;
        }
        case TacOperandKind::None:
            return TacValue{};
    }

    return TacValue{};
}

int Interpreter::readAddressOperand(const IntermediateInstruction& instruction) const
{
    if (instruction.operand.kind != TacOperandKind::Address)
    {
        throw runtime_error("instruction expects address operand: " + instruction.toString());
    }

    return instruction.operand.address;
}

string Interpreter::readLabelOperand(const IntermediateInstruction& instruction) const
{
    if (instruction.operand.kind != TacOperandKind::Label || instruction.operand.label.empty())
    {
        throw runtime_error("instruction expects label operand: " + instruction.toString());
    }

    if (labelMap.find(instruction.operand.label) == labelMap.end())
    {
        throw runtime_error("label not found: " + instruction.operand.label);
    }

    return instruction.operand.label;
}

TacOperation Interpreter::readOperationOperand(const IntermediateInstruction& instruction) const
{
    if (instruction.operand.kind != TacOperandKind::Operation)
    {
        throw runtime_error("instruction expects operation operand: " + instruction.toString());
    }

    return instruction.operand.operationCode;
}

void Interpreter::storeAtAddress(int level, int address, const TacValue& value)
{
    if (address < 0)
    {
        throw runtime_error("negative memory address: " + to_string(address));
    }

    size_t frameIndex = frameIndexForLevel(callStack, level);
    const StackFrame& frame = callStack[frameIndex];
    size_t relativeAddress = static_cast<size_t>(address);

    if (relativeAddress >= frame.frameSize)
    {
        throw runtime_error("memory access out of bounds at address " + to_string(address));
    }

    size_t absoluteAddress = frame.basePointer + relativeAddress;
    if (absoluteAddress >= memory.size())
    {
        throw runtime_error("memory access out of bounds at address " + to_string(absoluteAddress));
    }

    memory[absoluteAddress] = value;
}

TacValue Interpreter::loadAtAddress(int level, int address) const
{
    if (address < 0)
    {
        throw runtime_error("negative memory address: " + to_string(address));
    }

    size_t frameIndex = frameIndexForLevel(callStack, level);
    const StackFrame& frame = callStack[frameIndex];
    size_t relativeAddress = static_cast<size_t>(address);

    if (relativeAddress >= frame.frameSize)
    {
        throw runtime_error("memory access out of bounds at address " + to_string(address));
    }

    size_t absoluteAddress = frame.basePointer + relativeAddress;
    if (absoluteAddress >= memory.size())
    {
        throw runtime_error("memory access out of bounds at address " + to_string(absoluteAddress));
    }

    return memory[absoluteAddress];
}

bool Interpreter::isTruthy(const TacValue& value) const
{
    if (value.type == TacValueType::None || value.text.empty()) return false;
    if (isNumericValue(value)) return toReal(value) != 0.0;
    return value.text != "false" && value.text != "FALSE";
}

void Interpreter::executeInt(const IntermediateInstruction& instruction)
{
    int frameSize = readAddressOperand(instruction);
    if (frameSize < 0)
    {
        throw runtime_error("negative frame size: " + to_string(frameSize));
    }

    if (callStack.empty() || callStack.back().frameSize != 0)
    {
        pushFrame(instruction.level, frameSize, -1, -1, -1);
        return;
    }

    StackFrame& frame = callStack.back();
    frame.level = instruction.level;
    frame.frameSize = static_cast<size_t>(frameSize);

    memory.resize(frame.basePointer + frame.frameSize, defaultValue());
    if (frame.frameSize > 0) memory[frame.basePointer] = makeInteger(frame.staticLink);
    if (frame.frameSize > 1) memory[frame.basePointer + 1] = makeInteger(frame.dynamicLink);
    if (frame.frameSize > 2) memory[frame.basePointer + 2] = makeInteger(frame.returnAddress);

    bp = frame.basePointer;
    sp = evalStack.size();
}

void Interpreter::executeLiteral(const IntermediateInstruction& instruction)
{
    if (instruction.operand.kind != TacOperandKind::Literal)
    {
        throw runtime_error("LIT expects literal operand");
    }

    pushValue(instruction.operand.literal);
}

void Interpreter::executeLoadAddress(const IntermediateInstruction& instruction)
{
    int address = readAddressOperand(instruction);
    if (address < 0)
    {
        throw runtime_error("negative memory address: " + to_string(address));
    }

    size_t frameIndex = frameIndexForLevel(callStack, instruction.level);
    const StackFrame& frame = callStack[frameIndex];
    size_t relativeAddress = static_cast<size_t>(address);
    if (relativeAddress >= frame.frameSize)
    {
        throw runtime_error("memory access out of bounds at address " + to_string(address));
    }

    pushValue(makeInteger(static_cast<long long>(frame.basePointer + relativeAddress)));
}

void Interpreter::executeLoad(const IntermediateInstruction& instruction)
{
    pushValue(loadAtAddress(instruction.level, readAddressOperand(instruction)));
}

void Interpreter::executeIndirectLoad()
{
    size_t address = toAddress(popValue());
    if (address >= memory.size())
    {
        throw runtime_error("memory access out of bounds at address " + to_string(address));
    }

    pushValue(memory[address]);
}

void Interpreter::executeStore(const IntermediateInstruction& instruction)
{
    TacValue value = popValue();
    storeAtAddress(instruction.level, readAddressOperand(instruction), value);
}

void Interpreter::executeIndirectStore()
{
    TacValue value = popValue();
    size_t address = toAddress(popValue());
    if (address >= memory.size())
    {
        throw runtime_error("memory access out of bounds at address " + to_string(address));
    }

    memory[address] = value;
}

void Interpreter::executeArithmetic(const IntermediateInstruction& instruction)
{
    TacValue right = popValue();
    TacValue left = popValue();
    bool realResult = isRealValue(left) || isRealValue(right);

    if (instruction.opcode == TacOpcode::Div)
    {
        double denominator = toReal(right);
        if (denominator == 0.0)
        {
            throw runtime_error("division by zero");
        }

        if (realResult)
        {
            pushValue(makeReal(toReal(left) / denominator));
        }
        else
        {
            pushValue(makeInteger(toInteger(left) / toInteger(right)));
        }
        return;
    }

    if (realResult)
    {
        double leftNumber = toReal(left);
        double rightNumber = toReal(right);

        if (instruction.opcode == TacOpcode::Add) pushValue(makeReal(leftNumber + rightNumber));
        else if (instruction.opcode == TacOpcode::Sub) pushValue(makeReal(leftNumber - rightNumber));
        else if (instruction.opcode == TacOpcode::Mul) pushValue(makeReal(leftNumber * rightNumber));
        else throw runtime_error("unsupported arithmetic instruction");
    }
    else
    {
        long long leftNumber = toInteger(left);
        long long rightNumber = toInteger(right);

        if (instruction.opcode == TacOpcode::Add) pushValue(makeInteger(leftNumber + rightNumber));
        else if (instruction.opcode == TacOpcode::Sub) pushValue(makeInteger(leftNumber - rightNumber));
        else if (instruction.opcode == TacOpcode::Mul) pushValue(makeInteger(leftNumber * rightNumber));
        else throw runtime_error("unsupported arithmetic instruction");
    }
}

void Interpreter::executeOperation(const IntermediateInstruction& instruction)
{
    TacOperation operation = readOperationOperand(instruction);

    if (operation == TacOperation::Wrt)
    {
        cout << outputText(popValue());
        return;
    }

    if (operation == TacOperation::Wrtln)
    {
        if (!evalStack.empty()) cout << outputText(popValue());
        cout << '\n';
        return;
    }

    if (operation == TacOperation::Neg)
    {
        TacValue value = popValue();
        if (isRealValue(value)) pushValue(makeReal(-toReal(value)));
        else pushValue(makeInteger(-toInteger(value)));
        return;
    }

    if (operation == TacOperation::Mod)
    {
        TacValue right = popValue();
        TacValue left = popValue();
        long long denominator = toInteger(right);
        if (denominator == 0)
        {
            throw runtime_error("modulo by zero");
        }
        pushValue(makeInteger(toInteger(left) % denominator));
        return;
    }

    if (operation == TacOperation::Add ||
        operation == TacOperation::Sub ||
        operation == TacOperation::Mul ||
        operation == TacOperation::Div)
    {
        IntermediateInstruction arithmetic;
        arithmetic.opcode = operation == TacOperation::Add ? TacOpcode::Add :
                            operation == TacOperation::Sub ? TacOpcode::Sub :
                            operation == TacOperation::Mul ? TacOpcode::Mul :
                            TacOpcode::Div;
        executeArithmetic(arithmetic);
        return;
    }

    TacValue right = popValue();
    TacValue left = popValue();
    bool numericComparison = isNumericValue(left) && isNumericValue(right);
    bool result = false;

    switch (operation)
    {
        case TacOperation::Eql:
            result = numericComparison ? fabs(toReal(left) - toReal(right)) < numeric_limits<double>::epsilon()
                                       : left.text == right.text;
            break;
        case TacOperation::Neq:
            result = numericComparison ? fabs(toReal(left) - toReal(right)) >= numeric_limits<double>::epsilon()
                                       : left.text != right.text;
            break;
        case TacOperation::Lss:
            result = numericComparison ? toReal(left) < toReal(right)
                                       : left.text < right.text;
            break;
        case TacOperation::Geq:
            result = numericComparison ? toReal(left) >= toReal(right)
                                       : left.text >= right.text;
            break;
        case TacOperation::Gtr:
            result = numericComparison ? toReal(left) > toReal(right)
                                       : left.text > right.text;
            break;
        case TacOperation::Leq:
            result = numericComparison ? toReal(left) <= toReal(right)
                                       : left.text <= right.text;
            break;
        default:
            throw runtime_error("unsupported OPR operation: " + to_string(static_cast<int>(operation)));
    }

    pushValue(makeBoolean(result));
}

void Interpreter::executeJump(const IntermediateInstruction& instruction)
{
    string label = readLabelOperand(instruction);
    pc = labelMap.at(label);
}

void Interpreter::executeConditionalJump(const IntermediateInstruction& instruction)
{
    TacValue condition = popValue();
    if (!isTruthy(condition))
    {
        string label = readLabelOperand(instruction);
        pc = labelMap.at(label);
    }
}

void Interpreter::executeCall(const IntermediateInstruction& instruction)
{
    string label = readLabelOperand(instruction);
    int currentFrameIndex = callStack.empty() ? -1 : static_cast<int>(callStack.size() - 1);
    pushFrame(instruction.level, 0, static_cast<int>(pc), currentFrameIndex, currentFrameIndex);
    pc = labelMap.at(label);
}

void Interpreter::executeReturn(const IntermediateInstruction&)
{
    if (callStack.empty())
    {
        pc = program == nullptr ? 0 : program->size();
        return;
    }

    int returnAddress = callStack.back().returnAddress;
    popFrame();

    if (returnAddress < 0)
    {
        pc = program == nullptr ? 0 : program->size();
    }
    else
    {
        pc = static_cast<size_t>(returnAddress);
    }
}
