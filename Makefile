CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -O2 -I./src
SRCDIR = src
BUILDDIR = build
BINDIR = bin

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)
TARGET = $(BINDIR)/rx

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    TARGET_SUFFIX = _linux
endif
ifeq ($(UNAME_S),Darwin)
    TARGET_SUFFIX = _macos
    CXX = clang++
endif
ifdef OS
    TARGET_SUFFIX = .exe
    CXX = x86_64-w64-mingw32-g++
endif

all: $(TARGET)$(TARGET_SUFFIX)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET)$(TARGET_SUFFIX): $(OBJECTS) | $(BINDIR)
	$(CXX) $(OBJECTS) -o $@

windows: 
	$(MAKE) CXX=x86_64-w64-mingw32-g++ TARGET_SUFFIX=.exe

linux: 
	$(MAKE) CXX=g++ TARGET_SUFFIX=_linux

macos: 
	$(MAKE) CXX=clang++ TARGET_SUFFIX=_macos

build_all: clean
	@echo "Building for all platforms..."
	-$(MAKE) linux
	-$(MAKE) windows  
	-$(MAKE) macos

clean:
	rm -rf $(BUILDDIR) $(BINDIR)

install: $(TARGET)$(TARGET_SUFFIX)
	cp $(TARGET)$(TARGET_SUFFIX) /usr/local/bin/rx
	chmod +x /usr/local/bin/rx

uninstall:
	rm -f /usr/local/bin/rx

test: $(TARGET)$(TARGET_SUFFIX)
	@echo 'print("Hello, RX World!")' > test.rx
	@echo "Testing RX interpreter..."
	./$(TARGET)$(TARGET_SUFFIX) test.rx
	@rm test.rx
	@echo "Test completed successfully!"

examples: $(TARGET)$(TARGET_SUFFIX)
	@echo "Running example programs..."
	@if [ -f "examples/hello.rx" ]; then \
		echo "=== Running hello.rx ==="; \
		./$(TARGET)$(TARGET_SUFFIX) examples/hello.rx; \
	fi
	@if [ -f "examples/calculator.rx" ]; then \
		echo "=== Running calculator.rx ==="; \
		./$(TARGET)$(TARGET_SUFFIX) examples/calculator.rx; \
	fi
	@if [ -f "examples/fibonacci.rx" ]; then \
		echo "=== Running fibonacci.rx ==="; \
		./$(TARGET)$(TARGET_SUFFIX) examples/fibonacci.rx; \
	fi

.PHONY: all clean install uninstall test examples windows linux macos build_all