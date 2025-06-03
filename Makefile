# RX Language Makefile

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -march=native -Isrc
LDFLAGS = -static -static-libgcc -static-libstdc++

# Source directories
SRCDIR = src
LIBDIRS = src/libraries/math src/libraries/colors src/libraries/time src/libraries/system

# Find all source files
SOURCES = $(wildcard $(SRCDIR)/*.cpp) \
          $(wildcard $(SRCDIR)/libraries/math/*.cpp) \
          $(wildcard $(SRCDIR)/libraries/colors/*.cpp) \
          $(wildcard $(SRCDIR)/libraries/time/*.cpp) \
          $(wildcard $(SRCDIR)/libraries/system/*.cpp)

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Target executable
TARGET = install/rx

# Default target
all: $(TARGET)

# Create install directory
install:
	mkdir -p install

# Build target
$(TARGET): install $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@
	@echo "Built: $(TARGET)"

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJECTS) $(TARGET)
	@echo "Cleaned build files"

# Test the build
test: $(TARGET)
	@echo 'print("Hello from RX!")' > test.rx
	./$(TARGET) test.rx
	@rm -f test.rx

# Install system-wide (Linux/macOS)
install-system: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/rx
	@echo "Installed RX to /usr/local/bin/rx"

# Uninstall system-wide
uninstall-system:
	sudo rm -f /usr/local/bin/rx
	@echo "Uninstalled RX from /usr/local/bin"

# Show help
help:
	@echo "RX Language Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  all              - Build the RX interpreter (default)"
	@echo "  clean            - Remove build files"
	@echo "  test             - Build and test the interpreter"
	@echo "  install-system   - Install RX system-wide (requires sudo)"
	@echo "  uninstall-system - Uninstall RX system-wide (requires sudo)"
	@echo "  help             - Show this help message"

.PHONY: all clean test install-system uninstall-system help install
