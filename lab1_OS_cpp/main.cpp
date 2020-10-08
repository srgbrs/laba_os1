#include <iostream>
#include <zconf.h>

using namespace std;

struct header_t
{
    size_t size;
    unsigned is_free;
    struct header_t *next;
};

struct header_t *head = NULL, *tail = NULL;

struct header_t *get_free_block(size_t size)
{
    struct header_t *curr = head;
    while(curr) {
        if (curr->is_free && curr->size >= size)
            return curr;
        curr = curr->next;
    }
    return NULL;
}

void *mem_alloc(size_t size)
{
    size_t total_size;
    void *block;
    struct header_t *header;

    header = get_free_block(size);
    if (header) {
        header->is_free = 0;
        return (void*)(header + 1);
    }
    total_size = sizeof(struct header_t) + size;
    block = sbrk(total_size);
    if (block == (void*) -1) {
        return NULL;
    }
    header = static_cast<header_t *>(block);
    header->size = size;
    header->is_free = 0;
    header->next = NULL;
    if (!head)
        head = header;
    if (tail)
        tail->next = header;
    tail = header;
    
    return (void*)(header + 1);
}

void mem_free(void *block)
{
    struct header_t *header, *tmp;
    void *programbreak;

    header = (struct header_t*)block - 1;
    programbreak = sbrk(0);
    
    if ((char*)block + header->size == programbreak)
    {
        if (head == tail)
        {
            head = tail = NULL;
        }
        else
        {
            tmp = head;
            while (tmp) {
                if(tmp->next == tail)
                {
                    tmp->next = NULL;
                    tail = tmp;
                }
                tmp = tmp->next;
            }
        }
        sbrk(0 - sizeof(struct header_t) - header->size);
        return;
    }
    header->is_free = 1;
}

void *mem_realloc(void *block, size_t size)
{
    struct header_t *header;
    void *ret;
    
    if (!block || !size)
        return mem_alloc(size);

    header = (struct header_t*)block - 1;
    
    if (header->size >= size)
        return block;
    ret = mem_alloc(size);
    
    if (ret) {
        memcpy(ret, block, header->size);
        mem_free(block);
    }
    return ret;
}

void mem_dump()
{
    header_t *curr = head;
    printf("head adress : %p, tail adress : %p \n", (void*)head, (void*)tail);
    while(curr)
    {
        cout << ((void*)curr) << " |" << curr->size << " |" << curr->is_free
        << " |" << (void*)curr->next << "\n";
        
        curr = curr->next;
    }
}

int main(int argc, const char * argv[]) {

    void *x7 = mem_alloc(1);
    void *x8 = mem_alloc(2);
    mem_dump();
    
    mem_realloc(x7, 4);
    mem_dump();
    
    mem_free(x8);
    mem_dump();
    
    void *x9 = mem_alloc(10);
    mem_dump();
    
    return 0;
}
