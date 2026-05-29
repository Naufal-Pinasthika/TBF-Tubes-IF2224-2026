#pragma once

#include "IntermediateCode.hpp"

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

using namespace std;

using RuntimeValue = variant<monostate, int, double, bool, char, string>;

struct StackFrame
{
    int level = 0;
    int returnAddress = -1;
    int staticLink = -1;
    int dynamicLink = -1;
    unordered_map<int, RuntimeValue> locals;
};

class Interpreter
{
private:
    const TacProgram* program = nullptr;
    vector<StackFrame> stack;
    vector<RuntimeValue> evalStack;
    unordered_map<string, int> labelMap;
    size_t instructionPointer = 0;
    size_t maxFrames = 1000;

    void buildLabelMap();
    void pushFrame(int level, int returnAddress, int staticLink, int dynamicLink);
    void popFrame();

    RuntimeValue popValue();
    void pushValue(const RuntimeValue& value);
    RuntimeValue readOperand(const TacOperand& operand) const;
    void storeAtLevel(int level, int address, const RuntimeValue& value);
    RuntimeValue loadAtLevel(int level, int address) const;

    void executeLiteral(const TacInstruction& instruction);
    void executeLoad(const TacInstruction& instruction);
    void executeStore(const TacInstruction& instruction);
    void executeJump(const TacInstruction& instruction);
    void executeConditionalJump(const TacInstruction& instruction);
    void executeOperation(const TacInstruction& instruction);
    void executeCall(const TacInstruction& instruction);
    void executeReturn(const TacInstruction& instruction);

public:
    Interpreter() = default;

    void load(const TacProgram& program);
    void run();
    void reset();

    const vector<StackFrame>& getStack() const;
    const vector<RuntimeValue>& getEvalStack() const;
};