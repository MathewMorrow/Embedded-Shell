#include "ring_buffer.h"

// TO DO: Can this be made data size agnostic ie. char, 16bit, 32bit?
// Perhaps with a MACRO definition for data type but that would fix across the entire project
// Another option could be to define and increment the buffer pointers externally?

// Initialize the ring buffer
void ring_buffer_init(ringBuffer_t *rb)
{
    rb->initialized = true;
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
}

bool ring_buffer_is_initialized(ringBuffer_t *rb)
{
    return rb->initialized;
}

// Clear the ring buffer by resetting the pointers and count
void ring_buffer_clear(ringBuffer_t *rb)
{
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
}

// Check if the ring buffer is full
bool ring_buffer_is_full(ringBuffer_t *rb)
{
    return rb->count == RING_BUFFER_SIZE;
}

// Check if the ring buffer is empty
bool ring_buffer_is_empty(ringBuffer_t *rb)
{
    return rb->count == 0;
}

// Check how much data is in the ring buffer
uint16_t ring_buffer_data_ready(ringBuffer_t *rb)
{
    return rb->count;
}

// Read a single character from the ring buffer
uint8_t ring_buffer_get_char(ringBuffer_t *rb)
{
    if(ring_buffer_is_empty(rb) || ring_buffer_is_initialized(rb) == false) {
        return '\0';
    }

    uint8_t newChar = rb->buffer[rb->tail];  // Read from the buffer
    rb->tail = (rb->tail + 1) % RING_BUFFER_SIZE;  // Increment tail
    rb->count--;  // Decrease the number of stored bytes
    return newChar;
}

// Write a single character to the ring buffer
bool ring_buffer_store_char(ringBuffer_t *rb, uint8_t data)
{
    if (ring_buffer_is_full(rb)  || ring_buffer_is_initialized(rb) == false) {
        return false;
    }

    rb->buffer[rb->head] = data;  // Store the data in the buffer
    rb->head = (rb->head + 1) % RING_BUFFER_SIZE;  // Increment head
    rb->count++;  // Increase the number of stored bytes
    return true;
}