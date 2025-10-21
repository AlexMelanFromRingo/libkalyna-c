# Kalyna Cipher - Integration Examples

Complete guide for integrating Kalyna into your projects.

---

## 🚀 Quick Integration

### Method 1: Direct Source Include

**Simplest approach** - just compile together:

```bash
# Your project structure
myproject/
├── main.c
├── kalyna_optimized.c    # Copy from library
├── tables.c              # Copy from library
├── kalyna.h              # Copy from library
├── tables.h              # Copy from library
└── transformations.h     # Copy from library

# Compile
gcc -O3 -march=native main.c kalyna_optimized.c tables.c -o myapp
```

---

### Method 2: Static Library

**Best for multiple projects:**

```bash
# Build library once
cd kalyna-cipher
make lib
# Creates: libkalyna_optimized.a

# Copy to your project
cp libkalyna_optimized.a /path/to/myproject/
cp kalyna.h /path/to/myproject/

# Link in your project
gcc -O3 myapp.c -L. -lkalyna_optimized -o myapp
```

---

### Method 3: System-Wide Installation

**For system-wide usage:**

```bash
# Build library
make lib

# Install (requires sudo)
sudo cp libkalyna_optimized.a /usr/local/lib/
sudo cp kalyna.h /usr/local/include/
sudo ldconfig  # Update library cache

# Use in any project
gcc myapp.c -lkalyna_optimized -o myapp
```

---

## 💻 Code Examples

### Example 1: Simple File Encryption

```c
// encrypt_file.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kalyna.h"

int encrypt_file(const char* input_path, const char* output_path, 
                 const uint64_t* key) {
    kalyna_t* ctx = KalynaInit(128, 128);
    if (!ctx) return -1;
    
    KalynaKeyExpand((uint64_t*)key, ctx);
    
    FILE* in = fopen(input_path, "rb");
    FILE* out = fopen(output_path, "wb");
    
    if (!in || !out) {
        KalynaDelete(ctx);
        return -1;
    }
    
    uint64_t block[2];
    uint64_t encrypted[2];
    size_t read;
    
    while ((read = fread(block, 1, 16, in)) > 0) {
        // Pad last block if needed
        if (read < 16) {
            memset((uint8_t*)block + read, 16 - read, 16 - read);
        }
        
        KalynaEncipher(block, ctx, encrypted);
        fwrite(encrypted, 1, 16, out);
    }
    
    fclose(in);
    fclose(out);
    KalynaDelete(ctx);
    
    return 0;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Usage: %s <input> <output>\n", argv[0]);
        return 1;
    }
    
    // Example key (use secure key generation in production!)
    uint64_t key[2] = {
        0x0706050403020100ULL,
        0x0f0e0d0c0b0a0908ULL
    };
    
    if (encrypt_file(argv[1], argv[2], key) == 0) {
        printf("File encrypted successfully!\n");
        return 0;
    } else {
        printf("Encryption failed!\n");
        return 1;
    }
}
```

**Build:**
```bash
gcc -O3 encrypt_file.c kalyna_optimized.c tables.c -o encrypt_file
./encrypt_file plaintext.txt ciphertext.bin
```

---

### Example 2: String Encryption Library

```c
// kalyna_string.h
#ifndef KALYNA_STRING_H
#define KALYNA_STRING_H

#include <stdint.h>
#include <stddef.h>

// Encrypt string (returns hex-encoded ciphertext)
char* kalyna_encrypt_string(const char* plaintext, const uint64_t* key);

// Decrypt hex-encoded ciphertext
char* kalyna_decrypt_string(const char* ciphertext_hex, const uint64_t* key);

#endif
```

```c
// kalyna_string.c
#include "kalyna_string.h"
#include "kalyna.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void bytes_to_hex(const uint8_t* bytes, size_t len, char* hex) {
    for (size_t i = 0; i < len; i++) {
        sprintf(hex + i*2, "%02x", bytes[i]);
    }
}

static void hex_to_bytes(const char* hex, uint8_t* bytes, size_t len) {
    for (size_t i = 0; i < len; i++) {
        sscanf(hex + i*2, "%2hhx", &bytes[i]);
    }
}

char* kalyna_encrypt_string(const char* plaintext, const uint64_t* key) {
    size_t len = strlen(plaintext);
    size_t padded_len = ((len + 15) / 16) * 16;
    
    uint8_t* padded = calloc(padded_len, 1);
    memcpy(padded, plaintext, len);
    
    // PKCS#7 padding
    uint8_t pad_value = padded_len - len;
    memset(padded + len, pad_value, pad_value);
    
    kalyna_t* ctx = KalynaInit(128, 128);
    KalynaKeyExpand((uint64_t*)key, ctx);
    
    uint8_t* ciphertext = malloc(padded_len);
    
    for (size_t i = 0; i < padded_len; i += 16) {
        KalynaEncipher((uint64_t*)(padded + i), 
                      ctx, 
                      (uint64_t*)(ciphertext + i));
    }
    
    char* hex = malloc(padded_len * 2 + 1);
    bytes_to_hex(ciphertext, padded_len, hex);
    hex[padded_len * 2] = '\0';
    
    free(padded);
    free(ciphertext);
    KalynaDelete(ctx);
    
    return hex;
}

char* kalyna_decrypt_string(const char* ciphertext_hex, const uint64_t* key) {
    size_t hex_len = strlen(ciphertext_hex);
    size_t bin_len = hex_len / 2;
    
    uint8_t* ciphertext = malloc(bin_len);
    hex_to_bytes(ciphertext_hex, ciphertext, bin_len);
    
    kalyna_t* ctx = KalynaInit(128, 128);
    KalynaKeyExpand((uint64_t*)key, ctx);
    
    uint8_t* plaintext = malloc(bin_len);
    
    for (size_t i = 0; i < bin_len; i += 16) {
        KalynaDecipher((uint64_t*)(ciphertext + i),
                      ctx,
                      (uint64_t*)(plaintext + i));
    }
    
    // Remove PKCS#7 padding
    uint8_t pad_value = plaintext[bin_len - 1];
    size_t real_len = bin_len - pad_value;
    
    char* result = malloc(real_len + 1);
    memcpy(result, plaintext, real_len);
    result[real_len] = '\0';
    
    free(ciphertext);
    free(plaintext);
    KalynaDelete(ctx);
    
    return result;
}
```

**Usage:**
```c
// main.c
#include "kalyna_string.h"
#include <stdio.h>

int main() {
    uint64_t key[2] = {0x0102030405060708ULL, 0x090a0b0c0d0e0f10ULL};
    
    const char* message = "Hello, Kalyna!";
    
    char* encrypted = kalyna_encrypt_string(message, key);
    printf("Encrypted: %s\n", encrypted);
    
    char* decrypted = kalyna_decrypt_string(encrypted, key);
    printf("Decrypted: %s\n", decrypted);
    
    free(encrypted);
    free(decrypted);
    
    return 0;
}
```

**Build:**
```bash
gcc -O3 main.c kalyna_string.c kalyna_optimized.c tables.c -o string_test
./string_test
```

---

### Example 3: Network Protocol Integration

```c
// secure_protocol.c
#include "kalyna.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define BLOCK_SIZE 16

typedef struct {
    int socket_fd;
    kalyna_t* ctx;
} secure_connection_t;

secure_connection_t* secure_connect(const char* host, int port, 
                                     const uint64_t* key) {
    secure_connection_t* conn = malloc(sizeof(secure_connection_t));
    
    // Create socket
    conn->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    // ... connect to server ...
    
    // Initialize encryption
    conn->ctx = KalynaInit(128, 128);
    KalynaKeyExpand((uint64_t*)key, conn->ctx);
    
    return conn;
}

ssize_t secure_send(secure_connection_t* conn, const void* data, size_t len) {
    size_t padded_len = ((len + 15) / 16) * 16;
    uint8_t* padded = calloc(padded_len, 1);
    memcpy(padded, data, len);
    
    // PKCS#7 padding
    uint8_t pad_value = padded_len - len;
    memset(padded + len, pad_value, pad_value);
    
    uint8_t* encrypted = malloc(padded_len);
    
    for (size_t i = 0; i < padded_len; i += BLOCK_SIZE) {
        KalynaEncipher((uint64_t*)(padded + i),
                      conn->ctx,
                      (uint64_t*)(encrypted + i));
    }
    
    ssize_t sent = send(conn->socket_fd, encrypted, padded_len, 0);
    
    free(padded);
    free(encrypted);
    
    return sent;
}

ssize_t secure_recv(secure_connection_t* conn, void* buffer, size_t max_len) {
    uint8_t encrypted[1024];
    ssize_t received = recv(conn->socket_fd, encrypted, sizeof(encrypted), 0);
    
    if (received <= 0) return received;
    
    uint8_t* decrypted = malloc(received);
    
    for (ssize_t i = 0; i < received; i += BLOCK_SIZE) {
        KalynaDecipher((uint64_t*)(encrypted + i),
                      conn->ctx,
                      (uint64_t*)(decrypted + i));
    }
    
    // Remove PKCS#7 padding
    uint8_t pad_value = decrypted[received - 1];
    size_t real_len = received - pad_value;
    
    memcpy(buffer, decrypted, real_len > max_len ? max_len : real_len);
    free(decrypted);
    
    return real_len;
}

void secure_close(secure_connection_t* conn) {
    close(conn->socket_fd);
    KalynaDelete(conn->ctx);
    free(conn);
}
```

---

### Example 4: Database Field Encryption

```c
// db_encryption.c
#include "kalyna.h"
#include <sqlite3.h>
#include <string.h>

static kalyna_t* global_ctx = NULL;

void db_crypto_init(const uint64_t* key) {
    global_ctx = KalynaInit(128, 128);
    KalynaKeyExpand((uint64_t*)key, global_ctx);
}

// Encrypt sensitive field before storing
char* encrypt_field(const char* plaintext) {
    size_t len = strlen(plaintext);
    size_t padded_len = ((len + 15) / 16) * 16;
    
    uint8_t* padded = calloc(padded_len, 1);
    memcpy(padded, plaintext, len);
    memset(padded + len, padded_len - len, padded_len - len);
    
    uint8_t* encrypted = malloc(padded_len);
    
    for (size_t i = 0; i < padded_len; i += 16) {
        KalynaEncipher((uint64_t*)(padded + i),
                      global_ctx,
                      (uint64_t*)(encrypted + i));
    }
    
    // Convert to base64 for storage
    char* base64 = base64_encode(encrypted, padded_len);
    
    free(padded);
    free(encrypted);
    
    return base64;
}

// Decrypt field after retrieval
char* decrypt_field(const char* ciphertext_base64) {
    size_t bin_len;
    uint8_t* encrypted = base64_decode(ciphertext_base64, &bin_len);
    
    uint8_t* plaintext = malloc(bin_len);
    
    for (size_t i = 0; i < bin_len; i += 16) {
        KalynaDecipher((uint64_t*)(encrypted + i),
                      global_ctx,
                      (uint64_t*)(plaintext + i));
    }
    
    // Remove padding
    uint8_t pad_value = plaintext[bin_len - 1];
    size_t real_len = bin_len - pad_value;
    
    char* result = malloc(real_len + 1);
    memcpy(result, plaintext, real_len);
    result[real_len] = '\0';
    
    free(encrypted);
    free(plaintext);
    
    return result;
}

void db_crypto_cleanup() {
    if (global_ctx) {
        KalynaDelete(global_ctx);
        global_ctx = NULL;
    }
}

// Example usage with SQLite
void store_user(sqlite3* db, const char* username, const char* ssn) {
    char* encrypted_ssn = encrypt_field(ssn);
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, 
        "INSERT INTO users (username, encrypted_ssn) VALUES (?, ?)",
        -1, &stmt, NULL);
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, encrypted_ssn, -1, SQLITE_STATIC);
    
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    free(encrypted_ssn);
}
```

---

## 🔗 Build System Integration

### CMake Integration

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(MyKalynaApp C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -march=native")

# Kalyna library
add_library(kalyna STATIC
    kalyna_optimized.c
    tables.c
)

target_include_directories(kalyna PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Your application
add_executable(myapp
    main.c
)

target_link_libraries(myapp kalyna)

# Optional: Install library
install(TARGETS kalyna DESTINATION lib)
install(FILES kalyna.h DESTINATION include)
```

**Usage:**
```bash
mkdir build && cd build
cmake ..
make
./myapp
```

---

### Makefile Integration

```makefile
# Your project Makefile
CC = gcc
CFLAGS = -O3 -march=native -Wall -Wextra
KALYNA_DIR = ../kalyna-cipher

# Kalyna sources
KALYNA_SRC = $(KALYNA_DIR)/kalyna_optimized.c $(KALYNA_DIR)/tables.c
KALYNA_HDR = $(KALYNA_DIR)/kalyna.h

# Your sources
APP_SRC = main.c mylib.c
APP_OBJ = $(APP_SRC:.c=.o)

myapp: $(APP_OBJ) $(KALYNA_SRC)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c $(KALYNA_HDR)
	$(CC) $(CFLAGS) -I$(KALYNA_DIR) -c $< -o $@

clean:
	rm -f $(APP_OBJ) myapp
```

---

### Autotools Integration

```bash
# configure.ac
AC_INIT([myapp], [1.0])
AM_INIT_AUTOMAKE
AC_PROG_CC

# Check for Kalyna
AC_CHECK_LIB([kalyna_optimized], [KalynaInit],
    [LIBS="-lkalyna_optimized $LIBS"],
    [AC_MSG_ERROR([Kalyna library not found])]
)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
```

```makefile
# Makefile.am
bin_PROGRAMS = myapp
myapp_SOURCES = main.c
myapp_LDADD = -lkalyna_optimized
```

---

## 🐍 Language Bindings

### Python (ctypes)

```python
# kalyna.py
from ctypes import *
import os

# Load library
lib_path = "./libkalyna_optimized.so"
kalyna = CDLL(lib_path)

# Define types
class KalynaContext(Structure):
    _fields_ = [
        ("nb", c_size_t),
        ("nk", c_size_t),
        ("nr", c_size_t),
        ("state", POINTER(c_uint64)),
        ("round_keys", POINTER(POINTER(c_uint64)))
    ]

# Function signatures
kalyna.KalynaInit.argtypes = [c_size_t, c_size_t]
kalyna.KalynaInit.restype = POINTER(KalynaContext)

kalyna.KalynaKeyExpand.argtypes = [POINTER(c_uint64), POINTER(KalynaContext)]
kalyna.KalynaKeyExpand.restype = None

kalyna.KalynaEncipher.argtypes = [POINTER(c_uint64), POINTER(KalynaContext), POINTER(c_uint64)]
kalyna.KalynaEncipher.restype = None

kalyna.KalynaDecipher.argtypes = [POINTER(c_uint64), POINTER(KalynaContext), POINTER(c_uint64)]
kalyna.KalynaDecipher.restype = None

kalyna.KalynaDelete.argtypes = [POINTER(KalynaContext)]
kalyna.KalynaDelete.restype = c_int

# Python wrapper class
class Kalyna:
    def __init__(self, block_size=128, key_size=128):
        self.ctx = kalyna.KalynaInit(block_size, key_size)
        if not self.ctx:
            raise RuntimeError("Failed to initialize Kalyna context")
        self.block_words = block_size // 64
    
    def set_key(self, key_bytes):
        key_array = (c_uint64 * (len(key_bytes) // 8))()
        for i in range(len(key_array)):
            key_array[i] = int.from_bytes(key_bytes[i*8:(i+1)*8], 'little')
        kalyna.KalynaKeyExpand(key_array, self.ctx)
    
    def encrypt(self, plaintext_bytes):
        pt = (c_uint64 * self.block_words)()
        ct = (c_uint64 * self.block_words)()
        
        for i in range(self.block_words):
            pt[i] = int.from_bytes(plaintext_bytes[i*8:(i+1)*8], 'little')
        
        kalyna.KalynaEncipher(pt, self.ctx, ct)
        
        result = b''
        for i in range(self.block_words):
            result += ct[i].to_bytes(8, 'little')
        return result
    
    def decrypt(self, ciphertext_bytes):
        ct = (c_uint64 * self.block_words)()
        pt = (c_uint64 * self.block_words)()
        
        for i in range(self.block_words):
            ct[i] = int.from_bytes(ciphertext_bytes[i*8:(i+1)*8], 'little')
        
        kalyna.KalynaDecipher(ct, self.ctx, pt)
        
        result = b''
        for i in range(self.block_words):
            result += pt[i].to_bytes(8, 'little')
        return result
    
    def __del__(self):
        if hasattr(self, 'ctx') and self.ctx:
            kalyna.KalynaDelete(self.ctx)

# Example usage
if __name__ == "__main__":
    cipher = Kalyna(128, 128)
    
    key = b'\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f'
    cipher.set_key(key)
    
    plaintext = b'\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f'
    
    ciphertext = cipher.encrypt(plaintext)
    print(f"Encrypted: {ciphertext.hex()}")
    
    decrypted = cipher.decrypt(ciphertext)
    print(f"Decrypted: {decrypted.hex()}")
    print(f"Match: {plaintext == decrypted}")
```

**Usage:**
```bash
# Build shared library
gcc -O3 -fPIC -shared kalyna_optimized.c tables.c -o libkalyna_optimized.so

# Run Python script
python3 kalyna.py
```

---

### Node.js (FFI)

```javascript
// kalyna.js
const ffi = require('ffi-napi');
const ref = require('ref-napi');

const uint64_t = ref.types.uint64;
const KalynaContext = ref.types.void;
const KalynaContextPtr = ref.refType(KalynaContext);

const kalyna = ffi.Library('./libkalyna_optimized.so', {
    'KalynaInit': [KalynaContextPtr, ['size_t', 'size_t']],
    'KalynaKeyExpand': ['void', [ref.refType(uint64_t), KalynaContextPtr]],
    'KalynaEncipher': ['void', [ref.refType(uint64_t), KalynaContextPtr, ref.refType(uint64_t)]],
    'KalynaDecipher': ['void', [ref.refType(uint64_t), KalynaContextPtr, ref.refType(uint64_t)]],
    'KalynaDelete': ['int', [KalynaContextPtr]]
});

class Kalyna {
    constructor(blockSize = 128, keySize = 128) {
        this.ctx = kalyna.KalynaInit(blockSize, keySize);
        this.blockWords = blockSize / 64;
    }
    
    setKey(keyBuffer) {
        const keyArray = new BigUint64Array(keyBuffer.buffer);
        kalyna.KalynaKeyExpand(keyArray, this.ctx);
    }
    
    encrypt(plaintextBuffer) {
        const pt = new BigUint64Array(plaintextBuffer.buffer);
        const ct = new BigUint64Array(this.blockWords);
        kalyna.KalynaEncipher(pt, this.ctx, ct);
        return Buffer.from(ct.buffer);
    }
    
    decrypt(ciphertextBuffer) {
        const ct = new BigUint64Array(ciphertextBuffer.buffer);
        const pt = new BigUint64Array(this.blockWords);
        kalyna.KalynaDecipher(ct, this.ctx, pt);
        return Buffer.from(pt.buffer);
    }
    
    close() {
        kalyna.KalynaDelete(this.ctx);
    }
}

// Example
const cipher = new Kalyna(128, 128);
const key = Buffer.from('00010203040506070809 0a0b0c0d0e0f', 'hex');
cipher.setKey(key);

const plaintext = Buffer.from('101112131415161718191a1b1c1d1e1f', 'hex');
const ciphertext = cipher.encrypt(plaintext);
console.log('Encrypted:', ciphertext.toString('hex'));

const decrypted = cipher.decrypt(ciphertext);
console.log('Decrypted:', decrypted.toString('hex'));

cipher.close();
```

---

## 🔒 Security Best Practices

### Secure Key Management

```c
// secure_key.c
#include "kalyna.h"
#include <fcntl.h>
#include <unistd.h>

// Generate cryptographically secure random key
int generate_secure_key(uint64_t* key, size_t key_words) {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return -1;
    
    ssize_t bytes_read = read(fd, key, key_words * sizeof(uint64_t));
    close(fd);
    
    return (bytes_read == (ssize_t)(key_words * sizeof(uint64_t))) ? 0 : -1;
}

// Securely clear sensitive data
void secure_zero(void* ptr, size_t len) {
    volatile uint8_t* p = ptr;
    while (len--) *p++ = 0;
}

// Example usage
void encrypt_with_secure_key(const uint8_t* data, size_t len) {
    uint64_t key[2];
    
    // Generate random key
    if (generate_secure_key(key, 2) != 0) {
        fprintf(stderr, "Failed to generate key\n");
        return;
    }
    
    kalyna_t* ctx = KalynaInit(128, 128);
    KalynaKeyExpand(key, ctx);
    
    // ... encryption ...
    
    // Cleanup: zero sensitive data
    secure_zero(key, sizeof(key));
    KalynaDelete(ctx);
}
```

---

### Memory Protection

```c
// protect_memory.c
#include <sys/mman.h>

// Lock sensitive pages in memory (prevent swapping to disk)
int protect_sensitive_data(kalyna_t* ctx) {
    // Lock context in memory
    if (mlock(ctx, sizeof(kalyna_t)) != 0) {
        return -1;
    }
    
    // Lock round keys
    for (size_t i = 0; i < ctx->nr + 1; i++) {
        if (mlock(ctx->round_keys[i], ctx->nb * sizeof(uint64_t)) != 0) {
            return -1;
        }
    }
    
    return 0;
}

void unprotect_sensitive_data(kalyna_t* ctx) {
    munlock(ctx, sizeof(kalyna_t));
    for (size_t i = 0; i < ctx->nr + 1; i++) {
        munlock(ctx->round_keys[i], ctx->nb * sizeof(uint64_t));
    }
}
```

---

## 📝 Complete Project Template

```
myproject/
├── CMakeLists.txt
├── src/
│   ├── main.c
│   └── crypto_utils.c
├── include/
│   └── crypto_utils.h
├── lib/
│   ├── kalyna/
│   │   ├── kalyna_optimized.c
│   │   ├── tables.c
│   │   ├── kalyna.h
│   │   ├── tables.h
│   │   └── transformations.h
│   └── CMakeLists.txt
└── tests/
    └── test_crypto.c
```

```cmake
# CMakeLists.txt (root)
cmake_minimum_required(VERSION 3.10)
project(MySecureApp C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(tests)
```

```cmake
# lib/CMakeLists.txt
add_library(kalyna STATIC
    kalyna/kalyna_optimized.c
    kalyna/tables.c
)

target_include_directories(kalyna PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/kalyna
)
```

```cmake
# src/CMakeLists.txt
add_executable(myapp
    main.c
    crypto_utils.c
)

target_include_directories(myapp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(myapp kalyna)
```

---

## ✅ Integration Checklist

### Before Integration
- [ ] Choose integration method (source/static/shared)
- [ ] Review security requirements
- [ ] Plan key management strategy
- [ ] Decide on block/key sizes

### During Integration
- [ ] Copy necessary files
- [ ] Update build system
- [ ] Add includes to your code
- [ ] Implement error handling
- [ ] Add tests

### After Integration  
- [ ] Run test vectors
- [ ] Benchmark performance
- [ ] Check for memory leaks
- [ ] Review security (keys, padding, etc.)
- [ ] Document usage

---

## 📞 Support

For integration help:
1. Check examples above
2. See `README.md` for API details
3. Review `QUICK_START.md`
4. Check existing issues
5. Open new issue with code sample

---

**Integration Guide Version**: 1.0  
**Last Updated**: October 2025  
**Tested Platforms**: Linux, macOS, Windows (MinGW)