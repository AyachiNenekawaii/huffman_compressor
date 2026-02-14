# Makefile for Huffman Compressor Project

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
DEBUG_FLAGS = -g -DDEBUG
LDFLAGS = 
INCLUDES = -I./include

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
HEADERS = $(wildcard $(INCLUDE_DIR)/*.hpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPENDS = $(OBJECTS:.o=.d)

# Target executable
TARGET = $(BIN_DIR)/huffman_compressor

# Default target
.PHONY: all clean debug install run docs help print-info check test

all: $(TARGET)

# Create directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Main target
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

# Debug build
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: clean $(TARGET)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# Install the executable
install: $(TARGET)
	install -d /usr/local/bin
	install $(TARGET) /usr/local/bin/

# Run the program
run: $(TARGET)
	./$(TARGET)

# Generate documentation (requires doxygen)
docs:
	@if [ ! -f Doxyfile ]; then \
		echo "Creating default Doxyfile..."; \
		doxygen -g Doxyfile; \
	fi
	doxygen Doxyfile

# Check code quality (requires cppcheck)
check:
	cppcheck --enable=all --inconclusive --std=c++17 $(SRC_DIR) $(INCLUDE_DIR)

# Run basic tests (if test files exist)
test:
	@echo "Running basic functionality tests..."
	@if [ -f $(TARGET) ]; then \
		echo "Testing help command..."; \
		./$(TARGET) --help; \
		echo "Testing version command..."; \
		./$(TARGET) --version; \
	else \
		echo "Executable not found. Please build first."; \
		exit 1; \
	fi

# Show help
help:
	@echo "Huffman Compressor Makefile"
	@echo "==========================="
	@echo "Available targets:"
	@echo "  all     - Build the project (default)"
	@echo "  debug   - Build with debug symbols and DEBUG flag"
	@echo "  clean   - Remove build artifacts"
	@echo "  install - Install executable to /usr/local/bin"
	@echo "  run     - Build and run the program"
	@echo "  docs    - Generate documentation (requires doxygen)"
	@echo "  check   - Run static analysis (requires cppcheck)"
	@echo "  test    - Run basic functionality tests"
	@echo "  help    - Show this help message"
	@echo "  print-info - Print build configuration info"

# Print build information
print-info:
	@echo "Build Configuration:"
	@echo "  Compiler: $(CXX)"
	@echo "  CXXFLAGS: $(CXXFLAGS)"
	@echo "  LDFLAGS: $(LDFLAGS)"
	@echo "  INCLUDES: $(INCLUDES)"
	@echo ""
	@echo "Directories:"
	@echo "  Source: $(SRC_DIR)"
	@echo "  Include: $(INCLUDE_DIR)"
	@echo "  Build: $(BUILD_DIR)"
	@echo "  Binary: $(BIN_DIR)"
	@echo ""
	@echo "Files:"
	@echo "  Sources: $(SOURCES)"
	@echo "  Headers: $(HEADERS)"
	@echo "  Objects: $(OBJECTS)"
	@echo "  Target: $(TARGET)"

# Phony targets
.PHONY: all clean debug install run docs help print-info check test

# Include dependency files
-include $(DEPENDS)
