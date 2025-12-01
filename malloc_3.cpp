//
// Created by Muham on 3/22/2024.
//


#include <unistd.h>
#include <cstring>
#include <cmath>
#include <sys/mman.h>
#include <ctime>
#include <cstdio>

const int MAX_ALLOCATION_SIZE = (int)pow(10, 8);
const int MAX_ORDER = 10;
const int ORDER_ARRAY_LENGTH = MAX_ORDER + 1;
const size_t MINIMUM_BLOCK_SIZE = 128;
const size_t MAXIMAL_BLOCK_SIZE = MINIMUM_BLOCK_SIZE * pow(2, MAX_ORDER);
const int INDEX_BASE = 3;
const int INDEX_OFFSET = (int)pow(INDEX_BASE, MAX_ORDER + 1) - 1;
const int NUM_OF_MAX_BLOCKS = 32;


typedef struct MallocMetadata {
    long index;
    size_t size;
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;
} MetaData;

class MemoryManager{
public:
    size_t numAllocatedBlocks ;
    size_t numAllocatedBytes ;
    size_t numFreeBlocks ;
    size_t numFreeBytes ;
    MetaData* array[ORDER_ARRAY_LENGTH];
    bool initMalloc;
public:
    MemoryManager():numAllocatedBlocks(0),numAllocatedBytes(0),numFreeBlocks(0),
                    numFreeBytes(0),initMalloc(false){

        for (int i = 0;i < ORDER_ARRAY_LENGTH; i++) {
            array[i] = nullptr;
        }
    }

    //................Getters & Setters..............//

    size_t getNumAllocatedBlocks() const;

    void AddNumAllocatedBlocks(size_t num);

    size_t getNumAllocatedBytes() const;

    void AddNumAllocatedBytes(size_t num);

    size_t getNumFreeBlocks() const;

    void AddNumFreeBlocks(size_t num);

    size_t getNumFreeBytes() const;

    void AddNumFreeBytes(size_t num);

    //...................Auxiliary Functions.........................//

    //does the main job of allocating a block for the request
    void* allocate(size_t size);

    //Adds a block of in index order to the list of blocks from this specific order, accordingly ( first, mid or last)
    void insertToList(int order, MetaData* mmd);

    // Merges two buddy blocks of the same order into one block then updates the required fields
    void mergeTwoBuddies(int order, MetaData* leftBuddy);

    // Recursively splits one block of a specific order into two blocks until it reaches the required size in argument
    int split(int order, MetaData* block, size_t size);

    //an auxiliary function to split ( helps in updating the fields )
    void splitAUX(MetaData *first,MetaData *second,int order,size_t ASS);

    //Returns the maximum Size (for a merged block ) of available blocks for merge
    size_t maxBuddiesSize(int order, long index);


    // Recursively merges two blocks of a specific order into one blocks until it reaches the required size in argument
    void* merge(MetaData* mmd, size_t size);

    //an auxiliary function to merge ( helps in updating the fields )
    void* mergeAUX(MetaData* mmd, size_t size);

    //finds the smallest power of two that size is bigger than
    static int smallestPowerBiggerEqual(size_t size);


};

//........................Getters & Setters......................//

size_t MemoryManager::getNumAllocatedBlocks() const {
    return numAllocatedBlocks;
}

void MemoryManager::AddNumAllocatedBlocks(size_t num) {
    MemoryManager::numAllocatedBlocks += num;
}

size_t MemoryManager::getNumAllocatedBytes() const {
    return numAllocatedBytes;
}

void MemoryManager::AddNumAllocatedBytes(size_t num) {
    MemoryManager::numAllocatedBytes += num;
}

size_t MemoryManager::getNumFreeBlocks() const {
    return numFreeBlocks;
}

void MemoryManager::AddNumFreeBlocks(size_t num) {
    MemoryManager::numFreeBlocks += num;
}

size_t MemoryManager::getNumFreeBytes() const {
    return numFreeBytes;
}

void MemoryManager::AddNumFreeBytes(size_t num) {
    MemoryManager::numFreeBytes += num;
}

//.........................AuxiliaryFunctions........................//

//finds the smallest power of two that size is bigger than
int MemoryManager::smallestPowerBiggerEqual(size_t size)
{
    size_t val = 1;
    for(int pwr = 0; pwr <= MAX_ORDER; pwr++, val *= 2)
    {
        if(val * MINIMUM_BLOCK_SIZE >= size)
            return pwr;
    }
    return -1;
}

//does the main job of allocating a block for the request
void* MemoryManager::allocate(size_t size)
{
    int minPwr = smallestPowerBiggerEqual(size + sizeof(MetaData));
    MetaData* block = nullptr;
    int i;
    for(i = minPwr; i <= MAX_ORDER; i++)
    {
        if(this->array[i] != nullptr)
        {
            block = this->array[i];
            break;
        }
    }

    if(block == nullptr)
        return nullptr;

    int order = split(i, block, size);
    block->is_free = false;
    if(block->prev != nullptr)
        block->prev->next = block->next;
    else
        this->array[order] = block->next;

    if(block->next != nullptr)
        block->next->prev = block->prev;
    block->next = nullptr;
    block->prev = nullptr;
    this->AddNumFreeBlocks((-1));
    this->AddNumFreeBytes(block->size*(-1));

    return block;
}

//Adds a block of a specific order to the array of
void MemoryManager::insertToList(int order, MetaData* mmd)
{
    mmd->next = nullptr;
    mmd->prev = nullptr;
    if(this->array[order] == nullptr)
    {
        this->array[order] = mmd;
        return;
    }
    MetaData* tmp = this->array[order];
    // Add at the  beginning of the list:
    if(tmp->index > mmd->index)
    {
        tmp->prev = mmd;
        mmd->next = tmp;
        this->array[order] = mmd;
        return;
    }

    // Add in the middle of the list:
    while(tmp->next != nullptr)
    {
        if(tmp->next->index > mmd->index)
            break;
        tmp = tmp->next;
    }

    mmd->next = tmp->next;
    tmp->next = mmd;
    mmd->prev = tmp;

    // Add at the end of list:
    if(mmd->next != nullptr)
        mmd->next->prev = mmd;

}

// Recursively splits one block of a specific order into two blocks until it reaches the required size in argument
int MemoryManager::split(int order, MetaData* block, size_t size)
{
    if(order < 0 || block == nullptr)
        return -1;
    if(order == 0)
        return 0;
    size_t afterSplitSize = (block->size + sizeof(MetaData)) / 2 - sizeof(MetaData);
    if(size > afterSplitSize)
        return order;
    MetaData* first = block;
    MetaData* second = (MetaData*)((char*)block + (sizeof(MetaData) + block->size) / 2);
    if(first->prev != nullptr)
        first->prev->next = first->next;
    else
        this->array[order] = first->next;
    if(first->next != nullptr)
        first->next->prev = first->prev;
    this->splitAUX(first,second,order,afterSplitSize);
    return split(order - 1, first, size);
}

//an auxiliary function to split ( helps in updating the fields )
void MemoryManager::splitAUX( MetaData* first,MetaData* second, int order,size_t ASS){
    first->next = nullptr;
    first->prev = nullptr;
    second->index = first->index * INDEX_BASE + 1;
    first->index = first->index * INDEX_BASE;
    first->size = ASS;
    second->size = ASS;
    second->is_free = true;
    insertToList(order - 1, first);
    insertToList(order - 1, second);
    this->AddNumAllocatedBlocks(1);
    this->AddNumAllocatedBytes(sizeof(MetaData)* (-1));
    this->AddNumFreeBlocks(1);
    this->AddNumFreeBytes(sizeof(MetaData)*(-1));
}

//Returns the maximum Size (for a merged block ) of available blocks for merge
size_t MemoryManager::maxBuddiesSize(int order, long index)
{
    if(order == MAX_ORDER)
        return MAXIMAL_BLOCK_SIZE;
    MetaData* tmp = this->array[order];
    while(tmp != nullptr)
    {
        signed long diff = tmp->index - index;
        if(abs(diff) == 1)
            return maxBuddiesSize(order + 1, index / INDEX_BASE);
        tmp = tmp->next;
    }
    return MINIMUM_BLOCK_SIZE * pow(2, order);
}

//an auxiliary function to merge ( helps in updating the fields )
void* MemoryManager::mergeAUX(MetaData* mmd, size_t size)
{
    int currOrder = smallestPowerBiggerEqual(mmd->size + sizeof(MetaData));
    int targetOrder = smallestPowerBiggerEqual(size + sizeof(MetaData));


    insertToList(currOrder, mmd);
    this->AddNumFreeBlocks(1);
    this->AddNumFreeBytes(mmd->size);

    long index = mmd->index;
    for(int i = currOrder; i < targetOrder; i++)
    {
        // Find the left buddy
        MetaData* leftBuddy = this->array[i];


        signed long diff = leftBuddy->index - index;
        while(diff != -1 && diff != 0) // mmd might be: left buddy => diff = 0 or right buddy => diff = -1
        {
            leftBuddy = leftBuddy->next;
            diff = leftBuddy->index - index;
        }

        mergeTwoBuddies(i, leftBuddy);
        index = index / INDEX_BASE;
    }

    MetaData* tmp = this->array[targetOrder];
    while(tmp->index != index)
    {
        tmp = tmp->next;
    }

    if(tmp->prev != nullptr)
        tmp->prev->next = tmp->next;
    else
        this->array[targetOrder] = tmp->next;

    if(tmp->next != nullptr)
        tmp->next->prev = tmp->prev;
    tmp->next = nullptr;
    tmp->prev = nullptr;

    this->AddNumFreeBytes(-1*tmp->size);
    this->AddNumFreeBlocks(-1);
    return tmp;
}

// Recursively merges two blocks of a specific order into one blocks until it reaches the required size in argument
void* MemoryManager::merge(MetaData* mmd, size_t size)
{
    // Check if merging gets a big enough block
    int order = smallestPowerBiggerEqual(mmd->size + sizeof(MetaData));
    if(maxBuddiesSize(order, mmd->index) < size)
        return nullptr;


    // Merge to a big block
    return mergeAUX(mmd, size);
}

// Merges two buddy blocks of the same order into one block then updates the required fields
void MemoryManager::mergeTwoBuddies(int order, MetaData* leftBuddy)
{
    MetaData* rightBuddy = leftBuddy->next;
    //left
    if(leftBuddy->prev != nullptr)
        leftBuddy->prev->next = leftBuddy->next;
    else
        this->array[order] = leftBuddy->next;

    if(leftBuddy->next != nullptr)
        leftBuddy->next->prev = leftBuddy->prev;
    leftBuddy->next = nullptr;
    leftBuddy->prev = nullptr;

    //right
    if(rightBuddy->prev != nullptr)
        rightBuddy->prev->next = rightBuddy->next;
    else
        this->array[order] = rightBuddy->next;

    if(rightBuddy->next != nullptr)
        rightBuddy->next->prev = rightBuddy->prev;
    rightBuddy->next = nullptr;
    rightBuddy->prev = nullptr;

    leftBuddy->index = leftBuddy->index / INDEX_BASE;
    leftBuddy->size += rightBuddy->size + sizeof(MetaData);
    insertToList(order + 1, leftBuddy);
    this->AddNumAllocatedBlocks(-1);
    this->AddNumAllocatedBytes(sizeof(MetaData));
    this->AddNumFreeBlocks(-1);
    this->AddNumFreeBytes(sizeof(MetaData));
}

// -------------------------* Main Functions *------------------------

MemoryManager MM = MemoryManager();

void* smalloc(size_t size)
{
    if(!MM.initMalloc)
    {
        MM.initMalloc = true;
        void* result = sbrk((int)(NUM_OF_MAX_BLOCKS * MAXIMAL_BLOCK_SIZE));
        if(*(int*)result == -1)
            return nullptr;
        void* tmp = result;
        for(int i = 0; i < NUM_OF_MAX_BLOCKS; i++)
        {
            MetaData* mmd = (MetaData*) tmp;
            mmd->size = MAXIMAL_BLOCK_SIZE - sizeof(MetaData);
            mmd->index = INDEX_OFFSET * i + 1;
            MM.insertToList(MAX_ORDER, mmd);
            tmp = (void*)((char*)tmp + MAXIMAL_BLOCK_SIZE);
        }

        MM.numAllocatedBlocks = NUM_OF_MAX_BLOCKS;
        MM.numAllocatedBytes = NUM_OF_MAX_BLOCKS * (MAXIMAL_BLOCK_SIZE - sizeof(MetaData));
        MM.numFreeBlocks = MM.numAllocatedBlocks;
        MM.numFreeBytes = MM.numAllocatedBytes;
        if(result == nullptr)
            return nullptr;
    }

    if(size == 0 || size > MAX_ALLOCATION_SIZE)
        return nullptr;

    void* res = nullptr;
    if(size > MAXIMAL_BLOCK_SIZE - sizeof(MetaData))
    {
        res = mmap(nullptr, size + sizeof(MetaData), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if(res == MAP_FAILED)
            return nullptr;
        MetaData* mmd = (MetaData*)res;
        mmd->size = size;
        mmd->is_free = false;
        mmd->prev = nullptr;
        mmd->next = nullptr;
        MM.AddNumAllocatedBlocks(1);
        MM.AddNumAllocatedBytes(size);
    }
    else
        res = MM.allocate(size);

    if(res == nullptr)
        return nullptr;

    return (char*)res + sizeof(MetaData);
}

void* scalloc(size_t num, size_t size)
{
    void* res = smalloc(num * size);
    if(res == nullptr)
        return nullptr;
    memset(res, 0, size * num);

    return res;
}

void sfree(void* p)
{
    if(p == nullptr)
        return;
    MetaData* mmd = (MetaData*)((char*)p - sizeof(MetaData));

    if(mmd->is_free)
        return;
    if(mmd->size > MAXIMAL_BLOCK_SIZE - sizeof(MetaData))
    {
        MM.AddNumAllocatedBlocks((-1));
        MM.AddNumAllocatedBytes(mmd->size*(-1));
        munmap((void*)mmd, mmd->size + sizeof(MetaData));
        return;
    }
    MM.AddNumFreeBlocks(1);
    MM.AddNumFreeBytes(mmd->size);
    mmd->is_free = true;
    int order = MM.smallestPowerBiggerEqual(mmd->size + sizeof(MetaData));
    MM.insertToList(order, mmd);
    //mergeTwo
    for(int i = 0; i < ORDER_ARRAY_LENGTH - 1; i++)
    {
        MetaData* tmp = MM.array[i];
        while(tmp != nullptr && tmp->next != nullptr)
        {
            if(tmp->index + 1 == tmp->next->index)
                MM.mergeTwoBuddies(i, tmp);
            tmp = tmp->next;
        }
    }
}

void* srealloc(void* oldp, size_t size)
{
    if(size == 0 || size > MAX_ALLOCATION_SIZE)
        return nullptr;

    if(oldp == nullptr)
        return smalloc(size);

    MetaData* mmd = (MetaData*)((char*)oldp - sizeof(MetaData));
    if(mmd->size > MAXIMAL_BLOCK_SIZE - sizeof(MetaData)){
        if(mmd->size == size)
            return oldp;
        void* res = smalloc(size);
        if(res == nullptr)
            return nullptr;
        size_t copyBytes = mmd->size;
        if(size < mmd->size)
            copyBytes = size;
        memmove(res, oldp, copyBytes);
        sfree(oldp);
        return res;
    }
    if(mmd->size >= size)
        return oldp;
    void* res = MM.merge(mmd, size);
    if(res != nullptr) {
        res = (char *) res + sizeof(MetaData);
        memmove(res, oldp, mmd->size);
        return res;
    }
    res = smalloc(size);
    if(res == nullptr)
        return nullptr;
    memmove(res, oldp, mmd->size);
    sfree(oldp);
    return res;
}




size_t _num_free_blocks(){
    return MM.getNumFreeBlocks();
}

size_t _num_free_bytes(){
    return MM.getNumFreeBytes();
}

size_t _num_allocated_blocks(){
    return MM.getNumAllocatedBlocks();
}

size_t _num_allocated_bytes(){
    return MM.getNumAllocatedBytes();
}

size_t _num_meta_data_bytes(){
    return MM.getNumAllocatedBlocks() * sizeof (MetaData);
}

size_t _size_meta_data(){
    return sizeof(MetaData);
}
