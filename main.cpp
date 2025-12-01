#include <iostream>
#include "malloc_3.cpp"  // or whichever allocator you want to test

int main() {
    void* p = smalloc(100);
    void* q = smalloc(200);

    sfree(p);
    sfree(q);

    std::cout << "Allocated blocks: " << _num_allocated_blocks() << std::endl;
    return 0;
}
