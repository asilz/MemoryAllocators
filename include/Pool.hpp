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

template <class T, size_t capacity>
class PoolAllocator
{
private:
    std::bitset<capacity> allocMap;
    std::byte buf[capacity * sizeof(T)];

public:
    template <typename... Args>
    T *alloc(Args &&...args);
    void free(T *ptr);
    PoolAllocator();
    ~PoolAllocator();
};

template <size_t bitCount>
constexpr ssize_t getFirstUnset(const std::bitset<bitCount> &set)
{
    for (size_t i = 0; i < bitCount; ++i)
    {
        if (set[i] == false)
        {
            return i;
        }
    }
    return -ENOMEM;
}

template <class T, size_t capacity>
template <typename... Args>
T *PoolAllocator<T, capacity>::alloc(Args &&...args)
{
    ssize_t index = getFirstUnset(allocMap);
    if (index < 0)
    {
        return nullptr;
    }
    allocMap[index] = true;
    T *obj = new (&buf[index]) T(std::forward<Args>(args)...);
    return obj;
}

template <class T, size_t capacity>
void PoolAllocator<T, capacity>::free(T *ptr)
{
    uintptr_t index = ((uintptr_t)(ptr - reinterpret_cast<T *>(buf))) / sizeof(T);
    allocMap[index] = false;
    ptr->~T();
}

template <class T, size_t capacity>
PoolAllocator<T, capacity>::PoolAllocator() : allocMap(std::bitset<capacity>())
{
}

template <class T, size_t capacity>
PoolAllocator<T, capacity>::~PoolAllocator()
{
    for (size_t i = 0; i < capacity; ++i)
    {
        if (allocMap[i])
        {
            T *obj = reinterpret_cast<T *>(&buf[i]);
            obj->~T();
        }
    }
}
