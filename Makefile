CXX ?= g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Iinclude -g $(shell pkg-config --cflags raylib) -MMD -MP
LDFLAGS :=
LIBS := $(shell pkg-config --libs raylib)

SRC := $(shell find src -name "*.cpp")
OBJ := $(patsubst src/%.cpp,obj/%.o,$(SRC))
DEPS := $(OBJ:.o=.d)

TARGET := bin/game

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p $(dir $@)
	$(CXX) $^ -o $@ $(LDFLAGS) $(LIBS)

obj/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -rf obj bin

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run