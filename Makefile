## Makefile for the lexer/parser project

CXX := g++
SRC_DIR := src
BUILD_DIR := build
BIN_DIR := bin

CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -I$(SRC_DIR)

TARGET := $(BIN_DIR)/program.exe

SOURCES := $(shell find $(SRC_DIR) -name '*.cpp')
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))

INPUT ?= test/input/tc1.txt

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

run: $(TARGET)
	$(TARGET) $(notdir $(INPUT))

clean:
	@rm -rf $(BUILD_DIR) $(BIN_DIR)
