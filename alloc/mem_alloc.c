#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>


typedef char ALIGN[16];
union header {
    struct {
        size_t size;
        unsigned is_free;
        struct header_t* next;
    } meta;
    ALIGN mem_block;
};

typedef union header header_t;

pthread_mutex_t global_malloc_lock;
header_t* head = NULL;
header_t* tail = NULL;

void* malloc(size_t size);
header_t* get_free_block(size_t size);
void free(void* block);
void* realloc(void* block, size_t size);

void* malloc(size_t size) {
    size_t total_size;
    void* block;
    header_t* header;

    if (!size) {
        return NULL;
    }

    pthread_mutex_lock(&global_malloc_lock);

    header = get_free_block(size);
    if (header) {
        header->meta.is_free = 0;
        pthread_mutex_unlock(&global_malloc_lock);
        return (void*)(header + 1);
    }

    total_size = sizeof(header_t) + size;
    block = sbrk(total_size);
    if (block == (void*) -1) {
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL;
    }

    header = block;
    header->meta.size = size;
    header->meta.is_free = 0;
    header->meta.next = NULL;

    if (!head) {
        head = header;
    }

    if (tail) {
        tail->meta.next = header;
    }

    tail = header;
    pthread_mutex_unlock(&global_malloc_lock);

    return (void*)(header + 1);
}

header_t* get_free_block(size_t size) {
    header_t* current = head;

    while (current) {
        if (current->meta.is_free && current->meta.size >= size) {
            return current;
        }

        current = current->meta.next;
    }

    return NULL;
}

void free(void* block) {
    header_t *header, *current;
    void* brk;

    if (!block) {
        return;
    }
    pthread_mutex_lock(&global_malloc_lock);
    header = (header_t*)block - 1;

    brk = sbrk(0);

    if ((char*)block + header->meta.size == brk) {
        if (head == tail) {
            head = tail = NULL;
        } else {
            current = head;
            while (current) {
                if (current->meta.next == tail) {
                    current->meta.next = NULL;
                    tail = current;
                }

                current = current->meta.next;
            }
        }

        sbrk(0 - sizeof(header_t) - header->meta.size);
        pthread_mutex_unlock(&global_malloc_lock);

        return;
    }

    header->meta.is_free = 1;
    pthread_mutex_unlock(&global_malloc_lock);
}

void* realloc(void* block, size_t size) {
    header_t* header;
    void* new_block;

    if (!block || !size) {
        return malloc(size);
    }
    
    header = (header_t*)block - 1;
    
    if (header->meta.size >= size) {
        return block;
    }

    new_block = malloc(size);

    if (new_block) {
        memcpy(new_block, block, header->meta.size);
        free(block);
    }

    return new_block;
}
