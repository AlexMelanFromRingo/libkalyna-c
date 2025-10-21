CC = gcc
CFLAGS = -Wall -Wextra
CFLAGS_RELEASE = -O3 -march=native -DNDEBUG -flto
CFLAGS_DEBUG = -O0 -g -DDEBUG

# Source files
SOURCES = kalyna.c tables.c
SOURCES_OPT = kalyna_optimized.c tables.c
HEADERS = kalyna.h tables.h transformations.h

# Object files
OBJECTS = $(SOURCES:.c=.o)
OBJECTS_OPT = kalyna_optimized.o tables.o

# Targets
REFERENCE = kalyna-reference
BENCHMARK = kalyna-benchmark
BENCHMARK_OPT = kalyna-benchmark-optimized
LIB_STATIC = libkalyna.a
LIB_STATIC_OPT = libkalyna_optimized.a
LIB_SHARED = libkalyna.so

.PHONY: all clean test benchmark benchmark-opt compare lib help

# Default target: build and run tests
all: test

# Build reference implementation with tests
$(REFERENCE): $(SOURCES) main.c $(HEADERS)
	$(CC) $(CFLAGS) $(CFLAGS_RELEASE) $(SOURCES) main.c -o $(REFERENCE)

# Build reference benchmark
$(BENCHMARK): $(SOURCES) benchmark.c $(HEADERS)
	$(CC) $(CFLAGS) $(CFLAGS_RELEASE) $(SOURCES) benchmark.c -o $(BENCHMARK)

# Build optimized benchmark
$(BENCHMARK_OPT): $(SOURCES_OPT) benchmark.c $(HEADERS)
	$(CC) $(CFLAGS) $(CFLAGS_RELEASE) $(SOURCES_OPT) benchmark.c -o $(BENCHMARK_OPT)

# Build static library (reference)
$(LIB_STATIC): $(OBJECTS)
	ar rcs $(LIB_STATIC) $(OBJECTS)
	@echo "Static library created: $(LIB_STATIC)"

# Build static library (optimized)
$(LIB_STATIC_OPT): $(OBJECTS_OPT)
	ar rcs $(LIB_STATIC_OPT) $(OBJECTS_OPT)
	@echo "Optimized static library created: $(LIB_STATIC_OPT)"

# Build shared library
$(LIB_SHARED): $(SOURCES_OPT) $(HEADERS)
	$(CC) $(CFLAGS) $(CFLAGS_RELEASE) -fPIC -shared $(SOURCES_OPT) -o $(LIB_SHARED)
	@echo "Shared library created: $(LIB_SHARED)"

# Build object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(CFLAGS_RELEASE) -c $< -o $@

# Run tests
test: $(REFERENCE)
	@echo "Running test vectors..."
	./$(REFERENCE)

# Run reference benchmark
benchmark: $(BENCHMARK)
	@echo "Running REFERENCE benchmark..."
	@echo "================================"
	./$(BENCHMARK)

# Run optimized benchmark
benchmark-opt: $(BENCHMARK_OPT)
	@echo "Running OPTIMIZED benchmark..."
	@echo "================================"
	./$(BENCHMARK_OPT)

# Compare both implementations
compare: $(BENCHMARK) $(BENCHMARK_OPT)
	@echo ""
	@echo "╔════════════════════════════════════════════════════════╗"
	@echo "║     KALYNA CIPHER - PERFORMANCE COMPARISON             ║"
	@echo "╚════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "┌────────────────────────────────────────────────────────┐"
	@echo "│  REFERENCE Implementation (clarity-focused)            │"
	@echo "└────────────────────────────────────────────────────────┘"
	./$(BENCHMARK)
	@echo ""
	@echo "┌────────────────────────────────────────────────────────┐"
	@echo "│  OPTIMIZED Implementation (performance-focused)        │"
	@echo "└────────────────────────────────────────────────────────┘"
	./$(BENCHMARK_OPT)
	@echo ""
	@echo "╔════════════════════════════════════════════════════════╗"
	@echo "║  Key optimizations in optimized version:               ║"
	@echo "║  • Eliminated malloc/free in hot paths                 ║"
	@echo "║  • Stack-based buffers for better cache locality       ║"
	@echo "║  • Inlined type conversions                            ║"
	@echo "║  • Fixed all compiler warnings                         ║"
	@echo "║  • Link-time optimization (-flto)                      ║"
	@echo "╚════════════════════════════════════════════════════════╝"
	@echo ""

# Build all libraries
lib: $(LIB_STATIC) $(LIB_STATIC_OPT) $(LIB_SHARED)

# Build debug version
debug: CFLAGS_RELEASE = $(CFLAGS_DEBUG)
debug: $(REFERENCE)
	@echo "Debug build complete"

# Clean build artifacts
clean:
	rm -f $(REFERENCE) $(BENCHMARK) $(BENCHMARK_OPT)
	rm -f $(LIB_STATIC) $(LIB_STATIC_OPT) $(LIB_SHARED)
	rm -f $(OBJECTS) $(OBJECTS_OPT)
	rm -f *.o
	@echo "Clean complete"

# Help target
help:
	@echo "Kalyna Block Cipher - Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  make                  - Build and run test vectors (default)"
	@echo "  make test             - Build and run test vectors"
	@echo "  make benchmark        - Run reference benchmark"
	@echo "  make benchmark-opt    - Run optimized benchmark"
	@echo "  make compare          - Compare both implementations side-by-side"
	@echo "  make lib              - Build all libraries (reference + optimized)"
	@echo "  make debug            - Build with debug symbols"
	@echo "  make clean            - Remove all build artifacts"
	@echo "  make help             - Show this help message"
	@echo ""
	@echo "Library outputs:"
	@echo "  libkalyna.a           - Static library (reference)"
	@echo "  libkalyna_optimized.a - Static library (optimized)"
	@echo "  libkalyna.so          - Shared library (optimized)"
	@echo ""
	@echo "Recommended workflow:"
	@echo "  1. make test          # Verify correctness"
	@echo "  2. make compare       # See performance difference"
	@echo "  3. make lib           # Build for integration"
	@echo ""
	@echo "Performance comparison:"
	@echo "  make compare          # Side-by-side benchmark"