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
};

class Interpreter
{
private:
    const TacProgram* program = nullptr;
    vector<TacValue> memory;
    vector<StackFrame> callStack;
    vector<TacValue> evalStack;
    unordered_map<string, size_t> labelMap;
    size_t pc = 0;
    size_t bp = 0;
    size_t sp = 0;
    size_t maxEvalStackSize = 4096;
    size_t maxCallDepth = 1000;

    void buildLabelMap();
    void pushFrame(int level, int frameSize, int returnAddress, int staticLink, int dynamicLink);
    void popFrame();

    TacValue popValue();
    void pushValue(const TacValue& value);
    TacValue readOperand(const TacOperand& operand) const;
    int readAddressOperand(const IntermediateInstruction& instruction) const;
    string readLabelOperand(const IntermediateInstruction& instruction) const;
    TacOperation readOperationOperand(const IntermediateInstruction& instruction) const;
    void storeAtAddress(int level, int address, const TacValue& value);
    TacValue loadAtAddress(int level, int address) const;
    bool isTruthy(const TacValue& value) const;

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

    void load(const TacProgram& program);
    void run();
    void reset();

    const vector<TacValue>& getMemory() const;
    const vector<StackFrame>& getCallStack() const;
    const vector<TacValue>& getEvalStack() const;
    size_t getProgramCounter() const;
    size_t getBasePointer() const;
    size_t getStackPointer() const;
};
