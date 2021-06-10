TARGET_EXEC ?= jc

BUILD_DIR ?= ./build
SRC_DIRS ?= ./src

CXX := g++
SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

INC_FLAGS := $(addprefix -I,$(INC_DIRS))
LD_FLAGS := -pthread -lcryptopp 

CPPFLAGS ?= $(INC_FLAGS) -std=c++17 -O2 -fPIC 

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET_EXEC): $(BUILD_DIR) $(OBJS)
	$(CXX) $(BUILD_DIR)/*.o $(LD_FLAGS) -o $(TARGET_EXEC) 

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) -c $< -o $(BUILD_DIR)/$(notdir $@)


.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)


