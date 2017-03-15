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
TEST(raw_small, instantiation)
{
    using ci = Counting< int >;
    using rs = raw_small< ci >;

    {
        SCOPED_TRACE("empty");
        AllocationCountsChecker ch;
        {
            auto cd = ci::check_ctors_dtors(0, 0);
            rs rf;
            EXPECT_EQ(rf.capacity(), rs::array_size);
        }
        ch.check_all_delta(0, 0, 0);
    }

    {
        SCOPED_TRACE("small");
        AllocationCountsChecker ch;
        auto num = rs::array_size - 1; // fits in the inplace storage
        {
            auto cd = ci::check_ctors_dtors(0, 0);
            rs rf(num);
            EXPECT_EQ(rf.capacity(), rs::array_size);
        }
        ch.check_all_delta(0, 0, 0);
    }

    {
        SCOPED_TRACE("small-limit");
        AllocationCountsChecker ch;
        auto num = rs::array_size; // fits in the inplace storage
        {
            auto cd = ci::check_ctors_dtors(0, 0);
            rs rf(num);
            EXPECT_EQ(rf.capacity(), rs::array_size);
            ch.check_curr_delta(0, 0);
        }
        ch.check_all_delta(0, 0, 0);
    }

    {
        SCOPED_TRACE("large");
        AllocationCountsChecker ch;
        auto num = rs::array_size + 1; // does not fit in the inplace storage
        {
            SCOPED_TRACE("large-test");
            auto cd = ci::check_ctors_dtors(0, 0);
            rs rf(num);
            EXPECT_EQ(rf.capacity(), num);
            ch.check_curr_delta(1, num * sizeof(ci));
        }
        ch.check_curr_delta(0, 0);
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
template< class T >
struct raw_small_inheriting : public raw_small< T >
{
    using raw_small< T >::raw_small;
};
TEST(raw_small, inheriting)
{
    using ci = Counting< raw_small_inheriting< int > >;
    auto array_size = raw_small< int >::array_size;

    {
        SCOPED_TRACE("small");
        auto cd = ci::check_ctors_dtors(1, 1);
        {
            ci rf(array_size - 1);
            EXPECT_EQ(rf.obj.capacity(), array_size);
        }
    }

    {
        SCOPED_TRACE("large");
        auto cd = ci::check_ctors_dtors(1, 1);
        {
            ci rf(array_size + 1);
            EXPECT_EQ(rf.obj.capacity(), array_size + 1);
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
template< class Raw >
void test_raw_construction(Raw &rp)
{
    using traits = typename Raw::raw_traits;
    using value_type = typename Raw::value_type;

    C4_STATIC_ASSERT(is_instance_of_tpl< Counting C4_COMMA value_type >::value);
    C4_STATIC_ASSERT(std::is_integral< typename value_type::value_type >::value);

    {
        auto cd = value_type::check_ctors_dtors(10, 0);
        {
            traits::construct_n(rp, 0, 10, /*the value*/123);
        }
    }

    {
        auto cd = value_type::check_ctors_dtors(10, 0);
        {
            traits::construct_n(rp, 0, 10, /*the value*/123);
        }
    }

    {
        auto cd = value_type::check_ctors_dtors(10, 0);
        {
            traits::construct_n(rp, 10, 10, /*the value*/123);
        }
    }
}
template< class Raw >
void test_raw_construction_paged(Raw &rp)
{
    using traits = typename Raw::raw_traits;
    using value_type = typename Raw::value_type;
    using T = typename Raw::value_type::value_type;
    using I = typename Raw::size_type;

    test_raw_construction(rp);

    C4_STATIC_ASSERT(std::is_integral< T >::value);
    C4_STATIC_ASSERT(std::is_trivially_destructible< T >::value);

    const auto do_test = [](const char *name, Raw &r_, I first, I num, T val)
    {
        SCOPED_TRACE(name);
        C4_ASSERT(r_.capacity() >= first + num);
        auto cd = value_type::check_ctors_dtors(num, 0);
        {
            for(I p = 0, e = r_.num_pages(); p < e; ++p)
            {
                memset(&r_[p * r_.page_size()], 0, sizeof(T) * r_.page_size());
            }

            traits::construct_n(r_, first, num, /*the value*/val);

            for(I i = 0, e = first; i < e; ++i)
            {
                EXPECT_EQ(r_[i].obj, 0);
            }
            for(I i = first, e = first + num; i < e; ++i)
            {
                EXPECT_EQ(r_[i].obj, val);
            }
            for(I i = first + num, e = r_.capacity(); i < e; ++i)
            {
                EXPECT_EQ(r_[i].obj, 0);
            }
        }
    };

    do_test("case 1.1", rp, 0, rp.page_size(), 123);
    do_test("case 1.2", rp, 0, rp.page_size(), 23);
    do_test("case 1.3", rp, 0, rp.capacity(), 123);
    do_test("case 1.4", rp, 0, rp.capacity(), 23);

    do_test("case 2.1", rp, rp.page_size(), rp.page_size(), 123);
    do_test("case 2.2", rp, rp.page_size(), rp.page_size(), 23);

    do_test("case 3.1", rp, rp.page_size() / 2, rp.page_size(), 123);
    do_test("case 3.2", rp, rp.page_size() / 2, rp.page_size(), 23);
    do_test("case 3.3", rp, rp.page_size() / 2, rp.page_size() / 2, 123);
    do_test("case 3.4", rp, rp.page_size() / 2, rp.page_size() / 2, 23);
    do_test("case 3.5", rp, rp.page_size() / 2 - 1, rp.page_size() / 2, 123);
    do_test("case 3.6", rp, rp.page_size() / 2 + 1, rp.page_size() / 2, 23);

    do_test("case 4.1", rp, rp.page_size() / 3, rp.page_size(), 123);
    do_test("case 4.2", rp, rp.page_size() / 3, rp.page_size(), 23);

    do_test("case 5.1", rp, rp.page_size() - 1, rp.page_size(), 123);
    do_test("case 5.2", rp, rp.page_size() - 1, rp.page_size(), 23);

    do_test("case 6.1", rp, rp.page_size() + 1, rp.page_size(), 123);
    do_test("case 6.2", rp, rp.page_size() + 1, rp.page_size(), 23);
}
TEST(raw_fixed, construction)
{
    using ci = Counting< int >;
    raw_fixed< ci, 1000 > rp;
    test_raw_construction(rp);
}
TEST(raw, construction)
{
    using ci = Counting< int >;
    raw< ci > rp(1000);
    test_raw_construction(rp);
}
TEST(raw_small, construction)
{
    using ci = Counting< int >;
    raw_small< ci > rp(1000);
    test_raw_construction(rp);
}
TEST(raw_paged, construction)
{
    using ci = Counting< int >;
    raw_paged< ci > rp(1000);
    test_raw_construction_paged(rp);
}
TEST(raw_paged_rt, construction)
{
    using ci = Counting< int >;
    raw_paged_rt< ci > rp(1000);
    test_raw_construction_paged(rp);
}

C4_END_NAMESPACE(stg)
C4_END_NAMESPACE(c4)
