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

//-----------------------------------------------------------------------------
template< class RawPagedContainer >
void test_raw_page_addressing(RawPagedContainer const& rp)
{
    using I = typename RawPagedContainer::size_type;

    I ps = rp.page_size();
    I np = rp.num_pages();

    I elm = 0;
    for(I i = 0; i < np; ++i)
    {
        for(I j = 0; j < ps; ++j)
        {
            EXPECT_EQ(&rp[elm], &rp.m_pages[i][j])
                    << "  i=" << i << " j=" << j << " e=" << elm
                    << "  np=" << np << "  ps=" << ps;
            elm++;
        }
    }
}

TEST(raw_paged, addressing)
{
    C4_SIZE_TYPE sz = 1000;

    {
        SCOPED_TRACE("page size==default");
        using rptype = raw_paged< int >;
        rptype rp(sz);
        test_raw_page_addressing(rp);
    }

    {
        SCOPED_TRACE("page size==512");
        using rptype = raw_paged< int, 512 >;
        EXPECT_EQ(rptype::page_size(), 512);
        rptype rp(sz);
        test_raw_page_addressing(rp);
    }

    {
        SCOPED_TRACE("page size==32");
        using rptype = raw_paged< int, 32 >;
        EXPECT_EQ(rptype::page_size(), 32);
        rptype rp(sz);
        test_raw_page_addressing(rp);
    }

    {
        SCOPED_TRACE("page size==4");
        using rptype = raw_paged< int, 4 >;
        EXPECT_EQ(rptype::page_size(), 4);
        rptype rp(sz);
        test_raw_page_addressing(rp);
    }

    {
        SCOPED_TRACE("page size==2");
        using rptype = raw_paged< int, 2 >;
        EXPECT_EQ(rptype::page_size(), 2);
        rptype rp(sz);
        test_raw_page_addressing(rp);
    }
}


TEST(raw_paged_rt, addressing)
{
    C4_SIZE_TYPE sz = 1000;
    using rptype = raw_paged_rt< int >;

    {
        SCOPED_TRACE("page size==default");
        rptype rp(sz);
        test_raw_page_addressing(rp);
    }

    {
        SCOPED_TRACE("page size==512");
        rptype rp(sz, 512);
        EXPECT_EQ(rp.page_size(), 512);
        test_raw_page_addressing(rp);
    }

    {
        SCOPED_TRACE("page size==32");
        rptype rp(sz, 32);
        EXPECT_EQ(rp.page_size(), 32);
        test_raw_page_addressing(rp);
    }

    {
        SCOPED_TRACE("page size==4");
        rptype rp(sz, 4);
        EXPECT_EQ(rp.page_size(), 4);
        test_raw_page_addressing(rp);
    }

    {
        SCOPED_TRACE("page size==2");
        rptype rp(sz, 2);
        EXPECT_EQ(rp.page_size(), 2);
        test_raw_page_addressing(rp);
    }
}

//-----------------------------------------------------------------------------
template< class Raw, class... CtorArgs >
void test_raw_construction(CtorArgs&& ...args)
{
    using traits = typename Raw::raw_traits;
    using value_type = typename Raw::value_type;

    {
        auto cd = value_type::check_ctors_dtors(0, 0);
        {
            Raw rp(std::forward< CtorArgs >(args)...);
            traits::construct_n(rp, 0, 10);
        }
    }

    {
        auto cd = value_type::check_ctors_dtors(10, 0);
        {
            Raw rp(std::forward< CtorArgs >(args)...);
            traits::construct_n(rp, 0, 10);
        }
    }
}
TEST(raw_fixed, construction)
{
    using ci = Counting< int >;
    test_raw_construction< raw_fixed< ci, 1000 > >();
}
TEST(raw, construction)
{
    using ci = Counting< int >;
    test_raw_construction< raw< ci > >(1000);
}
TEST(raw_paged, construction)
{
    using ci = Counting< int >;
    test_raw_construction< raw_paged< ci > >(1000);
}
TEST(raw_paged_rt, construction)
{
    using ci = Counting< int >;
    test_raw_construction< raw_paged_rt< ci > >(1000);
}

C4_END_NAMESPACE(stg)
C4_END_NAMESPACE(c4)
