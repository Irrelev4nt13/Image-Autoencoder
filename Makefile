# Flags
CXX := g++
FLAGS = -std=c++11 -Wall -Wextra -O2

# Use up to 8 threads
MAKEFLAGS += -j8

# Folders
BIN_DIR := bin
BUILD_DIR := build
MODULES_DIR := modules
SRC_DIR := src
TEST_DIR := tests

# Variables 
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
TEST_FILES := $(wildcard $(TEST_DIR)/*.cpp)

SRC_EXEC_FILES := $(SRC_FILES:$(SRC_DIR)/%.cpp=$(BIN_DIR)/%)
TEST_EXEC_FILES := $(TEST_FILES:$(TEST_DIR)/%.cpp=$(BIN_DIR)/%)

LSH := $(BIN_DIR)/lsh
CUBE := $(BIN_DIR)/cube
CLUSTER := $(BIN_DIR)/cluster
GRAPH := $(BIN_DIR)/graph
BRUTE := $(BIN_DIR)/brute

LSH_OBJ := $(BUILD_DIR)/lsh.o
CUBE_OBJ := $(BUILD_DIR)/cube.o
CLUSTER_OBJ := $(BUILD_DIR)/cluster.o
GRAPH_OBJ := $(BUILD_DIR)/graph.o
BRUTE_OBJ := $(BUILD_DIR)/brute.o

LSH_TEST := $(BIN_DIR)/lsh_test
CUBE_TEST := $(BIN_DIR)/cube_test
CLUSTER_TEST := $(BIN_DIR)/cluster_test
GRAPH_TEST := $(BIN_DIR)/graph_test
BRUTE_TEST := $(BIN_DIR)/brute_test

LSH_TEST_OBJ := $(BUILD_DIR)/lsh_test.o
CUBE_TEST_OBJ := $(BUILD_DIR)/cube_test.o
GRAPH_TEST_OBJ := $(BUILD_DIR)/graph_test.o
CLUSTER_TEST_OBJ := $(BUILD_DIR)/cluster_test.o
BRUTE_TEST_OBJ := $(BUILD_DIR)/brute_test.o

LSH_MODULES := $(shell find $(MODULES_DIR)/Lsh -name '*.cpp')
CUBE_MODULES := $(shell find $(MODULES_DIR)/Cube -name '*.cpp')
CLUSTER_MODULES := $(shell find $(MODULES_DIR)/Cluster -name '*.cpp')
COMMON_MODULES := $(shell find $(MODULES_DIR)/Common -name '*.cpp')
ALL_MODULES := $(shell find $(MODULES_DIR) -name '*.cpp')

LSH_OBJ_MODULES := $(LSH_MODULES:$(MODULES_DIR)/Lsh/%.cpp=$(BUILD_DIR)/Lsh/%.o)
CUBE_OBJ_MODULES := $(CUBE_MODULES:$(MODULES_DIR)/Cube/%.cpp=$(BUILD_DIR)/Cube/%.o)
CLUSTER_OBJ_MODULES := $(CLUSTER_MODULES:$(MODULES_DIR)/Cluster/%.cpp=$(BUILD_DIR)/Cluster/%.o)
COMMON_OBJ_MODULES := $(COMMON_MODULES:$(MODULES_DIR)/Common/%.cpp=$(BUILD_DIR)/Common/%.o)
ALL_OBJ_MODULES := $(ALL_MODULES:$(MODULES_DIR)/%.cpp=$(BUILD_DIR)/%.o)

LIBS := $(shell find $(MODULES_DIR) -name '*.hpp')
INCLUDE_DIRS := $(shell find $(MODULES_DIR) -type d)
INCLUDE_FLAGS := $(addprefix -I, $(INCLUDE_DIRS) $(MODULES_DIR))

# Compile targets
all: $(SRC_EXEC_FILES)

tests: $(TEST_EXEC_FILES)

lsh: $(LSH)

cube: $(CUBE)

cluster: $(CLUSTER)

graph: $(GRAPH)

brute: $(BRUTE)

lsh-test: $(LSH_TEST)

cube-test: $(CUBE_TEST)

cluster-test: $(CLUSTER_TEST)

graph-test: $(GRAPH_TEST)

brute-test: $(BRUTE_TEST)

.PHONY: all clean lsh cube graph tests lsh-test cube-test graph-test \
		test-lsh test-cube test-graph venv activate-venv

clean:
	rm -rf $(BIN_DIR)/* $(BUILD_DIR)/*

# Run targets with arguments
ARGS_LSH := -d datasets/train-images-reduced.idx3-ubyte -q datasets/t10k-images-reduced.idx3-ubyte -k 4 -L 5 -o output/lsh_output.txt -N 5 -R 10000 \
-sd 60000 -sq 200 -dinit datasets/train-images.idx3-ubyte -qinit datasets/t10k-images.idx3-ubyte

ARGS_CUBE := -d datasets/train-images-reduced.idx3-ubyte -q datasets/t10k-images-reduced.idx3-ubyte -k 14 -M 6000 -probes 15 -o output/cube_output.txt -N 5 -R 10000 \
-sd 60000 -sq 200 -dinit datasets/train-images.idx3-ubyte -qinit datasets/t10k-images.idx3-ubyte

ARGS_GRAPH := -d datasets/train-images-reduced.idx3-ubyte -q datasets/t10k-images-reduced.idx3-ubyte -k 40 -E 30 -R 10 -N 3 -l 2000 -m 1 -o output/graph_output.txt \
-sd 5000 -sq 2000 -dinit datasets/train-images.idx3-ubyte -qinit datasets/t10k-images.idx3-ubyte

ARGS_CLUSTER := -i datasets/train-images-reduced.idx3-ubyte -c conf/cluster.conf -o output/cluster_output.txt -m Classic -sd 5000 \
-dinit datasets/train-images.idx3-ubyte

ARGS_BRUTE := -d datasets/train-images-reduced.idx3-ubyte -q datasets/t10k-images-reduced.idx3-ubyte -o output/brute_output.txt -N 5 \
-sd 60000 -sq 200 -dinit datasets/train-images.idx3-ubyte -qinit datasets/t10k-images.idx3-ubyte

run-reduce:
	@python3 src/reduce.py -d datasets/train-images.idx3-ubyte -q datasets/t10k-images.idx3-ubyte \
	-od datasets/train-images-reduced.idx3-ubyte -oq datasets/t10k-images-reduced.idx3-ubyte

run-lsh: lsh
	./$(LSH) $(ARGS_LSH)

run-cube: cube
	./$(CUBE) $(ARGS_CUBE)

run-cluster: cluster
	./$(CLUSTER) $(ARGS_CLUSTER)

run-graph: graph
	./$(GRAPH) $(ARGS_GRAPH)

run-brute: brute
	./$(BRUTE) $(ARGS_BRUTE)

test-lsh: lsh-test
	./$(LSH_TEST) $(ARGS_LSH)

test-cube: cube-test
	./$(CUBE_TEST) $(ARGS_CUBE)

test-cluster: cluster-test
	./$(CLUSTER_TEST) $(ARGS_CLUSTER) 

test-graph: graph-test
	./$(GRAPH_TEST) $(ARGS_GRAPH)

test-brute: brute-test
	./$(BRUTE_TEST) $(ARGS_BRUTE)

# Compile files
$(BUILD_DIR)/%.o: $(MODULES_DIR)/%.cpp $(LIBS)
	@mkdir -p $(@D)
	$(CXX) -c $(filter-out %.hpp, $<) -o $@ $(INCLUDE_FLAGS) $(FLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(LIBS)
	$(CXX) -c $(filter-out %.hpp, $<) -o $@ $(INCLUDE_FLAGS) $(FLAGS)

$(BUILD_DIR)/%.o: $(TEST_DIR)/%.cpp $(LIBS)
	$(CXX) -c $(filter-out %.hpp, $<) -o $@ $(INCLUDE_FLAGS) $(FLAGS)

$(LSH): $(LSH_OBJ) $(LSH_OBJ_MODULES) $(COMMON_OBJ_MODULES)
	$(CXX) $^ -o $@ $(INCLUDE_FLAGS)

$(LSH_TEST): $(LSH_TEST_OBJ) $(LSH_OBJ_MODULES) $(COMMON_OBJ_MODULES)
	$(CXX) $^ -o $@ $(INCLUDE_FLAGS)

$(CUBE): $(CUBE_OBJ) $(CUBE_OBJ_MODULES) $(COMMON_OBJ_MODULES)
	$(CXX) $^ -o $@ $(INCLUDE_FLAGS)

$(CUBE_TEST): $(CUBE_TEST_OBJ) $(CUBE_OBJ_MODULES) $(COMMON_OBJ_MODULES)
	$(CXX) $^ -o $@ $(INCLUDE_FLAGS)

$(CLUSTER): $(CLUSTER_OBJ) $(ALL_OBJ_MODULES)
	$(CXX) $^ -o $@ $(INCLUDE_FLAGS)

$(CLUSTER_TEST): $(CLUSTER_TEST_OBJ) $(ALL_OBJ_MODULES)
	$(CXX) $^ -o $@ $(INCLUDE_FLAGS)

$(GRAPH): $(GRAPH_OBJ) $(ALL_OBJ_MODULES)
	$(CXX) $^ -o $@ $(INCLUDE_FLAGS) -pthread

$(GRAPH_TEST): $(GRAPH_TEST_OBJ) $(ALL_OBJ_MODULES)
	$(CXX) $^ -o $@ $(INCLUDE_FLAGS) -pthread

$(BRUTE): $(BRUTE_OBJ) $(COMMON_OBJ_MODULES)
	$(CXX) $^ -o $@ $(INCLUDE_FLAGS)

$(BRUTE_TEST): $(BRUTE_TEST_OBJ) $(COMMON_OBJ_MODULES)
	$(CXX) $^ -o $@ $(INCLUDE_FLAGS)
