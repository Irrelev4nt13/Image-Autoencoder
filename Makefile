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

# Variables 
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)

SRC_EXEC_FILES := $(SRC_FILES:$(SRC_DIR)/%.cpp=$(BIN_DIR)/%)

CLUSTER := $(BIN_DIR)/cluster
GRAPH := $(BIN_DIR)/graph
BRUTE := $(BIN_DIR)/brute

CLUSTER_OBJ := $(BUILD_DIR)/cluster.o
GRAPH_OBJ := $(BUILD_DIR)/graph.o
BRUTE_OBJ := $(BUILD_DIR)/brute.o

COMMON_MODULES := $(shell find $(MODULES_DIR)/Common -name '*.cpp')
ALL_MODULES := $(shell find $(MODULES_DIR) -name '*.cpp')

COMMON_OBJ_MODULES := $(COMMON_MODULES:$(MODULES_DIR)/Common/%.cpp=$(BUILD_DIR)/Common/%.o)
ALL_OBJ_MODULES := $(ALL_MODULES:$(MODULES_DIR)/%.cpp=$(BUILD_DIR)/%.o)

LIBS := $(shell find $(MODULES_DIR) -name '*.hpp')
INCLUDE_DIRS := $(shell find $(MODULES_DIR) -type d)
INCLUDE_FLAGS := $(addprefix -I, $(INCLUDE_DIRS) $(MODULES_DIR))

# Compile targets
all: $(SRC_EXEC_FILES)

cluster: $(CLUSTER)

graph: $(GRAPH)

brute: $(BRUTE)

.PHONY: all clean cluster graph brute run-reduce run-cluster run-graph run-brute

clean:
	rm -rf $(BIN_DIR)/* $(BUILD_DIR)/*

# Run targets with arguments
ARGS_GRAPH := -d datasets/train-images-reduced-10.idx3-ubyte -q datasets/t10k-images-reduced-10.idx3-ubyte -k 40 -E 30 -R 10 -N 3 -l 500 -m 2 -o output/graph_output_mrng_ls_20_l_500.txt \
-sd 60000 -sq 200 -dinit datasets/train-images.idx3-ubyte -qinit datasets/t10k-images.idx3-ubyte

ARGS_CLUSTER := -i datasets/t10k-images-reduced-10.idx3-ubyte -c conf/cluster.conf -o output/cluster_output.txt -m Classic -sd 10000 \
-dinit datasets/t10k-images.idx3-ubyte -init

ARGS_BRUTE := -d datasets/train-images-reduced-10.idx3-ubyte -q datasets/t10k-images-reduced-10.idx3-ubyte -o output/brute_output.txt -N 5 \
-sd 60000 -sq 200 -dinit datasets/train-images.idx3-ubyte -qinit datasets/t10k-images.idx3-ubyte

run-reduce:
	@python3 src/reduce.py -d datasets/train-images.idx3-ubyte -q datasets/t10k-images.idx3-ubyte \
	-od datasets/train-images-reduced.idx3-ubyte -oq datasets/t10k-images-reduced.idx3-ubyte

run-cluster: cluster
	./$(CLUSTER) $(ARGS_CLUSTER)

run-graph: graph
	./$(GRAPH) $(ARGS_GRAPH)

run-brute: brute
	./$(BRUTE) $(ARGS_BRUTE)

# Compile files
$(BUILD_DIR)/%.o: $(MODULES_DIR)/%.cpp $(LIBS)
	@mkdir -p $(@D)
	$(CXX) -c $(filter-out %.hpp, $<) -o $@ $(INCLUDE_FLAGS) $(FLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(LIBS)
	$(CXX) -c $(filter-out %.hpp, $<) -o $@ $(INCLUDE_FLAGS) $(FLAGS)

$(CLUSTER): $(CLUSTER_OBJ) $(ALL_OBJ_MODULES)
	$(CXX) $^ -o $@ $(INCLUDE_FLAGS) -pthread

$(GRAPH): $(GRAPH_OBJ) $(ALL_OBJ_MODULES)
	$(CXX) $^ -o $@ $(INCLUDE_FLAGS) -pthread

$(BRUTE): $(BRUTE_OBJ) $(COMMON_OBJ_MODULES)
	$(CXX) $^ -o $@ $(INCLUDE_FLAGS)
