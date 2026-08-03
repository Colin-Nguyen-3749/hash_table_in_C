#include <stdlib.h>
#include <string.h>

#include "hash_table.h"

// Define initialization functions for ht_item

// This function allocates a chink of memory the size of an ht_item,
// and then saves a copy of the strings k and v in the new memory chunk

// Marked as static because it will only be called by code that's internal to 
// the hash table
static ht_item* ht_new_item(const char* k, const char* v) {
    ht_item* i = malloc(sizeof(ht_item)); // find the size of the item
    i->key = strdup(k);
    i->value = strdup(v); // strdup duplicates a string by dynamically allocating memory
    // using malloc and copying the source string into it
    return i;
}

// This initializes a new table, where size defines how many items we can store.
// For now, size is fixed at 53 (this can be expanded later with resizing).
// Initialize this array using calloc, filling the allocated memory with NULL bytes. 
// These NULL entries tell us that the buckets are empty.
ht_hash_table* ht_new() {
    ht_hash_table* ht = malloc(sizeof(ht_hash_table)); // remember that sizeof tells us the 
                                                       // size of a datatype!
    
    ht->size = 53;
    ht->count = 0;
    ht->items = calloc((size_t)ht->size, sizeof(ht_item*)); 

    return ht;
}
// These functions are for deleting ht_items and ht_hash_tables in order
// to free the memory we've allocated to prevent memory leaks.
static void ht_del_item(ht_item* i) {
    free(i->key);
    free(i->value);
    free(i);
}

void ht_del_hash_table(ht_hash_table* ht) {

    // Just iteratively go through the hash table to delete any non-empty buckets
    for (int i = 0; i < ht->size; i++) {
        ht_item* item = ht->items[i];
        if (item != NULL) {
            ht_del_item(item);
        }
    }

    free(ht->items);
    free(ht);
}

// This is our own simple version of a hash function (it is not really thay complicated tbh)
// Remember, an ideal hash function results in an even distribution of the buckets;
// if not, it'll be more prone to collisions, where two or more inputs are assigned the same bucket.
static int ht_hash(const char* s, const int a, const int m) {
    long hash = 0;
    const int len_s = strlen(s);

    for (int i = 0; i < len_s; i++) {
        hash += (long)pow(a, len_s - (i+1)) * s[i];
        hash = hash % m;
    }

    return (int)hash;
}

