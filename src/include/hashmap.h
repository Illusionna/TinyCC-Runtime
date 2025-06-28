#ifndef _HASHMAP_H
#define _HASHMAP_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct Node {
    char *key;
    char *value;
    struct Node *next;
} Node;


typedef struct {
    Node **table;
    int bucket;
    int count;
} HashMap;


/**
 * @brief ELF function in Unix HashMap.
 * @param str The dictionary key.
 * @param size The HashMap bucket.
 * @return The hash value.
**/
static unsigned int __hash_function_ELF__(char *str, int size) {
    unsigned int hash = 0;
    unsigned int x = 0;
    while (*str) {
        hash = (hash << 4) + *(str++);
        if ((x = hash & 0xF0000000L) != 0) hash = hash ^ (x >> 24);
        hash = hash & (~x);
    }
    return hash % size;
}


/**
 * @brief BKDR function in Java HashMap.
 * @param str The dictionary key.
 * @param size The HashMap bucket.
 * @return The hash value.
**/
static unsigned int __hash_function_BKDR__(char *str, int size) {
    unsigned int seed = 13131;
    unsigned int hash = 0;
    while (*str) hash = (hash * seed) + *(str++);
    return hash % size;
}


static HashMap *hashmap_create() {
    int BUCKETS = 8;
    HashMap *dict = (HashMap *)malloc(sizeof(HashMap));
    dict->bucket = BUCKETS;
    dict->count = 0;
    dict->table = (Node **)calloc(BUCKETS, sizeof(Node *));
    return dict;
}


static void __hashmap_expand__(HashMap *dict) {
    Node **old_table = dict->table;
    int old_buckets = dict->bucket;

    dict->bucket = dict->bucket * 2;    // Double the expansion factor of the HashMap.
    dict->count = 0;
    dict->table = (Node **)calloc(dict->bucket, sizeof(Node *));

    for (int i = 0; i < old_buckets; i++) {
        Node *node = old_table[i];
        while (node) {
            Node *next = node->next;
            unsigned int hash = __hash_function_ELF__(node->key, dict->bucket);
            node->next = dict->table[hash];
            dict->table[hash] = node;
            dict->count++;
            node = next;
        }
    }
    // Do not free the old nodes, because they are already linked to the new table.
    free(old_table);
}


static void hashmap_put(HashMap *dict, char *key, char *value) {
    // Expand the HashMap in order to avoid too many collisions.
    double LOAD_FACTOR = 0.75;
    if (dict->count > dict->bucket * LOAD_FACTOR) __hashmap_expand__(dict);

    unsigned int hash = __hash_function_ELF__(key, dict->bucket);
    Node *node = dict->table[hash];

    // Hash collision occurs when different keys are assigned to the same bucket.
    while (node) {
        // If the key already exists, update the value.
        if (strcmp(node->key, key) == 0) {
            free(node->value);
            node->value = strdup(value);
            return;     // Exit right now.
        }
        node = node->next;
    }

    // If the key does not exist, create a new node and insert it to the head of the linked list.
    node = (Node *)malloc(sizeof(Node));
    node->key = strdup(key);
    node->value = strdup(value);
    node->next = dict->table[hash];
    dict->table[hash] = node;
    dict->count++;
}


static char *hashmap_get(HashMap *dict, char *key) {
    Node *node = dict->table[__hash_function_ELF__(key, dict->bucket)];
    while (node) {
        if (strcmp(node->key, key) == 0) return node->value;
        node = node->next;
    }
    return NULL;
}


static int hashmap_remove(HashMap *dict, char *key) {
    // The dictionary is empty or the key is NULL.
    if ((dict->count == 0) || (!key)) return 1;

    unsigned int hash = __hash_function_ELF__(key, dict->bucket);
    Node *node = dict->table[hash];
    Node *prev = NULL;

    while (node) {
        if (strcmp(node->key, key) == 0) {
            if (prev) prev->next = node->next;
            else dict->table[hash] = node->next;
            free(node->key);
            free(node->value);
            free(node);
            dict->count--;
            return 0;
        }
        prev = node;
        node = node->next;
    }
    return 1;   // The key does not exist in the HashMap dictionary.
}


static void hashmap_destroy(HashMap *dict) {
    for (int i = 0; i < dict->bucket; i++) {
        Node *node = dict->table[i];
        while (node) {
            Node *temp = node;
            node = node->next;
            free(temp->key);
            free(temp->value);
            free(temp);
        }
    }
    free(dict->table);
    free(dict);
}


static void hashmap_view(HashMap *dict) {
    for (int i = 0; i < dict->bucket; i++) {
        printf("bucket_%d --> ", i);
        Node *node = dict->table[i];
        if (!node) {
            printf("(null)\n");
        } else {
            while (node) {
                printf("{%s: %s}", node->key, node->value);
                node = node->next;
                if (node) printf(" --> ");
            }
            printf(" --> (null)\n");
        }
    }
    printf("HashMap Dictionary Information: buckets = %d, count = %d, load_factor = %.2f%%\n", dict->bucket, dict->count, 100.0F * dict->count / dict->bucket);
}


static void hashmap_print(HashMap *dict) {
    int n = 0;
    for (int i = 0; i < dict->bucket; i++) {
        Node *node = dict->table[i];
        while (node) {
            printf("[%d]:  %s  -->  %s\n", n, node->key, node->value);
            node = node->next;
            n++;
        }
    }
    if (n == 0) printf("(null)\n");
}


#endif




// int main(int argc, char *argv[], char *env[]) {
//     HashMap *dict = hashmap_create();

//     hashmap_put(dict, "1", "com");
//     hashmap_put(dict, "11", "org");
//     hashmap_put(dict, "111", "xyz");
//     hashmap_put(dict, "2", "net");
//     hashmap_put(dict, "33", "cn");
//     hashmap_put(dict, "3", "edu");

//     hashmap_print(dict);

//     printf("%s\n", hashmap_get(dict, "111") ? hashmap_get(dict, "111") : "(null)");
//     printf("%s\n", hashmap_get(dict, "1314") ? hashmap_get(dict, "111") : "(null)");

//     printf("delete %s\n", hashmap_remove(dict, "11") == 0 ? "success" : "failed");

//     hashmap_view(dict);

//     hashmap_destroy(dict);

//     return 0;
// }