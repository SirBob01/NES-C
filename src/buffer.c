#include "./buffer.h"

buffer_t *create_buffer(unsigned capacity) {
    buffer_t *buffer = (buffer_t *)malloc(sizeof(buffer_t));
    buffer->memory = allocate_memory(capacity);
    buffer->read = 0;
    buffer->write = 0;
    buffer->mask = capacity - 1;

    if (capacity == 0 || (capacity & buffer->mask)) {
        fprintf(stderr,
                "Error: Buffer capacity must be a power of 2 (received %d)\n",
                capacity);
        exit(1);
    }
    return buffer;
}

void destroy_buffer(buffer_t *buffer) {
    free_memory(&buffer->memory);
    free(buffer);
}

unsigned get_size_buffer(buffer_t *buffer) {
    return buffer->write - buffer->read;
}

unsigned read_buffer(buffer_t *buffer, unsigned char *dst, unsigned n) {
    // Compute copy partitions
    unsigned size = get_size_buffer(buffer);
    unsigned offset = buffer->read & buffer->mask;
    unsigned length = min(n, size);
    unsigned l_length = min(length, buffer->memory.size - offset);
    unsigned r_length = length - l_length;

    // Perform copy
    unsigned char *src = buffer->memory.buffer;
    memcpy(dst, src + offset, l_length);
    memcpy(dst + l_length, src, r_length);

    buffer->read += length;
    return length;
}

unsigned write_buffer(buffer_t *buffer, unsigned char *src, unsigned n) {
    // Compute copy partitions
    unsigned size = get_size_buffer(buffer);
    unsigned remaining = buffer->memory.size - size;
    unsigned offset = buffer->write & buffer->mask;
    unsigned length = min(n, remaining);
    unsigned l_length = min(length, buffer->memory.size - offset);

    // Perform copy
    unsigned char *dst = buffer->memory.buffer;
    memcpy(dst + offset, src, l_length);
    memcpy(dst, src + l_length, length - l_length);

    buffer->write += length;
    return length;
}

void clear_buffer(buffer_t *buffer) {
    buffer->read = 0;
    buffer->write = 0;
}