#include "Interpreter.hpp"

#include <cctype>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace
{
    const long long IC_INT_MIN = -2147483648LL;
    const long long IC_INT_MAX = 2147483647LL;

    ICValue defaultValue()
    {
        return ICValue{};
    }

    void requireInitialized(const ICValue& value, const string& context)
    {
        if (value.type == ICValueType::None)
        {
            throw runtime_error("uninitialized value access" + context);
        }
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

    long long ensureIntegerRange(long long number, const string& context)
    {
        string suffix = context.empty() ? "" : " in " + context;
        if (number > IC_INT_MAX)
        {
            throw runtime_error("Integer Overflow" + suffix);
        }
        if (number < IC_INT_MIN)
        {
            throw runtime_error("Integer Underflow" + suffix);
        }
        return number;
    }

    double ensureFiniteReal(double number, const string& context)
    {
        if (!isfinite(number))
        {
            string suffix = context.empty() ? "" : " in " + context;
            throw runtime_error("real overflow" + suffix);
        }
        return number;
    }

    bool isNegativeIntegerText(const string& text)
    {
        size_t index = 0;
        while (index < text.size() && isspace(static_cast<unsigned char>(text[index]))) ++index;
        return index < text.size() && text[index] == '-';
    }

    ICValue makeInteger(long long number)
    {
        ensureIntegerRange(number, "integer value");
        ICValue value;
        value.type = ICValueType::Integer;
        value.text = to_string(number);
        return value;
    }

    ICValue makeAddress(long long address, long long base, long long limit)
    {
        ICValue value;
        value.type = ICValueType::Integer;
        value.text = to_string(address);
        value.isAddress = true;
        value.addressBase = base;
        value.addressLimit = limit;
        return value;
    }

    ICValue makeReal(double number)
    {
        ensureFiniteReal(number, "real value");
        ICValue value;
        value.type = ICValueType::Real;
        value.text = formatReal(number);
        return value;
    }

    ICValue makeBoolean(bool boolean)
    {
        ICValue value;
        value.type = ICValueType::Boolean;
        value.text = boolean ? "1" : "0";
        return value;
    }

    bool isNumericValue(const ICValue& value)
    {
        return value.type == ICValueType::Integer ||
               value.type == ICValueType::Real ||
               value.type == ICValueType::Boolean;
    }

    bool isRealValue(const ICValue& value)
    {
        return value.type == ICValueType::Real;
    }

    double toReal(const ICValue& value)
    {
        requireInitialized(value, "");
        if (value.text.empty()) return 0.0;

        try
        {
            double number = stod(value.text);
            return ensureFiniteReal(number, "real value");
        }
        catch (const exception&)
        {
            throw runtime_error("expected numeric value, got '" + value.text + "'");
        }
    }

    long long toRawInteger(const ICValue& value)
    {
        requireInitialized(value, "");
        if (value.text.empty()) return 0;

        try
        {
            size_t parsed = 0;
            long long number = stoll(value.text, &parsed);
            if (parsed != value.text.size())
            {
                throw invalid_argument("trailing characters");
            }
            return number;
        }
        catch (const out_of_range&)
        {
            throw runtime_error(string(isNegativeIntegerText(value.text) ? "Integer Underflow" : "Integer Overflow") +
                                " in integer value");
        }
        catch (const invalid_argument&)
        {
            throw runtime_error("expected integer value, got '" + value.text + "'");
        }
        catch (const exception&)
        {
            throw runtime_error("expected integer value, got '" + value.text + "'");
        }
    }

    long long toInteger(const ICValue& value)
    {
        return ensureIntegerRange(toRawInteger(value), "integer value");
    }

    size_t toAddress(const ICValue& value)
    {
        long long address = toRawInteger(value);
        if (address < 0)
        {
            throw runtime_error("negative memory address: " + to_string(address));
        }

        return static_cast<size_t>(address);
    }

    long long checkedAdd(long long left, long long right)
    {
        return ensureIntegerRange(left + right, "addition");
    }

    long long checkedSub(long long left, long long right)
    {
        return ensureIntegerRange(left - right, "subtraction");
    }

    long long checkedMul(long long left, long long right)
    {
        return ensureIntegerRange(left * right, "multiplication");
    }

    long long checkedDiv(long long left, long long right)
    {
        if (right == 0)
        {
            throw runtime_error("division by zero");
        }
        if (left == IC_INT_MIN && right == -1)
        {
            throw runtime_error("Integer Overflow in division");
        }
        return ensureIntegerRange(left / right, "division");
    }

    long long checkedMod(long long left, long long right)
    {
        if (right == 0)
        {
            throw runtime_error("modulo by zero");
        }
        if (left == IC_INT_MIN && right == -1)
        {
            throw runtime_error("Integer Overflow in modulo");
        }
        return ensureIntegerRange(left % right, "modulo");
    }

    long long checkedNeg(long long number)
    {
        if (number == IC_INT_MIN)
        {
            throw runtime_error("Integer Overflow in negation");
        }
        return ensureIntegerRange(-number, "negation");
    }

    void validateStackValue(const ICValue& value)
    {
        if (value.type == ICValueType::Integer && !value.isAddress)
        {
            toInteger(value);
        }
        else if (value.type == ICValueType::Real)
        {
            toReal(value);
        }
    }

    string outputText(const ICValue& value)
    {
        requireInitialized(value, "");
        if (value.type == ICValueType::Boolean)
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

    bool isProtectedFrameMetadataAddress(const vector<StackFrame>& callStack, size_t address)
    {
        for (const StackFrame& frame : callStack)
        {
            size_t protectedSlots = frame.frameSize < 3 ? frame.frameSize : 3;
            if (address >= frame.basePointer && address < frame.basePointer + protectedSlots)
            {
                return true;
            }
        }
        return false;
    }

    void validateAddressRange(const ICValue& addressValue, size_t address)
    {
        if (!addressValue.isAddress)
        {
            throw runtime_error("Memory Access Out of Bounds: indirect access requires an address");
        }

        long long rawAddress = static_cast<long long>(address);
        if (rawAddress < addressValue.addressBase || rawAddress >= addressValue.addressLimit)
        {
            throw runtime_error("Memory Access Out of Bounds: address " + to_string(rawAddress) +
                                " is outside allocated range [" + to_string(addressValue.addressBase) +
                                ", " + to_string(addressValue.addressLimit) + ")");
        }
    }

    bool labelsFunction(const string& label)
    {
        return label.size() >= 2 && label[0] == 'F' && label[1] == '_';
    }
}

void Interpreter::load(const ICProgram& program)
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
            case ICOpCode::Lit:
                executeLiteral(instruction);
                break;
            case ICOpCode::Lda:
                executeLoadAddress(instruction);
                break;
            case ICOpCode::Lod:
                executeLoad(instruction);
                break;
            case ICOpCode::Ldi:
                executeIndirectLoad();
                break;
            case ICOpCode::Sto:
                executeStore(instruction);
                break;
            case ICOpCode::Sti:
                executeIndirectStore();
                break;
            case ICOpCode::Add:
            case ICOpCode::Sub:
            case ICOpCode::Mul:
            case ICOpCode::Div:
                executeArithmetic(instruction);
                break;
            case ICOpCode::Opr:
                executeOperation(instruction);
                break;
            case ICOpCode::Jmp:
                executeJump(instruction);
                break;
            case ICOpCode::Jpc:
                executeConditionalJump(instruction);
                break;
            case ICOpCode::Cal:
                executeCall(instruction);
                break;
            case ICOpCode::Ret:
                executeReturn(instruction);
                break;
            case ICOpCode::Int:
                executeInt(instruction);
                break;
            case ICOpCode::Label:
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

const vector<ICValue>& Interpreter::getMemory() const
{
    return memory;
}

const vector<StackFrame>& Interpreter::getCallStack() const
{
    return callStack;
}

const vector<ICValue>& Interpreter::getEvalStack() const
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
        if (instruction.opcode != ICOpCode::Label) continue;
        if (instruction.operand.kind != ICOperandKind::Label || instruction.operand.label.empty()) continue;

        const string& label = instruction.operand.label;
        if (labelMap.find(label) != labelMap.end())
        {
            throw runtime_error("Duplicate label: " + label);
        }
        labelMap[label] = i;
    }
}

void Interpreter::pushFrame(int level, int frameSize, int returnAddress, int staticLink, int dynamicLink, int argumentCount, bool expectsReturnValue)
{
    if (callStack.size() >= maxCallDepth)
    {
        throw runtime_error("Stack Overflow: maximum call depth exceeded");
    }

    if (frameSize < 0)
    {
        throw runtime_error("negative frame size: " + to_string(frameSize));
    }

    if (argumentCount < 0)
    {
        throw runtime_error("negative argument count: " + to_string(argumentCount));
    }

    if (evalStack.size() < static_cast<size_t>(argumentCount))
    {
        throw runtime_error("Stack Underflow: missing call arguments");
    }

    StackFrame frame;
    frame.level = level;
    frame.returnAddress = returnAddress;
    frame.staticLink = staticLink;
    frame.dynamicLink = dynamicLink;
    frame.basePointer = memory.size();
    frame.frameSize = static_cast<size_t>(frameSize);
    frame.evalStackBase = evalStack.size() - static_cast<size_t>(argumentCount);
    frame.argumentCount = argumentCount;
    frame.expectsReturnValue = expectsReturnValue;

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
        throw runtime_error("Stack Underflow: no frame to return from");
    }

    StackFrame frame = callStack.back();

    if (frame.basePointer > memory.size())
    {
        throw runtime_error("Stack Corruption: invalid frame base pointer");
    }

    if (frame.frameSize > memory.size() - frame.basePointer)
    {
        throw runtime_error("Stack Corruption: invalid frame extent");
    }

    if (frame.frameSize > 0 && toRawInteger(memory[frame.basePointer]) != frame.staticLink)
    {
        throw runtime_error("Stack Corruption: static link was overwritten");
    }
    if (frame.frameSize > 1 && toRawInteger(memory[frame.basePointer + 1]) != frame.dynamicLink)
    {
        throw runtime_error("Stack Corruption: dynamic link was overwritten");
    }
    if (frame.frameSize > 2 && toRawInteger(memory[frame.basePointer + 2]) != frame.returnAddress)
    {
        throw runtime_error("Stack Corruption: return address was overwritten");
    }

    callStack.pop_back();

    if (frame.frameSize > 0)
    {
        memory.resize(frame.basePointer);
    }

    bp = callStack.empty() ? 0 : callStack.back().basePointer;
    sp = evalStack.size();
}

ICValue Interpreter::popValue()
{
    if (evalStack.empty())
    {
        throw runtime_error("Stack Underflow: operand stack is empty");
    }

    ICValue value = evalStack.back();
    evalStack.pop_back();
    sp = evalStack.size();
    return value;
}

void Interpreter::pushValue(const ICValue& value)
{
    if (evalStack.size() >= maxEvalStackSize)
    {
        throw runtime_error("Stack Overflow: operand stack limit exceeded");
    }

    validateStackValue(value);
    evalStack.push_back(value);
    sp = evalStack.size();
}

ICValue Interpreter::readOperand(const ICOperand& operand) const
{
    switch (operand.kind)
    {
        case ICOperandKind::Literal:
            return operand.literal;
        case ICOperandKind::Address:
            return makeInteger(operand.address);
        case ICOperandKind::Operation:
            return makeInteger(static_cast<int>(operand.operationCode));
        case ICOperandKind::Label:
        {
            ICValue value;
            value.type = ICValueType::String;
            value.text = operand.label;
            return value;
        }
        case ICOperandKind::None:
            return ICValue{};
    }

    return ICValue{};
}

int Interpreter::readAddressOperand(const IntermediateInstruction& instruction) const
{
    if (instruction.operand.kind != ICOperandKind::Address)
    {
        throw runtime_error("instruction expects address operand: " + instruction.toString());
    }

    return instruction.operand.address;
}

string Interpreter::readLabelOperand(const IntermediateInstruction& instruction) const
{
    if (instruction.operand.kind != ICOperandKind::Label || instruction.operand.label.empty())
    {
        throw runtime_error("instruction expects label operand: " + instruction.toString());
    }

    if (labelMap.find(instruction.operand.label) == labelMap.end())
    {
        throw runtime_error("Label not found: " + instruction.operand.label);
    }

    return instruction.operand.label;
}

ICOperation Interpreter::readOperationOperand(const IntermediateInstruction& instruction) const
{
    if (instruction.operand.kind != ICOperandKind::Operation)
    {
        throw runtime_error("instruction expects operation operand: " + instruction.toString());
    }

    return instruction.operand.operationCode;
}

void Interpreter::storeAtAddress(int level, int address, const ICValue& value)
{
    if (address < 0)
    {
        throw runtime_error("negative memory address: " + to_string(address));
    }
    if (address < 3)
    {
        throw runtime_error("Stack Smashing: cannot write stack frame metadata");
    }

    size_t frameIndex = frameIndexForLevel(callStack, level);
    const StackFrame& frame = callStack[frameIndex];
    size_t relativeAddress = static_cast<size_t>(address);

    if (relativeAddress >= frame.frameSize)
    {
        throw runtime_error("Memory Access Out of Bounds at address " + to_string(address));
    }

    size_t absoluteAddress = frame.basePointer + relativeAddress;
    if (absoluteAddress >= memory.size())
    {
        throw runtime_error("Memory Access Out of Bounds at address " + to_string(absoluteAddress));
    }

    memory[absoluteAddress] = value;
}

ICValue Interpreter::loadAtAddress(int level, int address) const
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
        throw runtime_error("Memory Access Out of Bounds at address " + to_string(address));
    }

    size_t absoluteAddress = frame.basePointer + relativeAddress;
    if (absoluteAddress >= memory.size())
    {
        throw runtime_error("Memory Access Out of Bounds at address " + to_string(absoluteAddress));
    }

    const ICValue& value = memory[absoluteAddress];
    requireInitialized(value, " at address " + to_string(address));
    return value;
}

bool Interpreter::isTruthy(const ICValue& value) const
{
    requireInitialized(value, "");
    if (value.text.empty()) return false;
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
        pushFrame(instruction.level, frameSize, -1, -1, -1, 0, false);
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
    if (instruction.operand.kind != ICOperandKind::Literal)
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
    size_t span = instruction.operand.addressSpan > 0 ? static_cast<size_t>(instruction.operand.addressSpan) : 1;
    if (relativeAddress >= frame.frameSize || span > frame.frameSize - relativeAddress)
    {
        throw runtime_error("Memory Access Out of Bounds at address " + to_string(address));
    }

    long long absoluteAddress = static_cast<long long>(frame.basePointer + relativeAddress);
    pushValue(makeAddress(absoluteAddress, absoluteAddress, absoluteAddress + static_cast<long long>(span)));
}

void Interpreter::executeLoad(const IntermediateInstruction& instruction)
{
    pushValue(loadAtAddress(instruction.level, readAddressOperand(instruction)));
}

void Interpreter::executeIndirectLoad()
{
    ICValue addressValue = popValue();
    size_t address = toAddress(addressValue);
    if (address >= memory.size())
    {
        throw runtime_error("Memory Access Out of Bounds at address " + to_string(address));
    }
    validateAddressRange(addressValue, address);

    const ICValue& value = memory[address];
    requireInitialized(value, " at address " + to_string(address));
    pushValue(value);
}

void Interpreter::executeStore(const IntermediateInstruction& instruction)
{
    ICValue value = popValue();
    storeAtAddress(instruction.level, readAddressOperand(instruction), value);
}

void Interpreter::executeIndirectStore()
{
    ICValue value = popValue();
    ICValue addressValue = popValue();
    size_t address = toAddress(addressValue);
    if (address >= memory.size())
    {
        throw runtime_error("Memory Access Out of Bounds at address " + to_string(address));
    }
    validateAddressRange(addressValue, address);
    if (isProtectedFrameMetadataAddress(callStack, address))
    {
        throw runtime_error("Stack Smashing: cannot write stack frame metadata");
    }

    memory[address] = value;
}

void Interpreter::executeArithmetic(const IntermediateInstruction& instruction)
{
    ICValue right = popValue();
    ICValue left = popValue();
    bool realResult = isRealValue(left) || isRealValue(right);

    if (instruction.opcode == ICOpCode::Div)
    {
        double denominator = toReal(right);
        if (denominator == 0.0)
        {
            throw runtime_error("division by zero");
        }

        if (realResult)
        {
            pushValue(makeReal(ensureFiniteReal(toReal(left) / denominator, "division")));
        }
        else
        {
            pushValue(makeInteger(checkedDiv(toInteger(left), toInteger(right))));
        }
        return;
    }

    if (realResult)
    {
        double leftNumber = toReal(left);
        double rightNumber = toReal(right);

        if (instruction.opcode == ICOpCode::Add) pushValue(makeReal(ensureFiniteReal(leftNumber + rightNumber, "addition")));
        else if (instruction.opcode == ICOpCode::Sub) pushValue(makeReal(ensureFiniteReal(leftNumber - rightNumber, "subtraction")));
        else if (instruction.opcode == ICOpCode::Mul) pushValue(makeReal(ensureFiniteReal(leftNumber * rightNumber, "multiplication")));
        else throw runtime_error("unsupported arithmetic instruction");
    }
    else
    {
        long long leftNumber = toInteger(left);
        long long rightNumber = toInteger(right);
        long long result = 0;

        if (instruction.opcode == ICOpCode::Add) result = checkedAdd(leftNumber, rightNumber);
        else if (instruction.opcode == ICOpCode::Sub) result = checkedSub(leftNumber, rightNumber);
        else if (instruction.opcode == ICOpCode::Mul) result = checkedMul(leftNumber, rightNumber);
        else throw runtime_error("unsupported arithmetic instruction");

        if (instruction.opcode == ICOpCode::Add && left.isAddress != right.isAddress)
        {
            const ICValue& source = left.isAddress ? left : right;
            pushValue(makeAddress(result, source.addressBase, source.addressLimit));
            return;
        }

        if (instruction.opcode == ICOpCode::Sub && left.isAddress && !right.isAddress)
        {
            pushValue(makeAddress(result, left.addressBase, left.addressLimit));
            return;
        }

        pushValue(makeInteger(result));
    }
}

void Interpreter::executeOperation(const IntermediateInstruction& instruction)
{
    ICOperation operation = readOperationOperand(instruction);

    if (operation == ICOperation::Wrt)
    {
        cout << outputText(popValue());
        return;
    }

    if (operation == ICOperation::Wrtln)
    {
        if (!evalStack.empty()) cout << outputText(popValue());
        cout << '\n';
        return;
    }

    if (operation == ICOperation::Neg)
    {
        ICValue value = popValue();
        if (isRealValue(value)) pushValue(makeReal(ensureFiniteReal(-toReal(value), "negation")));
        else pushValue(makeInteger(checkedNeg(toInteger(value))));
        return;
    }

    if (operation == ICOperation::Mod)
    {
        ICValue right = popValue();
        ICValue left = popValue();
        pushValue(makeInteger(checkedMod(toInteger(left), toInteger(right))));
        return;
    }

    if (operation == ICOperation::Add ||
        operation == ICOperation::Sub ||
        operation == ICOperation::Mul ||
        operation == ICOperation::Div)
    {
        IntermediateInstruction arithmetic;
        arithmetic.opcode = operation == ICOperation::Add ? ICOpCode::Add :
                            operation == ICOperation::Sub ? ICOpCode::Sub :
                            operation == ICOperation::Mul ? ICOpCode::Mul :
                            ICOpCode::Div;
        executeArithmetic(arithmetic);
        return;
    }

    ICValue right = popValue();
    ICValue left = popValue();
    bool numericComparison = isNumericValue(left) && isNumericValue(right);
    bool result = false;

    switch (operation)
    {
        case ICOperation::Eql:
            result = numericComparison ? fabs(toReal(left) - toReal(right)) < numeric_limits<double>::epsilon()
                                       : left.text == right.text;
            break;
        case ICOperation::Neq:
            result = numericComparison ? fabs(toReal(left) - toReal(right)) >= numeric_limits<double>::epsilon()
                                       : left.text != right.text;
            break;
        case ICOperation::Lss:
            result = numericComparison ? toReal(left) < toReal(right)
                                       : left.text < right.text;
            break;
        case ICOperation::Geq:
            result = numericComparison ? toReal(left) >= toReal(right)
                                       : left.text >= right.text;
            break;
        case ICOperation::Gtr:
            result = numericComparison ? toReal(left) > toReal(right)
                                       : left.text > right.text;
            break;
        case ICOperation::Leq:
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
    ICValue condition = popValue();
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
    pushFrame(instruction.level, 0, static_cast<int>(pc), currentFrameIndex, currentFrameIndex, instruction.argumentCount, labelsFunction(label));
    pc = labelMap.at(label);
}

void Interpreter::executeReturn(const IntermediateInstruction&)
{
    if (callStack.empty())
    {
        throw runtime_error("Stack Underflow: no frame to return from");
    }

    const StackFrame& frame = callStack.back();
    int returnAddress = frame.returnAddress;
    size_t maxResultCount = frame.expectsReturnValue ? 1 : 0;
    size_t maxExpectedStackSize = frame.evalStackBase + maxResultCount;

    if (evalStack.size() < frame.evalStackBase)
    {
        throw runtime_error("Stack Corruption: operand stack below frame base");
    }

    if (evalStack.size() > maxExpectedStackSize)
    {
        throw runtime_error("Stack Corruption: operand stack has leftover values at return");
    }

    if (returnAddress >= 0 && (program == nullptr || static_cast<size_t>(returnAddress) >= program->size()))
    {
        throw runtime_error("Stack Corruption: invalid return address");
    }

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
