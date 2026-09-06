/**
 * @file hash_table.h
 * @brief Production-grade Double Hashing Hash Table in C.
 * @author i7modes
 * @license MIT
 */

#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Status of an individual slot in the hash table.
 */
typedef enum {
    SLOT_EMPTY = 0,     /**< Slot has never contained an element */
    SLOT_OCCUPIED = 1,  /**< Slot contains an active key */
    SLOT_DELETED = -1   /**< Tombstone: slot was occupied but has been deleted */
} HashSlotStatus;

/**
 * @brief An individual slot entry.
 */
typedef struct HashSlot {
    char *word;             /**< Dynamically allocated string key */
    int frequency;          /**< Occurrence frequency counter */
    HashSlotStatus status;  /**< Current occupancy status */
} HashSlot;

/**
 * @brief Double Hashing Hash Table structure.
 */
typedef struct HashTable {
    HashSlot *slots;            /**< Array of table slots */
    size_t capacity;            /**< Total number of slots (always a prime number) */
    size_t size;                /**< Number of active occupied keys */
    size_t tombstones;          /**< Number of tombstone slots */
    float max_load_factor;      /**< Threshold ratio to trigger auto-rehashing (default: 0.70) */
    bool case_insensitive;      /**< If true, string comparisons ignore case */
} HashTable;

/* -------------------------------------------------------------------------- */
/*                         Lifecycle & Memory Management                      */
/* -------------------------------------------------------------------------- */

/**
 * @brief Creates a new HashTable with the specified initial capacity.
 * The actual capacity will be rounded up to the next prime number >= initial_capacity.
 * @param initial_capacity Requested slot count.
 * @param case_insensitive Whether lookups and insertions should ignore ASCII casing.
 * @return Pointer to newly allocated HashTable, or NULL on memory failure.
 */
HashTable* HT_Create(size_t initial_capacity, bool case_insensitive);

/**
 * @brief Clears all entries from the table without deallocating the table itself.
 * All keys are freed and slots reset to SLOT_EMPTY.
 * @param ht Pointer to the hash table.
 */
void HT_Clear(HashTable *ht);

/**
 * @brief Deallocates the hash table and all stored keys, setting *ht_ptr to NULL.
 * @param ht_ptr Pointer to the hash table pointer variable.
 */
void HT_Destroy(HashTable **ht_ptr);

/* -------------------------------------------------------------------------- */
/*                              Core Operations                               */
/* -------------------------------------------------------------------------- */

/**
 * @brief Inserts a word into the hash table.
 * If the word already exists, its frequency counter is incremented by 1.
 * If the word does not exist, it is inserted with frequency = 1.
 * Triggers automatic rehashing if load factor exceeds max_load_factor.
 * @param ht Pointer to the hash table.
 * @param word Null-terminated string key.
 * @return true on successful insertion/increment, false on error or NULL argument.
 */
bool HT_Insert(HashTable *ht, const char *word);

/**
 * @brief Sets a word's key with an explicit frequency value.
 * If the key exists, its frequency is overwritten.
 * @param ht Pointer to the hash table.
 * @param word Null-terminated string key.
 * @param frequency Frequency value to store.
 * @return true on success, false on failure.
 */
bool HT_Put(HashTable *ht, const char *word, int frequency);

/**
 * @brief Searches for a word and returns its frequency.
 * @param ht Pointer to the hash table.
 * @param word Word to look up.
 * @return Frequency (> 0) if found, or 0 if not found / table is NULL.
 */
int HT_GetFrequency(const HashTable *ht, const char *word);

/**
 * @brief Checks whether the table contains the specified word.
 * @param ht Pointer to the hash table.
 * @param word Word to check.
 * @return true if found, false otherwise.
 */
bool HT_Contains(const HashTable *ht, const char *word);

/**
 * @brief Deletes a word from the table by marking its slot as SLOT_DELETED (tombstone).
 * The string memory is deallocated immediately.
 * @param ht Pointer to the hash table.
 * @param word Word to delete.
 * @return true if the word was found and deleted, false if not found.
 */
bool HT_Delete(HashTable *ht, const char *word);

/**
 * @brief Manually rehashes the table to a new prime capacity.
 * All active keys are re-inserted using the new capacity, purging all tombstones.
 * @param ht Pointer to the hash table.
 * @param new_capacity Minimum new capacity (rounded up to next prime).
 * @return true on success, false on memory allocation failure.
 */
bool HT_Rehash(HashTable *ht, size_t new_capacity);

/* -------------------------------------------------------------------------- */
/*                               Inspection & Stats                           */
/* -------------------------------------------------------------------------- */

/**
 * @brief Returns the number of active occupied keys.
 */
size_t HT_Size(const HashTable *ht);

/**
 * @brief Returns the total capacity (slot count) of the table.
 */
size_t HT_Capacity(const HashTable *ht);

/**
 * @brief Returns the current load factor: (size + tombstones) / capacity.
 */
float HT_LoadFactor(const HashTable *ht);

/**
 * @brief Checks if the hash table is completely empty.
 */
bool HT_IsEmpty(const HashTable *ht);

/**
 * @brief Prints the hash table slots, statuses, words, and frequencies.
 * @param ht Pointer to the hash table.
 * @param stream Output file stream (e.g. stdout).
 */
void HT_Print(const HashTable *ht, FILE *stream);

/**
 * @brief Iterates over all active entries in the table.
 * @param ht Pointer to the hash table.
 * @param callback Function called for each active (word, frequency) pair.
 * @param user_data Arbitrary pointer passed through to callback.
 */
void HT_ForEach(const HashTable *ht,
                void (*callback)(const char *word, int frequency, void *user_data),
                void *user_data);

/* -------------------------------------------------------------------------- */
/*                         Hashing Utilities & Math                           */
/* -------------------------------------------------------------------------- */

/**
 * @brief Primary hash function h1(key) using polynomial rolling hash.
 */
size_t HT_Hash1(const char *word, size_t capacity, bool case_insensitive);

/**
 * @brief Secondary hash function h2(key) for double hashing step size.
 * Guarantees h2(key) >= 1 and coprime with prime capacity.
 */
size_t HT_Hash2(const char *word, size_t prime_r, bool case_insensitive);

/**
 * @brief Returns the smallest prime number greater than or equal to n.
 */
size_t HT_NextPrime(size_t n);

/* -------------------------------------------------------------------------- */
/*                       Legacy API Compatibility Layer                       */
/* -------------------------------------------------------------------------- */

/**
 * Original struct definition for backward compatibility.
 */
struct hashTable {
    char Word[50];
    int Frequency;
    int status; /* Empty -> 0, Occupied -> 1, Deleted -> -1 */
};

typedef struct hashTable* hashTableP;

extern int currentTableSize;

void hashInitialize(hashTableP table, int tableSize);
int  hashFunction(char *Word, int TableSize);
void hashInsert(hashTableP table, char *Word, int TableSize);
void hashDisplay(hashTableP table, int TableSize);
int  hashSearch(hashTableP table, char *Word, int TableSize);
void hashDelete(hashTableP table, char *Word, int TableSize);

#ifdef __cplusplus
}
#endif

#endif /* HASH_TABLE_H */
