CXX := g++
FLAGS = -std=c++11 -Wall -Wextra -O2

BIN_DIR := bin
BUILD_DIR := build
MODULES_DIR := modules
SRC_DIR := src

LIBS := $(shell find $(MODULES_DIR) -name '*.hpp')

LSH_MODULES := $(shell find $(MODULES_DIR)/Lsh -name '*.cpp')
CUBE_MODULES := $(shell find $(MODULES_DIR)/Cube -name '*.cpp')
COMMON_MODULES := $(shell find $(MODULES_DIR)/Common -name '*.cpp')
ALL_MODULES := $(shell find $(MODULES_DIR) -name '*.cpp')

LSH_OBJ_MODULES := $(LSH_MODULES:$(MODULES_DIR)/Lsh/%.cpp=$(BUILD_DIR)/Lsh/%.o)
CUBE_OBJ_MODULES := $(CUBE_MODULES:$(MODULES_DIR)/Cube/%.cpp=$(BUILD_DIR)/Cube/%.o)
COMMON_OBJ_MODULES := $(COMMON_MODULES:$(MODULES_DIR)/Common/%.cpp=$(BUILD_DIR)/Common/%.o)
ALL_OBJ_MODULES := $(ALL_MODULES:$(MODULES_DIR)/%.cpp=$(BUILD_DIR)/%.o)

INCLUDE_DIRS := $(shell find $(MODULES_DIR) -type d)
INCLUDE_FLAGS := $(addprefix -I, $(INCLUDE_DIRS) $(MODULES_DIR))

MAKEFLAGS += -j8

all: tests

$(BUILD_DIR)/%.o: $(MODULES_DIR)/%.cpp $(LIBS)
	@mkdir -p $(@D)
	$(CXX) -c $(filter-out %.hpp, $<) -o $@ $(INCLUDE_FLAGS) $(FLAGS)

.PHONY: all clean lsh cube graph tests lsh-test cube-test graph-test \
		test-lsh test-cube test-graph venv activate-venv

clean:
	rm -rf $(BIN_DIR)/* $(BUILD_DIR)/*

run-reduce:
	@python3 src/reduce.py

# Test targets

TEST_DIR := tests

TEST_FILES := $(wildcard $(TEST_DIR)/*.cpp)

LSH_TEST := $(BIN_DIR)/lsh_test
CUBE_TEST := $(BIN_DIR)/cube_test
GRAPH_TEST := $(BIN_DIR)/graph_test

LSH_TEST_OBJ := $(BUILD_DIR)/lsh_test.o
CUBE_TEST_OBJ := $(BUILD_DIR)/cube_test.o
GRAPH_TEST_OBJ := $(BUILD_DIR)/graph_test.o

TEST_EXEC_FILES := $(TEST_FILES:$(TEST_DIR)/%.cpp=$(BIN_DIR)/%)

tests: $(TEST_EXEC_FILES)

$(BUILD_DIR)/%.o: $(TEST_DIR)/%.cpp $(LIBS)
	$(CXX) -c $(filter-out %.hpp, $<) -o $@ $(INCLUDE_FLAGS) $(FLAGS)

$(LSH_TEST): $(LSH_TEST_OBJ) $(LSH_OBJ_MODULES) $(COMMON_OBJ_MODULES)
	$(CXX) $^ -o $@ $(INCLUDE_FLAGS)

$(CUBE_TEST): $(CUBE_TEST_OBJ) $(CUBE_OBJ_MODULES) $(COMMON_OBJ_MODULES)
	$(CXX) $^ -o $@ $(INCLUDE_FLAGS)

$(GRAPH_TEST): $(GRAPH_TEST_OBJ) $(ALL_OBJ_MODULES)
	$(CXX) $^ -o $@ $(INCLUDE_FLAGS) -pthread

lsh-test: $(LSH_TEST)

cube-test: $(CUBE_TEST)

graph-test: $(GRAPH_TEST)

test-lsh: lsh-test
	./$(LSH_TEST) $(ARGS_LSH)

test-cube: cube-test
	./$(CUBE_TEST) $(ARGS_CUBE)

test-graph: graph-test
	./$(GRAPH_TEST) $(ARGS_GRAPH)