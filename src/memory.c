#include "./memory.h"

memory_t allocate_memory(unsigned long size) {
    memory_t memory;
    memory.size = size;
    memory.buffer = (unsigned char *)malloc(size);
    if (memory.buffer == NULL) {
        fprintf(stderr, "Error: unable to allocate %lu bytes\n", size);
        exit(1);
    }

    return memory;
}

void free_memory(memory_t *memory) { free(memory->buffer); }