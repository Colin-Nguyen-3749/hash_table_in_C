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

// Collisions aren't good, but at the same time they're almost inevitable sometimes. 
// Hash tables have to map an infinite number of inputs to a finite number of buckets,
// and by the pigeonhole principle, there must be a collision. Therefore, we have to find 
// a way to deal with these. Here, we will be using open addressing with double hashing. 
// Double hashing makes use of two hash functions to calculate the index an item should be 
// stored at after i number of collisions. (Note that this is not the only technique to avoid
// collisions; I believe in CS 271 we used something called chaining which used doubly linked lists)

// Here's the general idea for our double hashing technique: 
// index = hash_a(string) + i * hash_b(string) % num_buckets

// This index should be used after i collisions; if there aren't any collisions, i = 0,
// and the entire hash_b function will be cancelled (there are no collisions, so no need to
// change anything). However, if there are any, then hash_b can help change the index again.
// Edge case: it's possible for hash_b to return 0, cancelling the second term when we don't want it to.
// This would cause the hash table to try to insert the item into the same bucket infinitely, which is bad.
// Let's avoid this by simply adding 1 to the result of the second hash to ensure it's never 0.
// index = (hash_a(string) + i * (hash_b(string) + 1)) % num_buckets
static int ht_get_hash(const char* s, const int num_buckets, const int attempt) {
    const int hash_a = ht_hash(s, HT_PRIME_1, num_buckets);
    const int hash_b = ht_hash(s, HT_PRIME_2, num_buckets);

    return (hash_a + (attempt * (hash_b + 1))) % num_buckets;
}


// Insert 
// To insert a new key-value pair, what we have to do is go through all of the 
// buckets until we find one that's empty. Then, we insert the item into that empty
// bucket and increment the count for the hash table to show that something has been added.
void ht_insert(ht_hash_table* ht, const char* key, const char* value) {
    ht_item* item = ht_new_item(key, value);
    int index = ht_get_hash(item->key, ht->size, 0);
    ht_item* cur_item = ht->items[index];
    int i = 1;

    // Iterate through the hash table to find an empty bucket
    // Edit: add to the while loop condition to skip over buckets marked as deleted
    while (cur_item != NULL && cur_item != &HT_DELETED_ITEM) {
        index = ht_get_hash(item->key, ht->size, i);
        cur_item = ht->items[index];
        i++;
    }

    // Insert in item and increment count
    ht->items[index] = item;
    ht->count++;
}

// Search
// Like inserting, we must iterate through the hash table, but now 
// we check if the item's key matches what we're looking for instead of 
// us looking for an empty bucket. If we found what we want, return that item's
// value. If we reach an empty bucket, return null to shows that nothing was found.
char* ht_search(ht_hash_table* ht, const char* key) {
    int index = ht_get_hash(key, ht->size, 0);
    ht_item* item = ht->items[index];
    int i = 1;

    // strcmp is a function that compares two null-terminated strings 
    // byte-by-byte based on their ASCII values
    // Edit: Add additional if statement to skip over buckets marked as 
    // empty from delete function
    while (item != NULL) {
        if (item != &HT_DELETED_ITEM) {
            if (strcmp(item->key, key) == 0) {
                return item->value;
                // If we found the right key
            }
            index = ht_get_hash(key, ht->size, i);
            item = ht->items[index];
            i++;
        }
    }
    return NULL;
}

// Delete
// Deleting is a bit more complex than our previous functions.
// If the item to be deleted is part of a collision chain, we have to be careful
// when removing it so that we don't make the rest of the chain impossible
// to access. So instead, let's mark the item as deleted. To do this, 
// use a pointer to a global sentinel item that represents that the bucket
// has a deleted item. 
static ht_item HT_DELETED_ITEM = {NULL, NULL};

void ht_delete(ht_hash_table* ht, const char* key) {
    int index = ht_get_hash(key, ht->sizze, 0);
    ht_item* item = ht->items [index];
    int i = 1;

    // Simply search for the item
    while (item != NULL) {
        if (item != &HT_DELETED_ITEM) {
            if (strcmp(item->key, key) == 0) {
                ht_del_item(item);
                ht->items[index] = &HT_DELETED_ITEM;
            }
        }
        index = ht_get_hash(key, ht->size, i);
        item = ht->items[index];
        i++;
    }
    // Decrement the hash table's count
    ht->count--;
}