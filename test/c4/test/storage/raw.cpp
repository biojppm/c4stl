#include "c4/storage/raw.hpp"
#include "c4/allocator.hpp"

#include "c4/libtest/supprwarn_push.hpp"
#include "c4/test/storage/raw.hpp"
#include "c4/test.hpp"

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
    using value_type0 = typename Raw::value_type<0>;
    using value_type1 = typename Raw::value_type<1>;

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

C4_END_NAMESPACE(stg)
C4_END_NAMESPACE(c4)

#include "c4/libtest/supprwarn_pop.hpp"
