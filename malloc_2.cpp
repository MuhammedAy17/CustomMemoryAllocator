//
// Created by Muham on 3/22/2024.
//

#include <unistd.h>
#include <cstring>

const size_t MAX_SIZE = 100000000;

typedef struct MallocMetadata{
    size_t size;
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;
}MetaData;

//Global Pointers and Variables
MetaData *head = nullptr;
MetaData *tail = nullptr;
size_t numAllocatedBlocks = 0;
size_t numAllocatedBytes = 0;
size_t numFreeBlocks = 0;
size_t numFreeBytes = 0;

void *smalloc(size_t size){
    if(size == 0 || size > MAX_SIZE){
        return nullptr;
    }
    MetaData *tmp = head;
    while(tmp != nullptr){
        if(!tmp->is_free || tmp->size < size){
            tmp = tmp->next;
            continue;
        }
        tmp->is_free = false;
        numFreeBlocks--;
        numFreeBytes -= tmp->size;
        return (char*)tmp + sizeof(MetaData);
    }
    void *result = sbrk(size + sizeof(MetaData));
    if(result == (void *)-1){
        return nullptr;
    }

    MetaData* newMetaData = (MetaData *)result;
    newMetaData->prev = tail;
    if(tail != nullptr) {
        tail->next = newMetaData;
    }
    tail = newMetaData;
    if(head == nullptr){
        head = newMetaData;
    }
    newMetaData->next = nullptr;
    newMetaData->size = size;
    newMetaData->is_free = false;

    numAllocatedBlocks++;
    numAllocatedBytes += size;
    return (char*)result + sizeof(MetaData);
}

void *scalloc(size_t num, size_t size){
    if(size == 0 || num == 0 || (size * num) > MAX_SIZE)
        return nullptr;
    size_t bytes = (size * num);
    void *result = smalloc(bytes);
    if (result == nullptr)
        return nullptr;
    memset(result,0,bytes);
    return result;
}

void sfree(void *p){
    if(p == nullptr)
        return;
    MetaData *data;
    data = (MetaData*)((char*)p - sizeof(MetaData));
    if(data->is_free)
        return;
    data->is_free = true;
    numFreeBlocks++;
    numFreeBytes += data->size;
}

void *srealloc(void *oldp, size_t size){
    if(size == 0 || size > MAX_SIZE){
        return nullptr;
    }
    if(oldp == nullptr)
        return smalloc(size);
    MetaData *olData;
    olData = (MetaData*)((char*)oldp - sizeof(MetaData));
    if(olData->size >= size)
        return oldp;
    void *result = smalloc(size);
    if (result == nullptr)
        return nullptr;
    memmove(result,oldp,olData->size);
    sfree(oldp);
    return result;
}

size_t _num_free_blocks(){
    return numFreeBlocks;
}

size_t _num_free_bytes(){
    return numFreeBytes;
}

size_t _num_allocated_blocks(){
    return numAllocatedBlocks;
}

size_t _num_allocated_bytes(){
    return numAllocatedBytes;
}

size_t _num_meta_data_bytes(){
    return numAllocatedBlocks * sizeof (MetaData);
}

size_t _size_meta_data(){
    return sizeof(MetaData);
}
