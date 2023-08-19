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

/* Trie: fast mapping of strings to values */

#include <stdlib.h>
#include <string.h>

#include "stl_trie.h"

/* malloc() / free() testing */

stl_trie *stl_trie_new(void)
{
	stl_trie *new_trie;

	new_trie = (stl_trie *)malloc(sizeof(stl_trie));

	if (new_trie == NULL)
	{
		return NULL;
	}

	new_trie->root_node = NULL;

	return new_trie;
}

static void stl_trie_free_list_push(stl_trie_node **list, stl_trie_node *node)
{
	node->data = *list;
	*list = node;
}

static stl_trie_node *stl_trie_free_list_pop(stl_trie_node **list)
{
	stl_trie_node *result;

	result = *list;
	*list = result->data;

	return result;
}

void stl_trie_free(stl_trie *trie)
{
	stl_trie_node *free_list;
	stl_trie_node *node;
	int i;

	free_list = NULL;

	/* Start with the root node */

	if (trie->root_node != NULL)
	{
		stl_trie_free_list_push(&free_list, trie->root_node);
	}

	/* Go through the free list, freeing nodes.  We add new nodes as
	 * we encounter them; in this way, all the nodes are freed
	 * non-recursively. */

	while (free_list != NULL)
	{
		node = stl_trie_free_list_pop(&free_list);

		/* Add all children of this node to the free list */

		for (i = 0; i < 256; ++i)
		{
			if (node->next[i] != NULL)
			{
				stl_trie_free_list_push(&free_list, node->next[i]);
			}
		}

		/* Free the node */

		free(node);
	}

	/* Free the stl_trie */

	free(trie);
}

static stl_trie_node *stl_trie_find_end(stl_trie *trie, char *key)
{
	stl_trie_node *node;
	char *p;

	/* Search down the trie until the end of string is reached */

	node = trie->root_node;

	for (p = key; *p != '\0'; ++p)
	{

		if (node == NULL)
		{
			/* Not found in the tree. Return. */

			return NULL;
		}

		/* Jump to the next node */

		node = node->next[(unsigned char)*p];
	}

	/* This key is present if the value at the last node is not NULL */

	return node;
}

static stl_trie_node *stl_trie_find_end_binary(stl_trie *trie, unsigned char *key,
											   int key_length)
{
	stl_trie_node *node;
	int j;
	int c;

	/* Search down the trie until the end of string is reached */

	node = trie->root_node;

	for (j = 0; j < key_length; j++)
	{

		if (node == NULL)
		{
			/* Not found in the tree. Return. */
			return NULL;
		}

		c = (unsigned char)key[j];

		/* Jump to the next node */

		node = node->next[c];
	}

	/* This key is present if the value at the last node is not NULL */

	return node;
}

/* Roll back an insert operation after a failed malloc() call. */

static void stl_trie_insert_rollback(stl_trie *trie, unsigned char *key)
{
	stl_trie_node *node;
	stl_trie_node **prev_ptr;
	stl_trie_node *next_node;
	stl_trie_node **next_prev_ptr;
	unsigned char *p;

	/* Follow the chain along.  We know that we will never reach the
	 * end of the string because stl_trie_insert never got that far.  As a
	 * result, it is not necessary to check for the end of string
	 * delimiter (NUL) */

	node = trie->root_node;
	prev_ptr = &trie->root_node;
	p = key;

	while (node != NULL)
	{

		/* Find the next node now. We might free this node. */

		next_prev_ptr = &node->next[(unsigned char)*p];
		next_node = *next_prev_ptr;
		++p;

		/* Decrease the use count and free the node if it
		 * reaches zero. */

		--node->use_count;

		if (node->use_count == 0)
		{
			free(node);

			if (prev_ptr != NULL)
			{
				*prev_ptr = NULL;
			}

			next_prev_ptr = NULL;
		}

		/* Update pointers */

		node = next_node;
		prev_ptr = next_prev_ptr;
	}
}

int stl_trie_insert(stl_trie *trie, char *key, void* value)
{
	stl_trie_node **rover;
	stl_trie_node *node;
	char *p;
	int c;

	/* Cannot insert NULL values */

	if (value == stl_trie_NULL)
	{
		return 0;
	}

	/* Search to see if this is already in the tree */

	node = stl_trie_find_end(trie, key);

	/* Already in the tree? If so, replace the existing value and
	 * return success. */

	if (node != NULL && node->data != stl_trie_NULL)
	{
		node->data = value;
		return 1;
	}

	/* Search down the trie until we reach the end of string,
	 * creating nodes as necessary */

	rover = &trie->root_node;
	p = key;

	for (;;)
	{

		node = *rover;

		if (node == NULL)
		{

			/* Node does not exist, so create it */

			node = (stl_trie_node *)calloc(1, sizeof(stl_trie_node));

			if (node == NULL)
			{

				/* Allocation failed.  Go back and undo
				 * what we have done so far. */

				stl_trie_insert_rollback(trie,
										 (unsigned char *)key);

				return 0;
			}

			node->data = stl_trie_NULL;

			/* Link in to the stl_trie */

			*rover = node;
		}

		/* Increase the node use count */

		++node->use_count;

		/* Current character */

		c = (unsigned char)*p;

		/* Reached the end of string?  If so, we're finished. */

		if (c == '\0')
		{

			/* Set the data at the node we have reached */

			node->data = value;

			break;
		}

		/* Advance to the next node in the chain */

		rover = &node->next[c];
		++p;
	}

	return 1;
}

int stl_trie_insert_binary(stl_trie *trie, unsigned char *key, int key_length,
						   void* value)
{
	stl_trie_node **rover;
	stl_trie_node *node;
	int p, c;

	/* Cannot insert NULL values */

	if (value == stl_trie_NULL)
	{
		return 0;
	}

	/* Search to see if this is already in the tree */

	node = stl_trie_find_end_binary(trie, key, key_length);

	/* Already in the tree? If so, replace the existing value and
	 * return success. */

	if (node != NULL && node->data != stl_trie_NULL)
	{
		node->data = value;
		return 1;
	}

	/* Search down the trie until we reach the end of string,
	 * creating nodes as necessary */

	rover = &trie->root_node;
	p = 0;

	for (;;)
	{

		node = *rover;

		if (node == NULL)
		{

			/* Node does not exist, so create it */

			node = (stl_trie_node *)calloc(1, sizeof(stl_trie_node));

			if (node == NULL)
			{

				/* Allocation failed.  Go back and undo
				 * what we have done so far. */

				stl_trie_insert_rollback(trie, key);

				return 0;
			}

			node->data = stl_trie_NULL;

			/* Link in to the stl_trie */

			*rover = node;
		}

		/* Increase the node use count */

		++node->use_count;

		/* Current character */

		c = (unsigned char)key[p];

		/* Reached the end of string?  If so, we're finished. */

		if (p == key_length)
		{

			/* Set the data at the node we have reached */

			node->data = value;

			break;
		}

		/* Advance to the next node in the chain */

		rover = &node->next[c];
		++p;
	}

	return 1;
}

int stl_trie_remove_binary(stl_trie *trie, unsigned char *key, int key_length)
{
	stl_trie_node *node;
	stl_trie_node *next;
	stl_trie_node **last_next_ptr;
	int p, c;

	/* Find the end node and remove the value */

	node = stl_trie_find_end_binary(trie, key, key_length);

	if (node != NULL && node->data != stl_trie_NULL)
	{
		node->data = stl_trie_NULL;
	}
	else
	{
		return 0;
	}

	/* Now traverse the tree again as before, decrementing the use
	 * count of each node.  Free back nodes as necessary. */

	node = trie->root_node;
	last_next_ptr = &trie->root_node;
	p = 0;

	for (;;)
	{

		/* Find the next node */
		c = (unsigned char)key[p];
		next = node->next[c];

		/* Free this node if necessary */

		--node->use_count;

		if (node->use_count <= 0)
		{
			free(node);

			/* Set the "next" pointer on the previous node to NULL,
			 * to unlink the freed node from the tree.  This only
			 * needs to be done once in a remove.  After the first
			 * unlink, all further nodes are also going to be
			 * free'd. */

			if (last_next_ptr != NULL)
			{
				*last_next_ptr = NULL;
				last_next_ptr = NULL;
			}
		}

		/* Go to the next character or finish */
		if (p == key_length)
		{
			break;
		}
		else
		{
			++p;
		}

		/* If necessary, save the location of the "next" pointer
		 * so that it may be set to NULL on the next iteration if
		 * the next node visited is freed. */

		if (last_next_ptr != NULL)
		{
			last_next_ptr = &node->next[c];
		}

		/* Jump to the next node */

		node = next;
	}

	/* Removed successfully */

	return 1;
}

int stl_trie_remove(stl_trie *trie, char *key)
{
	stl_trie_node *node;
	stl_trie_node *next;
	stl_trie_node **last_next_ptr;
	char *p;
	int c;

	/* Find the end node and remove the value */

	node = stl_trie_find_end(trie, key);

	if (node != NULL && node->data != stl_trie_NULL)
	{
		node->data = stl_trie_NULL;
	}
	else
	{
		return 0;
	}

	/* Now traverse the tree again as before, decrementing the use
	 * count of each node.  Free back nodes as necessary. */

	node = trie->root_node;
	last_next_ptr = &trie->root_node;
	p = key;

	for (;;)
	{

		/* Find the next node */

		c = (unsigned char)*p;
		next = node->next[c];

		/* Free this node if necessary */

		--node->use_count;

		if (node->use_count <= 0)
		{
			free(node);

			/* Set the "next" pointer on the previous node to NULL,
			 * to unlink the freed node from the tree.  This only
			 * needs to be done once in a remove.  After the first
			 * unlink, all further nodes are also going to be
			 * free'd. */

			if (last_next_ptr != NULL)
			{
				*last_next_ptr = NULL;
				last_next_ptr = NULL;
			}
		}

		/* Go to the next character or finish */

		if (c == '\0')
		{
			break;
		}
		else
		{
			++p;
		}

		/* If necessary, save the location of the "next" pointer
		 * so that it may be set to NULL on the next iteration if
		 * the next node visited is freed. */

		if (last_next_ptr != NULL)
		{
			last_next_ptr = &node->next[c];
		}

		/* Jump to the next node */

		node = next;
	}

	/* Removed successfully */

	return 1;
}

void* stl_trie_lookup(stl_trie *trie, char *key)
{
	stl_trie_node *node;

	node = stl_trie_find_end(trie, key);

	if (node != NULL)
	{
		return node->data;
	}
	else
	{
		return stl_trie_NULL;
	}
}

void* stl_trie_lookup_binary(stl_trie *trie, unsigned char *key, int key_length)
{
	stl_trie_node *node;

	node = stl_trie_find_end_binary(trie, key, key_length);

	if (node != NULL)
	{
		return node->data;
	}
	else
	{
		return stl_trie_NULL;
	}
}

unsigned int stl_trie_num_entries(stl_trie *trie)
{
	/* To find the number of entries, simply look at the use count
	 * of the root node. */

	if (trie->root_node == NULL)
	{
		return 0;
	}
	else
	{
		return trie->root_node->use_count;
	}
}
