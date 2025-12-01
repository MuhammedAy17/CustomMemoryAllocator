# Custom Memory Allocator (C++)

A minimal custom memory allocator written in C++, implementing three versions:
- `malloc_1.cpp` — basic sbrk-based allocator
- `malloc_2.cpp` — free-list allocator with smalloc/scalloc/sfree/srealloc
- `malloc_3.cpp` — buddy-system allocator with splitting and merging

This project demonstrates low-level memory management concepts similar to how `malloc()` works internally.
