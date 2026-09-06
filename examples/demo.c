/**
 * @file demo.c
 * @brief Demonstration of Double Hashing Hash Table library.
 * @author i7modes
 * @license MIT
 */

#include "hash_table.h"
#include <stdio.h>
#include <string.h>

static void print_section(const char *title)
{
    printf("\n========================================\n");
    printf("  %s\n", title);
    printf("========================================\n");
}

static void print_word_stat(const char *word, int frequency, void *user_data)
{
    int *count = (int *)user_data;
    printf("  [%2d] Word: \"%-12s\" | Frequency: %d\n", ++(*count), word, frequency);
}

int main(void)
{
    print_section("1. Word Frequency Counter with Double Hashing");
    printf("Creating case-insensitive Hash Table (initial capacity: 17)...\n");

    HashTable *ht = HT_Create(17, true);
    const char *corpus[] = {
        "To", "be", "or", "not", "to", "be",
        "that", "is", "the", "question",
        "Whether", "tis", "nobler", "in", "the", "mind",
        "to", "suffer", "The", "slings", "and", "arrows"
    };
    size_t corpus_len = sizeof(corpus) / sizeof(corpus[0]);

    for (size_t i = 0; i < corpus_len; i++)
    {
        HT_Insert(ht, corpus[i]);
    }

    printf("Total Words Ingested: %zu\n", corpus_len);
    printf("Distinct Vocabulary Size: %zu\n", HT_Size(ht));
    printf("Active Table Capacity: %zu\n", HT_Capacity(ht));
    printf("Current Load Factor: %.2f\n\n", HT_LoadFactor(ht));

    int counter = 0;
    printf("Word Frequencies:\n");
    HT_ForEach(ht, print_word_stat, &counter);

    print_section("2. Case-Insensitive Queries");
    const char *queries[] = {"to", "THE", "Be", "missing"};
    for (size_t i = 0; i < sizeof(queries) / sizeof(queries[0]); i++)
    {
        printf("Query \"%s\": Frequency = %d (Found? %s)\n",
               queries[i],
               HT_GetFrequency(ht, queries[i]),
               HT_Contains(ht, queries[i]) ? "YES" : "NO");
    }

    print_section("3. Deletions & Tombstone Inspection");
    printf("Deleting \"slings\" and \"arrows\"...\n");
    HT_Delete(ht, "slings");
    HT_Delete(ht, "arrows");
    printf("New Size: %zu, Tombstones: %zu, Load Factor: %.2f\n",
           HT_Size(ht), ht->tombstones, HT_LoadFactor(ht));

    printf("\nSlot Layout Snapshot (first 10 slots):\n");
    for (size_t i = 0; i < 10 && i < ht->capacity; i++)
    {
        if (ht->slots[i].status == SLOT_OCCUPIED)
        {
            printf("  Slot [%2zu]: OCCUPIED (\"%s\", freq: %d)\n",
                   i, ht->slots[i].word, ht->slots[i].frequency);
        }
        else if (ht->slots[i].status == SLOT_DELETED)
        {
            printf("  Slot [%2zu]: DELETED  (Tombstone - ready for reuse)\n", i);
        }
        else
        {
            printf("  Slot [%2zu]: EMPTY\n", i);
        }
    }

    print_section("4. Re-insertion & Tombstone Recycling");
    printf("Inserting \"slings\" back into the table...\n");
    HT_Insert(ht, "slings");
    printf("Tombstones after recycling: %zu, Size: %zu\n", ht->tombstones, HT_Size(ht));

    print_section("5. Automatic Rehashing Demonstration");
    printf("Initial capacity: %zu\n", HT_Capacity(ht));
    printf("Inserting 40 synthetic keys to exceed 0.70 load factor...\n");
    char key_buf[32];
    for (int i = 0; i < 40; i++)
    {
        snprintf(key_buf, sizeof(key_buf), "synth_token_%d", i);
        HT_Insert(ht, key_buf);
    }
    printf("New Expanded Capacity: %zu (Prime)\n", HT_Capacity(ht));
    printf("New Total Elements: %zu\n", HT_Size(ht));
    printf("New Load Factor: %.2f\n", HT_LoadFactor(ht));

    print_section("6. Legacy API Backward Compatibility");
    int legacy_size = 53;
    struct hashTable *legacy_table = (struct hashTable *)malloc(sizeof(struct hashTable) * legacy_size);
    if (legacy_table != NULL)
    {
        hashInitialize(legacy_table, legacy_size);
        hashInsert(legacy_table, "Mohammad", legacy_size);
        hashInsert(legacy_table, "University", legacy_size);
        hashInsert(legacy_table, "Mohammad", legacy_size);

        int idx = hashSearch(legacy_table, "Mohammad", legacy_size);
        printf("Legacy hashSearch(\"Mohammad\"): Index %d, Word \"%s\", Freq %d\n",
               idx, legacy_table[idx].Word, legacy_table[idx].Frequency);

        hashDelete(legacy_table, "Mohammad", legacy_size);
        printf("Legacy after delete: status = %d\n", legacy_table[idx].status);
        free(legacy_table);
    }

    print_section("7. Safe Memory Teardown");
    HT_Destroy(&ht);
    printf("Hash Table pointer after HT_Destroy: %p (0 memory leaks)\n", (void *)ht);

    printf("\nHash Table demonstrations completed successfully!\n");
    return 0;
}
