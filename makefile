CC = gcc
CFLAGS = -Wall -Wextra
CFLAGS_RELEASE = -O3 -march=native -DNDEBUG
CFLAGS_DEBUG = -O0 -g -DDEBUG

# Source files
SOURCES = kalyna.c tables.c
HEADERS = kalyna.h tables.h transformations.h

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Targets
REFERENCE = kalyna-reference
BENCHMARK = kalyna-benchmark
LIB_STATIC = libkalyna.a
LIB_SHARED = libkalyna.so

.PHONY: all clean test benchmark lib help

# Default target: build and run tests
all: test

# Build reference implementation with tests
$(REFERENCE): $(SOURCES) main.c $(HEADERS)
	$(CC) $(CFLAGS) $(CFLAGS_RELEASE) $(SOURCES) main.c -o $(REFERENCE)

# Build benchmark
$(BENCHMARK): $(SOURCES) benchmark.c $(HEADERS)
	$(CC) $(CFLAGS) $(CFLAGS_RELEASE) $(SOURCES) benchmark.c -o $(BENCHMARK)

# Build static library
$(LIB_STATIC): $(OBJECTS)
	ar rcs $(LIB_STATIC) $(OBJECTS)
	@echo "Static library created: $(LIB_STATIC)"

# Build shared library
$(LIB_SHARED): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(CFLAGS_RELEASE) -fPIC -shared $(SOURCES) -o $(LIB_SHARED)
	@echo "Shared library created: $(LIB_SHARED)"

# Build object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(CFLAGS_RELEASE) -c $< -o $@

# Run tests
test: $(REFERENCE)
	@echo "Running test vectors..."
	./$(REFERENCE)

# Run benchmark
benchmark: $(BENCHMARK)
	@echo "Running performance benchmark..."
	./$(BENCHMARK)

# Build all libraries
lib: $(LIB_STATIC) $(LIB_SHARED)

# Build debug version
debug: CFLAGS_RELEASE = $(CFLAGS_DEBUG)
debug: $(REFERENCE)
	@echo "Debug build complete"

# Clean build artifacts
clean:
	rm -f $(REFERENCE) $(BENCHMARK) $(LIB_STATIC) $(LIB_SHARED) $(OBJECTS)
	@echo "Clean complete"

# Help target
help:
	@echo "Kalyna Block Cipher - Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  make              - Build and run test vectors (default)"
	@echo "  make test         - Build and run test vectors"
	@echo "  make benchmark    - Build and run performance benchmark"
	@echo "  make lib          - Build static and shared libraries"
	@echo "  make debug        - Build with debug symbols"
	@echo "  make clean        - Remove all build artifacts"
	@echo "  make help         - Show this help message"
	@echo ""
	@echo "Library outputs:"
	@echo "  libkalyna.a       - Static library"
	@echo "  libkalyna.so      - Shared library"
	@echo ""
	@echo "Example usage:"
	@echo "  make test         # Verify implementation"
	@echo "  make benchmark    # Measure performance"
	@echo "  make lib          # Build for integration"