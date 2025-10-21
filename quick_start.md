# Kalyna Cipher - Quick Start Guide

## 🚀 Get Started in 5 Minutes

### 1. Build Everything
```bash
git clone <repository>
cd kalyna-cipher
make compare
```

This will:
- ✅ Build reference implementation
- ✅ Build optimized implementation  
- ✅ Run both benchmarks
- ✅ Show performance comparison

---

## 📊 See the Performance Difference

```bash
make compare
```

Output:
```
┌────────────────────────────────────────────────────────┐
│  REFERENCE Implementation (clarity-focused)            │
└────────────────────────────────────────────────────────┘
Kalyna-128/128 Encryption: 6.4 µs/op → 2.38 MB/s

┌────────────────────────────────────────────────────────┐
│  OPTIMIZED Implementation (performance-focused)        │
└────────────────────────────────────────────────────────┘
Kalyna-128/128 Encryption: 0.8 µs/op → 19.0 MB/s

Speedup: 8x faster! 🚀
```

---

## 💻 Use in Your Code

### Simple Example (128-bit block, 128-bit key)

```c
#include "kalyna.h"

int main() {
    // 1. Initialize cipher context
    kalyna_t* ctx = KalynaInit(128, 128);
    
    // 2. Prepare key (128 bits = 2 × 64-bit words)
    uint64_t key[2] = {
        0x0706050403020100ULL,
        0x0f0e0d0c0b0a0908ULL
    };
    
    // 3. Expand key
    KalynaKeyExpand(key, ctx);
    
    // 4. Encrypt data
    uint64_t plaintext[2] = {
        0x1716151413121110ULL,
        0x1f1e1d1c1b1a1918ULL
    };
    uint64_t ciphertext[2];
    
    KalynaEncipher(plaintext, ctx, ciphertext);
    
    // 5. Decrypt data
    uint64_t decrypted[2];
    KalynaDecipher(ciphertext, ctx, decrypted);
    
    // 6. Cleanup
    KalynaDelete(ctx);
    
    return 0;
}
```

### Compile and Run
```bash
# Using optimized library
gcc -O3 your_app.c kalyna_optimized.c tables.c -o your_app
./your_app
```

---

## 📦 Available Implementations

| File | Purpose | Performance | Use When |
|------|---------|-------------|----------|
| `kalyna.c` | Reference | Baseline (1x) | Learning, auditing |
| `kalyna_optimized.c` | Production | ~8x faster | Real applications |

---

## 🎯 Common Use Cases

### Case 1: Encrypt Single Block
```c
kalyna_t* ctx = KalynaInit(128, 128);
KalynaKeyExpand(my_key, ctx);

KalynaEncipher(plaintext, ctx, ciphertext);

KalynaDelete(ctx);
```

### Case 2: Encrypt Many Blocks (Efficient)
```c
// Setup once
kalyna_t* ctx = KalynaInit(256, 256);
KalynaKeyExpand(my_key, ctx);

// Encrypt many blocks
for (int i = 0; i < 1000; i++) {
    KalynaEncipher(blocks[i], ctx, encrypted[i]);
}

// Cleanup once
KalynaDelete(ctx);
```

### Case 3: Use Static Library
```bash
# Build library
make lib

# Link your application
gcc your_app.c -L. -lkalyna_optimized -o your_app
```

---

## 🔍 Verify Correctness

### Run Test Vectors
```bash
make test
```

Expected output:
```
=============
Kalyna (128, 128)
--- ENCIPHERING ---
Success enciphering ✓

--- DECIPHERING ---
Success deciphering ✓
```

All test vectors from DSTU 7624:2014 standard are verified!

---

## ⚡ Performance Tips

### ✅ DO
- Reuse context for multiple encryptions
- Use optimized version for production
- Align data buffers (for future SIMD)
- Batch process blocks

### ❌ DON'T
- Create/destroy context per operation
- Use reference version in hot paths
- Ignore compiler warnings
- Mix reference and optimized code

---

## 📚 More Information

- **Full API**: See `README.md`
- **Optimization Details**: See `OPTIMIZATION_GUIDE.md`
- **Performance Testing**: Run `make compare`
- **Rust Comparison**: See performance analysis in docs

---

## 🐛 Troubleshooting

### "Unsupported block size" Error
```c
// ❌ WRONG
kalyna_t* ctx = KalynaInit(192, 192);  // Not supported!

// ✅ CORRECT - Valid combinations:
KalynaInit(128, 128);  // Kalyna-128/128
KalynaInit(128, 256);  // Kalyna-128/256
KalynaInit(256, 256);  // Kalyna-256/256
KalynaInit(256, 512);  // Kalyna-256/512
KalynaInit(512, 512);  // Kalyna-512/512
```

### Compilation Warnings
```bash
# Use optimized version - zero warnings!
gcc -O3 kalyna_optimized.c tables.c benchmark.c -o bench
```

### Low Performance
```bash
# Make sure you're using optimized version with -O3
gcc -O3 -march=native kalyna_optimized.c ...

# Not reference version
gcc kalyna.c ...  # This is slow by design!
```

---

## 🎓 Learning Path

### Beginner
1. ✅ Run `make test` to see it works
2. ✅ Read `main.c` for usage examples
3. ✅ Try encrypting your own data

### Intermediate  
1. ✅ Read `kalyna.c` to understand algorithm
2. ✅ Run `make compare` to see optimization impact
3. ✅ Integrate into your project

### Advanced
1. ✅ Read `OPTIMIZATION_GUIDE.md`
2. ✅ Compare with Rust implementation
3. ✅ Implement SIMD optimizations

---

## 🚀 Ready to Go!

```bash
# All-in-one command
make compare && echo "✅ Kalyna is ready!"
```

Happy encrypting! 🔐
