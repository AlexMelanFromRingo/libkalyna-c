# Kalyna Cipher: C vs Rust Performance Comparison

## Test Environment
- **Architecture**: x86_64
- **Compiler**: GCC 13.3.0 (C), rustc (Rust)
- **Optimization**: `-O3 -march=native` (C), `--release` (Rust)
- **Same machine**: Yes ✓

---

## 🏆 Overall Winner: **Rust** (significantly faster)

---

## Detailed Performance Comparison

### Kalyna-128/128

| Metric | C Implementation | Rust Implementation | **Winner** | **Speedup** |
|--------|------------------|---------------------|------------|-------------|
| **Key Expansion** | 0.012 ms | 1.185 µs | **Rust** | **10.1x faster** |
| **Encryption Time/op** | 6.402 µs | 236.46 ns | **Rust** | **27.1x faster** |
| **Decryption Time/op** | 6.380 µs | 165.56 ns | **Rust** | **38.5x faster** |
| **Encryption Throughput** | 2.38 MB/s | 64.53 MiB/s (≈67.6 MB/s) | **Rust** | **28.4x faster** |
| **Decryption Throughput** | 2.39 MB/s | 92.17 MiB/s (≈96.6 MB/s) | **Rust** | **40.4x faster** |

### Kalyna-256/256

| Metric | C Implementation | Rust Implementation | **Winner** | **Speedup** |
|--------|------------------|---------------------|------------|-------------|
| **Key Expansion** | 0.033 ms | 2.038 µs | **Rust** | **16.2x faster** |
| **Encryption Time/op** | 17.145 µs | 362.72 ns | **Rust** | **47.3x faster** |
| **Decryption Time/op** | 17.114 µs | 357.49 ns | **Rust** | **47.9x faster** |
| **Encryption Throughput** | 1.78 MB/s | 84.14 MiB/s (≈88.2 MB/s) | **Rust** | **49.6x faster** |
| **Decryption Throughput** | 1.78 MB/s | 85.37 MiB/s (≈89.5 MB/s) | **Rust** | **50.3x faster** |

### Kalyna-512/512

| Metric | C Implementation | Rust Implementation | **Winner** | **Speedup** |
|--------|------------------|---------------------|------------|-------------|
| **Key Expansion** | 0.056 ms | 4.002 µs | **Rust** | **14.0x faster** |
| **Encryption Time/op** | 45.467 µs | 662.21 ns | **Rust** | **68.7x faster** |
| **Throughput** | 1.34 MB/s | 92.17 MiB/s (≈96.6 MB/s) | **Rust** | **72.1x faster** |

---

## 📈 Summary Statistics

### Average Speedup Across All Variants
- **Key Expansion**: ~13.4x faster in Rust
- **Encryption**: ~47.7x faster in Rust  
- **Decryption**: ~43.2x faster in Rust
- **Overall Throughput**: ~50x faster in Rust

---

## 🔍 Analysis: Why is Rust SO Much Faster?

### 1. **Reference Implementation vs Optimized Implementation**
   - **C code**: This is a **reference implementation** focused on clarity and correctness
   - **Rust code**: Likely uses optimizations like:
     - SIMD instructions
     - Better memory layout
     - Compiler optimizations (LLVM)
     - Efficient data structures

### 2. **Key Issues in C Implementation**

#### Memory Allocation Overhead
```c
// C code allocates on EVERY operation:
uint8_t* state = WordsToBytes(ctx->nb, ctx->state);
uint8_t* nstate = (uint8_t*) malloc(ctx->nb * sizeof(uint64_t));
// ... then free(state)
```
This happens in:
- `ShiftRows()` - 2 allocations per call
- `InvShiftRows()` - 2 allocations per call
- Every encryption does 2 rounds × 2 functions = multiple malloc/free cycles

**Impact**: Massive overhead! malloc/free are slow operations.

#### Inefficient Type Conversions
```c
// Constant conversion between words and bytes:
uint8_t* WordsToBytes(size_t length, uint64_t* words)
uint64_t* BytesToWords(size_t length, uint8_t* bytes)
```
Called repeatedly in `ShiftRows`, `MatrixMultiply`, etc.

#### Cache Inefficiency
- Multiple memory copies
- Poor locality due to pointer chasing
- Scattered memory access patterns

### 3. **Compiler Warnings Indicate Issues**
The 24 warnings show:
- Sign comparison issues (minor)
- Unused variables (minor)
- But mainly: **code structure isn't optimal for modern compilers**

---

## 🚀 Performance Optimization Opportunities for C

### High Impact Fixes:

#### 1. **Eliminate Dynamic Allocations**
```c
// BEFORE (current):
uint8_t* state = WordsToBytes(ctx->nb, ctx->state);
uint8_t* nstate = malloc(ctx->nb * sizeof(uint64_t));
// ...
free(state);

// AFTER (optimized):
uint8_t state_buffer[64];  // Stack allocation (max 512 bits)
uint8_t nstate_buffer[64];
// Work directly on stack buffers
```

**Expected improvement**: 5-10x faster

#### 2. **In-Place Operations**
```c
// Instead of copying, work directly on ctx->state
void ShiftRows(kalyna_t* ctx) {
    uint64_t temp[8];  // Stack allocation
    // Perform shifts directly using bit operations
    // No malloc/free, no WordsToBytes conversion
}
```

**Expected improvement**: 3-5x faster

#### 3. **Fix Sign Warnings**
```c
// BEFORE:
for (int i = 0; i < ctx->nb; ++i)

// AFTER:
for (size_t i = 0; i < ctx->nb; ++i)
```

**Expected improvement**: Compiler can better optimize

#### 4. **Use Compiler Intrinsics**
```c
#include <x86intrin.h>

// Use SIMD for parallel byte operations
__m128i state = _mm_loadu_si128((__m128i*)ctx->state);
// SIMD S-box lookup, SIMD MixColumns
```

**Expected improvement**: 2-4x faster (closer to Rust)

---

## 📊 Realistic Performance Targets for Optimized C

With all optimizations applied, C code should achieve:

| Variant | Current C | Optimized C (Target) | Rust (Actual) | Gap |
|---------|-----------|----------------------|---------------|-----|
| **128/128 Encryption** | 6.4 µs | ~0.5 µs | 0.236 µs | 2.1x slower |
| **256/256 Encryption** | 17.1 µs | ~1.0 µs | 0.363 µs | 2.8x slower |
| **512/512 Encryption** | 45.5 µs | ~2.0 µs | 0.662 µs | 3.0x slower |

Even optimized C will likely be 2-3x slower than Rust due to:
- LLVM's superior optimization
- Rust's zero-cost abstractions
- Better default inlining decisions

---

## 🎯 Recommendations

### For Production Use:
1. **Use Rust implementation** - It's 27-68x faster!
2. If C is required, **optimize the reference code** (see fixes above)

### For Educational/Reference:
1. **Keep C code as-is** - It's clear and correct
2. Add optimized version as `kalyna_optimized.c`
3. Document the performance differences

### For Benchmarking:
Current C code is NOT representative of what C can achieve. It's a reference implementation, not a performance implementation.

---

## 🐛 C Code Issues Summary

### Critical (Performance):
- ❌ Dynamic allocation in hot paths (ShiftRows, InvShiftRows)
- ❌ Excessive type conversions (Words ↔ Bytes)
- ❌ No SIMD usage
- ❌ Poor cache locality

### Minor (Warnings):
- ⚠️ 24 compiler warnings (mostly sign comparison)
- ⚠️ Unused variables
- ⚠️ Could use `const` more

### Correctness:
- ✅ All test vectors pass
- ✅ Encryption/decryption round-trip works
- ✅ Mathematically correct

---

## 💡 Conclusion

**The 27-68x performance gap is NOT a language issue** - it's a reference vs optimized implementation issue.

**Rust is faster because:**
1. ✅ It's an optimized implementation
2. ✅ LLVM compiler optimizations
3. ✅ Zero-cost abstractions
4. ✅ Better memory layout

**C is slower because:**
1. ❌ Reference implementation (clarity over speed)
2. ❌ Dynamic allocations in hot paths
3. ❌ No SIMD optimizations
4. ❌ Excessive type conversions

**Bottom line**: Use Rust for production. Keep C for reference/education. Or optimize C code for fair comparison.
