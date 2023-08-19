/*

Copyright (c) 2005-2008, Simon Howard

Permission to use, copy, modify, and/or distribute this software
for any purpose with or without fee is hereby granted, provided
that the above copyright notice and this permission notice appear
in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

 */

/**
 * @file stl_trie.h
 *
 * @brief Fast string lookups
 *
 * A stl_trie is a data structure which provides fast mappings from strings
 * to values.
 *
 * To create a new stl_trie, use @ref stl_trie_new.  To destroy a stl_trie,
 * use @ref stl_trie_free.
 *
 * To insert a value into a stl_trie, use @ref stl_trie_insert. To remove a value
 * from a stl_trie, use @ref stl_trie_remove.
 *
 * To look up a value from its key, use @ref stl_trie_lookup.
 *
 * To find the number of enstl_tries in a stl_trie, use @ref stl_trie_num_enstl_tries.
 */

#ifndef STL_TRIE_H
#define STL_TRIE_H

struct stl_trie_node
{
    void *data;
    unsigned int use_count;
    struct stl_trie_node *next[256];
};
typedef struct stl_trie_node stl_trie_node;
typedef struct
{
    stl_trie_node *root_node;
} stl_trie;

#define stl_trie_NULL ((void *)0)

stl_trie *stl_trie_new(void);

/**
 * Destroy a stl_trie.
 *
 * @param stl_trie               The stl_trie to destroy.
 */

void stl_trie_free(stl_trie *trie);

/**
 * Insert a new key-value pair into a stl_trie.  The key is a NUL-terminated
 * string.  For binary strings, use @ref stl_trie_insert_binary.
 *
 * @param stl_trie               The stl_trie.
 * @param key                The key to access the new value.
 * @param value              The value.
 * @return                   Non-zero if the value was inserted successfully,
 *                           or zero if it was not possible to allocate
 *                           memory for the new entry.
 */

int stl_trie_insert(stl_trie *trie, char *key, void *value);

/**
 * Insert a new key-value pair into a stl_trie. The key is a sequence of bytes.
 * For a key that is a NUL-terminated text string, use @ref stl_trie_insert.
 *
 * @param stl_trie               The stl_trie.
 * @param key                The key to access the new value.
 * @param key_length         The key length in bytes.
 * @param value              The value.
 * @return                   Non-zero if the value was inserted successfully,
 *                           or zero if it was not possible to allocate
 *                           memory for the new entry.
 */

int stl_trie_insert_binary(stl_trie *stl_trie, unsigned char *key,
                           int key_length, void *value);

/**
 * Look up a value from its key in a stl_trie.
 * The key is a NUL-terminated string; for binary strings, use
 * @ref stl_trie_lookup_binary.
 *
 * @param stl_trie               The stl_trie.
 * @param key                The key.
 * @return                   The value associated with the key, or
 *                           @ref stl_trie_NULL if not found in the stl_trie.
 */

void *stl_trie_lookup(stl_trie *stl_trie, char *key);

/**
 * Look up a value from its key in a stl_trie.
 * The key is a sequence of bytes; for a key that is a NUL-terminated
 * text string, use @ref stl_trie_lookup.
 *
 * @param stl_trie               The stl_trie.
 * @param key                The key.
 * @param key_length         The key length in bytes.
 * @return                   The value associated with the key, or
 *                           @ref stl_trie_NULL if not found in the stl_trie.
 */

void *stl_trie_lookup_binary(stl_trie *stl_trie, unsigned char *key, int key_length);

/**
 * Remove an entry from a stl_trie.
 * The key is a NUL-terminated string; for binary strings, use
 * @ref stl_trie_lookup_binary.
 *
 * @param stl_trie               The stl_trie.
 * @param key                The key of the entry to remove.
 * @return                   Non-zero if the key was removed successfully,
 *                           or zero if it is not present in the stl_trie.
 */

int stl_trie_remove(stl_trie *stl_trie, char *key);

/**
 * Remove an entry from a stl_trie.
 * The key is a sequence of bytes; for a key that is a NUL-terminated
 * text string, use @ref stl_trie_lookup.
 *
 * @param stl_trie               The stl_trie.
 * @param key                The key of the entry to remove.
 * @param key_length         The key length in bytes.
 * @return                   Non-zero if the key was removed successfully,
 *                           or zero if it is not present in the stl_trie.
 */

int stl_trie_remove_binary(stl_trie *stl_trie, unsigned char *key, int key_length);

/**
 * Find the number of enstl_tries in a stl_trie.
 *
 * @param stl_trie               The stl_trie.
 * @return                   Count of the number of enstl_tries in the stl_trie.
 */

unsigned int stl_trie_num_enstl_tries(stl_trie *stl_trie);

#endif /* #ifndef ALGORITHM_stl_trie_H */
