#include "c4/storage/contiguous.hpp"
#include "c4/test.hpp"

C4_BEGIN_NAMESPACE(c4)
C4_BEGIN_NAMESPACE(stg)

TEST(fixed_size, instantiation)
{
    using fstype = fixed_size< int, 4 >;
    fstype fs(aggregate, {0, 1, 2, 3});

    EXPECT_EQ(fs.size(), 4);

    for(int i = 0, e = (int)fs.size(); i < e; ++i)
    {
        EXPECT_EQ(fs[i], i);
    }
}

C4_END_NAMESPACE(stg)
C4_END_NAMESPACE(c4)
