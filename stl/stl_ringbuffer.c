#include "stl_ringbuffer.h"

/**
 * @file
 * Implementation of ring buffer functions.
 */

void ring_buffer_init(stl_ringbuffer *buffer, char *buf, size_t buf_size)
{
  STL_RING_BUFFER_ASSERT(STL_RING_BUFFER_IS_POWER_OF_TWO(buf_size) == 1);
  buffer->buffer = buf;
  buffer->buffer_mask = buf_size - 1;
  buffer->tail_index = 0;
  buffer->head_index = 0;
}

void ring_buffer_queue(stl_ringbuffer *buffer, char data)
{
  /* Is buffer full? */
  if (ring_buffer_is_full(buffer))
  {
    /* Is going to overwrite the oldest byte */
    /* Increase tail index */
    buffer->tail_index = ((buffer->tail_index + 1) & RING_BUFFER_MASK(buffer));
  }

  /* Place data in buffer */
  buffer->buffer[buffer->head_index] = data;
  buffer->head_index = ((buffer->head_index + 1) & RING_BUFFER_MASK(buffer));
}

void ring_buffer_queue_arr(stl_ringbuffer *buffer, const char *data, size_t size)
{
  /* Add bytes; one by one */
  size_t i;
  for (i = 0; i < size; i++)
  {
    ring_buffer_queue(buffer, data[i]);
  }
}

uint8_t ring_buffer_dequeue(stl_ringbuffer *buffer, char *data)
{
  if (ring_buffer_is_empty(buffer))
  {
    /* No items */
    return 0;
  }

  *data = buffer->buffer[buffer->tail_index];
  buffer->tail_index = ((buffer->tail_index + 1) & RING_BUFFER_MASK(buffer));
  return 1;
}

size_t ring_buffer_dequeue_arr(stl_ringbuffer *buffer, char *data, size_t len)
{
  if (ring_buffer_is_empty(buffer))
  {
    /* No items */
    return 0;
  }

  char *data_ptr = data;
  size_t cnt = 0;
  while ((cnt < len) && ring_buffer_dequeue(buffer, data_ptr))
  {
    cnt++;
    data_ptr++;
  }
  return cnt;
}

uint8_t ring_buffer_peek(stl_ringbuffer *buffer, char *data, size_t index)
{
  if (index >= ring_buffer_num_items(buffer))
  {
    /* No items at index */
    return 0;
  }

  /* Add index to pointer */
  size_t data_index = ((buffer->tail_index + index) & RING_BUFFER_MASK(buffer));
  *data = buffer->buffer[data_index];
  return 1;
}

extern inline uint8_t ring_buffer_is_empty(stl_ringbuffer *buffer);
extern inline uint8_t ring_buffer_is_full(stl_ringbuffer *buffer);
extern inline size_t ring_buffer_num_items(stl_ringbuffer *buffer);
