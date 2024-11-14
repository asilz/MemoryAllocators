#include <cstddef>
#include <bitset>
#include <cerrno>
#include <new>

struct TreeNode
{
    uint64_t a;
    uint32_t b;

    TreeNode(uint64_t a_, uint32_t b_) : a(a_), b(b_) { printf("Construct!\n"); };
    ~TreeNode() { printf("deleted!\n"); };
};

template <class T, size_t elementCap>
class PoolAllocator
{
private:
    std::bitset<elementCap> allocMap;
    std::byte buf[elementCap * sizeof(T)];

public:
    template <typename... Args>
    T *alloc(Args &&...args);
    void free(T *ptr);
    PoolAllocator();
    ~PoolAllocator();
};

template <size_t elementCap>
ssize_t getFirstUnset(std::bitset<elementCap> &set)
{
    for (size_t i = 0; i < elementCap; ++i)
    {
        if (set[i] == false)
        {
            return i;
        }
    }
    return -ENOMEM;
}

template <class T, size_t elementCap>
template <typename... Args>
T *PoolAllocator<T, elementCap>::alloc(Args &&...args)
{
    ssize_t index = getFirstUnset(allocMap);
    if (index < 0)
    {
        return nullptr;
    }
    allocMap[index] = true;
    T *obj = new (&buf[index * sizeof(T)]) T(args...);
    return obj;
}

template <class T, size_t elementCap>
void PoolAllocator<T, elementCap>::free(T *ptr)
{
    uintptr_t index = ((uintptr_t)(ptr - (T *)buf)) / sizeof(T);
    allocMap[index] = false;
    ptr->~T();
}

template <class T, size_t elementCap>
PoolAllocator<T, elementCap>::PoolAllocator()
{
    allocMap.reset();
}

template <class T, size_t elementCap>
PoolAllocator<T, elementCap>::~PoolAllocator()
{
    for (size_t i = 0; i < elementCap; ++i)
    {
        if (allocMap[i])
        {
            T *obj = reinterpret_cast<T *>(&buf[i * sizeof(T)]);
            obj->~T();
        }
    }
}
