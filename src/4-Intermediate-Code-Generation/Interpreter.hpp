#pragma once

#include "IntermediateCode.hpp"

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

struct StackFrame
{
    int level = 0;
    int returnAddress = -1;
    int staticLink = -1;
    int dynamicLink = -1;
    size_t basePointer = 0;
    size_t frameSize = 0;
    size_t evalStackBase = 0;
    int argumentCount = 0;
    bool expectsReturnValue = false;
};

class Interpreter
{
private:
    const ICProgram* program = nullptr;
    vector<ICValue> memory;
    vector<StackFrame> callStack;
    vector<ICValue> evalStack;
    unordered_map<string, size_t> labelMap;
    size_t pc = 0;
    size_t bp = 0;
    size_t sp = 0;
    size_t maxEvalStackSize = 4096;
    size_t maxCallDepth = 1000;

    void buildLabelMap();
    void pushFrame(int level, int frameSize, int returnAddress, int staticLink, int dynamicLink, int argumentCount, bool expectsReturnValue);
    void popFrame();

    ICValue popValue();
    void pushValue(const ICValue& value);
    ICValue readOperand(const ICOperand& operand) const;
    int readAddressOperand(const IntermediateInstruction& instruction) const;
    string readLabelOperand(const IntermediateInstruction& instruction) const;
    ICOperation readOperationOperand(const IntermediateInstruction& instruction) const;
    void storeAtAddress(int level, int address, const ICValue& value);
    ICValue loadAtAddress(int level, int address) const;
    bool isTruthy(const ICValue& value) const;

    void executeInt(const IntermediateInstruction& instruction);
    void executeLiteral(const IntermediateInstruction& instruction);
    void executeLoadAddress(const IntermediateInstruction& instruction);
    void executeLoad(const IntermediateInstruction& instruction);
    void executeIndirectLoad();
    void executeStore(const IntermediateInstruction& instruction);
    void executeIndirectStore();
    void executeArithmetic(const IntermediateInstruction& instruction);
    void executeOperation(const IntermediateInstruction& instruction);
    void executeJump(const IntermediateInstruction& instruction);
    void executeConditionalJump(const IntermediateInstruction& instruction);
    void executeCall(const IntermediateInstruction& instruction);
    void executeReturn(const IntermediateInstruction& instruction);

public:
    Interpreter() = default;

    void load(const ICProgram& program);
    void run();
    void reset();

    const vector<ICValue>& getMemory() const;
    const vector<StackFrame>& getCallStack() const;
    const vector<ICValue>& getEvalStack() const;
    size_t getProgramCounter() const;
    size_t getBasePointer() const;
    size_t getStackPointer() const;
};
