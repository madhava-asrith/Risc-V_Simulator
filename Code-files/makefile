# Compiler and target
CXX = g++
TARGET = riscv_sim

# Directories
INCLUDE_DIR = header
SRC_DIR = source
BUILD_DIR = build

# Create object file directory
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(wildcard $(SRC_DIR)/*.cpp))

# Include header files
INCLUDES = -I$(INCLUDE_DIR)

# Default rule
all: $(TARGET)

# Build the final executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

# Rule to compile .cpp files into .o files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Ensure the build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)
	rm -f *.output

# Phony targets
.PHONY: all clean
