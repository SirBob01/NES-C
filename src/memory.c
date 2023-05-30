#include "./memory.h"

memory_t allocate_memory(unsigned long size) {
    memory_t memory;
    memory.size = size;
    memory.buffer = (char *)malloc(size);
    assert(memory.buffer != NULL);

    return memory;
}

void free_memory(memory_t *memory) { free(memory->buffer); }