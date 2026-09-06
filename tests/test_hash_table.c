/**
 * @file test_hash_table.c
 * @brief Automated unit tests for Double Hashing Hash Table.
 * @author i7modes
 * @license MIT
 */

#include "hash_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static int total_tests = 0;
static int passed_tests = 0;

#define TEST_ASSERT(expr, msg) do { \
    total_tests++; \
    if (expr) { \
        passed_tests++; \
        printf("  [PASS] %s\n", msg); \
    } else { \
        printf("  [FAIL] %s (Line %d)\n", msg, __LINE__); \
    } \
} while (0)

static void test_empty_table_safety(void)
{
    printf("\n--- Test: Empty Table Safety ---\n");
    HashTable *ht = HT_Create(0, false);
    TEST_ASSERT(ht != NULL, "HT_Create with 0 capacity succeeds with default prime");
    TEST_ASSERT(HT_Size(ht) == 0, "Initial size is 0");
    TEST_ASSERT(HT_Capacity(ht) >= 17, "Capacity is at least 17");
    TEST_ASSERT(HT_IsEmpty(ht) == true, "Empty table returns true for HT_IsEmpty");
    TEST_ASSERT(HT_LoadFactor(ht) == 0.0f, "Load factor is 0.0");
    TEST_ASSERT(HT_Contains(ht, "nonexistent") == false, "Contains returns false on empty");
    TEST_ASSERT(HT_GetFrequency(ht, "nonexistent") == 0, "GetFrequency returns 0 on empty");
    TEST_ASSERT(HT_Delete(ht, "nonexistent") == false, "Delete returns false on empty");

    HT_Clear(ht);
    TEST_ASSERT(HT_Size(ht) == 0, "Clear on empty table is safe");

    HT_Destroy(&ht);
    TEST_ASSERT(ht == NULL, "Destroy sets pointer to NULL");

    HT_Destroy(&ht); // Safe double destroy
    TEST_ASSERT(ht == NULL, "Double destroy on NULL pointer is safe");
}

static void test_insert_and_frequency(void)
{
    printf("\n--- Test: Basic Insert & Frequency Counting ---\n");
    HashTable *ht = HT_Create(31, false);

    TEST_ASSERT(HT_Insert(ht, "apple") == true, "Insert 'apple'");
    TEST_ASSERT(HT_Insert(ht, "banana") == true, "Insert 'banana'");
    TEST_ASSERT(HT_Insert(ht, "apple") == true, "Re-insert 'apple' to increment frequency");

    TEST_ASSERT(HT_Size(ht) == 2, "Size is 2 distinct keys");
    TEST_ASSERT(HT_GetFrequency(ht, "apple") == 2, "Frequency of 'apple' is 2");
    TEST_ASSERT(HT_GetFrequency(ht, "banana") == 1, "Frequency of 'banana' is 1");
    TEST_ASSERT(HT_Contains(ht, "apple") == true, "Contains 'apple'");
    TEST_ASSERT(HT_Contains(ht, "banana") == true, "Contains 'banana'");
    TEST_ASSERT(HT_Contains(ht, "cherry") == false, "Does not contain 'cherry'");

    HT_Destroy(&ht);
}

static void test_case_sensitivity(void)
{
    printf("\n--- Test: Case Sensitivity vs Case Insensitivity ---\n");

    // Case-Insensitive
    HashTable *ci_ht = HT_Create(31, true);
    HT_Insert(ci_ht, "Hello");
    HT_Insert(ci_ht, "HELLO");
    HT_Insert(ci_ht, "hello");
    TEST_ASSERT(HT_Size(ci_ht) == 1, "Case-insensitive table treats Hello/HELLO/hello as 1 key");
    TEST_ASSERT(HT_GetFrequency(ci_ht, "hElLo") == 3, "Frequency across case variations is 3");
    HT_Destroy(&ci_ht);

    // Case-Sensitive
    HashTable *cs_ht = HT_Create(31, false);
    HT_Insert(cs_ht, "Hello");
    HT_Insert(cs_ht, "HELLO");
    HT_Insert(cs_ht, "hello");
    TEST_ASSERT(HT_Size(cs_ht) == 3, "Case-sensitive table treats Hello/HELLO/hello as 3 distinct keys");
    TEST_ASSERT(HT_GetFrequency(cs_ht, "Hello") == 1, "Frequency of 'Hello' is 1");
    TEST_ASSERT(HT_GetFrequency(cs_ht, "HELLO") == 1, "Frequency of 'HELLO' is 1");
    TEST_ASSERT(HT_GetFrequency(cs_ht, "hello") == 1, "Frequency of 'hello' is 1");
    HT_Destroy(&cs_ht);
}

static void test_deletion_and_tombstone_recycling(void)
{
    printf("\n--- Test: Deletion & Tombstone Recycling ---\n");
    HashTable *ht = HT_Create(17, false);

    HT_Insert(ht, "cat");
    HT_Insert(ht, "dog");
    HT_Insert(ht, "bird");
    TEST_ASSERT(HT_Size(ht) == 3, "Inserted 3 items");

    TEST_ASSERT(HT_Delete(ht, "dog") == true, "Delete 'dog' returns true");
    TEST_ASSERT(HT_Size(ht) == 2, "Size is now 2");
    TEST_ASSERT(HT_Contains(ht, "dog") == false, "'dog' is no longer in table");
    TEST_ASSERT(HT_GetFrequency(ht, "dog") == 0, "Frequency of deleted 'dog' is 0");
    TEST_ASSERT(HT_Delete(ht, "dog") == false, "Deleting 'dog' again returns false");

    // Existing keys still accessible past the tombstone
    TEST_ASSERT(HT_Contains(ht, "bird") == true, "'bird' remains accessible past tombstone");

    // Insert new key and ensure tombstone is recycled
    TEST_ASSERT(HT_Insert(ht, "elephant") == true, "Insert 'elephant'");
    TEST_ASSERT(HT_Contains(ht, "elephant") == true, "'elephant' is retrievable");

    // Re-insert 'dog'
    TEST_ASSERT(HT_Insert(ht, "dog") == true, "Re-insert 'dog'");
    TEST_ASSERT(HT_Contains(ht, "dog") == true, "'dog' is found again");
    TEST_ASSERT(HT_GetFrequency(ht, "dog") == 1, "Frequency of re-inserted 'dog' is 1");

    HT_Destroy(&ht);
}

static void test_dynamic_rehashing(void)
{
    printf("\n--- Test: Dynamic Resizing & Automatic Rehashing ---\n");
    HashTable *ht = HT_Create(17, false);
    size_t initial_cap = HT_Capacity(ht);
    TEST_ASSERT(initial_cap == 17, "Initial prime capacity is 17");

    char buffer[32];
    for (int i = 0; i < 60; i++)
    {
        snprintf(buffer, sizeof(buffer), "key_%d", i);
        HT_Insert(ht, buffer);
    }

    TEST_ASSERT(HT_Size(ht) == 60, "Size is 60 after inserting 60 unique keys");
    TEST_ASSERT(HT_Capacity(ht) > initial_cap, "Capacity expanded past initial 17");
    TEST_ASSERT(HT_LoadFactor(ht) < 0.75f, "Load factor is maintained below max threshold");

    // Verify all 60 keys are preserved accurately after multiple rehashes
    bool all_present = true;
    for (int i = 0; i < 60; i++)
    {
        snprintf(buffer, sizeof(buffer), "key_%d", i);
        if (HT_GetFrequency(ht, buffer) != 1)
        {
            all_present = false;
            break;
        }
    }
    TEST_ASSERT(all_present == true, "All 60 keys preserved accurately across rehashes");

    HT_Destroy(&ht);
}

static void foreach_counter(const char *word, int freq, void *user_data)
{
    (void)word;
    int *sum = (int *)user_data;
    *sum += freq;
}

static void test_put_and_foreach(void)
{
    printf("\n--- Test: HT_Put & HT_ForEach ---\n");
    HashTable *ht = HT_Create(31, false);

    HT_Put(ht, "item1", 10);
    HT_Put(ht, "item2", 20);
    HT_Put(ht, "item3", 30);

    TEST_ASSERT(HT_GetFrequency(ht, "item1") == 10, "item1 frequency is 10");
    TEST_ASSERT(HT_GetFrequency(ht, "item2") == 20, "item2 frequency is 20");
    TEST_ASSERT(HT_GetFrequency(ht, "item3") == 30, "item3 frequency is 30");

    // Overwrite frequency
    HT_Put(ht, "item2", 50);
    TEST_ASSERT(HT_GetFrequency(ht, "item2") == 50, "item2 updated to frequency 50");

    int total_occurrences = 0;
    HT_ForEach(ht, foreach_counter, &total_occurrences);
    TEST_ASSERT(total_occurrences == (10 + 50 + 30), "HT_ForEach visited all items and summed 90");

    HT_Destroy(&ht);
}

static void test_legacy_api(void)
{
    printf("\n--- Test: Legacy Academic API Compatibility ---\n");
    int table_size = 53;
    struct hashTable *my_table = (struct hashTable *)malloc(sizeof(struct hashTable) * table_size);
    assert(my_table != NULL);

    hashInitialize(my_table, table_size);
    TEST_ASSERT(currentTableSize == 0, "Legacy currentTableSize is 0");

    hashInsert(my_table, "hello", table_size);
    hashInsert(my_table, "world", table_size);
    hashInsert(my_table, "hello", table_size); // duplicate frequency increment
    TEST_ASSERT(currentTableSize == 2, "Legacy currentTableSize is 2");

    int idx = hashSearch(my_table, "hello", table_size);
    TEST_ASSERT(idx >= 0, "Legacy hashSearch found 'hello'");
    TEST_ASSERT(my_table[idx].Frequency == 2, "Legacy frequency is 2");

    int not_found = hashSearch(my_table, "missing", table_size);
    TEST_ASSERT(not_found == -2, "Legacy hashSearch for missing word returns -2");

    hashDelete(my_table, "hello", table_size);
    TEST_ASSERT(currentTableSize == 1, "Legacy currentTableSize is 1 after delete");
    TEST_ASSERT(my_table[idx].status == -1, "Slot marked as deleted (-1)");

    free(my_table);
}

int main(void)
{
    printf("========================================\n");
    printf("  Double Hashing Hash Table Tests\n");
    printf("========================================\n");

    test_empty_table_safety();
    test_insert_and_frequency();
    test_case_sensitivity();
    test_deletion_and_tombstone_recycling();
    test_dynamic_rehashing();
    test_put_and_foreach();
    test_legacy_api();

    printf("\n========================================\n");
    printf("  Results: %d/%d assertions passed\n", passed_tests, total_tests);
    printf("========================================\n");

    return (passed_tests == total_tests) ? 0 : 1;
}
