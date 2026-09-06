/**
 * @file hash_table.c
 * @brief Implementation of Double Hashing Hash Table in C.
 * @author i7modes
 * @license MIT
 */

#include "hash_table.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define DEFAULT_MAX_LOAD_FACTOR 0.70f

/* -------------------------------------------------------------------------- */
/*                         Internal Helpers & Math                            */
/* -------------------------------------------------------------------------- */

static inline int portable_strcasecmp(const char *s1, const char *s2)
{
    while (*s1 && *s2)
    {
        int c1 = tolower((unsigned char)*s1);
        int c2 = tolower((unsigned char)*s2);
        if (c1 != c2)
        {
            return c1 - c2;
        }
        s1++;
        s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

static char* ht_strdup(const char *s)
{
    if (s == NULL)
    {
        return NULL;
    }
    size_t len = strlen(s);
    char *copy = (char *)malloc(len + 1);
    if (copy != NULL)
    {
        memcpy(copy, s, len + 1);
    }
    return copy;
}

static bool is_prime(size_t n)
{
    if (n < 2)
    {
        return false;
    }
    if (n == 2 || n == 3)
    {
        return true;
    }
    if (n % 2 == 0 || n % 3 == 0)
    {
        return false;
    }

    for (size_t i = 5; i * i <= n; i += 6)
    {
        if (n % i == 0 || n % (i + 2) == 0)
        {
            return false;
        }
    }
    return true;
}

size_t HT_NextPrime(size_t n)
{
    if (n <= 2)
    {
        return 2;
    }
    if (n % 2 == 0)
    {
        n++;
    }
    while (!is_prime(n))
    {
        n += 2;
    }
    return n;
}

static size_t prev_prime(size_t n)
{
    if (n <= 3)
    {
        return 2;
    }
    size_t p = n - 1;
    if (p % 2 == 0)
    {
        p--;
    }
    while (p > 2 && !is_prime(p))
    {
        p -= 2;
    }
    return (p >= 2) ? p : 2;
}

/* -------------------------------------------------------------------------- */
/*                               Hash Functions                               */
/* -------------------------------------------------------------------------- */

size_t HT_Hash1(const char *word, size_t capacity, bool case_insensitive)
{
    if (word == NULL || capacity == 0)
    {
        return 0;
    }

    unsigned long long hash = 0;
    while (*word != '\0')
    {
        unsigned char c = (unsigned char)*word++;
        if (case_insensitive)
        {
            c = (unsigned char)tolower(c);
        }
        hash = (hash << 5) + c;
    }

    return (size_t)(hash % capacity);
}

size_t HT_Hash2(const char *word, size_t prime_r, bool case_insensitive)
{
    if (word == NULL || prime_r == 0)
    {
        return 1;
    }

    unsigned long long hash = 5381;
    while (*word != '\0')
    {
        unsigned char c = (unsigned char)*word++;
        if (case_insensitive)
        {
            c = (unsigned char)tolower(c);
        }
        hash = ((hash << 5) + hash) + c; /* djb2 */
    }

    size_t step = prime_r - (size_t)(hash % prime_r);
    return (step == 0) ? 1 : step;
}

/* -------------------------------------------------------------------------- */
/*                         Lifecycle & Memory Management                      */
/* -------------------------------------------------------------------------- */

HashTable* HT_Create(size_t initial_capacity, bool case_insensitive)
{
    HashTable *ht = (HashTable *)malloc(sizeof(HashTable));
    if (ht == NULL)
    {
        return NULL;
    }

    size_t prime_cap = HT_NextPrime(initial_capacity < 17 ? 17 : initial_capacity);
    ht->slots = (HashSlot *)calloc(prime_cap, sizeof(HashSlot));
    if (ht->slots == NULL)
    {
        free(ht);
        return NULL;
    }

    ht->capacity = prime_cap;
    ht->size = 0;
    ht->tombstones = 0;
    ht->max_load_factor = DEFAULT_MAX_LOAD_FACTOR;
    ht->case_insensitive = case_insensitive;

    return ht;
}

void HT_Clear(HashTable *ht)
{
    if (ht == NULL || ht->slots == NULL)
    {
        return;
    }

    for (size_t i = 0; i < ht->capacity; i++)
    {
        if (ht->slots[i].status == SLOT_OCCUPIED && ht->slots[i].word != NULL)
        {
            free(ht->slots[i].word);
            ht->slots[i].word = NULL;
        }
        ht->slots[i].frequency = 0;
        ht->slots[i].status = SLOT_EMPTY;
    }

    ht->size = 0;
    ht->tombstones = 0;
}

void HT_Destroy(HashTable **ht_ptr)
{
    if (ht_ptr == NULL || *ht_ptr == NULL)
    {
        return;
    }

    HashTable *ht = *ht_ptr;
    HT_Clear(ht);

    if (ht->slots != NULL)
    {
        free(ht->slots);
        ht->slots = NULL;
    }

    free(ht);
    *ht_ptr = NULL;
}

/* -------------------------------------------------------------------------- */
/*                               Rehashing                                    */
/* -------------------------------------------------------------------------- */

bool HT_Rehash(HashTable *ht, size_t new_capacity)
{
    if (ht == NULL)
    {
        return false;
    }

    size_t target_cap = HT_NextPrime(new_capacity);
    if (target_cap <= ht->size)
    {
        target_cap = HT_NextPrime(ht->size * 2 + 1);
    }

    HashSlot *new_slots = (HashSlot *)calloc(target_cap, sizeof(HashSlot));
    if (new_slots == NULL)
    {
        return false;
    }

    size_t prime_r = prev_prime(target_cap);

    for (size_t i = 0; i < ht->capacity; i++)
    {
        if (ht->slots[i].status == SLOT_OCCUPIED && ht->slots[i].word != NULL)
        {
            const char *key = ht->slots[i].word;
            size_t h1 = HT_Hash1(key, target_cap, ht->case_insensitive);
            size_t step = HT_Hash2(key, prime_r, ht->case_insensitive);
            size_t idx = h1;

            while (new_slots[idx].status == SLOT_OCCUPIED)
            {
                idx = (idx + step) % target_cap;
            }

            new_slots[idx].word = ht->slots[i].word;
            new_slots[idx].frequency = ht->slots[i].frequency;
            new_slots[idx].status = SLOT_OCCUPIED;
        }
    }

    free(ht->slots);
    ht->slots = new_slots;
    ht->capacity = target_cap;
    ht->tombstones = 0;

    return true;
}

/* -------------------------------------------------------------------------- */
/*                              Core Operations                               */
/* -------------------------------------------------------------------------- */

static bool string_equals(const char *a, const char *b, bool case_insensitive)
{
    if (a == NULL || b == NULL)
    {
        return false;
    }
    return case_insensitive ? (portable_strcasecmp(a, b) == 0) : (strcmp(a, b) == 0);
}

bool HT_Insert(HashTable *ht, const char *word)
{
    if (ht == NULL || word == NULL)
    {
        return false;
    }

    /* Auto-rehash check */
    if ((float)(ht->size + ht->tombstones + 1) / (float)ht->capacity >= ht->max_load_factor)
    {
        HT_Rehash(ht, ht->capacity * 2);
    }

    size_t prime_r = prev_prime(ht->capacity);
    size_t h1 = HT_Hash1(word, ht->capacity, ht->case_insensitive);
    size_t step = HT_Hash2(word, prime_r, ht->case_insensitive);

    size_t idx = h1;
    long first_tombstone = -1;

    for (size_t probe = 0; probe < ht->capacity; probe++)
    {
        if (ht->slots[idx].status == SLOT_OCCUPIED)
        {
            if (string_equals(ht->slots[idx].word, word, ht->case_insensitive))
            {
                ht->slots[idx].frequency++;
                return true;
            }
        }
        else if (ht->slots[idx].status == SLOT_DELETED)
        {
            if (first_tombstone == -1)
            {
                first_tombstone = (long)idx;
            }
        }
        else /* SLOT_EMPTY */
        {
            break;
        }

        idx = (idx + step) % ht->capacity;
    }

    size_t target_idx = (first_tombstone != -1) ? (size_t)first_tombstone : idx;

    char *word_copy = ht_strdup(word);
    if (word_copy == NULL)
    {
        return false;
    }

    if (ht->slots[target_idx].status == SLOT_DELETED)
    {
        ht->tombstones--;
    }

    ht->slots[target_idx].word = word_copy;
    ht->slots[target_idx].frequency = 1;
    ht->slots[target_idx].status = SLOT_OCCUPIED;
    ht->size++;

    return true;
}

bool HT_Put(HashTable *ht, const char *word, int frequency)
{
    if (ht == NULL || word == NULL)
    {
        return false;
    }

    if ((float)(ht->size + ht->tombstones + 1) / (float)ht->capacity >= ht->max_load_factor)
    {
        HT_Rehash(ht, ht->capacity * 2);
    }

    size_t prime_r = prev_prime(ht->capacity);
    size_t h1 = HT_Hash1(word, ht->capacity, ht->case_insensitive);
    size_t step = HT_Hash2(word, prime_r, ht->case_insensitive);

    size_t idx = h1;
    long first_tombstone = -1;

    for (size_t probe = 0; probe < ht->capacity; probe++)
    {
        if (ht->slots[idx].status == SLOT_OCCUPIED)
        {
            if (string_equals(ht->slots[idx].word, word, ht->case_insensitive))
            {
                ht->slots[idx].frequency = frequency;
                return true;
            }
        }
        else if (ht->slots[idx].status == SLOT_DELETED)
        {
            if (first_tombstone == -1)
            {
                first_tombstone = (long)idx;
            }
        }
        else
        {
            break;
        }

        idx = (idx + step) % ht->capacity;
    }

    size_t target_idx = (first_tombstone != -1) ? (size_t)first_tombstone : idx;

    char *word_copy = ht_strdup(word);
    if (word_copy == NULL)
    {
        return false;
    }

    if (ht->slots[target_idx].status == SLOT_DELETED)
    {
        ht->tombstones--;
    }

    ht->slots[target_idx].word = word_copy;
    ht->slots[target_idx].frequency = frequency;
    ht->slots[target_idx].status = SLOT_OCCUPIED;
    ht->size++;

    return true;
}

int HT_GetFrequency(const HashTable *ht, const char *word)
{
    if (ht == NULL || word == NULL || ht->size == 0)
    {
        return 0;
    }

    size_t prime_r = prev_prime(ht->capacity);
    size_t h1 = HT_Hash1(word, ht->capacity, ht->case_insensitive);
    size_t step = HT_Hash2(word, prime_r, ht->case_insensitive);

    size_t idx = h1;
    for (size_t probe = 0; probe < ht->capacity; probe++)
    {
        if (ht->slots[idx].status == SLOT_OCCUPIED)
        {
            if (string_equals(ht->slots[idx].word, word, ht->case_insensitive))
            {
                return ht->slots[idx].frequency;
            }
        }
        else if (ht->slots[idx].status == SLOT_EMPTY)
        {
            return 0;
        }

        idx = (idx + step) % ht->capacity;
    }

    return 0;
}

bool HT_Contains(const HashTable *ht, const char *word)
{
    return (HT_GetFrequency(ht, word) > 0);
}

bool HT_Delete(HashTable *ht, const char *word)
{
    if (ht == NULL || word == NULL || ht->size == 0)
    {
        return false;
    }

    size_t prime_r = prev_prime(ht->capacity);
    size_t h1 = HT_Hash1(word, ht->capacity, ht->case_insensitive);
    size_t step = HT_Hash2(word, prime_r, ht->case_insensitive);

    size_t idx = h1;
    for (size_t probe = 0; probe < ht->capacity; probe++)
    {
        if (ht->slots[idx].status == SLOT_OCCUPIED)
        {
            if (string_equals(ht->slots[idx].word, word, ht->case_insensitive))
            {
                free(ht->slots[idx].word);
                ht->slots[idx].word = NULL;
                ht->slots[idx].frequency = 0;
                ht->slots[idx].status = SLOT_DELETED;
                ht->size--;
                ht->tombstones++;
                return true;
            }
        }
        else if (ht->slots[idx].status == SLOT_EMPTY)
        {
            return false;
        }

        idx = (idx + step) % ht->capacity;
    }

    return false;
}

/* -------------------------------------------------------------------------- */
/*                               Inspection & Stats                           */
/* -------------------------------------------------------------------------- */

size_t HT_Size(const HashTable *ht)
{
    return (ht != NULL) ? ht->size : 0;
}

size_t HT_Capacity(const HashTable *ht)
{
    return (ht != NULL) ? ht->capacity : 0;
}

float HT_LoadFactor(const HashTable *ht)
{
    if (ht == NULL || ht->capacity == 0)
    {
        return 0.0f;
    }
    return (float)(ht->size + ht->tombstones) / (float)ht->capacity;
}

bool HT_IsEmpty(const HashTable *ht)
{
    return (ht == NULL || ht->size == 0);
}

void HT_Print(const HashTable *ht, FILE *stream)
{
    if (ht == NULL || stream == NULL)
    {
        return;
    }

    fprintf(stream, "HashTable [Capacity: %zu, Size: %zu, Tombstones: %zu, Load: %.2f]:\n",
            ht->capacity, ht->size, ht->tombstones, HT_LoadFactor(ht));

    for (size_t i = 0; i < ht->capacity; i++)
    {
        if (ht->slots[i].status == SLOT_OCCUPIED)
        {
            fprintf(stream, "  [%3zu] OCCUPIED | Word: \"%s\" | Freq: %d\n",
                    i, ht->slots[i].word, ht->slots[i].frequency);
        }
        else if (ht->slots[i].status == SLOT_DELETED)
        {
            fprintf(stream, "  [%3zu] DELETED (Tombstone)\n", i);
        }
    }
}

void HT_ForEach(const HashTable *ht,
                void (*callback)(const char *word, int frequency, void *user_data),
                void *user_data)
{
    if (ht == NULL || callback == NULL)
    {
        return;
    }

    for (size_t i = 0; i < ht->capacity; i++)
    {
        if (ht->slots[i].status == SLOT_OCCUPIED && ht->slots[i].word != NULL)
        {
            callback(ht->slots[i].word, ht->slots[i].frequency, user_data);
        }
    }
}

/* -------------------------------------------------------------------------- */
/*                       Legacy API Compatibility Layer                       */
/* -------------------------------------------------------------------------- */

int currentTableSize = 0;

void hashInitialize(hashTableP table, int tableSize)
{
    if (table == NULL)
    {
        return;
    }
    for (int i = 0; i < tableSize; i++)
    {
        table[i].status = 0;
        table[i].Word[0] = '\0';
        table[i].Frequency = 0;
    }
    currentTableSize = 0;
}

int hashFunction(char *Word, int TableSize)
{
    if (Word == NULL || TableSize <= 0)
    {
        return 0;
    }

    long long int hashValue = 0;
    while (*Word != '\0')
    {
        hashValue = (hashValue << 5) + *Word++;
    }

    return (int)(hashValue % TableSize);
}

void hashInsert(hashTableP table, char *Word, int TableSize)
{
    if (table == NULL || Word == NULL || TableSize <= 0)
    {
        return;
    }

    if (currentTableSize == TableSize)
    {
        printf("\nHash Table is full!\n\n");
        return;
    }

    int tempIndex = hashFunction(Word, TableSize);
    int index = tempIndex;
    int step_r = (TableSize > 307) ? 307 : (TableSize > 2 ? (int)prev_prime((size_t)TableSize) : 1);
    int i = 1;

    while (table[index].status == 1)
    {
        if (portable_strcasecmp(table[index].Word, Word) == 0)
        {
            table[index].Frequency++;
            return;
        }

        index = (tempIndex + i * (step_r - (tempIndex % step_r))) % TableSize;
        if (index == tempIndex || i >= TableSize)
        {
            printf("\nHash Table is full!!\n\n");
            return;
        }
        i++;
    }

    table[index].status = 1;
    strncpy(table[index].Word, Word, sizeof(table[index].Word) - 1);
    table[index].Word[sizeof(table[index].Word) - 1] = '\0';
    table[index].Frequency = 1;
    currentTableSize++;

    printf("\nThe word you entered has been inserted\n");
}

void hashDisplay(hashTableP table, int TableSize)
{
    if (table == NULL)
    {
        return;
    }

    for (int i = 0; i < TableSize; i++)
    {
        if (table[i].status == 1)
        {
            printf("%d. Word = %s | Frequency = %d\n", i, table[i].Word, table[i].Frequency);
        }
        else if (table[i].status == -1)
        {
            printf("%d. DELETED\n", i);
        }
        else
        {
            printf("%d. EMPTY\n", i);
        }
    }
}

int hashSearch(hashTableP table, char *Word, int TableSize)
{
    if (table == NULL || Word == NULL || TableSize <= 0 || currentTableSize == 0)
    {
        return -1;
    }

    int tempIndex = hashFunction(Word, TableSize);
    int index = tempIndex;
    int step_r = (TableSize > 307) ? 307 : (TableSize > 2 ? (int)prev_prime((size_t)TableSize) : 1);
    int i = 1;

    while (table[index].status != 0)
    {
        if (table[index].status == 1 && portable_strcasecmp(table[index].Word, Word) == 0)
        {
            return index;
        }

        index = (tempIndex + i * (step_r - (tempIndex % step_r))) % TableSize;
        if (i >= TableSize)
        {
            break;
        }
        i++;
    }

    return -2;
}

void hashDelete(hashTableP table, char *Word, int TableSize)
{
    if (table == NULL || Word == NULL)
    {
        return;
    }

    int index = hashSearch(table, Word, TableSize);
    if (index == -1)
    {
        printf("\nThe hash table is empty\n\n");
    }
    else if (index == -2)
    {
        printf("\nThe word you entered not found!\n\n");
    }
    else
    {
        table[index].status = -1;
        currentTableSize--;
        printf("\nThe word you entered has been deleted!\n\n");
    }
}
