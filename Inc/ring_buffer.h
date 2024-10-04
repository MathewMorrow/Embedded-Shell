#pragma once

#include <stdint.h>
#include <stdbool.h>

#define RING_BUFFER_SIZE (256)

// Struct to represent a ring buffer
typedef struct ringBuffer_t_s{
    bool initialized;
    volatile uint8_t buffer[RING_BUFFER_SIZE];  // The buffer array
    volatile uint16_t head;            // Index of the head (for storing data)
    volatile uint16_t tail;            // Index of the tail (for reading data)
    volatile uint16_t count;           // Number of elements currently in the buffer
} ringBuffer_t;

// Function prototypes
void ring_buffer_init(ringBuffer_t *rb);
bool ring_buffer_is_initialized(ringBuffer_t *rb);
void ring_buffer_clear(ringBuffer_t *rb);
bool ring_buffer_is_full(ringBuffer_t *rb);
bool ring_buffer_is_empty(ringBuffer_t *rb);
uint16_t ring_buffer_data_ready(ringBuffer_t *rb);
uint8_t ring_buffer_get_char(ringBuffer_t *rb);
bool ring_buffer_store_char(ringBuffer_t *rb, uint8_t data);