//
// Created by Muham on 3/22/2024.
//

#include <unistd.h>
#include <cstring>

const size_t MAX_SIZE = 100000000;

void *smalloc(size_t size){
    if(size == 0 || size > MAX_SIZE){
        return nullptr;
    }
    void *result = sbrk(size);
    if(result == (void *)-1){
        return nullptr;
    }
    return result;
}
