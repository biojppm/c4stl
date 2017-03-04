#include "c4/storage/raw.test.hpp"
#include "c4/storage/raw.hpp"
#include "c4/allocator.hpp"

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

TEST(raw, instantiation)
{
    {
        raw< float > rf;
        EXPECT_EQ(rf.capacity(), 0);
    }

    {
        raw< float > rf(10);
        EXPECT_EQ(rf.capacity(), 10);
    }

    {
        raw< float > rf(16);
        EXPECT_EQ(rf.capacity(), 16);
    }
}

TEST(raw_paged, instantiation)
{
    size_t ps;

    {
        raw_paged< float > rf;
        ps = rf.page_size();
        EXPECT_EQ(rf.capacity(), 0);
        EXPECT_EQ(rf.num_pages(), 0);
    }

    {
        raw_paged< float > rf(1);
        EXPECT_EQ(rf.capacity(), ps);
        EXPECT_EQ(rf.num_pages(), 1);
    }

    {
        raw_paged< float > rf(2 * ps - 1);
        EXPECT_EQ(rf.capacity(), 2 * ps);
        EXPECT_EQ(rf.num_pages(), 2);
    }

    {
        raw_paged< float > rf(2 * ps);
        EXPECT_EQ(rf.capacity(), 2 * ps);
        EXPECT_EQ(rf.num_pages(), 2);
    }

    {
        raw_paged< float > rf(2 * ps + 1);
        EXPECT_EQ(rf.capacity(), 3 * ps);
        EXPECT_EQ(rf.num_pages(), 3);
    }
}

TEST(raw_paged_rt, instantiation)
{
    {
        raw_paged_rt< float > rf;
        EXPECT_EQ(rf.capacity(), 0);
        EXPECT_EQ(rf.num_pages(), 0);
        EXPECT_EQ(rf.page_size(), 256); // the default
    }

    for(size_t ps : {32, 64, 128})
    {
        {
            raw_paged_rt< float > rf(1, ps);
            EXPECT_EQ(rf.capacity(), ps);
            EXPECT_EQ(rf.page_size(), ps);
            EXPECT_EQ(rf.num_pages(), 1);
        }

        {
            raw_paged_rt< float > rf(ps - 1, ps);
            EXPECT_EQ(rf.capacity(), ps);
            EXPECT_EQ(rf.page_size(), ps);
            EXPECT_EQ(rf.num_pages(), 1);
        }

        {
            raw_paged_rt< float > rf(ps, ps);
            EXPECT_EQ(rf.capacity(), ps);
            EXPECT_EQ(rf.page_size(), ps);
            EXPECT_EQ(rf.num_pages(), 1);
        }

        {
            raw_paged_rt< float > rf(ps + 1, ps);
            EXPECT_EQ(rf.capacity(), 2 * ps);
            EXPECT_EQ(rf.page_size(), ps);
            EXPECT_EQ(rf.num_pages(), 2);
        }

        {
            raw_paged_rt< float > rf(2 * ps - 1, ps);
            EXPECT_EQ(rf.capacity(), 2 * ps);
            EXPECT_EQ(rf.page_size(), ps);
            EXPECT_EQ(rf.num_pages(), 2);
        }

        {
            raw_paged_rt< float > rf(2 * ps, ps);
            EXPECT_EQ(rf.capacity(), 2 * ps);
            EXPECT_EQ(rf.page_size(), ps);
            EXPECT_EQ(rf.num_pages(), 2);
        }

        {
            raw_paged_rt< float > rf(2 * ps + 1, ps);
            EXPECT_EQ(rf.capacity(), 3 * ps);
            EXPECT_EQ(rf.page_size(), ps);
            EXPECT_EQ(rf.num_pages(), 3);
        }
    }

}

C4_END_NAMESPACE(stg)
C4_END_NAMESPACE(c4)
