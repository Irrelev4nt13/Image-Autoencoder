CXX := g++
FLAGS = -std=c++11 -Wall -Wextra -O2

BIN_DIR := bin
BUILD_DIR := build
MODULES_DIR := modules
SRC_DIR := src

LIBS := $(shell find $(MODULES_DIR) -name '*.hpp')

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
	@python3 src/reduce.py -d datasets/train-images.idx3-ubyte -q datasets/t10k-images.idx3-ubyte \
	-od datasets/train-images-reduced.idx3-ubyte -oq datasets/t10k-images-reduced.idx3-ubyte

# Test targets

SRC_DIR := src

TEST_FILES := $(wildcard $(SRC_DIR)/*.cpp)

LSH_TEST := $(BIN_DIR)/lsh
CUBE_TEST := $(BIN_DIR)/cube
GRAPH_TEST := $(BIN_DIR)/graph
CLUSTER := $(BIN_DIR)/cluster

LSH_TEST_OBJ := $(BUILD_DIR)/lsh.o
CUBE_TEST_OBJ := $(BUILD_DIR)/cube.o
GRAPH_TEST_OBJ := $(BUILD_DIR)/graph.o
CLUSTER_OBJ := $(BUILD_DIR)/cluster.o

TEST_EXEC_FILES := $(TEST_FILES:$(SRC_DIR)/%.cpp=$(BIN_DIR)/%)

tests: $(TEST_EXEC_FILES)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(LIBS)
	$(CXX) -c $(filter-out %.hpp, $<) -o $@ $(INCLUDE_FLAGS) $(FLAGS)

$(LSH_TEST): $(LSH_TEST_OBJ) $(LSH_OBJ_MODULES) $(COMMON_OBJ_MODULES)
	$(CXX) $^ -o $@ $(INCLUDE_FLAGS)

$(CUBE_TEST): $(CUBE_TEST_OBJ) $(CUBE_OBJ_MODULES) $(COMMON_OBJ_MODULES)
	$(CXX) $^ -o $@ $(INCLUDE_FLAGS)

$(GRAPH_TEST): $(GRAPH_TEST_OBJ) $(ALL_OBJ_MODULES)
	$(CXX) $^ -o $@ $(INCLUDE_FLAGS) -pthread

$(CLUSTER): $(CLUSTER_OBJ) $(CLUSTER_OBJ_MODULES) $(ALL_OBJ_MODULES)
	$(CXX) $^ -o $@ $(INCLUDE_FLAGS)

lsh-test: $(LSH_TEST)

cube-test: $(CUBE_TEST)

graph-test: $(GRAPH_TEST)

cluster-test: $(CLUSTER)

ARGS_LSH := -d datasets/train-images-reduced.idx3-ubyte -q datasets/t10k-images-reduced.idx3-ubyte -k 4 -L 5 -o output/lsh_output.txt -N 5 -R 10000 \
-sd 60000 -sq 200 -dinit datasets/train-images.idx3-ubyte -qinit datasets/t10k-images.idx3-ubyte

ARGS_CUBE := -d datasets/train-images-reduced.idx3-ubyte -q datasets/t10k-images-reduced.idx3-ubyte -k 14 -M 6000 -probes 15 -o output/cube_output.txt -N 5 -R 10000 \
-sd 60000 -sq 200

ARGS_GRAPH := -d datasets/train-images-reduced.idx3-ubyte -q datasets/t10k-images-reduced.idx3-ubyte -k 40 -E 30 -R 10 -N 3 -l 500 -m 2 -o output/graph_output.txt \
-sd 5000 -sq 200

ARGS_CLUSTER := -i datasets/train-images-reduced.idx3-ubyte -c conf/cluster.conf -o output/cluster_output.txt -m Hypercube -sd 30000

test-lsh: lsh-test
	./$(LSH_TEST) $(ARGS_LSH)

test-cube: cube-test
	./$(CUBE_TEST) $(ARGS_CUBE)

test-graph: graph-test
	./$(GRAPH_TEST) $(ARGS_GRAPH)

test-cluster: cluster-test
	./$(CLUSTER) $(ARGS_CLUSTER) 
