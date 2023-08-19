#include <inttypes.h>
#include <stddef.h>
#include <assert.h>
/**
 * @file
 * Prototypes and ures for the ring buffer module.
 */

#ifndef _STL_RINGBUFFER_H
#define _STL_RINGBUFFER_H

#define STL_RING_BUFFER_ASSERT(x) assert(x)

/**
 * Checks if the buffer_size is a power of two.
 * Due to the design only <tt> RING_BUFFER_SIZE-1 </tt> items
 * can be contained in the buffer.
 * buffer_size must be a power of two.
 */
#define STL_RING_BUFFER_IS_POWER_OF_TWO(buffer_size) ((buffer_size & (buffer_size - 1)) == 0)

/**
 * Used as a modulo operator
 * as <tt> a % b = (a & (b − 1)) </tt>
 * where \c a is a positive index in the buffer and
 * \c b is the (power of two) size of the buffer.
 */
#define RING_BUFFER_MASK(rb) (rb->buffer_mask)

/**
 * ure which holds a ring buffer.
 * The buffer contains a buffer array
 * as well as metadata for the ring buffer.
 */
typedef struct
{
  /** Buffer memory. */
  char *buffer;
  /** Buffer mask. */
  size_t buffer_mask;
  /** Index of tail. */
  size_t tail_index;
  /** Index of head. */
  size_t head_index;
} stl_ringbuffer;

/**
 * Initializes the ring buffer pointed to by <em>buffer</em>.
 * This function can also be used to empty/reset the buffer.
 * @param buffer The ring buffer to initialize.
 * @param buf The buffer allocated for the ringbuffer.
 * @param buf_size The size of the allocated ringbuffer.
 */
void ring_buffer_init(stl_ringbuffer *buffer, char *buf, size_t buf_size);

/**
 * Adds a byte to a ring buffer.
 * @param buffer The buffer in which the data should be placed.
 * @param data The byte to place.
 */
void ring_buffer_queue(stl_ringbuffer *buffer, char data);

/**
 * Adds an array of bytes to a ring buffer.
 * @param buffer The buffer in which the data should be placed.
 * @param data A pointer to the array of bytes to place in the queue.
 * @param size The size of the array.
 */
void ring_buffer_queue_arr(stl_ringbuffer *buffer, const char *data, size_t size);

/**
 * Returns the oldest byte in a ring buffer.
 * @param buffer The buffer from which the data should be returned.
 * @param data A pointer to the location at which the data should be placed.
 * @return 1 if data was returned; 0 otherwise.
 */
uint8_t ring_buffer_dequeue(stl_ringbuffer *buffer, char *data);

/**
 * Returns the <em>len</em> oldest bytes in a ring buffer.
 * @param buffer The buffer from which the data should be returned.
 * @param data A pointer to the array at which the data should be placed.
 * @param len The maximum number of bytes to return.
 * @return The number of bytes returned.
 */
size_t ring_buffer_dequeue_arr(stl_ringbuffer *buffer, char *data, size_t len);
/**
 * Peeks a ring buffer, i.e. returns an element without removing it.
 * @param buffer The buffer from which the data should be returned.
 * @param data A pointer to the location at which the data should be placed.
 * @param index The index to peek.
 * @return 1 if data was returned; 0 otherwise.
 */
uint8_t ring_buffer_peek(stl_ringbuffer *buffer, char *data, size_t index);

/**
 * Returns whether a ring buffer is empty.
 * @param buffer The buffer for which it should be returned whether it is empty.
 * @return 1 if empty; 0 otherwise.
 */
inline uint8_t ring_buffer_is_empty(stl_ringbuffer *buffer)
{
  return (buffer->head_index == buffer->tail_index);
}

/**
 * Returns whether a ring buffer is full.
 * @param buffer The buffer for which it should be returned whether it is full.
 * @return 1 if full; 0 otherwise.
 */
inline uint8_t ring_buffer_is_full(stl_ringbuffer *buffer)
{
  return ((buffer->head_index - buffer->tail_index) & RING_BUFFER_MASK(buffer)) == RING_BUFFER_MASK(buffer);
}

/**
 * Returns the number of items in a ring buffer.
 * @param buffer The buffer for which the number of items should be returned.
 * @return The number of items in the ring buffer.
 */
inline size_t ring_buffer_num_items(stl_ringbuffer *buffer)
{
  return ((buffer->head_index - buffer->tail_index) & RING_BUFFER_MASK(buffer));
}

#endif
