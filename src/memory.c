#include "./memory.h"

memory_t allocate_memory(unsigned long size) {
    memory_t memory;
    memory.size = size;
    memory.buffer = (unsigned char *)calloc(size, 1);
    if (memory.buffer == NULL) {
        fprintf(stderr, "Error: Unable to allocate %lu bytes\n", size);
        exit(1);
    }

    return memory;
}

bool is_free_memory(memory_t *memory) { return memory->buffer == NULL; }

void free_memory(memory_t *memory) {
    free(memory->buffer);
    memory->buffer = NULL;
    memory->size = 0;
}