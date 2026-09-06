# Double Hashing Hash Table in C

[![CI](https://github.com/i7modes/HashTables/actions/workflows/ci.yml/badge.svg)](https://github.com/i7modes/HashTables/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Standard](https://img.shields.io/badge/C-C99-00599C.svg?logo=c)](https://en.wikipedia.org/wiki/C99)
[![Memory Safety](https://img.shields.io/badge/Valgrind-0%20Leaks-brightgreen.svg)]()

A modular, production-grade **Double Hashing Hash Table** implemented in standard ISO C99. Utilizing open addressing with two independent hash functions, this library eliminates primary and secondary clustering while providing guaranteed coprime probing cycles across prime-sized arrays.

---

## Features

- **Double Hashing Collision Resolution**: Probe sequence $h(k, i) = (h_1(k) + i \cdot h_2(k)) \pmod M$ eliminates clustering entirely.
- **Guaranteed Full Cycle**: Table capacity $M$ is always prime and step $h_2(k) \in [1, R]$ (where $R < M$ is the preceding prime), mathematically guaranteeing $\gcd(h_2(k), M) = 1$ and a full permutation of slots.
- **Tombstone Recycling**: Deletions place tombstone markers (`SLOT_DELETED`) that keep probe chains intact while being aggressively recycled during subsequent insertions.
- **Dynamic Auto-Rehashing**: Expands capacity to the next prime when load factor exceeds 0.70, completely purging accumulated tombstones and restoring optimal $\mathcal{O}(1)$ performance.
- **Case-Insensitive Option**: Built-in toggle for case-folding word frequency analysis and dictionary lookups.
- **Zero Memory Leaks**: 100% clean Valgrind and AddressSanitizer (ASan) verification on every commit.
- **Legacy Compatibility Layer**: Seamless drop-in compatibility for academic coursework function signatures (`hashInitialize`, `hashInsert`, `hashSearch`, `hashDelete`, `hashDisplay`).

---

## Asymptotic Complexity

| Operation | Average Case | Worst Case (High Load / Pathological) |
| :--- | :---: | :---: |
| **Search / Lookup** | $\mathcal{O}(1)$ | $\mathcal{O}(n)$ |
| **Insert / Increment** | $\mathcal{O}(1)$ | $\mathcal{O}(n)$ (Amortized $\mathcal{O}(1)$ with dynamic rehash) |
| **Delete (Tombstone)** | $\mathcal{O}(1)$ | $\mathcal{O}(n)$ |
| **Rehash** | $\mathcal{O}(n)$ | $\mathcal{O}(n)$ |
| **Iteration (`HT_ForEach`)**| $\mathcal{O}(M)$ | $\mathcal{O}(M)$ |
| **Space Complexity** | $\mathcal{O}(M)$ | $\mathcal{O}(M)$ |

---

## Probing Comparison: Open Addressing Strategies

| Strategy | Probe Function | Primary Clustering | Secondary Clustering | Cache Locality |
| :--- | :--- | :---: | :---: | :---: |
| **Linear Probing** | $(h(k) + i) \bmod M$ | Severe | Severe | High |
| **Quadratic Probing** | $(h(k) + c_1 i + c_2 i^2) \bmod M$ | None | Mild | Moderate |
| **Double Hashing (This Lib)** | **$(h_1(k) + i \cdot h_2(k)) \bmod M$** | **None** | **None** | **High** |

---

## Architecture & Math

```
                   +----------------------------------+
                   |            Input Key             |
                   +----------------------------------+
                                  |
            +---------------------+---------------------+
            |                                           |
            v                                           v
      +------------+                             +------------+
      |   h1(k)    |                             |   h2(k)    |
      | Hash Index |                             | Step Size  |
      +------------+                             +------------+
            |                                           |
            +---------------------+---------------------+
                                  |
                                  v
                Index = (h1(k) + i * h2(k)) % Capacity
                                  |
                  +---------------+---------------+
                  |                               |
        [Occupied & Match]             [Empty or Tombstone]
                  |                               |
                  v                               v
          Found / Increment               Insert / Recycle Slot
```

---

## Project Structure

```
HashTables/
├── .github/
│   └── workflows/
│       └── ci.yml              # Multi-platform CI (Valgrind, ASan, MinGW)
├── examples/
│   └── demo.c                 # Interactive demo (word counts, rehashing, legacy)
├── include/
│   └── hash_table.h           # Public API & legacy compatibility wrappers
├── src/
│   └── hash_table.c           # Double hashing engine & memory management
├── tests/
│   └── test_hash_table.c      # Unit test suite covering all invariants
├── .gitignore
├── compile_flags.txt
├── LICENSE                    # MIT License
├── Makefile                   # Cross-platform build system
└── README.md
```

---

## Getting Started

### Prerequisites

- GCC or Clang with C99 support
- GNU Make (or `mingw32-make` on Windows)
- Valgrind (optional, for Linux memory checks)

### Building and Running

```bash
# Build test runner and demo binaries
make all

# Run the automated unit test suite
make check

# Run the interactive demonstration
make demo

# Clean build artifacts
make clean
```

---

## API Reference

### Lifecycle & Memory
```c
HashTable* HT_Create(size_t initial_capacity, bool case_insensitive);
void       HT_Clear(HashTable *ht);
void       HT_Destroy(HashTable **ht_ptr);
```

### Core Operations
```c
bool   HT_Insert(HashTable *ht, const char *word);
bool   HT_Put(HashTable *ht, const char *word, int frequency);
int    HT_GetFrequency(const HashTable *ht, const char *word);
bool   HT_Contains(const HashTable *ht, const char *word);
bool   HT_Delete(HashTable *ht, const char *word);
bool   HT_Rehash(HashTable *ht, size_t new_capacity);
```

### Diagnostics & Inspection
```c
size_t HT_Size(const HashTable *ht);
size_t HT_Capacity(const HashTable *ht);
float  HT_LoadFactor(const HashTable *ht);
bool   HT_IsEmpty(const HashTable *ht);
void   HT_Print(const HashTable *ht, FILE *stream);
void   HT_ForEach(const HashTable *ht, void (*callback)(const char *word, int frequency, void *user_data), void *user_data);
```

### Legacy API Compatibility Layer
```c
void hashInitialize(hashTableP table, int tableSize);
int  hashFunction(char *Word, int TableSize);
void hashInsert(hashTableP table, char *Word, int TableSize);
void hashDisplay(hashTableP table, int TableSize);
int  hashSearch(hashTableP table, char *Word, int TableSize);
void hashDelete(hashTableP table, char *Word, int TableSize);
```

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
