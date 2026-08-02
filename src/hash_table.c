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