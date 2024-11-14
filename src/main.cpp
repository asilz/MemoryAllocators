#include <MemoryAllocator.hpp>
#include <assert.h>

int main(void)
{
    PoolAllocator pool = PoolAllocator<TreeNode, 2>();
    TreeNode *node = pool.alloc(3, 4);
    assert(node);
    TreeNode *node2 = pool.alloc(7, 8);
    assert(node2);

    TreeNode *node3 = pool.alloc(9, 10);
    assert(node3 == nullptr);

    pool.free(node);
    TreeNode *node4 = pool.alloc(11, 12);
    assert(node4);
    return 0;
}