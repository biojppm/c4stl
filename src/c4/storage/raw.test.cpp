#include "c4/storage/raw.test.hpp"
#include "c4/storage/raw.hpp"
#include "c4/test.hpp"
#include "c4/allocator.hpp"

C4_BEGIN_NAMESPACE(c4)
C4_BEGIN_NAMESPACE(stg)

//-----------------------------------------------------------------------------
TEST(raw_fixed, instantiation)
{
    using ci = Counting< int >;

    {
        AllocationCountsChecker ch;
        {
            auto cd = ci::check_ctors_dtors(0, 0);
            raw_fixed< ci, 10 > rf;
            EXPECT_EQ(rf.capacity(), 10);
        }
        ch.check_all_delta(0, 0, 0);
    }

    {
        AllocationCountsChecker ch;
        {
            auto cd = ci::check_ctors_dtors(0, 0);
            raw_fixed< ci, 11 > rf;
            EXPECT_EQ(rf.capacity(), 11);
        }
        ch.check_all_delta(0, 0, 0);
    }
}

//-----------------------------------------------------------------------------
TEST(raw, instantiation)
{
    using ci = Counting< int >;

    {
        AllocationCountsChecker ch;
        {
            auto cd = ci::check_ctors_dtors(0, 0);
            raw< ci > rf;
            EXPECT_EQ(rf.capacity(), 0);
        }
        ch.check_all_delta(0, 0, 0);
    }

    {
        AllocationCountsChecker ch;
        {
            auto cd = ci::check_ctors_dtors(0, 0);
            raw< ci > rf(10);
            EXPECT_EQ(rf.capacity(), 10);
        }
        //WIP ch.check_all_delta(1, 10 * sizeof(int), 10 * sizeof(int));
    }

    {
        AllocationCountsChecker ch;
        {
            auto cd = ci::check_ctors_dtors(0, 0);
            raw< ci > rf(16);
            EXPECT_EQ(rf.capacity(), 16);
        }
        //WIP ch.check_all_delta(1, 16 * sizeof(int), 16 * sizeof(int));
    }
}

//-----------------------------------------------------------------------------
template< class T >
struct raw_inheriting : public raw< T >
{
    using raw< T >::raw;
};
TEST(raw, inheriting)
{
    using ci = Counting< raw_inheriting< int > >;
    {
        auto cd = ci::check_ctors_dtors(1, 1);
        {
            ci rf(10);
            EXPECT_EQ(rf.obj.capacity(), 10);
        }
    }
}

//-----------------------------------------------------------------------------
TEST(raw_paged, instantiation)
{
    using ci = Counting< int >;

    size_t ps;

    {
        AllocationCountsChecker ch;
        {
            raw_paged< ci > rf;
            ps = rf.page_size();
            EXPECT_EQ(rf.capacity(), 0);
            EXPECT_EQ(rf.num_pages(), 0);
        }
        //WIP ch.check_all_delta(0, 0, 0);
    }

    {
        raw_paged< ci > rf(1);
        EXPECT_EQ(rf.capacity(), ps);
        EXPECT_EQ(rf.num_pages(), 1);
    }

    {
        raw_paged< ci > rf(2 * ps - 1);
        EXPECT_EQ(rf.capacity(), 2 * ps);
        EXPECT_EQ(rf.num_pages(), 2);
    }

    {
        raw_paged< ci > rf(2 * ps);
        EXPECT_EQ(rf.capacity(), 2 * ps);
        EXPECT_EQ(rf.num_pages(), 2);
    }

    {
        raw_paged< ci > rf(2 * ps + 1);
        EXPECT_EQ(rf.capacity(), 3 * ps);
        EXPECT_EQ(rf.num_pages(), 3);
    }
}

//-----------------------------------------------------------------------------
TEST(raw_paged_rt, instantiation)
{
    using ci = Counting< int >;

    {
        raw_paged_rt< ci > rf;
        EXPECT_EQ(rf.capacity(), 0);
        EXPECT_EQ(rf.num_pages(), 0);
        EXPECT_EQ(rf.page_size(), 256); // the default
    }

    for(size_t ps : {32, 64, 128})
    {
        {
            raw_paged_rt< ci > rf(1, ps);
            EXPECT_EQ(rf.capacity(), ps);
            EXPECT_EQ(rf.page_size(), ps);
            EXPECT_EQ(rf.num_pages(), 1);
        }

        {
            raw_paged_rt< ci > rf(ps - 1, ps);
            EXPECT_EQ(rf.capacity(), ps);
            EXPECT_EQ(rf.page_size(), ps);
            EXPECT_EQ(rf.num_pages(), 1);
        }

        {
            raw_paged_rt< ci > rf(ps, ps);
            EXPECT_EQ(rf.capacity(), ps);
            EXPECT_EQ(rf.page_size(), ps);
            EXPECT_EQ(rf.num_pages(), 1);
        }

        {
            raw_paged_rt< ci > rf(ps + 1, ps);
            EXPECT_EQ(rf.capacity(), 2 * ps);
            EXPECT_EQ(rf.page_size(), ps);
            EXPECT_EQ(rf.num_pages(), 2);
        }

        {
            raw_paged_rt< ci > rf(2 * ps - 1, ps);
            EXPECT_EQ(rf.capacity(), 2 * ps);
            EXPECT_EQ(rf.page_size(), ps);
            EXPECT_EQ(rf.num_pages(), 2);
        }

        {
            raw_paged_rt< ci > rf(2 * ps, ps);
            EXPECT_EQ(rf.capacity(), 2 * ps);
            EXPECT_EQ(rf.page_size(), ps);
            EXPECT_EQ(rf.num_pages(), 2);
        }

        {
            raw_paged_rt< ci > rf(2 * ps + 1, ps);
            EXPECT_EQ(rf.capacity(), 3 * ps);
            EXPECT_EQ(rf.page_size(), ps);
            EXPECT_EQ(rf.num_pages(), 3);
        }
    }

}

//-----------------------------------------------------------------------------
template< class T >
struct raw_inheriting_paged : public raw_paged< T >
{
    using raw_paged< T >::raw_paged;
};
TEST(raw_paged, inheriting)
{
    using ci = Counting< raw_inheriting_paged< int > >;
    {
        auto cd = ci::check_ctors_dtors(1, 1);
        {
            ci rf(10);
            EXPECT_EQ(rf.obj.capacity(), 256);
        }
    }
}

//-----------------------------------------------------------------------------
template< class T >
struct raw_inheriting_paged_rt : public raw_paged_rt< T >
{
    using raw_paged_rt< T >::raw_paged_rt;
};
TEST(raw_paged_rt, inheriting)
{
    using ci = Counting< raw_inheriting_paged_rt< int > >;
    {
        auto cd = ci::check_ctors_dtors(1, 1);
        {
            ci rf(10);
            EXPECT_EQ(rf.obj.capacity(), 256);
        }
    }
}

C4_END_NAMESPACE(stg)
C4_END_NAMESPACE(c4)
