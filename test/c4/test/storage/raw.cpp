#include "c4/storage/raw.hpp"
#include "c4/allocator.hpp"

#include "c4/libtest/supprwarn_push.hpp"
#include "c4/test/storage/raw.hpp"
#include "c4/test.hpp"

#include <random>
#include <memory>

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

    {
        raw_fixed< int, 10 > rf(0);
        EXPECT_EQ(rf.capacity(), 10);
    }
    {
        raw_fixed< int, 10 > rf(5);
        EXPECT_EQ(rf.capacity(), 10);
    }
    {
        raw_fixed< int, 10 > rf(10);
        EXPECT_EQ(rf.capacity(), 10);
    }
    {
        C4_EXPECT_ASSERT_TRIGGERS(1);
        {
            raw_fixed< int, 10 > rf(15);
            EXPECT_EQ(rf.capacity(), 10);
        }
    }
}

//-----------------------------------------------------------------------------
TEST(raw_fixed_soa, instantiation)
{
    using ci = Counting< int >;
    using cf = Counting< float >;
    using ty = soa< ci, cf >;

    {
        AllocationCountsChecker ch;
        {
            auto cdi = ci::check_ctors_dtors(0, 0);
            auto cdf = cf::check_ctors_dtors(0, 0);
            raw_fixed_soa< ty, 10 > rf;
            EXPECT_EQ(rf.capacity(), 10);
        }
        ch.check_all_delta(0, 0, 0);
    }

    {
        AllocationCountsChecker ch;
        {
            auto cdi = ci::check_ctors_dtors(0, 0);
            auto cdf = cf::check_ctors_dtors(0, 0);
            raw_fixed_soa< ty, 11 > rf;
            EXPECT_EQ(rf.capacity(), 11);
        }
        ch.check_all_delta(0, 0, 0);
    }
    {
        raw_fixed_soa< soa<int, float>, 10 > rf(0);
        EXPECT_EQ(rf.capacity(), 10);
    }
    {
        raw_fixed_soa< soa<int, float>, 10 > rf(5);
        EXPECT_EQ(rf.capacity(), 10);
    }
    {
        raw_fixed_soa< soa<int, float>, 10 > rf(10);
        EXPECT_EQ(rf.capacity(), 10);
    }
    {
        C4_EXPECT_ASSERT_TRIGGERS(1);
        {
            raw_fixed_soa< soa<int, float>, 10 > rf(15);
            EXPECT_EQ(rf.capacity(), 10);
        }
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
    {
        raw_small< int, size_t, 10 > rf(0);
        EXPECT_EQ(rf.capacity(), 10);
    }
    {
        raw_small< int, size_t, 10 > rf(5);
        EXPECT_EQ(rf.capacity(), 10);
    }
    {
        raw_small< int, size_t, 10 > rf(10);
        EXPECT_EQ(rf.capacity(), 10);
    }
    {
        raw_small< int, size_t, 10 > rf(15);
        EXPECT_EQ(rf.capacity(), 15);
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

//-----------------------------------------------------------------------------
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
        using rptype = raw_paged< int, size_t, 512 >;
        EXPECT_EQ(rptype::page_size(), 512);
        rptype rp(sz);
        test_raw_page_addressing(rp);
    }

    {
        SCOPED_TRACE("page size==32");
        using rptype = raw_paged< int, size_t, 32 >;
        EXPECT_EQ(rptype::page_size(), 32);
        rptype rp(sz);
        test_raw_page_addressing(rp);
    }

    {
        SCOPED_TRACE("page size==4");
        using rptype = raw_paged< int, size_t, 4 >;
        EXPECT_EQ(rptype::page_size(), 4);
        rptype rp(sz);
        test_raw_page_addressing(rp);
    }

    {
        SCOPED_TRACE("page size==2");
        using rptype = raw_paged< int, size_t, 2 >;
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
    using traits = typename Raw::storage_traits;
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
void test_raw_soa_construction(Raw &rp)
{
    C4_STATIC_ASSERT(Raw::num_arrays == 2);

    using traits = typename Raw::storage_traits;
    using value_type0 = typename Raw::template value_type<0>;
    using value_type1 = typename Raw::template value_type<1>;

    C4_STATIC_ASSERT(is_instance_of_tpl< Counting C4_COMMA value_type0 >::value);
    C4_STATIC_ASSERT(is_instance_of_tpl< Counting C4_COMMA value_type1 >::value);
    C4_STATIC_ASSERT(std::is_integral< typename value_type0::value_type >::value);
    C4_STATIC_ASSERT(std::is_integral< typename value_type1::value_type >::value);

    {
        auto c0 = value_type0::check_ctors_dtors(10, 0);
        auto c1 = value_type1::check_ctors_dtors(10, 0);
        {
            traits::construct_n(rp, 0, 10, /*the value*/123);
        }
    }

    {
        auto c0 = value_type0::check_ctors_dtors(10, 0);
        auto c1 = value_type1::check_ctors_dtors(10, 0);
        {
            traits::construct_n(rp, 0, 10, /*the value*/123);
        }
    }

    {
        auto c0 = value_type0::check_ctors_dtors(10, 0);
        auto c1 = value_type1::check_ctors_dtors(10, 0);
        {
            traits::construct_n(rp, 10, 10, /*the value*/123);
        }
    }
}
template< class Raw >
void test_raw_construction_paged(Raw &rp)
{
    using traits = typename Raw::storage_traits;
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
TEST(raw_fixed_soa, construction)
{
    using ci = Counting< int >;
    using ch = Counting< uint16_t >;
    raw_fixed_soa< soa<ci,ch>, 1000 > rp;
    test_raw_soa_construction(rp);
}
TEST(raw, construction)
{
    using ci = Counting< int >;
    raw< ci > rp(1000);
    test_raw_construction(rp);
}
TEST(raw_soa, construction)
{
    using ci = Counting< int >;
    using ch = Counting< uint16_t >;
    raw_soa< soa<ci,ch> > rp(1000);
    test_raw_soa_construction(rp);
}
TEST(raw_small, construction)
{
    using ci = Counting< int >;
    raw_small< ci > rp(1000);
    test_raw_construction(rp);
}
TEST(raw_small_soa, construction)
{
    using ci = Counting< int >;
    using ch = Counting< uint16_t >;
    raw_small_soa< soa<ci,ch> > rp(1000);
    test_raw_soa_construction(rp);
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


//-----------------------------------------------------------------------------
template< class RawPaged >
void test_page_slack(RawPaged const& rp)
{
    C4_ASSERT(rp.page_size() == 8);
    EXPECT_EQ(rp._page_slack(/*pg*/0, /*sz*/0), 8);
    EXPECT_EQ(rp._page_slack(/*pg*/0, /*sz*/1), 7);
    EXPECT_EQ(rp._page_slack(/*pg*/0, /*sz*/2), 6);
    EXPECT_EQ(rp._page_slack(/*pg*/0, /*sz*/3), 5);
    EXPECT_EQ(rp._page_slack(/*pg*/0, /*sz*/4), 4);
    EXPECT_EQ(rp._page_slack(/*pg*/0, /*sz*/5), 3);
    EXPECT_EQ(rp._page_slack(/*pg*/0, /*sz*/6), 2);
    EXPECT_EQ(rp._page_slack(/*pg*/0, /*sz*/7), 1);
    EXPECT_EQ(rp._page_slack(/*pg*/0, /*sz*/8), 0);

    EXPECT_EQ(rp._page_slack(/*pg*/1, /*sz*/0), 8);
    EXPECT_EQ(rp._page_slack(/*pg*/1, /*sz*/1), 8);
    EXPECT_EQ(rp._page_slack(/*pg*/1, /*sz*/2), 8);
    EXPECT_EQ(rp._page_slack(/*pg*/1, /*sz*/3), 8);
    EXPECT_EQ(rp._page_slack(/*pg*/1, /*sz*/4), 8);
    EXPECT_EQ(rp._page_slack(/*pg*/1, /*sz*/5), 8);
    EXPECT_EQ(rp._page_slack(/*pg*/1, /*sz*/6), 8);
    EXPECT_EQ(rp._page_slack(/*pg*/1, /*sz*/7), 8);
    EXPECT_EQ(rp._page_slack(/*pg*/1, /*sz*/8), 8);

    EXPECT_EQ(rp._page_slack(/*pg*/1, /*sz*/8+0), 8);
    EXPECT_EQ(rp._page_slack(/*pg*/1, /*sz*/8+1), 7);
    EXPECT_EQ(rp._page_slack(/*pg*/1, /*sz*/8+2), 6);
    EXPECT_EQ(rp._page_slack(/*pg*/1, /*sz*/8+3), 5);
    EXPECT_EQ(rp._page_slack(/*pg*/1, /*sz*/8+4), 4);
    EXPECT_EQ(rp._page_slack(/*pg*/1, /*sz*/8+5), 3);
    EXPECT_EQ(rp._page_slack(/*pg*/1, /*sz*/8+6), 2);
    EXPECT_EQ(rp._page_slack(/*pg*/1, /*sz*/8+7), 1);
    EXPECT_EQ(rp._page_slack(/*pg*/1, /*sz*/8+8), 0);
}
TEST(raw_paged, page_slack)
{
    raw_paged< int, int, 8 > rp;
    test_page_slack(rp);
}
TEST(raw_paged_rt, page_slack)
{
    raw_paged_rt< int, int > rp(0, 8);
    test_page_slack(rp);
}


//-----------------------------------------------------------------------------
template< class Raw >
void test_raw_data()
{
    Raw r(10);
    EXPECT_EQ(r.template data<0>(), r.data());
    EXPECT_NE(r.data(), nullptr);
}

template< class RawStorage >
void test_raw_resize()
{
    RawStorage r(10);
    EXPECT_GE(r.capacity(), 10);
#define i(pos) r[pos]
    r._raw_make_room(0, 0, 3);
    i(0) = 0;
    i(1) = 1;
    i(2) = 2;
    r._raw_make_room(0, 3, 3);
    i(0) = 20;
    i(1) = 21;
    i(2) = 22;
    EXPECT_EQ(i(0), 20);
    EXPECT_EQ(i(1), 21);
    EXPECT_EQ(i(2), 22);
    EXPECT_EQ(i(3), 0);
    EXPECT_EQ(i(4), 1);
    EXPECT_EQ(i(5), 2);
    r._raw_make_room(3, 6, 3);
    i(3) = 3;
    i(4) = 4;
    i(5) = 5;
    EXPECT_EQ(i(0), 20);
    EXPECT_EQ(i(1), 21);
    EXPECT_EQ(i(2), 22);
    EXPECT_EQ(i(3), 3);
    EXPECT_EQ(i(4), 4);
    EXPECT_EQ(i(5), 5);
    EXPECT_EQ(i(6), 0);
    EXPECT_EQ(i(7), 1);
    EXPECT_EQ(i(8), 2);
    r._raw_destroy_room(3, 9, 3);
    EXPECT_EQ(i(0), 3);
    EXPECT_EQ(i(1), 4);
    EXPECT_EQ(i(2), 5);
    EXPECT_EQ(i(3), 0);
    EXPECT_EQ(i(4), 1);
    EXPECT_EQ(i(5), 2);
#undef i
}

template< class RawStorage >
void test_raw_soa_resize_1_type()
{
    C4_STATIC_ASSERT(RawStorage::num_arrays == 1);
    RawStorage r(10);
    EXPECT_GE(r.capacity(), 10);
#define i r.template get<0>
    r._raw_make_room(0, 0, 3);
    i(0) = 0;
    i(1) = 1;
    i(2) = 2;
    r._raw_make_room(0, 3, 3);
    i(0) = 20;
    i(1) = 21;
    i(2) = 22;
    EXPECT_EQ(i(0), 20);
    EXPECT_EQ(i(1), 21);
    EXPECT_EQ(i(2), 22);
    EXPECT_EQ(i(3), 0);
    EXPECT_EQ(i(4), 1);
    EXPECT_EQ(i(5), 2);
    r._raw_make_room(3, 6, 3);
    i(3) = 3;
    i(4) = 4;
    i(5) = 5;
    EXPECT_EQ(i(0), 20);
    EXPECT_EQ(i(1), 21);
    EXPECT_EQ(i(2), 22);
    EXPECT_EQ(i(3), 3);
    EXPECT_EQ(i(4), 4);
    EXPECT_EQ(i(5), 5);
    EXPECT_EQ(i(6), 0);
    EXPECT_EQ(i(7), 1);
    EXPECT_EQ(i(8), 2);
    r._raw_destroy_room(3, 9, 3);
    EXPECT_EQ(i(0), 3);
    EXPECT_EQ(i(1), 4);
    EXPECT_EQ(i(2), 5);
    EXPECT_EQ(i(3), 0);
    EXPECT_EQ(i(4), 1);
    EXPECT_EQ(i(5), 2);
#undef i
}

template< class RawStorage >
void test_raw_soa_resize_2_types()
{
    C4_STATIC_ASSERT(RawStorage::num_arrays == 2);
    static_assert(std::is_same< typename RawStorage::template value_type<0>, int >::value, "must be int");
    RawStorage r(10);
    EXPECT_GE(r.capacity(), 10);
    EXPECT_EQ(r.template data<0>(), r.data());
    EXPECT_NE(r.data(), nullptr);
#define i r.template get<0>
#define j r.template get<1>
    r._raw_make_room(0, 0, 3);
    i(0) = 0; j(0) = 10.f;
    i(1) = 1; j(1) = 11.f;
    i(2) = 2; j(2) = 12.f;
    r._raw_make_room(0, 3, 3);
    i(0) = 20; j(0) = 210.f;
    i(1) = 21; j(1) = 211.f;
    i(2) = 22; j(2) = 212.f;
    EXPECT_EQ(i(0), 20); EXPECT_FLOAT_EQ(j(0), 210.f);
    EXPECT_EQ(i(1), 21); EXPECT_FLOAT_EQ(j(1), 211.f);
    EXPECT_EQ(i(2), 22); EXPECT_FLOAT_EQ(j(2), 212.f);
    EXPECT_EQ(i(3), 0);  EXPECT_FLOAT_EQ(j(3), 10.f);
    EXPECT_EQ(i(4), 1);  EXPECT_FLOAT_EQ(j(4), 11.f);
    EXPECT_EQ(i(5), 2);  EXPECT_FLOAT_EQ(j(5), 12.f);
    r._raw_make_room(3, 6, 3);
    i(3) = 3; j(3) = 13.f;
    i(4) = 4; j(4) = 14.f;
    i(5) = 5; j(5) = 15.f;
    EXPECT_EQ(i(0), 20); EXPECT_FLOAT_EQ(j(0), 210.f);
    EXPECT_EQ(i(1), 21); EXPECT_FLOAT_EQ(j(1), 211.f);
    EXPECT_EQ(i(2), 22); EXPECT_FLOAT_EQ(j(2), 212.f);
    EXPECT_EQ(i(3), 3);  EXPECT_FLOAT_EQ(j(3), 13.f);
    EXPECT_EQ(i(4), 4);  EXPECT_FLOAT_EQ(j(4), 14.f);
    EXPECT_EQ(i(5), 5);  EXPECT_FLOAT_EQ(j(5), 15.f);
    EXPECT_EQ(i(6), 0);  EXPECT_FLOAT_EQ(j(6), 10.f);
    EXPECT_EQ(i(7), 1);  EXPECT_FLOAT_EQ(j(7), 11.f);
    EXPECT_EQ(i(8), 2);  EXPECT_FLOAT_EQ(j(8), 12.f);
    r._raw_destroy_room(3, 9, 3);
    EXPECT_EQ(i(0), 3); EXPECT_FLOAT_EQ(j(0), 13.f);
    EXPECT_EQ(i(1), 4); EXPECT_FLOAT_EQ(j(1), 14.f);
    EXPECT_EQ(i(2), 5); EXPECT_FLOAT_EQ(j(2), 15.f);
    EXPECT_EQ(i(3), 0); EXPECT_FLOAT_EQ(j(3), 10.f);
    EXPECT_EQ(i(4), 1); EXPECT_FLOAT_EQ(j(4), 11.f);
    EXPECT_EQ(i(5), 2); EXPECT_FLOAT_EQ(j(5), 12.f);
    r._raw_destroy_room(3, 6, 3);
    EXPECT_EQ(i(0), 0); EXPECT_FLOAT_EQ(j(3), 10.f);
    EXPECT_EQ(i(1), 1); EXPECT_FLOAT_EQ(j(4), 11.f);
    EXPECT_EQ(i(2), 2); EXPECT_FLOAT_EQ(j(5), 12.f);
    r._raw_destroy_room(3, 3, 3);
    EXPECT_EQ(i(0), 0); EXPECT_FLOAT_EQ(j(3), 10.f);
    EXPECT_EQ(i(1), 1); EXPECT_FLOAT_EQ(j(4), 11.f);
    EXPECT_EQ(i(2), 2); EXPECT_FLOAT_EQ(j(5), 12.f);
#undef i
#undef j
}

TEST(raw_fixed, resize)
{
    test_raw_resize< raw_fixed< int, 10 > >();
}
TEST(raw_fixed_soa, soa_resize_1_bare)
{
    test_raw_soa_resize_1_type< raw_fixed_soa< int, 10 > >();
}
TEST(raw_fixed_soa, soa_resize_1_soa)
{
    test_raw_soa_resize_1_type< raw_fixed_soa< soa<int>, 10 > >();
}
TEST(raw_fixed_soa, soa_resize_2)
{
    test_raw_soa_resize_2_types< raw_fixed_soa< soa<int,float>, 10 > >();
}

TEST(raw, resize)
{
    test_raw_resize< raw< int > >();
}
TEST(raw_soa, soa_resize_1_bare)
{
    test_raw_soa_resize_1_type< raw_soa< int > >();
}
TEST(raw_soa, soa_resize_1_soa)
{
    test_raw_soa_resize_1_type< raw_soa< soa<int> > >();
}
TEST(raw_soa, soa_resize_2)
{
    test_raw_soa_resize_2_types< raw_soa< soa<int,float> > >();
}


TEST(raw_small, resize)
{
    test_raw_resize< raw_small< int > >();
}
TEST(raw_small_soa, soa_resize_1_bare)
{
    test_raw_soa_resize_1_type< raw_small_soa< int,size_t,10 > >();
    test_raw_soa_resize_1_type< raw_small_soa< int,size_t,4  > >();
}
TEST(raw_small_soa, soa_resize_1_soa)
{
    test_raw_soa_resize_1_type< raw_small_soa< soa<int>,size_t,10 > >();
    test_raw_soa_resize_1_type< raw_small_soa< soa<int>,size_t,4 > >();
}
TEST(raw_small_soa, soa_resize_2)
{
    test_raw_soa_resize_2_types< raw_small_soa< soa<int,float>,size_t,10 > >();
    test_raw_soa_resize_2_types< raw_small_soa< soa<int,float>,size_t,4 > >();
}



template< class PagedStorage >
struct PagedStorageInsertionTester
{
    using T = typename PagedStorage::value_type;
    using I = typename PagedStorage::size_type;

    std::unique_ptr< PagedStorage > s;
    std::vector< T > checker;
    std::uniform_int_distribution< I > dist;
    std::mt19937 engine;
    std::vector< T > tmp;
    int num_adds;

    template< class... Args >
    static PagedStorageInsertionTester create(Args&&... args)
    {
        PagedStorageInsertionTester pst;
        pst.s.reset(new PagedStorage(std::forward< Args >(args)...));
        pst.dist = std::uniform_int_distribution<I>{0, 200};
        pst.checker.reserve(6 * pst.s->page_size());
        pst.num_adds = 0;
        return pst;
    }

    void clear()
    {
        checker.clear();
        s->_raw_reserve(0);
    }

    bool full_capacity() const
    {
        return checker.size() == s->capacity();
    }

    bool add_and_test(I pos, I num)
    {
        I currsz = szconv<I>(checker.size()); auto *ptr = s.get();
        ptr->_raw_make_room(pos, currsz, num);
        tmp.resize(num);
        for(I i = 0; i < num; ++i)
        {
            //tmp[i] = num_adds;
            tmp[i] = num_adds*1000 + dist(engine);
            (*ptr)[pos + i] = tmp[i];
        }
        auto cpos = checker.cbegin() + pos;
        checker.insert(cpos, tmp.begin(), tmp.end());
        C4_ASSERT(s->capacity() >= checker.size());
        bool ok = true;
        for(I i = 0; i < checker.size(); ++i)
        {
            EXPECT_EQ((*s)[i], checker[i]) << "i=" << i;
            ok &= ((*s)[i] == checker[i]);
        }
        num_adds++;
        return ok;
    }
};


template< class PagedStorage, class... Args >
void test_paged_resize(Args... args)
{
    PagedStorage r(args...);

    typename PagedStorage::size_type sz = 0;
    typename PagedStorage::size_type ps = r.page_size();

    EXPECT_EQ(r.num_pages(), 0);
    EXPECT_EQ(r.capacity(), 0);

    r._raw_reserve(ps);
    sz += ps;
    EXPECT_EQ(r.num_pages(), 1);
    EXPECT_EQ(r.capacity(), sz);
    for(int i = 0; i < ps; ++i)
    {
        r[i] = i;
    }

    r._raw_make_room(0, sz, 0); // must not change
    EXPECT_EQ(r.num_pages(), 1);
    EXPECT_EQ(r.capacity(), sz);
    for(int i = 0; i < ps; ++i)
    {
        EXPECT_EQ(r[i], i);
    }

    // add one page at the beginning - no data moves
    r._raw_make_room(0, sz, ps);
    sz += ps;
    EXPECT_EQ(r.num_pages(), 2);
    EXPECT_EQ(r.capacity(), sz);
    for(int i = 0; i < ps; ++i)
    {
        r[i] = ps - 1 - i;
    }
    for(int i = 0; i < ps; ++i)
    {
        EXPECT_EQ(r[i], ps - 1 - i);
        EXPECT_EQ(r[ps + i], i);
    }

    // add one page at the end - no data moves
    r._raw_make_room(sz, sz, ps);
    sz += ps;
    EXPECT_EQ(r.num_pages(), 3);
    EXPECT_EQ(r.capacity(), sz);
    for(int i = 0; i < ps; ++i)
    {
        r[2 * ps + i] = 2 * ps + i;
    }
    for(int i = 0; i < ps; ++i)
    {
        EXPECT_EQ(r[i], ps - 1 - i);
        EXPECT_EQ(r[ps + i], i);
        EXPECT_EQ(r[2 * ps + i], 2 * ps + i);
    }

    // add one page in the middle - no data moves
    r._raw_make_room(ps, sz, ps);
    sz += ps;
    EXPECT_EQ(r.num_pages(), 4);
    EXPECT_EQ(r.capacity(), sz);
    for(int i = 0; i < ps; ++i)
    {
        r[ps + i] = 3 * ps + i;
    }
    for(int i = 0; i < ps; ++i)
    {
        EXPECT_EQ(r[i], ps - 1 - i);
        EXPECT_EQ(r[ps + i], 3 * ps + i);
        EXPECT_EQ(r[2 * ps + i], i);
        EXPECT_EQ(r[3 * ps + i], 2 * ps + i);
    }

    // add one page at the middle of the first page
    r._raw_make_room(ps/2, sz, ps);
    sz += ps;
    EXPECT_EQ(r.num_pages(), 5);
    EXPECT_EQ(r.capacity(), sz);
    for(int i = 0; i < ps; ++i)
    {
        r[ps/2 + i] = 4 * ps + i;
    }
    for(int i = 0; i < ps; ++i)
    {
        if(i < ps/2)
        {
            EXPECT_EQ(r[i], ps - 1 - i); // 1st page, 1st half
            EXPECT_EQ(r[ps/2 + i], 4 * ps + i); // 1st page, 2nd half
        }
        else
        {
            EXPECT_EQ(r[ps/2 + i], 4 * ps + i); // 2nd page, 1st half
            EXPECT_EQ(r[ps + i], ps - 1 - i); // 2nd page, 2nd half
        }
        EXPECT_EQ(r[2 * ps + i], 3 * ps + i);
        EXPECT_EQ(r[3 * ps + i], i);
        EXPECT_EQ(r[4 * ps + i], 2 * ps + i);
    }


    {
        using ttype = PagedStorageInsertionTester< PagedStorage >;
        auto psit = ttype::create(args...);
        auto s = psit.s.get();

        C4_ASSERT(s->page_size() >= 4);

        { SCOPED_TRACE("add 1/4 at page beginning, no spilling, I");   psit.add_and_test(0, ps/4); EXPECT_EQ(s->num_pages(), 1); }
        { SCOPED_TRACE("add 1/4 at page beginning, no spilling, II");  psit.add_and_test(0, ps/4); EXPECT_EQ(s->num_pages(), 1); }
        { SCOPED_TRACE("add 1/4 at page interior, no spilling, I");    psit.add_and_test(ps/4, ps/4); EXPECT_EQ(s->num_pages(), 1); }
        { SCOPED_TRACE("add 1/4 at page interior, no spilling, II");   psit.add_and_test(ps/4, ps/4); EXPECT_EQ(s->num_pages(), 1); }
        EXPECT_TRUE(psit.full_capacity());

        { SCOPED_TRACE("add 1/4 at middle of first page, spill 1->2, I");   psit.add_and_test(ps/4, ps/4); EXPECT_EQ(s->num_pages(), 2); }
        { SCOPED_TRACE("add 1/4 at middle of first page, spill 1->2, II");  psit.add_and_test(ps/4, ps/4); EXPECT_EQ(s->num_pages(), 2); }
        { SCOPED_TRACE("add 1/4 at middle of first page, spill 1->2, III"); psit.add_and_test(ps/4, ps/4); EXPECT_EQ(s->num_pages(), 2); }
        { SCOPED_TRACE("add 1/4 at middle of first page, spill 1->2, IV");  psit.add_and_test(ps/4, ps/4); EXPECT_EQ(s->num_pages(), 2); }
        EXPECT_TRUE(psit.full_capacity());

        { SCOPED_TRACE("add 1/4 at middle of first page, spill 0->1,1->2, I"); psit.add_and_test(ps/4, ps/4); EXPECT_EQ(s->num_pages(), 3); }
        { SCOPED_TRACE("add 1/4 at middle of first page, spill 0->1,1->2, II"); psit.add_and_test(ps/4, ps/4); EXPECT_EQ(s->num_pages(), 3); }
        { SCOPED_TRACE("add 1/4 at middle of first page, spill 0->1,1->2, III"); psit.add_and_test(ps/4, ps/4); EXPECT_EQ(s->num_pages(), 3); }
        { SCOPED_TRACE("add 1/4 at middle of first page, spill 0->1,1->2, IV"); psit.add_and_test(ps/4, ps/4); EXPECT_EQ(s->num_pages(), 3); }
        EXPECT_TRUE(psit.full_capacity());

        { SCOPED_TRACE("add 1/4 at middle of first page, spill 0->1,1->2,2->3, I"); psit.add_and_test(ps/4, ps/4); EXPECT_EQ(s->num_pages(), 4); }
        { SCOPED_TRACE("add 1/4 at middle of first page, spill 0->1,1->2,2->3, II"); psit.add_and_test(ps/4, ps/4); EXPECT_EQ(s->num_pages(), 4); }
        { SCOPED_TRACE("add 1/4 at middle of first page, spill 0->1,1->2,2->3, III"); psit.add_and_test(ps/4, ps/4); EXPECT_EQ(s->num_pages(), 4); }
        { SCOPED_TRACE("add 1/4 at middle of first page, spill 0->1,1->2,2->3, IV"); psit.add_and_test(ps/4, ps/4); EXPECT_EQ(s->num_pages(), 4); }
        EXPECT_TRUE(psit.full_capacity());

        { SCOPED_TRACE("add 1/4 at beginning of page 1, spill 1->2,2->3,3->4, I"); psit.add_and_test(ps, ps/4); EXPECT_EQ(s->num_pages(), 5); }
        { SCOPED_TRACE("add 1/4 at beginning of page 1, spill 1->2,2->3,3->4, II"); psit.add_and_test(ps, ps/4); EXPECT_EQ(s->num_pages(), 5); }
        { SCOPED_TRACE("add 1/4 at beginning of page 1, spill 1->2,2->3,3->4, III"); psit.add_and_test(ps, ps/4); EXPECT_EQ(s->num_pages(), 5); }
        { SCOPED_TRACE("add 1/4 at beginning of page 1, spill 1->2,2->3,3->4, IV"); psit.add_and_test(ps, ps/4); EXPECT_EQ(s->num_pages(), 5); }
        EXPECT_TRUE(psit.full_capacity());

        { SCOPED_TRACE("add (ps-1) @begin(pg1), I");   psit.add_and_test(ps, ps-1); EXPECT_EQ(s->num_pages(), 6); }
        { SCOPED_TRACE("add (ps-1) @begin(pg1), II");  psit.add_and_test(ps, ps-1); EXPECT_EQ(s->num_pages(), 7); }
        { SCOPED_TRACE("add (ps-1) @begin(pg1), III"); psit.add_and_test(ps, ps-1); EXPECT_EQ(s->num_pages(), 8); }
        { SCOPED_TRACE("add (ps-1) @begin(pg1), IV");  psit.add_and_test(ps, ps-1); EXPECT_EQ(s->num_pages(), 9); }
        { SCOPED_TRACE("add 4");           psit.add_and_test(ps, 4);    EXPECT_EQ(s->num_pages(), 9); }
        EXPECT_TRUE(psit.full_capacity());

        { SCOPED_TRACE("add ps @begin(pg1)");   psit.add_and_test(ps, ps); EXPECT_EQ(s->num_pages(), 10); }
        { SCOPED_TRACE("add 2*ps @begin(pg1)");  psit.add_and_test(ps, 2*ps); EXPECT_EQ(s->num_pages(), 12); }
        { SCOPED_TRACE("add 3*ps @begin(pg1)");  psit.add_and_test(ps, 3*ps); EXPECT_EQ(s->num_pages(), 15); }
        EXPECT_TRUE(psit.full_capacity());

        { SCOPED_TRACE("add ps @begin(pg1)+1");   psit.add_and_test(ps+1, ps); EXPECT_EQ(s->num_pages(), 16); }
        { SCOPED_TRACE("add 2*ps @begin(pg1)+1");  psit.add_and_test(ps+1, 2*ps); EXPECT_EQ(s->num_pages(), 18); }
        { SCOPED_TRACE("add 3*ps @begin(pg1)+1");  psit.add_and_test(ps+1, 3*ps); EXPECT_EQ(s->num_pages(), 21); }
        EXPECT_TRUE(psit.full_capacity());

        { SCOPED_TRACE("add ps+1   @begin(pg1)");  psit.add_and_test(ps, ps+1);   EXPECT_EQ(s->num_pages(), 23); }
        { SCOPED_TRACE("add 2*ps+1 @begin(pg1)");  psit.add_and_test(ps, 2*ps+1); EXPECT_EQ(s->num_pages(), 25); }
        { SCOPED_TRACE("add 3*ps+1 @begin(pg1)");  psit.add_and_test(ps, 3*ps+1); EXPECT_EQ(s->num_pages(), 28); }
        { SCOPED_TRACE("add ps-3");                psit.add_and_test(ps, ps-3);   EXPECT_EQ(s->num_pages(), 28); }
        EXPECT_TRUE(psit.full_capacity());

        { SCOPED_TRACE("add ps+1 @begin(pg1)+1");    psit.add_and_test(ps+1, ps+1); EXPECT_EQ(s->num_pages(), 30); }
        { SCOPED_TRACE("add 2*ps+1 @begin(pg1)+1");  psit.add_and_test(ps+1, 2*ps+1); EXPECT_EQ(s->num_pages(), 32); }
        { SCOPED_TRACE("add 3*ps+1 @begin(pg1)+1");  psit.add_and_test(ps+1, 3*ps+1); EXPECT_EQ(s->num_pages(), 35); }
        { SCOPED_TRACE("add ps-3");                  psit.add_and_test(ps+1, ps-3);   EXPECT_EQ(s->num_pages(), 35); }
        EXPECT_TRUE(psit.full_capacity());

        for(int num = 0; num < ps; ++num)
        {
            for(int pos = 0; pos < ps; ++pos)
            {
                for(int pg = 0; pg < 3; ++pg)
                {
                    for(int ipg = 1; ipg < 4; ++ipg)
                    {
                        psit.clear(); psit.add_and_test(0, ipg*ps); EXPECT_EQ(s->num_pages(), ipg);
                        { SCOPED_TRACE("add   ps+num page[pg][pos]");  EXPECT_TRUE(psit.add_and_test(std::min(ps*pg+pos, s->capacity()),   ps+num)) << "num=" << num << "  pg=" << pg << "  pos=" << pos; }
                        psit.clear(); psit.add_and_test(0, ipg*ps); EXPECT_EQ(s->num_pages(), ipg);
                        { SCOPED_TRACE("add 2*ps+num page[pg][pos]");  EXPECT_TRUE(psit.add_and_test(std::min(ps*pg+pos, s->capacity()), 2*ps+num)) << "num=" << num << "  pg=" << pg << "  pos=" << pos; }
                        psit.clear(); psit.add_and_test(0, ipg*ps); EXPECT_EQ(s->num_pages(), ipg);
                        { SCOPED_TRACE("add 3*ps+num page[pg][pos]");  EXPECT_TRUE(psit.add_and_test(std::min(ps*pg+pos, s->capacity()), 3*ps+num)) << "num=" << num << "  pg=" << pg << "  pos=" << pos; }
                        psit.clear(); psit.add_and_test(0, ipg*ps); EXPECT_EQ(s->num_pages(), ipg);
                        { SCOPED_TRACE("add 4*ps+num page[pg][pos]");  EXPECT_TRUE(psit.add_and_test(std::min(ps*pg+pos, s->capacity()), 4*ps+num)) << "num=" << num << "  pg=" << pg << "  pos=" << pos; }
                        psit.clear(); psit.add_and_test(0, ipg*ps); EXPECT_EQ(s->num_pages(), ipg);
                        { SCOPED_TRACE("add 5*ps+num page[pg][pos]");  EXPECT_TRUE(psit.add_and_test(std::min(ps*pg+pos, s->capacity()), 5*ps+num)) << "num=" << num << "  pg=" << pg << "  pos=" << pos; }

                        if(ps*pg < pos) continue;

                        psit.clear(); psit.add_and_test(0, ipg*ps); EXPECT_EQ(s->num_pages(), ipg);
                        { SCOPED_TRACE("add   ps+num page[pg][-pos]");  EXPECT_TRUE(psit.add_and_test(std::min(ps*pg-pos, s->capacity()),   ps-num)) << "num=" << num << "  pg=" << pg << "  pos=" << pos; }
                        psit.clear(); psit.add_and_test(0, ipg*ps); EXPECT_EQ(s->num_pages(), ipg);
                        { SCOPED_TRACE("add 2*ps+num page[pg][-pos]");  EXPECT_TRUE(psit.add_and_test(std::min(ps*pg-pos, s->capacity()), 2*ps-num)) << "num=" << num << "  pg=" << pg << "  pos=" << pos; }
                        psit.clear(); psit.add_and_test(0, ipg*ps); EXPECT_EQ(s->num_pages(), ipg);
                        { SCOPED_TRACE("add 3*ps+num page[pg][-pos]");  EXPECT_TRUE(psit.add_and_test(std::min(ps*pg-pos, s->capacity()), 3*ps-num)) << "num=" << num << "  pg=" << pg << "  pos=" << pos; }
                        psit.clear(); psit.add_and_test(0, ipg*ps); EXPECT_EQ(s->num_pages(), ipg);
                        { SCOPED_TRACE("add 4*ps+num page[pg][-pos]");  EXPECT_TRUE(psit.add_and_test(std::min(ps*pg-pos, s->capacity()), 4*ps-num)) << "num=" << num << "  pg=" << pg << "  pos=" << pos; }
                        psit.clear(); psit.add_and_test(0, ipg*ps); EXPECT_EQ(s->num_pages(), ipg);
                        { SCOPED_TRACE("add 5*ps+num page[pg][-pos]");  EXPECT_TRUE(psit.add_and_test(std::min(ps*pg-pos, s->capacity()), 5*ps-num)) << "num=" << num << "  pg=" << pg << "  pos=" << pos; }
                    }
                }
            }
        }
    }
}

TEST(raw_paged, make_room)
{
    test_paged_resize< raw_paged< int, int, /*page_size*/8 > >(/*capacity*/0);
    test_paged_resize< raw_paged< int, int, /*page_size*/16 > >(/*capacity*/0);
}

TEST(raw_paged_rt, make_room)
{
    test_paged_resize< raw_paged_rt< int, int > >(/*capacity*/0, /*page_size*/8);
    test_paged_resize< raw_paged_rt< int, int > >(/*capacity*/0, /*page_size*/16);
}


C4_END_NAMESPACE(stg)
C4_END_NAMESPACE(c4)

#include "c4/libtest/supprwarn_pop.hpp"
