/*************************************************************************
  > File Name: stl_set.c
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Wed 05 Apr 2023 12:56:09 PM HKT
 ************************************************************************/

#include "stl_set.h"
#include <stdio.h>
#include <stdlib.h>
typedef struct
{
  stl_set *set;
  stl_set_entry *next_entry;
  unsigned int next_chain;
} stl_set_iterator;

static const unsigned int set_primes[] = {
	193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317,
	196613, 393241, 786433, 1572869, 3145739, 6291469,
	12582917, 25165843, 50331653, 100663319, 201326611,
	402653189, 805306457, 1610612741,
};


static const unsigned int set_num_primes = sizeof(set_primes) / sizeof(int);

static int stl_set_allocate_table(stl_set *set)
{
  /* Determine the table size based on the current prime index.
   * An attempt is made here to ensure sensible behavior if the
   * maximum prime is exceeded, but in practice other things are
   * likely to break long before that happens. */

  if (set->prime_index < set_num_primes)
  {
    set->table_size = set_primes[set->prime_index];
  }
  else
  {
    set->table_size = set->entries * 10;
  }

  /* Allocate the table and initialise to NULL */

  set->table = calloc(set->table_size, sizeof(stl_set_entry *));

  return set->table != NULL;
}
stl_set *stl_set_new(stl_set_entry_hash_func hash_func, stl_set_entry_equal_func equal_func)
{
  stl_set *new_set;

  /* Allocate a new set and fill in the fields */

  new_set = (stl_set *)malloc(sizeof(stl_set));

  if (new_set == NULL)
  {
    return NULL;
  }

  new_set->hash_func = hash_func;
  new_set->equal_func = equal_func;
  new_set->entries = 0;
  new_set->prime_index = 0;
  new_set->free_func = NULL;

  /* Allocate the table */

  if (!stl_set_allocate_table(new_set))
  {
    free(new_set);
    return NULL;
  }

  return new_set;
}
static void stl_set_free_entry(stl_set *set, stl_set_entry *entry)
{
  /* If there is a free function registered, call it to free the
   * data for this entry first */

  if (set->free_func != NULL)
  {
    set->free_func(entry->data);
  }

  /* Free the entry structure */

  free(entry);
}

void stl_set_free(stl_set *set)
{
  stl_set_entry *rover;
  stl_set_entry *next;
  unsigned int i;

  /* Free all entries in all chains */

  for (i = 0; i < set->table_size; ++i)
  {
    rover = set->table[i];

    while (rover != NULL)
    {
      next = rover->next;

      /* Free this entry */

      stl_set_free_entry(set, rover);

      /* Advance to the next entry in the chain */

      rover = next;
    }
  }

  /* Free the table */

  free(set->table);

  /* Free the set structure */

  free(set);
}

static int stl_set_enlarge(stl_set *set)
{
  stl_set_entry *rover;
  stl_set_entry *next;
  stl_set_entry **old_table;
  unsigned int old_table_size;
  unsigned int old_prime_index;
  unsigned int index;
  unsigned int i;

  /* Store the old table */

  old_table = set->table;
  old_table_size = set->table_size;
  old_prime_index = set->prime_index;

  /* Use the next table size from the prime number array */

  ++set->prime_index;

  /* Allocate the new table */

  if (!stl_set_allocate_table(set))
  {
    set->table = old_table;
    set->table_size = old_table_size;
    set->prime_index = old_prime_index;

    return 0;
  }

  /* Iterate through all entries in the old table and add them
   * to the new one */

  for (i = 0; i < old_table_size; ++i)
  {

    /* Walk along this chain */

    rover = old_table[i];

    while (rover != NULL)
    {

      next = rover->next;

      /* Hook this entry into the new table */

      index = set->hash_func(rover->data) % set->table_size;
      rover->next = set->table[index];
      set->table[index] = rover;

      /* Advance to the next entry in the chain */

      rover = next;
    }
  }

  /* Free back the old table */

  free(old_table);

  /* Resized successfully */

  return 1;
}

int stl_set_insert(stl_set *set, void *data)
{
  stl_set_entry *newentry;
  stl_set_entry *rover;
  unsigned int index;

  /* The hash table becomes less efficient as the number of entries
   * increases. Check if the percentage used becomes large. */

  if ((set->entries * 3) / set->table_size > 0)
  {

    /* The table is more than 1/3 full and must be increased
     * in size */

    if (!stl_set_enlarge(set))
    {
      return 0;
    }
  }

  /* Use the hash of the data to determine an index to insert into the
   * table at. */

  index = set->hash_func(data) % set->table_size;

  /* Walk along this chain and attempt to determine if this data has
   * already been added to the table */

  rover = set->table[index];

  while (rover != NULL)
  {

    if (set->equal_func(data, rover->data) != 0)
    {

      /* This data is already in the set */

      return 0;
    }

    rover = rover->next;
  }

  /* Not in the set.  We must add a new entry. */

  /* Make a new entry for this data */

  newentry = (stl_set_entry *)malloc(sizeof(stl_set_entry));

  if (newentry == NULL)
  {
    return 0;
  }

  newentry->data = data;

  /* Link into chain */

  newentry->next = set->table[index];
  set->table[index] = newentry;

  /* Keep track of the number of entries in the set */

  ++set->entries;

  /* Added successfully */

  return 1;
}

int stl_set_remove(stl_set *set, void *data)
{
  stl_set_entry **rover;
  stl_set_entry *entry;
  unsigned int index;

  /* Look up the data by its hash key */

  index = set->hash_func(data) % set->table_size;

  /* Search this chain, until the corresponding entry is found */

  rover = &set->table[index];

  while (*rover != NULL)
  {
    if (set->equal_func(data, (*rover)->data) != 0)
    {

      /* Found the entry */

      entry = *rover;

      /* Unlink from the linked list */

      *rover = entry->next;

      /* Update counter */

      --set->entries;

      /* Free the entry and return */

      stl_set_free_entry(set, entry);

      return 1;
    }

    /* Advance to the next entry */

    rover = &((*rover)->next);
  }

  /* Not found in set */

  return 0;
}

int stl_set_query(stl_set *set, void *data)
{
  stl_set_entry *rover;
  unsigned int index;

  /* Look up the data by its hash key */

  index = set->hash_func(data) % set->table_size;

  /* Search this chain, until the corresponding entry is found */

  rover = set->table[index];

  while (rover != NULL)
  {
    if (set->equal_func(data, rover->data) != 0)
    {

      /* Found the entry */

      return 1;
    }

    /* Advance to the next entry in the chain */

    rover = rover->next;
  }

  /* Not found */

  return 0;
}

unsigned int set_num_entries(stl_set *set)
{
  return set->entries;
}

void **stl_set_to_array(stl_set *set)
{
  void **array;
  int array_counter;
  unsigned int i;
  stl_set_entry *rover;

  /* Create an array to hold the set entries */

  array = malloc(sizeof(void *) * set->entries);

  if (array == NULL)
  {
    return NULL;
  }

  array_counter = 0;

  /* Iterate over all entries in all chains */

  for (i = 0; i < set->table_size; ++i)
  {

    rover = set->table[i];

    while (rover != NULL)
    {
      array[array_counter++] = rover->data;
      rover = rover->next;
    }
  }

  return array;
}

static void stl_set_iterate(stl_set *set, stl_set_iterator *iter)
{
  unsigned int chain;

  iter->set = set;
  iter->next_entry = NULL;

  /* Find the first entry */

  for (chain = 0; chain < set->table_size; ++chain)
  {

    /* There is a value at the start of this chain */

    if (set->table[chain] != NULL)
    {
      iter->next_entry = set->table[chain];
      break;
    }
  }

  iter->next_chain = chain;
}

static void *stl_set_iter_next(stl_set_iterator *iterator)
{
  stl_set *set;
  void *result;
  stl_set_entry *current_entry;
  unsigned int chain;

  set = iterator->set;

  /* No more entries? */

  if (iterator->next_entry == NULL)
  {
    return NULL;
  }

  /* We have the result immediately */

  current_entry = iterator->next_entry;
  result = current_entry->data;

  /* Advance next_entry to the next SetEntry in the Set. */

  if (current_entry->next != NULL)
  {

    /* Use the next value in this chain */

    iterator->next_entry = current_entry->next;
  }
  else
  {

    /* Default value if no valid chain is found */

    iterator->next_entry = NULL;

    /* No more entries in this chain.  Search the next chain */

    chain = iterator->next_chain + 1;

    while (chain < set->table_size)
    {

      /* Is there a chain at this table entry? */

      if (set->table[chain] != NULL)
      {

        /* Valid chain found! */

        iterator->next_entry = set->table[chain];

        break;
      }

      /* Keep searching until we find an empty chain */

      ++chain;
    }

    iterator->next_chain = chain;
  }

  return result;
}

int stl_set_iter_has_more(stl_set_iterator *iterator)
{
  return iterator->next_entry != NULL;
}
stl_set *stl_set_union(stl_set *set1, stl_set *set2)
{
  stl_set_iterator iterator;
  stl_set *new_set;
  void *value;

  new_set = stl_set_new(set1->hash_func, set1->equal_func);

  if (new_set == NULL)
  {
    return NULL;
  }

  /* Add all values from the first set */

  stl_set_iterate(set1, &iterator);

  while (stl_set_iter_has_more(&iterator))
  {

    /* Read the next value */

    value = stl_set_iter_next(&iterator);

    /* Copy the value into the new set */

    if (!stl_set_insert(new_set, value))
    {

      /* Failed to insert */

      stl_set_free(new_set);
      return NULL;
    }
  }

  /* Add all values from the second set */

  stl_set_iterate(set2, &iterator);

  while (stl_set_iter_has_more(&iterator))
  {

    /* Read the next value */

    value = stl_set_iter_next(&iterator);

    /* Has this value been put into the new set already?
     * If so, do not insert this again */

    if (stl_set_query(new_set, value) == 0)
    {
      if (!stl_set_insert(new_set, value))
      {

        /* Failed to insert */

        stl_set_free(new_set);
        return NULL;
      }
    }
  }

  return new_set;
}

stl_set *stl_set_intersection(stl_set *set1, stl_set *set2)
{
  stl_set *new_set;
  stl_set_iterator iterator;
  void *value;

  new_set = stl_set_new(set1->hash_func, set2->equal_func);

  if (new_set == NULL)
  {
    return NULL;
  }

  /* Iterate over all values in set 1. */

  stl_set_iterate(set1, &iterator);

  while (stl_set_iter_has_more(&iterator))
  {

    /* Get the next value */

    value = stl_set_iter_next(&iterator);

    /* Is this value in set 2 as well?  If so, it should be
     * in the new set. */

    if (stl_set_query(set2, value) != 0)
    {

      /* Copy the value first before inserting,
       * if necessary */

      if (!stl_set_insert(new_set, value))
      {
        stl_set_free(new_set);

        return NULL;
      }
    }
  }

  return new_set;
}




extern inline void stl_set_register_free_func(stl_set *set, stl_set_entry_free_func free_func);