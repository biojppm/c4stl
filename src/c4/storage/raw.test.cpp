#include "c4/storage/raw.test.hpp"
#include "c4/storage/raw.hpp"

C4_BEGIN_NAMESPACE(c4)
C4_BEGIN_NAMESPACE(stg)

TEST(raw_fixed, instantiation)
{
    {
        raw_fixed< float, 10 > rf;
        EXPECT_EQ(rf.capacity(), 10);
    }

    {
        raw_fixed< float, 11 > rf;
        EXPECT_EQ(rf.capacity(), 11);
    }
}

C4_END_NAMESPACE(stg)
C4_END_NAMESPACE(c4)
