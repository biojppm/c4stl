#include "c4/storage/growth.hpp"
#include "c4/test.hpp"

C4_BEGIN_NAMESPACE(c4)
C4_BEGIN_NAMESPACE(stg)

TEST(growth, least)
{
    EXPECT_EQ(growth_least   ::next_size(1,  0,  0),   0);
    EXPECT_EQ(growth_least_wm::next_size(1,  0,  0),   0);

    EXPECT_EQ(growth_least   ::next_size(1,  0,  1),   1);
    EXPECT_EQ(growth_least_wm::next_size(1,  0,  1),   1);

    EXPECT_EQ(growth_least   ::next_size(1,  0, 10),  10);
    EXPECT_EQ(growth_least_wm::next_size(1,  0, 10),  10);

    EXPECT_EQ(growth_least   ::next_size(1, 10,  0),   0);
    EXPECT_EQ(growth_least_wm::next_size(1, 10,  0),  10);

    EXPECT_EQ(growth_least   ::next_size(1, 10,  1),   1);
    EXPECT_EQ(growth_least_wm::next_size(1, 10,  1),  10);

    EXPECT_EQ(growth_least   ::next_size(1, 10, 10),  10);
    EXPECT_EQ(growth_least_wm::next_size(1, 10, 10),  10);
}

C4_END_NAMESPACE(stg)
C4_END_NAMESPACE(c4)
