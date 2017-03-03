#include "c4/string.hpp"
#include "c4/config.hpp"
#include "c4/test.hpp"

#ifdef __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wwritable-strings" // ISO C++11 does not allow conversion from string literal to char*
#elif defined(__GNUC__)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wwrite-strings" // ISO C++ forbids converting a string constant to ‘C* {aka char*}’
#endif

C4_BEGIN_NAMESPACE(c4)

template< class C, class Sz, class Str, class SubStr >
void PrintTo(string_impl< C, Sz, Str, SubStr > const& s, ::std::ostream* os)
{
    os->write(s.data(), s.size());
}

#define _C4_TEST_STRINGBASE_DERIVED_NON_RESIZEABLE(classname, classtype)   \
TEST(classname, empty_ctor)                                     \
{                                                               \
    test_stringbase_empty_ctor< classtype >();                  \
}                                                               \
TEST(classname, copy_move_small_small)                          \
{                                                               \
    test_stringbase_copy_move_small_small< classtype >();       \
}                                                               \
TEST(classname, copy_move_big_big)                              \
{                                                               \
    test_stringbase_copy_move_big_big< classtype >();           \
}                                                               \
TEST(classname, copy_move_small_big)                            \
{                                                               \
    test_stringbase_copy_move_small_big< classtype >();         \
}                                                               \
TEST(classname, copy_move_big_small)                            \
{                                                               \
    test_stringbase_copy_move_big_small< classtype >();         \
}                                                               \
TEST(classname, template_ctor_and_init)                         \
{                                                               \
    test_stringbase_template_ctor_and_init< classtype >();      \
}                                                               \
TEST(classname, charbuf)                                        \
{                                                               \
    test_stringbase_char< classtype >();                        \
}                                                               \
TEST(classname, charbuf_and_size)                               \
{                                                               \
    test_stringbase_char_and_size< classtype >();               \
}                                                               \
TEST(classname, eq)                                             \
{                                                               \
    test_stringbase_eq< classtype >();                          \
}                                                               \
TEST(classname, ne)                                             \
{                                                               \
    test_stringbase_ne< classtype >();                          \
}                                                               \
TEST(classname, gt)                                             \
{                                                               \
    test_stringbase_gt< classtype >();                          \
}                                                               \
TEST(classname, lt)                                             \
{                                                               \
    test_stringbase_lt< classtype >();                          \
}                                                               \
TEST(classname, substr)                                         \
{                                                               \
    test_stringbase_substr< classtype >();                      \
}                                                               \
TEST(classname, range)                                          \
{                                                               \
    test_stringbase_range< classtype >();                       \
}                                                               \
TEST(classname, is_substr)                                      \
{                                                               \
    test_stringbase_is_substr< classtype >();                   \
}                                                               \
TEST(classname, compll)                                         \
{                                                               \
    test_stringbase_compll< classtype >();                      \
}                                                               \
TEST(classname, complr)                                         \
{                                                               \
    test_stringbase_complr< classtype >();                      \
}                                                               \
TEST(classname, find)                                           \
{                                                               \
    test_stringbase_find< classtype >();                        \
}                                                               \
TEST(classname, nextdiff)                                       \
{                                                               \
    test_stringbase_nextdiff< classtype >();                    \
}                                                               \
TEST(classname, prevdiff)                                       \
{                                                               \
    test_stringbase_prevdiff< classtype >();                    \
}                                                               \
TEST(classname, find_first_of)                                  \
{                                                               \
    test_stringbase_find_first_of< classtype >();               \
}                                                               \
TEST(classname, find_first_not_of)                              \
{                                                               \
    test_stringbase_find_first_not_of< classtype >();           \
}                                                               \
TEST(classname, find_last_of)                                   \
{                                                               \
    test_stringbase_find_last_of< classtype >();                \
}                                                               \
TEST(classname, find_last_not_of)                               \
{                                                               \
    test_stringbase_find_last_not_of< classtype >();            \
}                                                               \
TEST(classname, begins_with)                                    \
{                                                               \
    test_stringbase_begins_with< classtype >();                 \
}                                                               \
TEST(classname, ends_with)                                      \
{                                                               \
    test_stringbase_ends_with< classtype >();                   \
}                                                               \
TEST(classname, trimr)                                          \
{                                                               \
    test_stringbase_trimr< classtype >();                       \
}                                                               \
TEST(classname, triml)                                          \
{                                                               \
    test_stringbase_triml< classtype >();                       \
}                                                               \
TEST(classname, trim)                                           \
{                                                               \
    test_stringbase_trim< classtype >();                        \
}                                                               \
TEST(classname, trimwsr)                                        \
{                                                               \
    test_stringbase_trimwsr< classtype >();                     \
}                                                               \
TEST(classname, trimwsl)                                        \
{                                                               \
    test_stringbase_trimwsl< classtype >();                     \
}                                                               \
TEST(classname, trimws)                                         \
{                                                               \
    test_stringbase_trimws< classtype >();                      \
}                                                               \
TEST(classname, next_split)                                     \
{                                                               \
    test_stringbase_next_split< classtype >();                  \
}                                                               \
TEST(classname, split)                                          \
{                                                               \
    test_stringbase_split< classtype >();                       \
}                                                               \
TEST(classname, popr)                                           \
{                                                               \
    test_stringbase_popr< classtype >();                        \
}                                                               \
TEST(classname, popl)                                           \
{                                                               \
    test_stringbase_popl< classtype >();                        \
}                                                               \
TEST(classname, gpopr)                                          \
{                                                               \
    test_stringbase_gpopr< classtype >();                       \
}                                                               \
TEST(classname, gpopl)                                          \
{                                                               \
    test_stringbase_gpopl< classtype >();                       \
}                                                               \
TEST(classname, dirname)                                        \
{                                                               \
    test_stringbase_dirname< classtype >();                     \
}                                                               \
TEST(classname, basename)                                       \
{                                                               \
    test_stringbase_basename< classtype >();                    \
}                                                               \
TEST(classname, sstream)                                        \
{                                                               \
    test_stringbase_stream< classtype, c4::sstream<std::string> >();         \
}                                                               \
TEST(classname, hash)                                           \
{                                                               \
    test_stringbase_hash< classtype >();                        \
}


#define _C4_TEST_STRINGBASE_DERIVED(classname, classtype) \
                                                        \
_C4_TEST_STRINGBASE_DERIVED_NON_RESIZEABLE(classname, classtype)   \
                                                        \
TEST(classname, sum)                                    \
{                                                       \
    test_stringbase_sum< classtype >();                 \
}                                                       \
TEST(classname, dircat)                                 \
{                                                       \
    test_stringbase_dircat< classtype >();              \
}                                                       \
TEST(classname, append)                                 \
{                                                       \
    test_stringbase_append< classtype >();              \
}                                                       \
TEST(classname, prepend)                                \
{                                                       \
    test_stringbase_prepend< classtype >();             \
}                                                       \
TEST(classname, append_dir)                             \
{                                                       \
    test_stringbase_append_dir< classtype >();          \
}                                                       \
TEST(classname, prepend_dir)                            \
{                                                       \
    test_stringbase_prepend_dir< classtype >();         \
}                                                       \
TEST(classname, erase)                                  \
{                                                       \
    test_stringbase_erase< classtype >();               \
}                                                       \
TEST(classname, string_vs_wstring)                      \
{                                                       \
    test_string_vs_wstring< classtype >();              \
}

//-------------------------------------------
template< class S >
void test_stringbase_empty_ctor()
{
    {
        S n, n2;
        C4_EXPECT_EQ(n.empty(), true);
        C4_EXPECT_EQ(n.size(), strlen(""));
        C4_EXPECT_EQ(n, "");

        n2 = n;
        C4_EXPECT_EQ(n2.empty(), true);
        C4_EXPECT_EQ(n2.size(), strlen(""));
        C4_EXPECT_EQ(n2, "");
    }
}

#define NTEST "shortname"
#define MTEST "This is a big name to effectively ensure that it will extend beyond the small string optimization."
#define BTEST "see: " MTEST

#ifdef C4_CLANG

#endif

/// copy ctor+assign a small string to a small string
template< typename S >
void test_stringbase_copy_move_small_small()
{
    S n(NTEST);

    { // copy ctor with small
        S n2(n);
        C4_EXPECT_EQ(n.empty(), false);
        C4_EXPECT_EQ(n2.empty(), false);
        C4_EXPECT_EQ(n2, n);
        C4_EXPECT_EQ(n2, NTEST);
    }
    { // move ctor with small
        S n2(n);
        C4_EXPECT_EQ(n.empty(), false);
        C4_EXPECT_EQ(n2.empty(), false);
        C4_EXPECT_EQ(n2, n);
        C4_EXPECT_EQ(n2, NTEST);
        S n21(std::move(n2));
        C4_EXPECT_EQ(n.empty(), false);
        C4_EXPECT_EQ(n2.empty(), true);
        C4_EXPECT_EQ(n21.empty(), false);
        C4_EXPECT_EQ(n21, n);
        C4_EXPECT_EQ(n21, NTEST);
    }
    { // copy assign small to currently empty
        S n21;
        n21 = n;
        C4_EXPECT_EQ(n.empty(), false);
        C4_EXPECT_EQ(n21.empty(), false);
        C4_EXPECT_EQ(n21, n);
        C4_EXPECT_EQ(n21, NTEST);
    }
    { // move assign small to currently empty
        S n21, n2(n);
        C4_EXPECT_EQ(n.empty(), false);
        C4_EXPECT_EQ(n2.empty(), false);
        C4_EXPECT_EQ(n21.empty(), true);
        n21 = std::move(n2);
        C4_EXPECT_EQ(n.empty(), false);
        C4_EXPECT_EQ(n2.empty(), true);
        C4_EXPECT_EQ(n21.empty(), false);
        C4_EXPECT_EQ(n21, n);
        C4_EXPECT_EQ(n21, NTEST);
    }
    { // copy assign small to currently non-empty small
        S n2(n);
        C4_EXPECT_EQ(n2.empty(), false);
        C4_EXPECT_EQ(n2, n);
        C4_EXPECT_EQ(n2, NTEST);
        n2 = "small";
        C4_EXPECT_EQ(n2.empty(), false);
        C4_EXPECT_EQ(n2, "small");
    }
    { // move assign small to currently non-empty small
        S n2(n), n3("small");
        C4_EXPECT_EQ(n2.empty(), false);
        C4_EXPECT_EQ(n2, n);
        C4_EXPECT_EQ(n2, NTEST);
        C4_EXPECT_EQ(n3.empty(), false);
        n2 = std::move(n3);
        C4_EXPECT_EQ(n2.empty(), false);
        C4_EXPECT_EQ(n3.empty(), true);
        C4_EXPECT_EQ(n2, "small");
        C4_EXPECT_EQ(n2, "small");
    }
}

/// copy ctor+assign a big string to a big string
template< typename S >
void test_stringbase_copy_move_big_big()
{
    S n(MTEST);
    
    C4_EXPECT_EQ(n.size(), S::traits_type::length(MTEST));
    C4_EXPECT_EQ(n, MTEST);

    { // copy ctor with big
        S n2(n);
        C4_EXPECT_EQ(n.empty(), false);
        C4_EXPECT_EQ(n2.empty(), false);
        C4_EXPECT_EQ(n2.size(), n.size());
        C4_EXPECT_EQ(n2.capacity(), n.capacity());
        C4_EXPECT_EQ(n2, n);
        C4_EXPECT_EQ(n2, MTEST);
    }
    { // move ctor with big
        S n2(n);
        C4_EXPECT_EQ(n.empty(), false);
        C4_EXPECT_EQ(n2.empty(), false);
        C4_EXPECT_EQ(n2, n);
        C4_EXPECT_EQ(n2, MTEST);
        S n21(std::move(n2));
        C4_EXPECT_EQ(n.empty(), false);
        C4_EXPECT_EQ(n2.empty(), true);
        C4_EXPECT_EQ(n21.empty(), false);
        C4_EXPECT_EQ(n21, n);
        C4_EXPECT_EQ(n21, MTEST);
    }
    { // copy assign big to currently empty
        S n21;
        n21 = n;
        C4_EXPECT_EQ(n.empty(), false);
        C4_EXPECT_EQ(n21.empty(), false);
        C4_EXPECT_EQ(n21, n);
        C4_EXPECT_EQ(n21, MTEST);
    }
    { // move assign big to currently empty
        S n21, n2(n);
        C4_EXPECT_EQ(n.empty(), false);
        C4_EXPECT_EQ(n2.empty(), false);
        C4_EXPECT_EQ(n21.empty(), true);
        n21 = std::move(n2);
        C4_EXPECT_EQ(n.empty(), false);
        C4_EXPECT_EQ(n2.empty(), true);
        C4_EXPECT_EQ(n21.empty(), false);
        C4_EXPECT_EQ(n21, n);
        C4_EXPECT_EQ(n21, MTEST);
    }
    { // copy assign big to currently non-empty big
        S n2(n), n3(BTEST);
        C4_EXPECT_EQ(n2.empty(), false);
        C4_EXPECT_EQ(n2, n);
        C4_EXPECT_EQ(n2, MTEST);
        n2 = n3;
        C4_EXPECT_EQ(n2.empty(), false);
        C4_EXPECT_EQ(n2, BTEST);
    }
    { // move assign big to currently non-empty big
        S n2(n), n3(BTEST);
        C4_EXPECT_EQ(n2.empty(), false);
        C4_EXPECT_EQ(n2, n);
        C4_EXPECT_EQ(n2, MTEST);
        C4_EXPECT_EQ(n3.empty(), false);
        n2 = std::move(n3);
        C4_EXPECT_EQ(n2.empty(), false);
        C4_EXPECT_EQ(n3.empty(), true);
        C4_EXPECT_EQ(n2, BTEST);
        C4_EXPECT_EQ(n2, BTEST);
    }
}

/// assign a big string to a small string
template< typename S >
void test_stringbase_copy_move_big_small()
{
    S n(NTEST);

    { // copy assign big to currently non-empty small
        S n2(n), n3(BTEST);
        C4_EXPECT_EQ(n2.empty(), false);
        C4_EXPECT_EQ(n2, n);
        C4_EXPECT_EQ(n2, NTEST);
        n2 = n3;
        C4_EXPECT_EQ(n2.empty(), false);
        C4_EXPECT_EQ(n2, BTEST);
    }
    { // move assign big to currently non-empty small
        S n2(n), n3(BTEST);
        C4_EXPECT_EQ(n2.empty(), false);
        C4_EXPECT_EQ(n2, n);
        C4_EXPECT_EQ(n2, NTEST);
        C4_EXPECT_EQ(n3.empty(), false);
        n2 = std::move(n3);
        C4_EXPECT_EQ(n2.empty(), false);
        C4_EXPECT_EQ(n3.empty(), true);
        C4_EXPECT_EQ(n2, BTEST);
    }
}

/// assign a small string to a big string
template< typename S >
void test_stringbase_copy_move_small_big()
{
    S n(MTEST);

    { // copy assign small to currently non-empty big
        S n2(n), n3(NTEST);
        C4_EXPECT_EQ(n2.empty(), false);
        C4_EXPECT_EQ(n2, n);
        C4_EXPECT_EQ(n2, MTEST);
        n2 = n3;
        C4_EXPECT_EQ(n2.empty(), false);
        C4_EXPECT_EQ(n2, NTEST);
    }
    { // move assign small to currently non-empty big
        S n2(n), n3(NTEST);
        C4_EXPECT_EQ(n2.empty(), false);
        C4_EXPECT_EQ(n2, n);
        C4_EXPECT_EQ(n2, MTEST);
        C4_EXPECT_EQ(n3.empty(), false);
        n2 = std::move(n3);
        C4_EXPECT_EQ(n2.empty(), false);
        C4_EXPECT_EQ(n3.empty(), true);
        C4_EXPECT_EQ(n2, NTEST);
    }
}

template< typename S >
void test_stringbase_template_ctor_and_init()
{
    S n(NTEST);
    C4_EXPECT_EQ(n.empty(), false);
    C4_EXPECT_EQ(n.size(), strlen(NTEST));
    C4_EXPECT_EQ(n, NTEST);
    n.clear();
    C4_EXPECT_EQ(n.empty(), true);
    C4_EXPECT_EQ(n.size(), strlen(""));
    C4_EXPECT_EQ(n, "");
    n.assign(MTEST);
    C4_EXPECT_EQ(n.empty(), false);
    C4_EXPECT_EQ(n.size(), strlen(MTEST));
    C4_EXPECT_EQ(n, MTEST);
    n.clear();
    C4_EXPECT_EQ(n.empty(), true);
    C4_EXPECT_EQ(n.size(), strlen(""));
    C4_EXPECT_EQ(n, "");
    n = NTEST;
    C4_EXPECT_EQ(n.empty(), false);
    C4_EXPECT_EQ(n.size(), strlen(NTEST));
    C4_EXPECT_EQ(n, NTEST);
}
template< typename S >
void test_stringbase_char()
{
    typename S::char_type *cn = "akjasdkja", *cm = "98trkjjdffgyi";
    typename S::char_type *cn3 = "akj", *cm3 = "98t";

    // const char* version
    S n(cn);
    C4_EXPECT_EQ(n.empty(), false);
    C4_EXPECT_EQ(n.size(), strlen(cn));
    C4_EXPECT_EQ(n, cn);
    n.clear();
    C4_EXPECT_EQ(n.empty(), true);
    C4_EXPECT_EQ(n.size(), strlen(""));
    C4_EXPECT_EQ(n, "");
    n.assign(cm);
    C4_EXPECT_EQ(n.empty(), false);
    C4_EXPECT_EQ(n.size(), strlen(cm));
    C4_EXPECT_EQ(n, cm);
    n.clear();
    C4_EXPECT_EQ(n.empty(), true);
    C4_EXPECT_EQ(n.size(), strlen(""));
    C4_EXPECT_EQ(n, "");
    n = cn;
    C4_EXPECT_EQ(n.empty(), false);
    C4_EXPECT_EQ(n.size(), strlen(cn));
    C4_EXPECT_EQ(n, cn);
}
template< typename S >
void test_stringbase_char_and_size()
{
    typename S::char_type *cn = "akjasdkja", *cm = "98trkjjdffgyi";
    typename S::char_type *cn3 = "akj", *cm3 = "98t";

    // const char* + size version
    S n(cn, 3);
    C4_EXPECT_EQ(n.empty(), false);
    C4_EXPECT_EQ(n.size(), 3);
    C4_EXPECT_EQ(n, cn3);
    C4_EXPECT_NE(n, cn);
    n.clear();
    C4_EXPECT_EQ(n.empty(), true);
    C4_EXPECT_EQ(n.size(), 0);
    C4_EXPECT_EQ(n, "");
    n.assign(cm, 3);
    C4_EXPECT_EQ(n.empty(), false);
    C4_EXPECT_EQ(n.size(), 3);
    C4_EXPECT_EQ(n, cm3);
    C4_EXPECT_NE(n, cm);
    n.clear();
    C4_EXPECT_EQ(n.empty(), true);
    C4_EXPECT_EQ(n.size(), 0);
    C4_EXPECT_EQ(n, "");
    n = cn3;
    C4_EXPECT_EQ(n.empty(), false);
    C4_EXPECT_EQ(n.size(), 3);
    C4_EXPECT_EQ(n, cn3);
    C4_EXPECT_NE(n, cn);
}

template< typename S >
void test_stringbase_eq()
{
    S n("0123");
    C4_EXPECT_EQ(n, "0123");
    C4_EXPECT_EQ(n, S("0123"));
}
template< typename S >
void test_stringbase_ne()
{
    S n("0123");
    C4_EXPECT_NE(n, "03");
    C4_EXPECT_NE(n, S("03"));
}
template< typename S >
void test_stringbase_gt()
{
    S n("0123");
    C4_EXPECT_GT("1234", n);
    C4_EXPECT_GT(S("1234"), n);
}
template< typename S >
void test_stringbase_lt()
{
    S n("0123");
    C4_EXPECT_LT(n, "1234");
    C4_EXPECT_LT(n, S("1234"));
}

template< typename S >
void test_stringbase_substr()
{
    S n("01234567890123456789");
    C4_EXPECT_EQ(n.size(), 20);
    C4_EXPECT_EQ(n.substr(), n);
    C4_EXPECT_EQ(n.is_substr(n.substr()), true);

    C4_EXPECT_EQ(n.substr(0, 0).empty(), true);
    C4_EXPECT_EQ(n.substr(0, 0).data(), n.data());
    C4_EXPECT_EQ(n.substr(0, 0).data(), n.data());
    C4_EXPECT_EQ(n.is_substr(n.substr(0, 0)), true);

    C4_EXPECT_EQ(n.substr(0, 3), "012");
    C4_EXPECT_EQ(n.is_substr(n.substr(0, 3)), true);
    C4_EXPECT_EQ(n.substr(0, 10), "0123456789");
    C4_EXPECT_EQ(n.substr(10, 10), "0123456789");
}

template< typename S >
void test_stringbase_range()
{
    S n("01234567890123456789");
    C4_EXPECT_EQ(n.size(), 20);
    C4_EXPECT_EQ(n.range(), n);
    C4_EXPECT_EQ(n.is_substr(n.range()), true);

    C4_EXPECT_EQ(n.range(0, 0).empty(), true);
    C4_EXPECT_EQ(n.range(0, 0).data(), n.data());
    C4_EXPECT_EQ(n.range(0, 0).data(), n.data());
    C4_EXPECT_EQ(n.is_substr(n.range(0, 0)), true);

    C4_EXPECT_EQ(n.range(0, 3), "012");
    C4_EXPECT_EQ(n.is_substr(n.range(0, 3)), true);
    C4_EXPECT_EQ(n.range(0, 10), "0123456789");
    C4_EXPECT_EQ(n.range(10, 10), "");
    C4_EXPECT_EQ(n.range(10, 11), "0");
    C4_EXPECT_EQ(n.range(10, 12), "01");

    C4_EXPECT_EQ(n.range(10, 10), n.substr(10, 0));

}

template< typename S >
void test_stringbase_is_substr()
{
    S n("abcdefghij");
    S m("012345");
    C4_EXPECT_EQ(n.is_substr(n.substr()), true);
    C4_EXPECT_EQ(n.is_substr(n.substr(0, 3)), true);
    C4_EXPECT_EQ(n.is_substr(n.substr(0, 0)), true);

    C4_EXPECT_EQ(n.is_substr(m.substr()), false);
    C4_EXPECT_EQ(n.is_substr(m.substr(0, 3)), false);
    C4_EXPECT_EQ(n.is_substr(m.substr(0, 0)), false);
}

template< typename S >
void test_stringbase_compll()
{
    S n("0123456789");

    C4_EXPECT_EQ(n.compll(n.substr()), "");
    C4_EXPECT_EQ(n.is_substr(n.compll(n.substr())), true);
    C4_EXPECT_EQ(n.compll(n.substr(0, 0)), "");
    C4_EXPECT_EQ(n.is_substr(n.compll(n.substr(0, 0))), true);

    C4_EXPECT_EQ(n.compll(n.substr(0, 1)), "");
    C4_EXPECT_EQ(n.compll(n.substr(0, 3)), "");

    C4_EXPECT_EQ(n.compll(n.range(5, 10)), "01234");
    C4_EXPECT_EQ(n.compll(n.range(5, 5)), "01234");

    C4_EXPECT_EQ(n.compll(n.substr(n.size(), 0)), n);
    C4_EXPECT_EQ(n.compll(n.range(n.size(), n.size())), n);
}

template< typename S >
void test_stringbase_complr()
{
    S n("0123456789");

    C4_EXPECT_EQ(n.complr(n.substr()), "");
    C4_EXPECT_EQ(n.is_substr(n.complr(n.substr())), true);
    C4_EXPECT_EQ(n.complr(n.substr(0, 0)), "0123456789");
    C4_EXPECT_EQ(n.is_substr(n.complr(n.substr(0, 0))), true);

    C4_EXPECT_EQ(n.complr(n.substr(0, 1)), "123456789");
    C4_EXPECT_EQ(n.complr(n.substr(0, 3)), "3456789");

    C4_EXPECT_EQ(n.complr(n.substr(5)), "");
    C4_EXPECT_EQ(n.complr(n.range(5, 10)), "");
 
    C4_EXPECT_EQ(n.complr(n.substr(5, 0)), "56789");
    C4_EXPECT_EQ(n.complr(n.range(5, 5)), "56789");

    C4_EXPECT_EQ(n.complr(n.substr(0, 0)), n);
    C4_EXPECT_EQ(n.complr(n.range(0, 0)), n);
}

template< typename S >
void test_stringbase_nextdiff()
{
    C4_EXPECT_EQ(S("0123456789").nextdiff(0), 1);
    C4_EXPECT_EQ(S("0123456789").nextdiff(4), 5);
    C4_EXPECT_EQ(S("0123456789").nextdiff(9), S::npos);
    C4_EXPECT_EQ(S("0000007").nextdiff(0), 6);
    C4_EXPECT_EQ(S("7000007").nextdiff(0), 1);
    C4_EXPECT_EQ(S("0000007").nextdiff(6), S::npos);
}

template< typename S >
void test_stringbase_prevdiff()
{
    C4_EXPECT_EQ(S("7777770").prevdiff(6), 5);
}

template< typename S >
void test_stringbase_find()
{
    S n("012345");

    C4_EXPECT_EQ(n.find("0"), 0);
    C4_EXPECT_EQ(n.find("01"), 0);
    C4_EXPECT_EQ(n.find("1"), 1);
    C4_EXPECT_EQ(n.find("12"), 1);
    C4_EXPECT_EQ(n.find("234"), 2);
    C4_EXPECT_EQ(n.find("345"), 3);
    C4_EXPECT_EQ(n.find("3456"), S::npos);
    C4_EXPECT_EQ(n.find("7"), S::npos);
    C4_EXPECT_EQ(n.find('7'), S::npos);
    C4_EXPECT_EQ(n.find('2', 3), S::npos);
    C4_EXPECT_EQ(n.find('3', 2), 3);

    C4_EXPECT_EQ(n.find((const char*)"0"),  0);
    C4_EXPECT_EQ(n.find((const char*)"01"), 0);
    C4_EXPECT_EQ(n.find((const char*)"1"),  1);
    C4_EXPECT_EQ(n.find((const char*)"12"), 1);
    C4_EXPECT_EQ(n.find((const char*)"234"), 2);
    C4_EXPECT_EQ(n.find((const char*)"345"), 3);
    C4_EXPECT_EQ(n.find((const char*)"3456"), S::npos);

    n = "";
    C4_EXPECT_EQ(n.find("0"), S::npos);
    C4_EXPECT_EQ(n.find("01"), S::npos);
    C4_EXPECT_EQ(n.find("1"), S::npos);
    C4_EXPECT_EQ(n.find("12"), S::npos);
    C4_EXPECT_EQ(n.find("234"), S::npos);
    C4_EXPECT_EQ(n.find("345"), S::npos);
    C4_EXPECT_EQ(n.find("3456"), S::npos);
    C4_EXPECT_EQ(n.find("7"), S::npos);
    C4_EXPECT_EQ(n.find('7'), S::npos);

    C4_EXPECT_EQ(n.find((const char*)"0"),  S::npos);
    C4_EXPECT_EQ(n.find((const char*)"01"), S::npos);
    C4_EXPECT_EQ(n.find((const char*)"1"),  S::npos);
    C4_EXPECT_EQ(n.find((const char*)"12"), S::npos);
    C4_EXPECT_EQ(n.find((const char*)"234"), S::npos);
    C4_EXPECT_EQ(n.find((const char*)"345"), S::npos);
    C4_EXPECT_EQ(n.find((const char*)"3456"), S::npos);
}
template< typename S >
void test_stringbase_find_first_of()
{
    S n("012345");
    C4_EXPECT_EQ(n.find_first_of("01"), 0);
    C4_EXPECT_EQ(n.find_first_of("10"), 0);
    C4_EXPECT_EQ(n.find_first_of("45"), 4);
    C4_EXPECT_EQ(n.find_first_of("54"), 4);
    C4_EXPECT_EQ(n.find_first_of("67"), S::npos);

    n = "-";
    C4_EXPECT_EQ(n.find_first_of("01"), S::npos);
    C4_EXPECT_EQ(n.find_first_of("10"), S::npos);
    C4_EXPECT_EQ(n.find_first_of("45"), S::npos);
    C4_EXPECT_EQ(n.find_first_of("54"), S::npos);
    C4_EXPECT_EQ(n.find_first_of("67"), S::npos);

    n = "";
    C4_EXPECT_EQ(n.find_first_of("01"), S::npos);
    C4_EXPECT_EQ(n.find_first_of("10"), S::npos);
    C4_EXPECT_EQ(n.find_first_of("45"), S::npos);
    C4_EXPECT_EQ(n.find_first_of("54"), S::npos);
    C4_EXPECT_EQ(n.find_first_of("67"), S::npos);
}
template< typename S >
void test_stringbase_find_first_not_of()
{
    S n("012345");
    C4_EXPECT_EQ(n.find_first_not_of("01"), 2);
    C4_EXPECT_EQ(n.find_first_not_of("10"), 2);
    C4_EXPECT_EQ(n.find_first_not_of("45"), 0);
    C4_EXPECT_EQ(n.find_first_not_of("54"), 0);
    C4_EXPECT_EQ(n.find_first_not_of("67"), 0);

    n = "-";
    C4_EXPECT_EQ(n.find_first_not_of("01"), 0);
    C4_EXPECT_EQ(n.find_first_not_of("10"), 0);
    C4_EXPECT_EQ(n.find_first_not_of("45"), 0);
    C4_EXPECT_EQ(n.find_first_not_of("54"), 0);
    C4_EXPECT_EQ(n.find_first_not_of("67"), 0);

    n = "";
    C4_EXPECT_EQ(n.find_first_not_of("01"), S::npos);
    C4_EXPECT_EQ(n.find_first_not_of("10"), S::npos);
    C4_EXPECT_EQ(n.find_first_not_of("45"), S::npos);
    C4_EXPECT_EQ(n.find_first_not_of("54"), S::npos);
    C4_EXPECT_EQ(n.find_first_not_of("67"), S::npos);
}
template< typename S >
void test_stringbase_find_last_of()
{
    S n("012345");
    C4_EXPECT_EQ(n.find_last_of("01"), 1);
    C4_EXPECT_EQ(n.find_last_of("10"), 1);
    C4_EXPECT_EQ(n.find_last_of("45"), 5);
    C4_EXPECT_EQ(n.find_last_of("54"), 5);
    C4_EXPECT_EQ(n.find_last_of("67"), S::npos);

    n = "-";
    C4_EXPECT_EQ(n.find_last_of("01"), S::npos);
    C4_EXPECT_EQ(n.find_last_of("10"), S::npos);
    C4_EXPECT_EQ(n.find_last_of("45"), S::npos);
    C4_EXPECT_EQ(n.find_last_of("54"), S::npos);
    C4_EXPECT_EQ(n.find_last_of("67"), S::npos);

    n = "";
    C4_EXPECT_EQ(n.find_last_of("01"), S::npos);
    C4_EXPECT_EQ(n.find_last_of("10"), S::npos);
    C4_EXPECT_EQ(n.find_last_of("45"), S::npos);
    C4_EXPECT_EQ(n.find_last_of("54"), S::npos);
    C4_EXPECT_EQ(n.find_last_of("67"), S::npos);
}
template< typename S >
void test_stringbase_find_last_not_of()
{
    S n("012345");
    C4_EXPECT_EQ(n.find_last_not_of("01"), 5);
    C4_EXPECT_EQ(n.find_last_not_of("10"), 5);
    C4_EXPECT_EQ(n.find_last_not_of("45"), 3);
    C4_EXPECT_EQ(n.find_last_not_of("54"), 3);
    C4_EXPECT_EQ(n.find_last_not_of("67"), 5);

    n = "";
    C4_EXPECT_EQ(n.find_last_not_of("01"), S::npos);
    C4_EXPECT_EQ(n.find_last_not_of("10"), S::npos);
    C4_EXPECT_EQ(n.find_last_not_of("45"), S::npos);
    C4_EXPECT_EQ(n.find_last_not_of("54"), S::npos);
}
template< class S >
void test_stringbase_begins_with()
{
    S test("0123456789");

    C4_EXPECT(test.begins_with('0'));
    C4_EXPECT(test.begins_with("0"));
    C4_EXPECT(test.begins_with("03", 1));

    C4_EXPECT( ! test.begins_with('1'));
    C4_EXPECT( ! test.begins_with("1"));
    C4_EXPECT( ! test.begins_with("13", 1));

    C4_EXPECT(test.begins_with("01"));
    C4_EXPECT(test.begins_with("01", 2));
    C4_EXPECT(test.begins_with("013", 2));

    C4_EXPECT( ! test.begins_with("101"));
    C4_EXPECT( ! test.begins_with("101", 2));
    C4_EXPECT( ! test.begins_with("1013", 2));

    C4_EXPECT(test.begins_with("012"));
    C4_EXPECT(test.begins_with("012", 3));
    C4_EXPECT(test.begins_with("0124", 3));

    C4_EXPECT( ! test.begins_with("1012"));
    C4_EXPECT( ! test.begins_with("1012", 3));
    C4_EXPECT( ! test.begins_with("10124", 3));
}

template< class S >
void test_stringbase_ends_with()
{
    S test("0123456789");

    C4_EXPECT(test.ends_with('9'));
    C4_EXPECT(test.ends_with("9"));
    C4_EXPECT(test.ends_with("93", 1));

    C4_EXPECT( ! test.ends_with('1'));
    C4_EXPECT( ! test.ends_with("1"));
    C4_EXPECT( ! test.ends_with("13", 1));

    C4_EXPECT(test.ends_with("89"));
    C4_EXPECT(test.ends_with("89", 2));
    C4_EXPECT(test.ends_with("893", 2));

    C4_EXPECT( ! test.ends_with("101"));
    C4_EXPECT( ! test.ends_with("101", 2));
    C4_EXPECT( ! test.ends_with("1013", 2));

    C4_EXPECT(test.ends_with("789"));
    C4_EXPECT(test.ends_with("789", 3));
    C4_EXPECT(test.ends_with("7894", 3));

    C4_EXPECT( ! test.ends_with("1012"));
    C4_EXPECT( ! test.ends_with("1012", 3));
    C4_EXPECT( ! test.ends_with("10124", 3));
}

template< class S >
void test_stringbase_triml()
{
    C4_EXPECT_EQ(S("aaabbb"   ).triml('a' ), "bbb");
    C4_EXPECT_EQ(S("aaabbb"   ).triml('b' ), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb"   ).triml('c' ), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb"   ).triml("ab"), "");
    C4_EXPECT_EQ(S("aaabbb"   ).triml("ba"), "");
    C4_EXPECT_EQ(S("aaabbb"   ).triml("cd"), "aaabbb");
    C4_EXPECT_EQ(S("aaa...bbb").triml('a' ), "...bbb");
    C4_EXPECT_EQ(S("aaa...bbb").triml('b' ), "aaa...bbb");
    C4_EXPECT_EQ(S("aaa...bbb").triml('c' ), "aaa...bbb");
    C4_EXPECT_EQ(S("aaa...bbb").triml("ab"), "...bbb");
    C4_EXPECT_EQ(S("aaa...bbb").triml("ba"), "...bbb");
    C4_EXPECT_EQ(S("aaa...bbb").triml("ab."), "");
    C4_EXPECT_EQ(S("aaa...bbb").triml("a."), "bbb");
    C4_EXPECT_EQ(S("aaa...bbb").triml(".a"), "bbb");
    C4_EXPECT_EQ(S("aaa...bbb").triml("b."), "aaa...bbb");
    C4_EXPECT_EQ(S("aaa...bbb").triml(".b"), "aaa...bbb");
    C4_EXPECT_EQ(S("aaa...bbb").triml("cd"), "aaa...bbb");

    C4_EXPECT_EQ(S("ab"   ).triml('a' ), "b");
    C4_EXPECT_EQ(S("ab"   ).triml('b' ), "ab");
    C4_EXPECT_EQ(S("ab"   ).triml('c' ), "ab");
    C4_EXPECT_EQ(S("ab"   ).triml("ab"), "");
    C4_EXPECT_EQ(S("ab"   ).triml("ba"), "");
    C4_EXPECT_EQ(S("ab"   ).triml("cd"), "ab");
    C4_EXPECT_EQ(S("a...b").triml('a' ), "...b");
    C4_EXPECT_EQ(S("a...b").triml('b' ), "a...b");
    C4_EXPECT_EQ(S("a...b").triml('c' ), "a...b");
    C4_EXPECT_EQ(S("a...b").triml("ab"), "...b");
    C4_EXPECT_EQ(S("a...b").triml("ba"), "...b");
    C4_EXPECT_EQ(S("a...b").triml("ab."), "");
    C4_EXPECT_EQ(S("a...b").triml("a."), "b");
    C4_EXPECT_EQ(S("a...b").triml(".a"), "b");
    C4_EXPECT_EQ(S("a...b").triml("b."), "a...b");
    C4_EXPECT_EQ(S("a...b").triml(".b"), "a...b");
    C4_EXPECT_EQ(S("a...b").triml("cd"), "a...b");
}
template< class S >
void test_stringbase_trimr()
{
    C4_EXPECT_EQ(S("aaabbb"   ).trimr('a' ), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb"   ).trimr('b' ), "aaa");
    C4_EXPECT_EQ(S("aaabbb"   ).trimr('c' ), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb"   ).trimr("ab"), "");
    C4_EXPECT_EQ(S("aaabbb"   ).trimr("ba"), "");
    C4_EXPECT_EQ(S("aaabbb"   ).trimr("cd"), "aaabbb");
    C4_EXPECT_EQ(S("aaa...bbb").trimr('a' ), "aaa...bbb");
    C4_EXPECT_EQ(S("aaa...bbb").trimr('b' ), "aaa...");
    C4_EXPECT_EQ(S("aaa...bbb").trimr('c' ), "aaa...bbb");
    C4_EXPECT_EQ(S("aaa...bbb").trimr("ab"), "aaa...");
    C4_EXPECT_EQ(S("aaa...bbb").trimr("ba"), "aaa...");
    C4_EXPECT_EQ(S("aaa...bbb").trimr("ab."), "");
    C4_EXPECT_EQ(S("aaa...bbb").trimr("a."), "aaa...bbb");
    C4_EXPECT_EQ(S("aaa...bbb").trimr(".a"), "aaa...bbb");
    C4_EXPECT_EQ(S("aaa...bbb").trimr("b."), "aaa");
    C4_EXPECT_EQ(S("aaa...bbb").trimr(".b"), "aaa");
    C4_EXPECT_EQ(S("aaa...bbb").trimr("cd"), "aaa...bbb");

    C4_EXPECT_EQ(S("ab"   ).trimr('a' ), "ab");
    C4_EXPECT_EQ(S("ab"   ).trimr('b' ), "a");
    C4_EXPECT_EQ(S("ab"   ).trimr('c' ), "ab");
    C4_EXPECT_EQ(S("ab"   ).trimr("ab"), "");
    C4_EXPECT_EQ(S("ab"   ).trimr("ba"), "");
    C4_EXPECT_EQ(S("ab"   ).trimr("cd"), "ab");
    C4_EXPECT_EQ(S("a...b").trimr('a' ), "a...b");
    C4_EXPECT_EQ(S("a...b").trimr('b' ), "a...");
    C4_EXPECT_EQ(S("a...b").trimr('c' ), "a...b");
    C4_EXPECT_EQ(S("a...b").trimr("ab"), "a...");
    C4_EXPECT_EQ(S("a...b").trimr("ba"), "a...");
    C4_EXPECT_EQ(S("a...b").trimr("ab."), "");
    C4_EXPECT_EQ(S("a...b").trimr("a."), "a...b");
    C4_EXPECT_EQ(S("a...b").trimr(".a"), "a...b");
    C4_EXPECT_EQ(S("a...b").trimr("b."), "a");
    C4_EXPECT_EQ(S("a...b").trimr(".b"), "a");
    C4_EXPECT_EQ(S("a...b").trimr("cd"), "a...b");
}
template< class S >
void test_stringbase_trim()
{
    C4_EXPECT_EQ(S("aaabbb"   ).trim('a' ), "bbb");
    C4_EXPECT_EQ(S("aaabbb"   ).trim('b' ), "aaa");
    C4_EXPECT_EQ(S("aaabbb"   ).trim('c' ), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb"   ).trim("ab"), "");
    C4_EXPECT_EQ(S("aaabbb"   ).trim("ba"), "");
    C4_EXPECT_EQ(S("aaabbb"   ).trim("cd"), "aaabbb");
    C4_EXPECT_EQ(S("aaa...bbb").trim('a' ), "...bbb");
    C4_EXPECT_EQ(S("aaa...bbb").trim('b' ), "aaa...");
    C4_EXPECT_EQ(S("aaa...bbb").trim('c' ), "aaa...bbb");
    C4_EXPECT_EQ(S("aaa...bbb").trim("ab"), "...");
    C4_EXPECT_EQ(S("aaa...bbb").trim("ba"), "...");
    C4_EXPECT_EQ(S("aaa...bbb").trim('c' ), "aaa...bbb");
    C4_EXPECT_EQ(S("aaa...bbb").trim("ab."), "");
    C4_EXPECT_EQ(S("aaa...bbb").trim("." ), "aaa...bbb");
    C4_EXPECT_EQ(S("aaa...bbb").trim("a."), "bbb");
    C4_EXPECT_EQ(S("aaa...bbb").trim(".a"), "bbb");
    C4_EXPECT_EQ(S("aaa...bbb").trim("b."), "aaa");
    C4_EXPECT_EQ(S("aaa...bbb").trim(".b"), "aaa");
    C4_EXPECT_EQ(S("aaa...bbb").trim("cd"), "aaa...bbb");

    C4_EXPECT_EQ(S("ab"   ).trim('a' ), "b");
    C4_EXPECT_EQ(S("ab"   ).trim('b' ), "a");
    C4_EXPECT_EQ(S("ab"   ).trim('c' ), "ab");
    C4_EXPECT_EQ(S("ab"   ).trim("ab"), "");
    C4_EXPECT_EQ(S("ab"   ).trim("ba"), "");
    C4_EXPECT_EQ(S("ab"   ).trim("cd"), "ab");
    C4_EXPECT_EQ(S("a...b").trim('a' ), "...b");
    C4_EXPECT_EQ(S("a...b").trim('b' ), "a...");
    C4_EXPECT_EQ(S("a...b").trim('c' ), "a...b");
    C4_EXPECT_EQ(S("a...b").trim("ab"), "...");
    C4_EXPECT_EQ(S("a...b").trim("ba"), "...");
    C4_EXPECT_EQ(S("a...b").trim('c' ), "a...b");
    C4_EXPECT_EQ(S("a...b").trim("ab."), "");
    C4_EXPECT_EQ(S("a...b").trim("." ), "a...b");
    C4_EXPECT_EQ(S("a...b").trim("a."), "b");
    C4_EXPECT_EQ(S("a...b").trim(".a"), "b");
    C4_EXPECT_EQ(S("a...b").trim("b."), "a");
    C4_EXPECT_EQ(S("a...b").trim(".b"), "a");
    C4_EXPECT_EQ(S("a...b").trim("cd"), "a...b");
}

template< class S >
void test_stringbase_trimwsl()
{
    C4_EXPECT_EQ(S("aaabbb").trimwsl(), "aaabbb");
    C4_EXPECT_EQ(S(" aaabbb").trimwsl(), "aaabbb");
    C4_EXPECT_EQ(S("  aaabbb").trimwsl(), "aaabbb");
    C4_EXPECT_EQ(S("   aaabbb").trimwsl(), "aaabbb");
    C4_EXPECT_EQ(S("\taaabbb").trimwsl(), "aaabbb");
    C4_EXPECT_EQ(S("\t\taaabbb").trimwsl(), "aaabbb");
    C4_EXPECT_EQ(S("\t\t\taaabbb").trimwsl(), "aaabbb");
    C4_EXPECT_EQ(S("\naaabbb").trimwsl(), "aaabbb");
    C4_EXPECT_EQ(S("\n\naaabbb").trimwsl(), "aaabbb");
    C4_EXPECT_EQ(S("\n\n\naaabbb").trimwsl(), "aaabbb");
    C4_EXPECT_EQ(S("\raaabbb").trimwsl(), "aaabbb");
    C4_EXPECT_EQ(S("\r\raaabbb").trimwsl(), "aaabbb");
    C4_EXPECT_EQ(S("\r\r\raaabbb").trimwsl(), "aaabbb");
    C4_EXPECT_EQ(S("   \t\t\t\n\n\n\r\r\raaabbb").trimwsl(), "aaabbb");

    C4_EXPECT_EQ(S("aaabbb  ").trimwsl(), "aaabbb  ");
    C4_EXPECT_EQ(S(" aaabbb  ").trimwsl(), "aaabbb  ");
    C4_EXPECT_EQ(S("  aaabbb  ").trimwsl(), "aaabbb  ");
    C4_EXPECT_EQ(S("   aaabbb  ").trimwsl(), "aaabbb  ");
    C4_EXPECT_EQ(S("\taaabbb  ").trimwsl(), "aaabbb  ");
    C4_EXPECT_EQ(S("\t\taaabbb  ").trimwsl(), "aaabbb  ");
    C4_EXPECT_EQ(S("\t\t\taaabbb  ").trimwsl(), "aaabbb  ");
    C4_EXPECT_EQ(S("\naaabbb  ").trimwsl(), "aaabbb  ");
    C4_EXPECT_EQ(S("\n\naaabbb  ").trimwsl(), "aaabbb  ");
    C4_EXPECT_EQ(S("\n\n\naaabbb  ").trimwsl(), "aaabbb  ");
    C4_EXPECT_EQ(S("\raaabbb  ").trimwsl(), "aaabbb  ");
    C4_EXPECT_EQ(S("\r\raaabbb  ").trimwsl(), "aaabbb  ");
    C4_EXPECT_EQ(S("\r\r\raaabbb  ").trimwsl(), "aaabbb  ");
    C4_EXPECT_EQ(S("   \t\t\t\n\n\n\r\r\raaabbb  ").trimwsl(), "aaabbb  ");

    using C = typename S::char_type;
    auto must_be_same = [](C * str){
        C4_EXPECT_EQ(S(str).trimwsl(), str) << "not ok:" << str;
    };

    must_be_same("...  aaabbb");
    must_be_same("...   aaabbb");
    must_be_same("...    aaabbb");
    must_be_same("... \taaabbb");
    must_be_same("... \t\taaabbb");
    must_be_same("... \t\t\taaabbb");
    must_be_same("... \naaabbb");
    must_be_same("... \n\naaabbb");
    must_be_same("... \n\n\naaabbb");
    must_be_same("... \raaabbb");
    must_be_same("... \r\raaabbb");
    must_be_same("... \r\r\raaabbb");
    must_be_same("...    \t\t\t\n\n\n\r\r\raaabbb");
}
template< class S >
void test_stringbase_trimwsr()
{
    C4_EXPECT_EQ(S("aaabbb").trimwsr(), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb ").trimwsr(), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb  ").trimwsr(), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb   ").trimwsr(), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb\t").trimwsr(), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb\t\t").trimwsr(), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb\t\t\t").trimwsr(), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb\n").trimwsr(), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb\n\n").trimwsr(), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb\n\n\n").trimwsr(), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb\r").trimwsr(), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb\r\r").trimwsr(), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb\r\r\r").trimwsr(), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb   \t\t\t\n\n\n\r\r\r").trimwsr(), "aaabbb");

    C4_EXPECT_EQ(S("  aaabbb").trimwsr(), "  aaabbb");
    C4_EXPECT_EQ(S("  aaabbb ").trimwsr(), "  aaabbb");
    C4_EXPECT_EQ(S("  aaabbb  ").trimwsr(), "  aaabbb");
    C4_EXPECT_EQ(S("  aaabbb   ").trimwsr(), "  aaabbb");
    C4_EXPECT_EQ(S("  aaabbb\t").trimwsr(), "  aaabbb");
    C4_EXPECT_EQ(S("  aaabbb\t\t").trimwsr(), "  aaabbb");
    C4_EXPECT_EQ(S("  aaabbb\t\t\t").trimwsr(), "  aaabbb");
    C4_EXPECT_EQ(S("  aaabbb\n").trimwsr(), "  aaabbb");
    C4_EXPECT_EQ(S("  aaabbb\n\n").trimwsr(), "  aaabbb");
    C4_EXPECT_EQ(S("  aaabbb\n\n\n").trimwsr(), "  aaabbb");
    C4_EXPECT_EQ(S("  aaabbb\r").trimwsr(), "  aaabbb");
    C4_EXPECT_EQ(S("  aaabbb\r\r").trimwsr(), "  aaabbb");
    C4_EXPECT_EQ(S("  aaabbb\r\r\r").trimwsr(), "  aaabbb");
    C4_EXPECT_EQ(S("  aaabbb   \t\t\t\n\n\n\r\r\r").trimwsr(), "  aaabbb");

    using C = typename S::char_type;
    auto must_be_same = [](C * str){
        C4_EXPECT_EQ(S(str).trimwsr(), str) << "not ok:" << str;
    };

    must_be_same("aaabbb ...");
    must_be_same("aaabbb  ...");
    must_be_same("aaabbb   ...");
    must_be_same("aaabbb\t...");
    must_be_same("aaabbb\t\t...");
    must_be_same("aaabbb\t\t\t...");
    must_be_same("aaabbb\n...");
    must_be_same("aaabbb\n\n...");
    must_be_same("aaabbb\n\n\n...");
    must_be_same("aaabbb\r...");
    must_be_same("aaabbb\r\r...");
    must_be_same("aaabbb\r\r\r...");
    must_be_same("aaabbb   \t\t\t\n\n\n\r\r\r...");
}

template< class S >
void test_stringbase_trimws()
{
    C4_EXPECT_EQ(S("aaabbb").trimws(), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb ").trimws(), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb  ").trimws(), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb   ").trimws(), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb\t").trimws(), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb\t\t").trimws(), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb\t\t\t").trimws(), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb\n").trimws(), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb\n\n").trimws(), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb\n\n\n").trimws(), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb\r").trimws(), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb\r\r").trimws(), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb\r\r\r").trimws(), "aaabbb");
    C4_EXPECT_EQ(S("aaabbb   \t\t\t\n\n\n\r\r\r").trimws(), "aaabbb");

    C4_EXPECT_EQ(S("  aaabbb").trimws(), "aaabbb");
    C4_EXPECT_EQ(S("  aaabbb ").trimws(), "aaabbb");
    C4_EXPECT_EQ(S("  aaabbb  ").trimws(), "aaabbb");
    C4_EXPECT_EQ(S("  aaabbb   ").trimws(), "aaabbb");
    C4_EXPECT_EQ(S("  aaabbb\t").trimws(), "aaabbb");
    C4_EXPECT_EQ(S("  aaabbb\t\t").trimws(), "aaabbb");
    C4_EXPECT_EQ(S("  aaabbb\t\t\t").trimws(), "aaabbb");
    C4_EXPECT_EQ(S("  aaabbb\n").trimws(), "aaabbb");
    C4_EXPECT_EQ(S("  aaabbb\n\n").trimws(), "aaabbb");
    C4_EXPECT_EQ(S("  aaabbb\n\n\n").trimws(), "aaabbb");
    C4_EXPECT_EQ(S("  aaabbb\r").trimws(), "aaabbb");
    C4_EXPECT_EQ(S("  aaabbb\r\r").trimws(), "aaabbb");
    C4_EXPECT_EQ(S("  aaabbb\r\r\r").trimws(), "aaabbb");
    C4_EXPECT_EQ(S("  aaabbb   \t\t\t\n\n\n\r\r\r").trimws(), "aaabbb");

    using C = typename S::char_type;
    auto must_be_same = [](C * str){
        C4_EXPECT_EQ(S(str).trimws(), str) << "not ok:" << str;
    };

    must_be_same("aaabbb ...");
    must_be_same("aaabbb  ...");
    must_be_same("aaabbb   ...");
    must_be_same("aaabbb\t...");
    must_be_same("aaabbb\t\t...");
    must_be_same("aaabbb\t\t\t...");
    must_be_same("aaabbb\n...");
    must_be_same("aaabbb\n\n...");
    must_be_same("aaabbb\n\n\n...");
    must_be_same("aaabbb\r...");
    must_be_same("aaabbb\r\r...");
    must_be_same("aaabbb\r\r\r...");
    must_be_same("aaabbb   \t\t\t\n\n\n\r\r\r...");

    must_be_same("...  aaabbb");
    must_be_same("...   aaabbb");
    must_be_same("...    aaabbb");
    must_be_same("... \taaabbb");
    must_be_same("... \t\taaabbb");
    must_be_same("... \t\t\taaabbb");
    must_be_same("... \naaabbb");
    must_be_same("... \n\naaabbb");
    must_be_same("... \n\n\naaabbb");
    must_be_same("... \raaabbb");
    must_be_same("... \r\raaabbb");
    must_be_same("... \r\r\raaabbb");
    must_be_same("...    \t\t\t\n\n\n\r\r\raaabbb");

    must_be_same("...  aaabbaaabbb ...");
    must_be_same("...   aaabaaabbb  ...");
    must_be_same("...    aaaaaabbb   ...");
    must_be_same("... \taaabaaabbb\t...");
    must_be_same("... \t\taaaaabbb\t\t...");
    must_be_same("... \t\t\taaabbb\t\t\t...");
    must_be_same("... \naaabaaabbb\n...");
    must_be_same("... \n\naaaaabbb\n\n...");
    must_be_same("... \n\n\naaabbb\n\n\n...");
    must_be_same("... \raaabaaabbb\r...");
    must_be_same("... \r\raaaaabbb\r\r...");
    must_be_same("... \r\r\raaabbb\r\r\r...");
    must_be_same("...    \t\t\t\n\n\n\r\r\raaabbb   \t\t\t\n\n\n\r\r\r...");
}

template< class S >
void test_stringbase_popr()
{
    C4_EXPECT_EQ(S("0/1/2"    ).popr('/'      ), "2");
    C4_EXPECT_EQ(S("0/1/2"    ).popr('/', true), "2");
    C4_EXPECT_EQ(S("0/1/2/"   ).popr('/'      ), "");
    C4_EXPECT_EQ(S("0/1/2/"   ).popr('/', true), "2/");
    C4_EXPECT_EQ(S("0/1/2///" ).popr('/'      ), "");
    C4_EXPECT_EQ(S("0/1/2///" ).popr('/', true), "2///");

    C4_EXPECT_EQ(S("0/1//2"    ).popr('/'      ), "2");
    C4_EXPECT_EQ(S("0/1//2"    ).popr('/', true), "2");
    C4_EXPECT_EQ(S("0/1//2/"   ).popr('/'      ), "");
    C4_EXPECT_EQ(S("0/1//2/"   ).popr('/', true), "2/");
    C4_EXPECT_EQ(S("0/1//2///" ).popr('/'      ), "");
    C4_EXPECT_EQ(S("0/1//2///" ).popr('/', true), "2///");

    C4_EXPECT_EQ(S("0/1///2"    ).popr('/'      ), "2");
    C4_EXPECT_EQ(S("0/1///2"    ).popr('/', true), "2");
    C4_EXPECT_EQ(S("0/1///2/"   ).popr('/'      ), "");
    C4_EXPECT_EQ(S("0/1///2/"   ).popr('/', true), "2/");
    C4_EXPECT_EQ(S("0/1///2///" ).popr('/'      ), "");
    C4_EXPECT_EQ(S("0/1///2///" ).popr('/', true), "2///");

    C4_EXPECT_EQ(S("/0/1/2"   ).popr('/'      ), "2");
    C4_EXPECT_EQ(S("/0/1/2"   ).popr('/', true), "2");
    C4_EXPECT_EQ(S("/0/1/2/"  ).popr('/'      ), "");
    C4_EXPECT_EQ(S("/0/1/2/"  ).popr('/', true), "2/");
    C4_EXPECT_EQ(S("/0/1/2///").popr('/'      ), "");
    C4_EXPECT_EQ(S("/0/1/2///").popr('/', true), "2///");

    C4_EXPECT_EQ(S("0"        ).popr('/'      ), "0");
    C4_EXPECT_EQ(S("0"        ).popr('/', true), "0");
    C4_EXPECT_EQ(S("0/"       ).popr('/'      ), "");
    C4_EXPECT_EQ(S("0/"       ).popr('/', true), "0/");
    C4_EXPECT_EQ(S("0///"     ).popr('/'      ), "");
    C4_EXPECT_EQ(S("0///"     ).popr('/', true), "0///");
                              
    C4_EXPECT_EQ(S("/0"       ).popr('/'      ), "0");
    C4_EXPECT_EQ(S("/0"       ).popr('/', true), "0");
    C4_EXPECT_EQ(S("/0/"      ).popr('/'      ), "");
    C4_EXPECT_EQ(S("/0/"      ).popr('/', true), "0/");
    C4_EXPECT_EQ(S("/0///"    ).popr('/'      ), "");
    C4_EXPECT_EQ(S("/0///"    ).popr('/', true), "0///");
                              
    C4_EXPECT_EQ(S("/"        ).popr('/'      ), "");
    C4_EXPECT_EQ(S("/"        ).popr('/', true), "");
    C4_EXPECT_EQ(S("///"      ).popr('/'      ), "");
    C4_EXPECT_EQ(S("///"      ).popr('/', true), "");
                              
    C4_EXPECT_EQ(S(""         ).popr('/'      ), "");
    C4_EXPECT_EQ(S(""         ).popr('/', true), "");

    C4_EXPECT_EQ(S("0-1-2"    ).popr('-'      ), "2");
    C4_EXPECT_EQ(S("0-1-2"    ).popr('-', true), "2");
    C4_EXPECT_EQ(S("0-1-2-"   ).popr('-'      ), "");
    C4_EXPECT_EQ(S("0-1-2-"   ).popr('-', true), "2-");
    C4_EXPECT_EQ(S("0-1-2---" ).popr('-'      ), "");
    C4_EXPECT_EQ(S("0-1-2---" ).popr('-', true), "2---");

    C4_EXPECT_EQ(S("0-1--2"    ).popr('-'      ), "2");
    C4_EXPECT_EQ(S("0-1--2"    ).popr('-', true), "2");
    C4_EXPECT_EQ(S("0-1--2-"   ).popr('-'      ), "");
    C4_EXPECT_EQ(S("0-1--2-"   ).popr('-', true), "2-");
    C4_EXPECT_EQ(S("0-1--2---" ).popr('-'      ), "");
    C4_EXPECT_EQ(S("0-1--2---" ).popr('-', true), "2---");

    C4_EXPECT_EQ(S("0-1---2"    ).popr('-'      ), "2");
    C4_EXPECT_EQ(S("0-1---2"    ).popr('-', true), "2");
    C4_EXPECT_EQ(S("0-1---2-"   ).popr('-'      ), "");
    C4_EXPECT_EQ(S("0-1---2-"   ).popr('-', true), "2-");
    C4_EXPECT_EQ(S("0-1---2---" ).popr('-'      ), "");
    C4_EXPECT_EQ(S("0-1---2---" ).popr('-', true), "2---");

    C4_EXPECT_EQ(S("-0-1-2"   ).popr('-'      ), "2");
    C4_EXPECT_EQ(S("-0-1-2"   ).popr('-', true), "2");
    C4_EXPECT_EQ(S("-0-1-2-"  ).popr('-'      ), "");
    C4_EXPECT_EQ(S("-0-1-2-"  ).popr('-', true), "2-");
    C4_EXPECT_EQ(S("-0-1-2---").popr('-'      ), "");
    C4_EXPECT_EQ(S("-0-1-2---").popr('-', true), "2---");

    C4_EXPECT_EQ(S("0"        ).popr('-'      ), "0");
    C4_EXPECT_EQ(S("0"        ).popr('-', true), "0");
    C4_EXPECT_EQ(S("0-"       ).popr('-'      ), "");
    C4_EXPECT_EQ(S("0-"       ).popr('-', true), "0-");
    C4_EXPECT_EQ(S("0---"     ).popr('-'      ), "");
    C4_EXPECT_EQ(S("0---"     ).popr('-', true), "0---");
                              
    C4_EXPECT_EQ(S("-0"       ).popr('-'      ), "0");
    C4_EXPECT_EQ(S("-0"       ).popr('-', true), "0");
    C4_EXPECT_EQ(S("-0-"      ).popr('-'      ), "");
    C4_EXPECT_EQ(S("-0-"      ).popr('-', true), "0-");
    C4_EXPECT_EQ(S("-0---"    ).popr('-'      ), "");
    C4_EXPECT_EQ(S("-0---"    ).popr('-', true), "0---");
                              
    C4_EXPECT_EQ(S("-"        ).popr('-'      ), "");
    C4_EXPECT_EQ(S("-"        ).popr('-', true), "");
    C4_EXPECT_EQ(S("---"      ).popr('-'      ), "");
    C4_EXPECT_EQ(S("---"      ).popr('-', true), "");
                              
    C4_EXPECT_EQ(S(""         ).popr('-'      ), "");
    C4_EXPECT_EQ(S(""         ).popr('-', true), "");
}
template< class S >
void test_stringbase_popl() // pop left
{
    C4_EXPECT_EQ(S("0/1/2"    ).popl('/'      ), "0");
    C4_EXPECT_EQ(S("0/1/2"    ).popl('/', true), "0");
    C4_EXPECT_EQ(S("0/1/2/"   ).popl('/'      ), "0");
    C4_EXPECT_EQ(S("0/1/2/"   ).popl('/', true), "0");
    C4_EXPECT_EQ(S("0/1/2///" ).popl('/'      ), "0");
    C4_EXPECT_EQ(S("0/1/2///" ).popl('/', true), "0");

    C4_EXPECT_EQ(S("0//1/2"    ).popl('/'      ), "0");
    C4_EXPECT_EQ(S("0//1/2"    ).popl('/', true), "0");
    C4_EXPECT_EQ(S("0//1/2/"   ).popl('/'      ), "0");
    C4_EXPECT_EQ(S("0//1/2/"   ).popl('/', true), "0");
    C4_EXPECT_EQ(S("0//1/2///" ).popl('/'      ), "0");
    C4_EXPECT_EQ(S("0//1/2///" ).popl('/', true), "0");

    C4_EXPECT_EQ(S("0///1/2"    ).popl('/'      ), "0");
    C4_EXPECT_EQ(S("0///1/2"    ).popl('/', true), "0");
    C4_EXPECT_EQ(S("0///1/2/"   ).popl('/'      ), "0");
    C4_EXPECT_EQ(S("0///1/2/"   ).popl('/', true), "0");
    C4_EXPECT_EQ(S("0///1/2///" ).popl('/'      ), "0");
    C4_EXPECT_EQ(S("0///1/2///" ).popl('/', true), "0");

    C4_EXPECT_EQ(S("/0/1/2"   ).popl('/'      ), "");
    C4_EXPECT_EQ(S("/0/1/2"   ).popl('/', true), "/0");
    C4_EXPECT_EQ(S("/0/1/2/"  ).popl('/'      ), "");
    C4_EXPECT_EQ(S("/0/1/2/"  ).popl('/', true), "/0");
    C4_EXPECT_EQ(S("/0/1/2///").popl('/'      ), "");
    C4_EXPECT_EQ(S("/0/1/2///").popl('/', true), "/0");
    C4_EXPECT_EQ(S("///0/1/2" ).popl('/'      ), "");
    C4_EXPECT_EQ(S("///0/1/2" ).popl('/', true), "///0");
    C4_EXPECT_EQ(S("///0/1/2/").popl('/'      ), "");
    C4_EXPECT_EQ(S("///0/1/2/").popl('/', true), "///0");
    C4_EXPECT_EQ(S("///0/1/2/").popl('/'      ), "");
    C4_EXPECT_EQ(S("///0/1/2/").popl('/', true), "///0");

    C4_EXPECT_EQ(S("0"        ).popl('/'      ), "0");
    C4_EXPECT_EQ(S("0"        ).popl('/', true), "0");
    C4_EXPECT_EQ(S("0/"       ).popl('/'      ), "0");
    C4_EXPECT_EQ(S("0/"       ).popl('/', true), "0");
    C4_EXPECT_EQ(S("0///"     ).popl('/'      ), "0");
    C4_EXPECT_EQ(S("0///"     ).popl('/', true), "0");
                              
    C4_EXPECT_EQ(S("/0"       ).popl('/'      ), "");
    C4_EXPECT_EQ(S("/0"       ).popl('/', true), "/0");
    C4_EXPECT_EQ(S("/0/"      ).popl('/'      ), "");
    C4_EXPECT_EQ(S("/0/"      ).popl('/', true), "/0");
    C4_EXPECT_EQ(S("/0///"    ).popl('/'      ), "");
    C4_EXPECT_EQ(S("/0///"    ).popl('/', true), "/0");
    C4_EXPECT_EQ(S("///0///"  ).popl('/'      ), "");
    C4_EXPECT_EQ(S("///0///"  ).popl('/', true), "///0");
                
    C4_EXPECT_EQ(S("/"        ).popl('/'      ), "");
    C4_EXPECT_EQ(S("/"        ).popl('/', true), "");
    C4_EXPECT_EQ(S("///"      ).popl('/'      ), "");
    C4_EXPECT_EQ(S("///"      ).popl('/', true), "");
                              
    C4_EXPECT_EQ(S(""         ).popl('/'      ), "");
    C4_EXPECT_EQ(S(""         ).popl('/', true), "");

    C4_EXPECT_EQ(S("0-1-2"    ).popl('-'      ), "0");
    C4_EXPECT_EQ(S("0-1-2"    ).popl('-', true), "0");
    C4_EXPECT_EQ(S("0-1-2-"   ).popl('-'      ), "0");
    C4_EXPECT_EQ(S("0-1-2-"   ).popl('-', true), "0");
    C4_EXPECT_EQ(S("0-1-2---" ).popl('-'      ), "0");
    C4_EXPECT_EQ(S("0-1-2---" ).popl('-', true), "0");

    C4_EXPECT_EQ(S("0--1-2"    ).popl('-'      ), "0");
    C4_EXPECT_EQ(S("0--1-2"    ).popl('-', true), "0");
    C4_EXPECT_EQ(S("0--1-2-"   ).popl('-'      ), "0");
    C4_EXPECT_EQ(S("0--1-2-"   ).popl('-', true), "0");
    C4_EXPECT_EQ(S("0--1-2---" ).popl('-'      ), "0");
    C4_EXPECT_EQ(S("0--1-2---" ).popl('-', true), "0");

    C4_EXPECT_EQ(S("0---1-2"    ).popl('-'      ), "0");
    C4_EXPECT_EQ(S("0---1-2"    ).popl('-', true), "0");
    C4_EXPECT_EQ(S("0---1-2-"   ).popl('-'      ), "0");
    C4_EXPECT_EQ(S("0---1-2-"   ).popl('-', true), "0");
    C4_EXPECT_EQ(S("0---1-2---" ).popl('-'      ), "0");
    C4_EXPECT_EQ(S("0---1-2---" ).popl('-', true), "0");

    C4_EXPECT_EQ(S("-0-1-2"   ).popl('-'      ), "");
    C4_EXPECT_EQ(S("-0-1-2"   ).popl('-', true), "-0");
    C4_EXPECT_EQ(S("-0-1-2-"  ).popl('-'      ), "");
    C4_EXPECT_EQ(S("-0-1-2-"  ).popl('-', true), "-0");
    C4_EXPECT_EQ(S("-0-1-2---").popl('-'      ), "");
    C4_EXPECT_EQ(S("-0-1-2---").popl('-', true), "-0");
    C4_EXPECT_EQ(S("---0-1-2" ).popl('-'      ), "");
    C4_EXPECT_EQ(S("---0-1-2" ).popl('-', true), "---0");
    C4_EXPECT_EQ(S("---0-1-2-").popl('-'      ), "");
    C4_EXPECT_EQ(S("---0-1-2-").popl('-', true), "---0");
    C4_EXPECT_EQ(S("---0-1-2-").popl('-'      ), "");
    C4_EXPECT_EQ(S("---0-1-2-").popl('-', true), "---0");

    C4_EXPECT_EQ(S("0"        ).popl('-'      ), "0");
    C4_EXPECT_EQ(S("0"        ).popl('-', true), "0");
    C4_EXPECT_EQ(S("0-"       ).popl('-'      ), "0");
    C4_EXPECT_EQ(S("0-"       ).popl('-', true), "0");
    C4_EXPECT_EQ(S("0---"     ).popl('-'      ), "0");
    C4_EXPECT_EQ(S("0---"     ).popl('-', true), "0");

    C4_EXPECT_EQ(S("-0"       ).popl('-'      ), "");
    C4_EXPECT_EQ(S("-0"       ).popl('-', true), "-0");
    C4_EXPECT_EQ(S("-0-"      ).popl('-'      ), "");
    C4_EXPECT_EQ(S("-0-"      ).popl('-', true), "-0");
    C4_EXPECT_EQ(S("-0---"    ).popl('-'      ), "");
    C4_EXPECT_EQ(S("-0---"    ).popl('-', true), "-0");
    C4_EXPECT_EQ(S("---0---"  ).popl('-'      ), "");
    C4_EXPECT_EQ(S("---0---"  ).popl('-', true), "---0");

    C4_EXPECT_EQ(S("-"        ).popl('-'      ), "");
    C4_EXPECT_EQ(S("-"        ).popl('-', true), "");
    C4_EXPECT_EQ(S("---"      ).popl('-'      ), "");
    C4_EXPECT_EQ(S("---"      ).popl('-', true), "");

    C4_EXPECT_EQ(S(""         ).popl('-'      ), "");
    C4_EXPECT_EQ(S(""         ).popl('-', true), "");
}

template< class S >
void test_stringbase_gpopl() // greedy pop left
{
    C4_EXPECT_EQ(S("0/1/2"      ).gpopl('/'      ), "0/1");
    C4_EXPECT_EQ(S("0/1/2"      ).gpopl('/', true), "0/1");
    C4_EXPECT_EQ(S("0/1/2/"     ).gpopl('/'      ), "0/1/2");
    C4_EXPECT_EQ(S("0/1/2/"     ).gpopl('/', true), "0/1");
    C4_EXPECT_EQ(S("0/1/2//"    ).gpopl('/'      ), "0/1/2/");
    C4_EXPECT_EQ(S("0/1/2//"    ).gpopl('/', true), "0/1");
    C4_EXPECT_EQ(S("0/1/2///"   ).gpopl('/'      ), "0/1/2//");
    C4_EXPECT_EQ(S("0/1/2///"   ).gpopl('/', true), "0/1");

    C4_EXPECT_EQ(S("0/1//2"     ).gpopl('/'      ), "0/1/");
    C4_EXPECT_EQ(S("0/1//2"     ).gpopl('/', true), "0/1");
    C4_EXPECT_EQ(S("0/1//2/"    ).gpopl('/'      ), "0/1//2");
    C4_EXPECT_EQ(S("0/1//2/"    ).gpopl('/', true), "0/1");
    C4_EXPECT_EQ(S("0/1//2//"   ).gpopl('/'      ), "0/1//2/");
    C4_EXPECT_EQ(S("0/1//2//"   ).gpopl('/', true), "0/1");
    C4_EXPECT_EQ(S("0/1//2///"  ).gpopl('/'      ), "0/1//2//");
    C4_EXPECT_EQ(S("0/1//2///"  ).gpopl('/', true), "0/1");

    C4_EXPECT_EQ(S("0/1///2"    ).gpopl('/'      ), "0/1//");
    C4_EXPECT_EQ(S("0/1///2"    ).gpopl('/', true), "0/1");
    C4_EXPECT_EQ(S("0/1///2/"   ).gpopl('/'      ), "0/1///2");
    C4_EXPECT_EQ(S("0/1///2/"   ).gpopl('/', true), "0/1");
    C4_EXPECT_EQ(S("0/1///2//"  ).gpopl('/'      ), "0/1///2/");
    C4_EXPECT_EQ(S("0/1///2//"  ).gpopl('/', true), "0/1");
    C4_EXPECT_EQ(S("0/1///2///" ).gpopl('/'      ), "0/1///2//");
    C4_EXPECT_EQ(S("0/1///2///" ).gpopl('/', true), "0/1");

    C4_EXPECT_EQ(S("/0/1/2"     ).gpopl('/'      ), "/0/1");
    C4_EXPECT_EQ(S("/0/1/2"     ).gpopl('/', true), "/0/1");
    C4_EXPECT_EQ(S("/0/1/2/"    ).gpopl('/'      ), "/0/1/2");
    C4_EXPECT_EQ(S("/0/1/2/"    ).gpopl('/', true), "/0/1");
    C4_EXPECT_EQ(S("/0/1/2//"   ).gpopl('/'      ), "/0/1/2/");
    C4_EXPECT_EQ(S("/0/1/2//"   ).gpopl('/', true), "/0/1");
    C4_EXPECT_EQ(S("/0/1/2///"  ).gpopl('/'      ), "/0/1/2//");
    C4_EXPECT_EQ(S("/0/1/2///"  ).gpopl('/', true), "/0/1");

    C4_EXPECT_EQ(S("//0/1/2"    ).gpopl('/'      ), "//0/1");
    C4_EXPECT_EQ(S("//0/1/2"    ).gpopl('/', true), "//0/1");
    C4_EXPECT_EQ(S("//0/1/2/"   ).gpopl('/'      ), "//0/1/2");
    C4_EXPECT_EQ(S("//0/1/2/"   ).gpopl('/', true), "//0/1");
    C4_EXPECT_EQ(S("//0/1/2//"  ).gpopl('/'      ), "//0/1/2/");
    C4_EXPECT_EQ(S("//0/1/2//"  ).gpopl('/', true), "//0/1");
    C4_EXPECT_EQ(S("//0/1/2///" ).gpopl('/'      ), "//0/1/2//");
    C4_EXPECT_EQ(S("//0/1/2///" ).gpopl('/', true), "//0/1");

    C4_EXPECT_EQ(S("///0/1/2"   ).gpopl('/'      ), "///0/1");
    C4_EXPECT_EQ(S("///0/1/2"   ).gpopl('/', true), "///0/1");
    C4_EXPECT_EQ(S("///0/1/2/"  ).gpopl('/'      ), "///0/1/2");
    C4_EXPECT_EQ(S("///0/1/2/"  ).gpopl('/', true), "///0/1");
    C4_EXPECT_EQ(S("///0/1/2//" ).gpopl('/'      ), "///0/1/2/");
    C4_EXPECT_EQ(S("///0/1/2//" ).gpopl('/', true), "///0/1");
    C4_EXPECT_EQ(S("///0/1/2///").gpopl('/'      ), "///0/1/2//");
    C4_EXPECT_EQ(S("///0/1/2///").gpopl('/', true), "///0/1");


    C4_EXPECT_EQ(S("0/1"      ).gpopl('/'      ), "0");
    C4_EXPECT_EQ(S("0/1"      ).gpopl('/', true), "0");
    C4_EXPECT_EQ(S("0/1/"     ).gpopl('/'      ), "0/1");
    C4_EXPECT_EQ(S("0/1/"     ).gpopl('/', true), "0");
    C4_EXPECT_EQ(S("0/1//"    ).gpopl('/'      ), "0/1/");
    C4_EXPECT_EQ(S("0/1//"    ).gpopl('/', true), "0");
    C4_EXPECT_EQ(S("0/1///"   ).gpopl('/'      ), "0/1//");
    C4_EXPECT_EQ(S("0/1///"   ).gpopl('/', true), "0");

    C4_EXPECT_EQ(S("0//1"     ).gpopl('/'      ), "0/");
    C4_EXPECT_EQ(S("0//1"     ).gpopl('/', true), "0");
    C4_EXPECT_EQ(S("0//1/"    ).gpopl('/'      ), "0//1");
    C4_EXPECT_EQ(S("0//1/"    ).gpopl('/', true), "0");
    C4_EXPECT_EQ(S("0//1//"   ).gpopl('/'      ), "0//1/");
    C4_EXPECT_EQ(S("0//1//"   ).gpopl('/', true), "0");
    C4_EXPECT_EQ(S("0//1///"  ).gpopl('/'      ), "0//1//");
    C4_EXPECT_EQ(S("0//1///"  ).gpopl('/', true), "0");

    C4_EXPECT_EQ(S("0///1"    ).gpopl('/'      ), "0//");
    C4_EXPECT_EQ(S("0///1"    ).gpopl('/', true), "0");
    C4_EXPECT_EQ(S("0///1/"   ).gpopl('/'      ), "0///1");
    C4_EXPECT_EQ(S("0///1/"   ).gpopl('/', true), "0");
    C4_EXPECT_EQ(S("0///1//"  ).gpopl('/'      ), "0///1/");
    C4_EXPECT_EQ(S("0///1//"  ).gpopl('/', true), "0");
    C4_EXPECT_EQ(S("0///1///" ).gpopl('/'      ), "0///1//");
    C4_EXPECT_EQ(S("0///1///" ).gpopl('/', true), "0");

    C4_EXPECT_EQ(S("/0/1"      ).gpopl('/'      ), "/0");
    C4_EXPECT_EQ(S("/0/1"      ).gpopl('/', true), "/0");
    C4_EXPECT_EQ(S("/0/1/"     ).gpopl('/'      ), "/0/1");
    C4_EXPECT_EQ(S("/0/1/"     ).gpopl('/', true), "/0");
    C4_EXPECT_EQ(S("/0/1//"    ).gpopl('/'      ), "/0/1/");
    C4_EXPECT_EQ(S("/0/1//"    ).gpopl('/', true), "/0");
    C4_EXPECT_EQ(S("/0/1///"   ).gpopl('/'      ), "/0/1//");
    C4_EXPECT_EQ(S("/0/1///"   ).gpopl('/', true), "/0");

    C4_EXPECT_EQ(S("/0//1"     ).gpopl('/'      ), "/0/");
    C4_EXPECT_EQ(S("/0//1"     ).gpopl('/', true), "/0");
    C4_EXPECT_EQ(S("/0//1/"    ).gpopl('/'      ), "/0//1");
    C4_EXPECT_EQ(S("/0//1/"    ).gpopl('/', true), "/0");
    C4_EXPECT_EQ(S("/0//1//"   ).gpopl('/'      ), "/0//1/");
    C4_EXPECT_EQ(S("/0//1//"   ).gpopl('/', true), "/0");
    C4_EXPECT_EQ(S("/0//1///"  ).gpopl('/'      ), "/0//1//");
    C4_EXPECT_EQ(S("/0//1///"  ).gpopl('/', true), "/0");

    C4_EXPECT_EQ(S("/0///1"    ).gpopl('/'      ), "/0//");
    C4_EXPECT_EQ(S("/0///1"    ).gpopl('/', true), "/0");
    C4_EXPECT_EQ(S("/0///1/"   ).gpopl('/'      ), "/0///1");
    C4_EXPECT_EQ(S("/0///1/"   ).gpopl('/', true), "/0");
    C4_EXPECT_EQ(S("/0///1//"  ).gpopl('/'      ), "/0///1/");
    C4_EXPECT_EQ(S("/0///1//"  ).gpopl('/', true), "/0");
    C4_EXPECT_EQ(S("/0///1///" ).gpopl('/'      ), "/0///1//");
    C4_EXPECT_EQ(S("/0///1///" ).gpopl('/', true), "/0");

    C4_EXPECT_EQ(S("//0/1"      ).gpopl('/'      ), "//0");
    C4_EXPECT_EQ(S("//0/1"      ).gpopl('/', true), "//0");
    C4_EXPECT_EQ(S("//0/1/"     ).gpopl('/'      ), "//0/1");
    C4_EXPECT_EQ(S("//0/1/"     ).gpopl('/', true), "//0");
    C4_EXPECT_EQ(S("//0/1//"    ).gpopl('/'      ), "//0/1/");
    C4_EXPECT_EQ(S("//0/1//"    ).gpopl('/', true), "//0");
    C4_EXPECT_EQ(S("//0/1///"   ).gpopl('/'      ), "//0/1//");
    C4_EXPECT_EQ(S("//0/1///"   ).gpopl('/', true), "//0");

    C4_EXPECT_EQ(S("//0//1"     ).gpopl('/'      ), "//0/");
    C4_EXPECT_EQ(S("//0//1"     ).gpopl('/', true), "//0");
    C4_EXPECT_EQ(S("//0//1/"    ).gpopl('/'      ), "//0//1");
    C4_EXPECT_EQ(S("//0//1/"    ).gpopl('/', true), "//0");
    C4_EXPECT_EQ(S("//0//1//"   ).gpopl('/'      ), "//0//1/");
    C4_EXPECT_EQ(S("//0//1//"   ).gpopl('/', true), "//0");
    C4_EXPECT_EQ(S("//0//1///"  ).gpopl('/'      ), "//0//1//");
    C4_EXPECT_EQ(S("//0//1///"  ).gpopl('/', true), "//0");

    C4_EXPECT_EQ(S("//0///1"    ).gpopl('/'      ), "//0//");
    C4_EXPECT_EQ(S("//0///1"    ).gpopl('/', true), "//0");
    C4_EXPECT_EQ(S("//0///1/"   ).gpopl('/'      ), "//0///1");
    C4_EXPECT_EQ(S("//0///1/"   ).gpopl('/', true), "//0");
    C4_EXPECT_EQ(S("//0///1//"  ).gpopl('/'      ), "//0///1/");
    C4_EXPECT_EQ(S("//0///1//"  ).gpopl('/', true), "//0");
    C4_EXPECT_EQ(S("//0///1///" ).gpopl('/'      ), "//0///1//");
    C4_EXPECT_EQ(S("//0///1///" ).gpopl('/', true), "//0");

    C4_EXPECT_EQ(S("0"      ).gpopl('/'      ), "");
    C4_EXPECT_EQ(S("0"      ).gpopl('/', true), "");
    C4_EXPECT_EQ(S("0/"     ).gpopl('/'      ), "0");
    C4_EXPECT_EQ(S("0/"     ).gpopl('/', true), "");
    C4_EXPECT_EQ(S("0//"    ).gpopl('/'      ), "0/");
    C4_EXPECT_EQ(S("0//"    ).gpopl('/', true), "");
    C4_EXPECT_EQ(S("0///"   ).gpopl('/'      ), "0//");
    C4_EXPECT_EQ(S("0///"   ).gpopl('/', true), "");

    C4_EXPECT_EQ(S("/0"      ).gpopl('/'      ), "");
    C4_EXPECT_EQ(S("/0"      ).gpopl('/', true), "");
    C4_EXPECT_EQ(S("/0/"     ).gpopl('/'      ), "/0");
    C4_EXPECT_EQ(S("/0/"     ).gpopl('/', true), "");
    C4_EXPECT_EQ(S("/0//"    ).gpopl('/'      ), "/0/");
    C4_EXPECT_EQ(S("/0//"    ).gpopl('/', true), "");
    C4_EXPECT_EQ(S("/0///"   ).gpopl('/'      ), "/0//");
    C4_EXPECT_EQ(S("/0///"   ).gpopl('/', true), "");

    C4_EXPECT_EQ(S("//0"      ).gpopl('/'      ), "/");
    C4_EXPECT_EQ(S("//0"      ).gpopl('/', true), "");
    C4_EXPECT_EQ(S("//0/"     ).gpopl('/'      ), "//0");
    C4_EXPECT_EQ(S("//0/"     ).gpopl('/', true), "");
    C4_EXPECT_EQ(S("//0//"    ).gpopl('/'      ), "//0/");
    C4_EXPECT_EQ(S("//0//"    ).gpopl('/', true), "");
    C4_EXPECT_EQ(S("//0///"   ).gpopl('/'      ), "//0//");
    C4_EXPECT_EQ(S("//0///"   ).gpopl('/', true), "");

    C4_EXPECT_EQ(S("///0"      ).gpopl('/'      ), "//");
    C4_EXPECT_EQ(S("///0"      ).gpopl('/', true), "");
    C4_EXPECT_EQ(S("///0/"     ).gpopl('/'      ), "///0");
    C4_EXPECT_EQ(S("///0/"     ).gpopl('/', true), "");
    C4_EXPECT_EQ(S("///0//"    ).gpopl('/'      ), "///0/");
    C4_EXPECT_EQ(S("///0//"    ).gpopl('/', true), "");
    C4_EXPECT_EQ(S("///0///"   ).gpopl('/'      ), "///0//");
    C4_EXPECT_EQ(S("///0///"   ).gpopl('/', true), "");

    C4_EXPECT_EQ(S("/"        ).gpopl('/'      ), "");
    C4_EXPECT_EQ(S("/"        ).gpopl('/', true), "");
    C4_EXPECT_EQ(S("//"       ).gpopl('/'      ), "/");
    C4_EXPECT_EQ(S("//"       ).gpopl('/', true), "");
    C4_EXPECT_EQ(S("///"      ).gpopl('/'      ), "//");
    C4_EXPECT_EQ(S("///"      ).gpopl('/', true), "");

    C4_EXPECT_EQ(S(""         ).gpopl('/'      ), "");
    C4_EXPECT_EQ(S(""         ).gpopl('/', true), "");
}
template< class S >
void test_stringbase_gpopr()
{
    C4_EXPECT_EQ(S("0/1/2"      ).gpopr('/'      ), "1/2");
    C4_EXPECT_EQ(S("0/1/2"      ).gpopr('/', true), "1/2");
    C4_EXPECT_EQ(S("0/1/2/"     ).gpopr('/'      ), "1/2/");
    C4_EXPECT_EQ(S("0/1/2/"     ).gpopr('/', true), "1/2/");
    C4_EXPECT_EQ(S("0/1/2//"    ).gpopr('/'      ), "1/2//");
    C4_EXPECT_EQ(S("0/1/2//"    ).gpopr('/', true), "1/2//");
    C4_EXPECT_EQ(S("0/1/2///"   ).gpopr('/'      ), "1/2///");
    C4_EXPECT_EQ(S("0/1/2///"   ).gpopr('/', true), "1/2///");

    C4_EXPECT_EQ(S("0//1/2"     ).gpopr('/'      ), "/1/2");
    C4_EXPECT_EQ(S("0//1/2"     ).gpopr('/', true),  "1/2");
    C4_EXPECT_EQ(S("0//1/2/"    ).gpopr('/'      ), "/1/2/");
    C4_EXPECT_EQ(S("0//1/2/"    ).gpopr('/', true),  "1/2/");
    C4_EXPECT_EQ(S("0//1/2//"   ).gpopr('/'      ), "/1/2//");
    C4_EXPECT_EQ(S("0//1/2//"   ).gpopr('/', true),  "1/2//");
    C4_EXPECT_EQ(S("0//1/2///"  ).gpopr('/'      ), "/1/2///");
    C4_EXPECT_EQ(S("0//1/2///"  ).gpopr('/', true),  "1/2///");

    C4_EXPECT_EQ(S("0///1/2"     ).gpopr('/'      ), "//1/2");
    C4_EXPECT_EQ(S("0///1/2"     ).gpopr('/', true),   "1/2");
    C4_EXPECT_EQ(S("0///1/2/"    ).gpopr('/'      ), "//1/2/");
    C4_EXPECT_EQ(S("0///1/2/"    ).gpopr('/', true),   "1/2/");
    C4_EXPECT_EQ(S("0///1/2//"   ).gpopr('/'      ), "//1/2//");
    C4_EXPECT_EQ(S("0///1/2//"   ).gpopr('/', true),   "1/2//");
    C4_EXPECT_EQ(S("0///1/2///"  ).gpopr('/'      ), "//1/2///");
    C4_EXPECT_EQ(S("0///1/2///"  ).gpopr('/', true),   "1/2///");


    C4_EXPECT_EQ(S("/0/1/2"      ).gpopr('/'      ), "0/1/2");
    C4_EXPECT_EQ(S("/0/1/2"      ).gpopr('/', true),   "1/2");
    C4_EXPECT_EQ(S("/0/1/2/"     ).gpopr('/'      ), "0/1/2/");
    C4_EXPECT_EQ(S("/0/1/2/"     ).gpopr('/', true),   "1/2/");
    C4_EXPECT_EQ(S("/0/1/2//"    ).gpopr('/'      ), "0/1/2//");
    C4_EXPECT_EQ(S("/0/1/2//"    ).gpopr('/', true),   "1/2//");
    C4_EXPECT_EQ(S("/0/1/2///"   ).gpopr('/'      ), "0/1/2///");
    C4_EXPECT_EQ(S("/0/1/2///"   ).gpopr('/', true),   "1/2///");

    C4_EXPECT_EQ(S("/0//1/2"     ).gpopr('/'      ), "0//1/2");
    C4_EXPECT_EQ(S("/0//1/2"     ).gpopr('/', true),    "1/2");
    C4_EXPECT_EQ(S("/0//1/2/"    ).gpopr('/'      ), "0//1/2/");
    C4_EXPECT_EQ(S("/0//1/2/"    ).gpopr('/', true),    "1/2/");
    C4_EXPECT_EQ(S("/0//1/2//"   ).gpopr('/'      ), "0//1/2//");
    C4_EXPECT_EQ(S("/0//1/2//"   ).gpopr('/', true),    "1/2//");
    C4_EXPECT_EQ(S("/0//1/2///"  ).gpopr('/'      ), "0//1/2///");
    C4_EXPECT_EQ(S("/0//1/2///"  ).gpopr('/', true),    "1/2///");

    C4_EXPECT_EQ(S("/0///1/2"     ).gpopr('/'      ), "0///1/2");
    C4_EXPECT_EQ(S("/0///1/2"     ).gpopr('/', true),     "1/2");
    C4_EXPECT_EQ(S("/0///1/2/"    ).gpopr('/'      ), "0///1/2/");
    C4_EXPECT_EQ(S("/0///1/2/"    ).gpopr('/', true),     "1/2/");
    C4_EXPECT_EQ(S("/0///1/2//"   ).gpopr('/'      ), "0///1/2//");
    C4_EXPECT_EQ(S("/0///1/2//"   ).gpopr('/', true),     "1/2//");
    C4_EXPECT_EQ(S("/0///1/2///"  ).gpopr('/'      ), "0///1/2///");
    C4_EXPECT_EQ(S("/0///1/2///"  ).gpopr('/', true),     "1/2///");


    C4_EXPECT_EQ(S("//0/1/2"      ).gpopr('/'      ), "/0/1/2");
    C4_EXPECT_EQ(S("//0/1/2"      ).gpopr('/', true),    "1/2");
    C4_EXPECT_EQ(S("//0/1/2/"     ).gpopr('/'      ), "/0/1/2/");
    C4_EXPECT_EQ(S("//0/1/2/"     ).gpopr('/', true),    "1/2/");
    C4_EXPECT_EQ(S("//0/1/2//"    ).gpopr('/'      ), "/0/1/2//");
    C4_EXPECT_EQ(S("//0/1/2//"    ).gpopr('/', true),    "1/2//");
    C4_EXPECT_EQ(S("//0/1/2///"   ).gpopr('/'      ), "/0/1/2///");
    C4_EXPECT_EQ(S("//0/1/2///"   ).gpopr('/', true),    "1/2///");

    C4_EXPECT_EQ(S("//0//1/2"     ).gpopr('/'      ), "/0//1/2");
    C4_EXPECT_EQ(S("//0//1/2"     ).gpopr('/', true),     "1/2");
    C4_EXPECT_EQ(S("//0//1/2/"    ).gpopr('/'      ), "/0//1/2/");
    C4_EXPECT_EQ(S("//0//1/2/"    ).gpopr('/', true),     "1/2/");
    C4_EXPECT_EQ(S("//0//1/2//"   ).gpopr('/'      ), "/0//1/2//");
    C4_EXPECT_EQ(S("//0//1/2//"   ).gpopr('/', true),     "1/2//");
    C4_EXPECT_EQ(S("//0//1/2///"  ).gpopr('/'      ), "/0//1/2///");
    C4_EXPECT_EQ(S("//0//1/2///"  ).gpopr('/', true),     "1/2///");

    C4_EXPECT_EQ(S("//0///1/2"     ).gpopr('/'      ), "/0///1/2");
    C4_EXPECT_EQ(S("//0///1/2"     ).gpopr('/', true),     "1/2");
    C4_EXPECT_EQ(S("//0///1/2/"    ).gpopr('/'      ), "/0///1/2/");
    C4_EXPECT_EQ(S("//0///1/2/"    ).gpopr('/', true),     "1/2/");
    C4_EXPECT_EQ(S("//0///1/2//"   ).gpopr('/'      ), "/0///1/2//");
    C4_EXPECT_EQ(S("//0///1/2//"   ).gpopr('/', true),     "1/2//");
    C4_EXPECT_EQ(S("//0///1/2///"  ).gpopr('/'      ), "/0///1/2///");
    C4_EXPECT_EQ(S("//0///1/2///"  ).gpopr('/', true),      "1/2///");


    C4_EXPECT_EQ(S("0/1"      ).gpopr('/'      ), "1");
    C4_EXPECT_EQ(S("0/1"      ).gpopr('/', true), "1");
    C4_EXPECT_EQ(S("0/1/"     ).gpopr('/'      ), "1/");
    C4_EXPECT_EQ(S("0/1/"     ).gpopr('/', true), "1/");
    C4_EXPECT_EQ(S("0/1//"    ).gpopr('/'      ), "1//");
    C4_EXPECT_EQ(S("0/1//"    ).gpopr('/', true), "1//");
    C4_EXPECT_EQ(S("0/1///"   ).gpopr('/'      ), "1///");
    C4_EXPECT_EQ(S("0/1///"   ).gpopr('/', true), "1///");

    C4_EXPECT_EQ(S("0//1"     ).gpopr('/'      ), "/1");
    C4_EXPECT_EQ(S("0//1"     ).gpopr('/', true),  "1");
    C4_EXPECT_EQ(S("0//1/"    ).gpopr('/'      ), "/1/");
    C4_EXPECT_EQ(S("0//1/"    ).gpopr('/', true),  "1/");
    C4_EXPECT_EQ(S("0//1//"   ).gpopr('/'      ), "/1//");
    C4_EXPECT_EQ(S("0//1//"   ).gpopr('/', true),  "1//");
    C4_EXPECT_EQ(S("0//1///"  ).gpopr('/'      ), "/1///");
    C4_EXPECT_EQ(S("0//1///"  ).gpopr('/', true),  "1///");

    C4_EXPECT_EQ(S("0///1"    ).gpopr('/'      ), "//1");
    C4_EXPECT_EQ(S("0///1"    ).gpopr('/', true),   "1");
    C4_EXPECT_EQ(S("0///1/"   ).gpopr('/'      ), "//1/");
    C4_EXPECT_EQ(S("0///1/"   ).gpopr('/', true),   "1/");
    C4_EXPECT_EQ(S("0///1//"  ).gpopr('/'      ), "//1//");
    C4_EXPECT_EQ(S("0///1//"  ).gpopr('/', true),   "1//");
    C4_EXPECT_EQ(S("0///1///" ).gpopr('/'      ), "//1///");
    C4_EXPECT_EQ(S("0///1///" ).gpopr('/', true),   "1///");


    C4_EXPECT_EQ(S("/0/1"      ).gpopr('/'      ), "0/1");
    C4_EXPECT_EQ(S("/0/1"      ).gpopr('/', true),   "1");
    C4_EXPECT_EQ(S("/0/1/"     ).gpopr('/'      ), "0/1/");
    C4_EXPECT_EQ(S("/0/1/"     ).gpopr('/', true),   "1/");
    C4_EXPECT_EQ(S("/0/1//"    ).gpopr('/'      ), "0/1//");
    C4_EXPECT_EQ(S("/0/1//"    ).gpopr('/', true),   "1//");
    C4_EXPECT_EQ(S("/0/1///"   ).gpopr('/'      ), "0/1///");
    C4_EXPECT_EQ(S("/0/1///"   ).gpopr('/', true),   "1///");

    C4_EXPECT_EQ(S("/0//1"     ).gpopr('/'      ), "0//1");
    C4_EXPECT_EQ(S("/0//1"     ).gpopr('/', true),    "1");
    C4_EXPECT_EQ(S("/0//1/"    ).gpopr('/'      ), "0//1/");
    C4_EXPECT_EQ(S("/0//1/"    ).gpopr('/', true),    "1/");
    C4_EXPECT_EQ(S("/0//1//"   ).gpopr('/'      ), "0//1//");
    C4_EXPECT_EQ(S("/0//1//"   ).gpopr('/', true),    "1//");
    C4_EXPECT_EQ(S("/0//1///"  ).gpopr('/'      ), "0//1///");
    C4_EXPECT_EQ(S("/0//1///"  ).gpopr('/', true),    "1///");

    C4_EXPECT_EQ(S("/0///1"    ).gpopr('/'      ), "0///1");
    C4_EXPECT_EQ(S("/0///1"    ).gpopr('/', true),     "1");
    C4_EXPECT_EQ(S("/0///1/"   ).gpopr('/'      ), "0///1/");
    C4_EXPECT_EQ(S("/0///1/"   ).gpopr('/', true),     "1/");
    C4_EXPECT_EQ(S("/0///1//"  ).gpopr('/'      ), "0///1//");
    C4_EXPECT_EQ(S("/0///1//"  ).gpopr('/', true),     "1//");
    C4_EXPECT_EQ(S("/0///1///" ).gpopr('/'      ), "0///1///");
    C4_EXPECT_EQ(S("/0///1///" ).gpopr('/', true),     "1///");


    C4_EXPECT_EQ(S("//0/1"      ).gpopr('/'      ), "/0/1");
    C4_EXPECT_EQ(S("//0/1"      ).gpopr('/', true),    "1");
    C4_EXPECT_EQ(S("//0/1/"     ).gpopr('/'      ), "/0/1/");
    C4_EXPECT_EQ(S("//0/1/"     ).gpopr('/', true),    "1/");
    C4_EXPECT_EQ(S("//0/1//"    ).gpopr('/'      ), "/0/1//");
    C4_EXPECT_EQ(S("//0/1//"    ).gpopr('/', true),    "1//");
    C4_EXPECT_EQ(S("//0/1///"   ).gpopr('/'      ), "/0/1///");
    C4_EXPECT_EQ(S("//0/1///"   ).gpopr('/', true),    "1///");

    C4_EXPECT_EQ(S("//0//1"     ).gpopr('/'      ), "/0//1");
    C4_EXPECT_EQ(S("//0//1"     ).gpopr('/', true),     "1");
    C4_EXPECT_EQ(S("//0//1/"    ).gpopr('/'      ), "/0//1/");
    C4_EXPECT_EQ(S("//0//1/"    ).gpopr('/', true),     "1/");
    C4_EXPECT_EQ(S("//0//1//"   ).gpopr('/'      ), "/0//1//");
    C4_EXPECT_EQ(S("//0//1//"   ).gpopr('/', true),     "1//");
    C4_EXPECT_EQ(S("//0//1///"  ).gpopr('/'      ), "/0//1///");
    C4_EXPECT_EQ(S("//0//1///"  ).gpopr('/', true),     "1///");

    C4_EXPECT_EQ(S("//0///1"    ).gpopr('/'      ), "/0///1");
    C4_EXPECT_EQ(S("//0///1"    ).gpopr('/', true),      "1");
    C4_EXPECT_EQ(S("//0///1/"   ).gpopr('/'      ), "/0///1/");
    C4_EXPECT_EQ(S("//0///1/"   ).gpopr('/', true),      "1/");
    C4_EXPECT_EQ(S("//0///1//"  ).gpopr('/'      ), "/0///1//");
    C4_EXPECT_EQ(S("//0///1//"  ).gpopr('/', true),      "1//");
    C4_EXPECT_EQ(S("//0///1///" ).gpopr('/'      ), "/0///1///");
    C4_EXPECT_EQ(S("//0///1///" ).gpopr('/', true),      "1///");


    C4_EXPECT_EQ(S("0"      ).gpopr('/'      ), "");
    C4_EXPECT_EQ(S("0"      ).gpopr('/', true), "");
    C4_EXPECT_EQ(S("0/"     ).gpopr('/'      ), "");
    C4_EXPECT_EQ(S("0/"     ).gpopr('/', true), "");
    C4_EXPECT_EQ(S("0//"    ).gpopr('/'      ), "/");
    C4_EXPECT_EQ(S("0//"    ).gpopr('/', true), "");
    C4_EXPECT_EQ(S("0///"   ).gpopr('/'      ), "//");
    C4_EXPECT_EQ(S("0///"   ).gpopr('/', true), "");

    C4_EXPECT_EQ(S("/0"      ).gpopr('/'      ), "0");
    C4_EXPECT_EQ(S("/0"      ).gpopr('/', true), "");
    C4_EXPECT_EQ(S("/0/"     ).gpopr('/'      ), "0/");
    C4_EXPECT_EQ(S("/0/"     ).gpopr('/', true), "");
    C4_EXPECT_EQ(S("/0//"    ).gpopr('/'      ), "0//");
    C4_EXPECT_EQ(S("/0//"    ).gpopr('/', true), "");
    C4_EXPECT_EQ(S("/0///"   ).gpopr('/'      ), "0///");
    C4_EXPECT_EQ(S("/0///"   ).gpopr('/', true), "");

    C4_EXPECT_EQ(S("//0"      ).gpopr('/'      ), "/0");
    C4_EXPECT_EQ(S("//0"      ).gpopr('/', true), "");
    C4_EXPECT_EQ(S("//0/"     ).gpopr('/'      ), "/0/");
    C4_EXPECT_EQ(S("//0/"     ).gpopr('/', true), "");
    C4_EXPECT_EQ(S("//0//"    ).gpopr('/'      ), "/0//");
    C4_EXPECT_EQ(S("//0//"    ).gpopr('/', true), "");
    C4_EXPECT_EQ(S("//0///"   ).gpopr('/'      ), "/0///");
    C4_EXPECT_EQ(S("//0///"   ).gpopr('/', true), "");

    C4_EXPECT_EQ(S("///0"      ).gpopr('/'      ), "//0");
    C4_EXPECT_EQ(S("///0"      ).gpopr('/', true), "");
    C4_EXPECT_EQ(S("///0/"     ).gpopr('/'      ), "//0/");
    C4_EXPECT_EQ(S("///0/"     ).gpopr('/', true), "");
    C4_EXPECT_EQ(S("///0//"    ).gpopr('/'      ), "//0//");
    C4_EXPECT_EQ(S("///0//"    ).gpopr('/', true), "");
    C4_EXPECT_EQ(S("///0///"   ).gpopr('/'      ), "//0///");
    C4_EXPECT_EQ(S("///0///"   ).gpopr('/', true), "");

    C4_EXPECT_EQ(S("/"        ).gpopr('/'      ), "");
    C4_EXPECT_EQ(S("/"        ).gpopr('/', true), "");
    C4_EXPECT_EQ(S("//"       ).gpopr('/'      ), "/");
    C4_EXPECT_EQ(S("//"       ).gpopr('/', true), "");
    C4_EXPECT_EQ(S("///"      ).gpopr('/'      ), "//");
    C4_EXPECT_EQ(S("///"      ).gpopr('/', true), "");

    C4_EXPECT_EQ(S(""         ).gpopr('/'      ), "");
    C4_EXPECT_EQ(S(""         ).gpopr('/', true), "");
}

template< class S >
void test_stringbase_basename()
{
    C4_EXPECT_EQ(S("0/1/2").basename(), "2");
    C4_EXPECT_EQ(S("0/1/2/").basename(), "2");
    C4_EXPECT_EQ(S("0/1/2///").basename(), "2");
    C4_EXPECT_EQ(S("/0/1/2").basename(), "2");
    C4_EXPECT_EQ(S("/0/1/2/").basename(), "2");
    C4_EXPECT_EQ(S("/0/1/2///").basename(), "2");
    C4_EXPECT_EQ(S("///0/1/2").basename(), "2");
    C4_EXPECT_EQ(S("///0/1/2/").basename(), "2");
    C4_EXPECT_EQ(S("///0/1/2///").basename(), "2");
    C4_EXPECT_EQ(S("/").basename(), "");
    C4_EXPECT_EQ(S("//").basename(), "");
    C4_EXPECT_EQ(S("///").basename(), "");
    C4_EXPECT_EQ(S("////").basename(), "");
    C4_EXPECT_EQ(S("").basename(), "");
}
template< class S >
void test_stringbase_dirname()
{
    C4_EXPECT_EQ(S("0/1/2").dirname(), "0/1/");
    C4_EXPECT_EQ(S("0/1/2/").dirname(), "0/1/");
    C4_EXPECT_EQ(S("/0/1/2").dirname(), "/0/1/");
    C4_EXPECT_EQ(S("/0/1/2/").dirname(), "/0/1/");
    C4_EXPECT_EQ(S("///0/1/2").dirname(), "///0/1/");
    C4_EXPECT_EQ(S("///0/1/2/").dirname(), "///0/1/");
    C4_EXPECT_EQ(S("/0").dirname(), "/");
    C4_EXPECT_EQ(S("/").dirname(), "/");
    C4_EXPECT_EQ(S("//").dirname(), "//");
    C4_EXPECT_EQ(S("///").dirname(), "///");
    C4_EXPECT_EQ(S("////").dirname(), "////");
    C4_EXPECT_EQ(S("").dirname(), "");
}

template< typename S >
void test_stringbase_next_split()
{
    using sstype = typename S::subcont_type;

    {
        S const n;
        typename S::size_type pos = 0;
        sstype ss;
        C4_EXPECT_EQ(n.next_split(':', &pos, &ss), false);
        C4_EXPECT_EQ(ss.empty(), true);
        C4_EXPECT_EQ(n.next_split(':', &pos, &ss), false);
        C4_EXPECT_EQ(ss.empty(), true);
        pos = 0;
        C4_EXPECT_EQ(n.next_split(',', &pos, &ss), false);
        C4_EXPECT_EQ(ss.empty(), true);
        C4_EXPECT_EQ(n.next_split(',', &pos, &ss), false);
        C4_EXPECT_EQ(ss.empty(), true);
    }

    {
        S const n("0");
        typename S::size_type pos = 0;
        sstype ss;
        C4_EXPECT_EQ(n.next_split(':', &pos, &ss), true);
        C4_EXPECT_EQ(ss.empty(), false);
        C4_EXPECT_EQ(n.next_split(':', &pos, &ss), false);
        C4_EXPECT_EQ(ss.empty(), true);
        C4_EXPECT_EQ(n.next_split(':', &pos, &ss), false);
        C4_EXPECT_EQ(ss.empty(), true);
        pos = 0;
        C4_EXPECT_EQ(n.next_split(',', &pos, &ss), true);
        C4_EXPECT_EQ(ss.empty(), false);
        C4_EXPECT_EQ(n.next_split(',', &pos, &ss), false);
        C4_EXPECT_EQ(ss.empty(), true);
        C4_EXPECT_EQ(n.next_split(',', &pos, &ss), false);
        C4_EXPECT_EQ(ss.empty(), true);
    }

    {
        S const n;
        C4_EXPECT_EQ(n.num_splits(':'), 0);
        typename S::size_type pos = 0;
        typename S::size_type count = 0;
        sstype ss;
        while(n.next_split(':', &pos, &ss))
        {
            ++count;
        }
        C4_EXPECT_EQ(count, 0);
        C4_EXPECT_EQ(count, n.num_splits(':'));
    }

    {
        S const n("0123456");
        C4_EXPECT_EQ(n.num_splits(':'), 1);
        typename S::size_type pos = 0;
        typename S::size_type count = 0;
        sstype ss;
        while(n.next_split(':', &pos, &ss))
        {
            switch(count)
            {
            case 0:
                C4_EXPECT_EQ(ss.size(), n.size());
                C4_EXPECT_EQ(ss.empty(), false);
                break;
            }
            ++count;
        }
        C4_EXPECT_EQ(count, 1);
        C4_EXPECT_EQ(count, n.num_splits(':'));
    }

    {
        S const n("0123456:");
        C4_EXPECT_EQ(n.num_splits(':'), 2);
        typename S::size_type pos = 0;
        typename S::size_type count = 0;
        sstype ss;
        while(n.next_split(':', &pos, &ss))
        {
            switch(count)
            {
            case 0:
                C4_EXPECT_EQ(ss.size(), n.size()-1);
                C4_EXPECT_EQ(ss.empty(), false);
                break;
            case 1:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                break;
            }
            ++count;
        }
        C4_EXPECT_EQ(count, 2);
        C4_EXPECT_EQ(count, n.num_splits(':'));
    }

    {
        S const n(":0123456:");
        C4_EXPECT_EQ(n.num_splits(':'), 3);
        typename S::size_type pos = 0;
        typename S::size_type count = 0;
        sstype ss;
        while(n.next_split(':', &pos, &ss))
        {
            switch(count)
            {
            case 0:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                break;
            case 1:
                C4_EXPECT_EQ(ss.size(), n.size()-2);
                C4_EXPECT_EQ(ss.empty(), false);
                break;
            case 2:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                break;
            }
            ++count;
        }
        C4_EXPECT_EQ(count, 3);
        C4_EXPECT_EQ(count, n.num_splits(':'));
    }

    {
        S const n(":");
        C4_EXPECT_EQ(n.num_splits(':'), 2);
        typename S::size_type pos = 0;
        typename S::size_type count = 0;
        sstype ss;
        while(n.next_split(':', &pos, &ss))
        {
            switch(count)
            {
            case 0:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                break;
            case 1:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                break;
            }
            ++count;
        }
        C4_EXPECT_EQ(count, 2);
        C4_EXPECT_EQ(count, n.num_splits(':'));
    }

    {
        S const n("01:23:45:67");
        C4_EXPECT_EQ(n.num_splits(':'), 4);
        typename S::size_type pos = 0;
        typename S::size_type count = 0;
        sstype ss;
        while(n.next_split(':', &pos, &ss))
        {
            switch(count)
            {
            case 0:
                C4_EXPECT_EQ(ss.size(), 2);
                C4_EXPECT_EQ(ss, "01");
                C4_EXPECT_NE(ss, "01:");
                C4_EXPECT_NE(ss, ":01:");
                C4_EXPECT_NE(ss, ":01");
                break;
            case 1:
                C4_EXPECT_EQ(ss.size(), 2);
                C4_EXPECT_EQ(ss, "23");
                C4_EXPECT_NE(ss, "23:");
                C4_EXPECT_NE(ss, ":23:");
                C4_EXPECT_NE(ss, ":23");
                break;
            case 2:
                C4_EXPECT_EQ(ss.size(), 2);
                C4_EXPECT_EQ(ss, "45");
                C4_EXPECT_NE(ss, "45:");
                C4_EXPECT_NE(ss, ":45:");
                C4_EXPECT_NE(ss, ":45");
                break;
            case 3:
                C4_EXPECT_EQ(ss.size(), 2);
                C4_EXPECT_EQ(ss, "67");
                C4_EXPECT_NE(ss, "67:");
                C4_EXPECT_NE(ss, ":67:");
                C4_EXPECT_NE(ss, ":67");
                break;
            }
            count++;
        }
        C4_EXPECT_EQ(count, 4);
        C4_EXPECT_EQ(count, n.num_splits(':'));
    }

    {
        const S n(":01:23:45:67:");
        C4_EXPECT_EQ(n.num_splits(':'), 6);
        typename S::size_type pos = 0;
        typename S::size_type count = 0;
        sstype ss;
        while(n.next_split(':', &pos, &ss))
        {
            switch(count)
            {
            case 0:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                break;
            case 1:
                C4_EXPECT_EQ(ss.size(), 2);
                C4_EXPECT_EQ(ss, "01");
                C4_EXPECT_NE(ss, "01:");
                C4_EXPECT_NE(ss, ":01:");
                C4_EXPECT_NE(ss, ":01");
                break;
            case 2:
                C4_EXPECT_EQ(ss.size(), 2);
                C4_EXPECT_EQ(ss, "23");
                C4_EXPECT_NE(ss, "23:");
                C4_EXPECT_NE(ss, ":23:");
                C4_EXPECT_NE(ss, ":23");
                break;
            case 3:
                C4_EXPECT_EQ(ss.size(), 2);
                C4_EXPECT_EQ(ss, "45");
                C4_EXPECT_NE(ss, "45:");
                C4_EXPECT_NE(ss, ":45:");
                C4_EXPECT_NE(ss, ":45");
                break;
            case 4:
                C4_EXPECT_EQ(ss.size(), 2);
                C4_EXPECT_EQ(ss, "67");
                C4_EXPECT_NE(ss, "67:");
                C4_EXPECT_NE(ss, ":67:");
                C4_EXPECT_NE(ss, ":67");
                break;
            case 5:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                break;
            }
            count++;
        }
        C4_EXPECT_EQ(count, 6);
        C4_EXPECT_EQ(count, n.num_splits(':'));
    }

    {
        const S n("::::01:23:45:67::::");
        C4_EXPECT_EQ(n.num_splits(':'), 12);
        typename S::size_type pos = 0;
        typename S::size_type count = 0;
        sstype ss;
        while(n.next_split(':', &pos, &ss))
        {
            switch(count)
            {
            case 0:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                C4_EXPECT_NE(ss, "::");
                break;
            case 1:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                C4_EXPECT_NE(ss, "::");
                break;
            case 2:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                C4_EXPECT_NE(ss, "::");
                break;
            case 3:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                C4_EXPECT_NE(ss, "::");
                break;
            case 4:
                C4_EXPECT_EQ(ss.size(), 2);
                C4_EXPECT_EQ(ss, "01");
                C4_EXPECT_NE(ss, "01:");
                C4_EXPECT_NE(ss, ":01:");
                C4_EXPECT_NE(ss, ":01");
                break;
            case 5:
                C4_EXPECT_EQ(ss.size(), 2);
                C4_EXPECT_EQ(ss, "23");
                C4_EXPECT_NE(ss, "23:");
                C4_EXPECT_NE(ss, ":23:");
                C4_EXPECT_NE(ss, ":23");
                break;
            case 6:
                C4_EXPECT_EQ(ss.size(), 2);
                C4_EXPECT_EQ(ss, "45");
                C4_EXPECT_NE(ss, "45:");
                C4_EXPECT_NE(ss, ":45:");
                C4_EXPECT_NE(ss, ":45");
                break;
            case 7:
                C4_EXPECT_EQ(ss.size(), 2);
                C4_EXPECT_EQ(ss, "67");
                C4_EXPECT_NE(ss, "67:");
                C4_EXPECT_NE(ss, ":67:");
                C4_EXPECT_NE(ss, ":67");
                break;
            case 8:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                C4_EXPECT_NE(ss, "::");
                break;
            case 9:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                C4_EXPECT_NE(ss, "::");
                break;
            case 10:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                C4_EXPECT_NE(ss, "::");
                break;
            case 11:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                C4_EXPECT_NE(ss, "::");
                break;
            }
            count++;
        }
        C4_EXPECT_EQ(count, 12);
        C4_EXPECT_EQ(count, n.num_splits(':'));
    }
}

template< typename S >
void test_stringbase_split()
{
    {
        S const n;
        C4_EXPECT_EQ(n.num_splits(':'), 0);
        {
            auto spl = n.split(':');
            auto beg = spl.begin();
            auto end = spl.end();
            C4_EXPECT_EQ(beg, end);
        }
    }

    {
        S const n("foo:bar:baz");
        C4_EXPECT_EQ(n.num_splits(':'), 3);
        auto spl = n.split(':');
        auto beg = spl.begin();
        auto end = spl.end();
        C4_EXPECT_EQ(beg->size(), 3);
        C4_EXPECT_EQ(end->size(), 0);
        C4_EXPECT_EQ(*beg, "foo");
        C4_EXPECT_NE(beg, end);
        auto it = beg;
        C4_EXPECT_EQ(it->size(), 3);
        C4_EXPECT_EQ(*it, "foo");
        C4_EXPECT_NE(it, end);
        C4_EXPECT_EQ(it, beg);
        C4_EXPECT_EQ(beg->size(), 3);
        C4_EXPECT_EQ(*beg, "foo");
        C4_EXPECT_NE(beg, end);
        ++it;
        C4_EXPECT_EQ(it->size(), 3);
        C4_EXPECT_EQ(*it, "bar");
        C4_EXPECT_NE(it, end);
        C4_EXPECT_NE(it, beg);
        C4_EXPECT_EQ(beg->size(), 3);
        C4_EXPECT_EQ(*beg, "foo");
        C4_EXPECT_NE(beg, end);
        ++it;
        C4_EXPECT_EQ(it->size(), 3);
        C4_EXPECT_EQ(*it, "baz");
        C4_EXPECT_NE(it, end);
        C4_EXPECT_NE(it, beg);
        C4_EXPECT_EQ(beg->size(), 3);
        C4_EXPECT_EQ(*beg, "foo");
        C4_EXPECT_NE(beg, end);
        ++it;
        C4_EXPECT_EQ(it->size(), 0);
        C4_EXPECT_EQ(it, end);
        C4_EXPECT_NE(it, beg);
        C4_EXPECT_EQ(beg->size(), 3);
        C4_EXPECT_EQ(*beg, "foo");
        C4_EXPECT_NE(beg, end);
        it = beg;
        C4_EXPECT_EQ(it->size(), 3);
        C4_EXPECT_EQ(*it, "foo");
        C4_EXPECT_NE(it, end);
        C4_EXPECT_EQ(it, beg);
        C4_EXPECT_EQ(beg->size(), 3);
        C4_EXPECT_EQ(*beg, "foo");
        C4_EXPECT_NE(beg, end);
        it++;
        C4_EXPECT_EQ(it->size(), 3);
        C4_EXPECT_EQ(*it, "bar");
        C4_EXPECT_NE(it, end);
        C4_EXPECT_NE(it, beg);
        C4_EXPECT_EQ(beg->size(), 3);
        C4_EXPECT_EQ(*beg, "foo");
        C4_EXPECT_NE(beg, end);
        it++;
        C4_EXPECT_EQ(it->size(), 3);
        C4_EXPECT_EQ(*it, "baz");
        C4_EXPECT_NE(it, end);
        C4_EXPECT_NE(it, beg);
        C4_EXPECT_EQ(beg->size(), 3);
        C4_EXPECT_EQ(*beg, "foo");
        C4_EXPECT_NE(beg, end);
        it++;
        C4_EXPECT_EQ(it->size(), 0);
        C4_EXPECT_EQ(it, end);
        C4_EXPECT_NE(it, beg);
        C4_EXPECT_EQ(beg->size(), 3);
        C4_EXPECT_EQ(*beg, "foo");
        C4_EXPECT_NE(beg, end);
    }

    {
        S const n("foo:bar:baz:");
        C4_EXPECT_EQ(n.num_splits(':'), 4);
        auto spl = n.split(':');
        auto beg = spl.begin();
        auto end = spl.end();
        C4_EXPECT_EQ(beg->size(), 3);
        C4_EXPECT_EQ(end->size(), 0);
        C4_EXPECT_EQ(*beg, "foo");
        C4_EXPECT_NE(beg, end);
        auto it = beg;
        C4_EXPECT_EQ(it->size(), 3);
        C4_EXPECT_EQ(*it, "foo");
        C4_EXPECT_NE(it, end);
        C4_EXPECT_EQ(it, beg);
        C4_EXPECT_EQ(beg->size(), 3);
        C4_EXPECT_EQ(*beg, "foo");
        C4_EXPECT_NE(beg, end);
        ++it;
        C4_EXPECT_EQ(it->size(), 3);
        C4_EXPECT_EQ(*it, "bar");
        C4_EXPECT_NE(it, end);
        C4_EXPECT_NE(it, beg);
        C4_EXPECT_EQ(beg->size(), 3);
        C4_EXPECT_EQ(*beg, "foo");
        C4_EXPECT_NE(beg, end);
        ++it;
        C4_EXPECT_EQ(it->size(), 3);
        C4_EXPECT_EQ(*it, "baz");
        C4_EXPECT_NE(it, end);
        C4_EXPECT_NE(it, beg);
        C4_EXPECT_EQ(beg->size(), 3);
        C4_EXPECT_EQ(*beg, "foo");
        C4_EXPECT_NE(beg, end);
        ++it;
        C4_EXPECT_EQ(it->size(), 0);
        C4_EXPECT_EQ(*it, "");
        C4_EXPECT_NE(it, end);
        C4_EXPECT_NE(it, beg);
        C4_EXPECT_EQ(beg->size(), 3);
        C4_EXPECT_EQ(*beg, "foo");
        C4_EXPECT_NE(beg, end);
        ++it;
        C4_EXPECT_EQ(it->size(), 0);
        C4_EXPECT_EQ(it, end);
        C4_EXPECT_NE(it, beg);
        C4_EXPECT_EQ(beg->size(), 3);
        C4_EXPECT_EQ(*beg, "foo");
        C4_EXPECT_NE(beg, end);
        //--------------------------
        it = beg;
        C4_EXPECT_EQ(it->size(), 3);
        C4_EXPECT_EQ(*it, "foo");
        C4_EXPECT_NE(it, end);
        C4_EXPECT_EQ(it, beg);
        C4_EXPECT_EQ(beg->size(), 3);
        C4_EXPECT_EQ(*beg, "foo");
        C4_EXPECT_NE(beg, end);
        it++;
        C4_EXPECT_EQ(it->size(), 3);
        C4_EXPECT_EQ(*it, "bar");
        C4_EXPECT_NE(it, end);
        C4_EXPECT_NE(it, beg);
        C4_EXPECT_EQ(beg->size(), 3);
        C4_EXPECT_EQ(*beg, "foo");
        C4_EXPECT_NE(beg, end);
        it++;
        C4_EXPECT_EQ(it->size(), 3);
        C4_EXPECT_EQ(*it, "baz");
        C4_EXPECT_NE(it, end);
        C4_EXPECT_NE(it, beg);
        C4_EXPECT_EQ(beg->size(), 3);
        C4_EXPECT_EQ(*beg, "foo");
        C4_EXPECT_NE(beg, end);
        it++;
        C4_EXPECT_EQ(it->size(), 0);
        C4_EXPECT_EQ(*it, "");
        C4_EXPECT_NE(it, end);
        C4_EXPECT_NE(it, beg);
        C4_EXPECT_EQ(beg->size(), 3);
        C4_EXPECT_EQ(*beg, "foo");
        C4_EXPECT_NE(beg, end);
        it++;
        C4_EXPECT_EQ(it->size(), 0);
        C4_EXPECT_EQ(it, end);
        C4_EXPECT_NE(it, beg);
        C4_EXPECT_EQ(beg->size(), 3);
        C4_EXPECT_EQ(*beg, "foo");
        C4_EXPECT_NE(beg, end);
    }

    {
        S const n;
        C4_EXPECT_EQ(n.num_splits(':'), 0);
        auto s = n.split(':');
        // check that multiple calls to begin() always yield the same result
        C4_EXPECT_EQ(*s.begin(), "");
        C4_EXPECT_EQ(*s.begin(), "");
        C4_EXPECT_EQ(*s.begin(), "");
        // check that multiple calls to end() always yield the same result
        auto e = s.end();
        C4_EXPECT_EQ(s.end(), e);
        C4_EXPECT_EQ(s.end(), e);
        //
        auto it = s.begin();
        C4_EXPECT_EQ(*it, "");
        C4_EXPECT_EQ(it->empty(), true);
        C4_EXPECT_EQ(it->size(), 0);
        ++it;
        C4_EXPECT_EQ(it, e);
    }

    {
        S const n("01:23:45:67");
        C4_EXPECT_EQ(n.num_splits(':'), 4);
        auto s = n.split(':');
        // check that multiple calls to begin() always yield the same result
        C4_EXPECT_EQ(*s.begin(), "01");
        C4_EXPECT_EQ(*s.begin(), "01");
        C4_EXPECT_EQ(*s.begin(), "01");
        // check that multiple calls to end() always yield the same result
        auto e = s.end();
        C4_EXPECT_EQ(s.end(), e);
        C4_EXPECT_EQ(s.end(), e);
        C4_EXPECT_EQ(s.end(), e);
        //
        auto it = s.begin();
        C4_EXPECT_EQ(*it, "01");
        C4_EXPECT_EQ(it->size(), 2);
        ++it;
        C4_EXPECT_EQ(*it, "23");
        C4_EXPECT_EQ(it->size(), 2);
        ++it;
        C4_EXPECT_EQ(*it, "45");
        C4_EXPECT_EQ(it->size(), 2);
        ++it;
        C4_EXPECT_EQ(*it, "67");
        C4_EXPECT_EQ(it->size(), 2);
        ++it;
        C4_EXPECT_EQ(it, s.end());
    }

    {
        S const n;
        typename S::size_type count = 0;
        for(auto &ss : n.split(':'))
        {
            ++count;
        }
        C4_EXPECT_EQ(count, 0);
        C4_EXPECT_EQ(count, n.num_splits(':'));
    }

    {
        S const n("0123456");
        C4_EXPECT_EQ(n.num_splits(':'), 1);
        {
            auto spl = n.split(':');
            auto beg = spl.begin();
            auto end = spl.end();
            C4_EXPECT_EQ(beg->size(), n.size());
            C4_EXPECT_EQ(end->size(), 0);
        }
        typename S::size_type count = 0;
        for(auto &ss : n.split(':'))
        {
            switch(count)
            {
            case 0:
                C4_EXPECT_EQ(ss.size(), n.size());
                C4_EXPECT_EQ(ss.empty(), false);
                break;
            }
            ++count;
        }
        C4_EXPECT_EQ(count, 1);
        C4_EXPECT_EQ(count, n.num_splits(':'));
    }

    {
        S const n("foo:bar");
        typename S::size_type count = 0;
        for(auto &ss : n.split(':'))
        {
            switch(count)
            {
            case 0:
                C4_EXPECT_EQ(ss.size(), 3);
                C4_EXPECT_EQ(ss.empty(), false);
                C4_EXPECT_EQ(ss, "foo");
                break;
            case 1:
                C4_EXPECT_EQ(ss.size(), 3);
                C4_EXPECT_EQ(ss.empty(), false);
                C4_EXPECT_EQ(ss, "bar");
                break;
            }
            ++count;
        }
        C4_EXPECT_EQ(count, 2);
        C4_EXPECT_EQ(count, n.num_splits(':'));
    }

    {
        S const n("0123456:");
        C4_EXPECT_EQ(n.num_splits(':'), 2);
        typename S::size_type count = 0;
        for(auto &ss : n.split(':'))
        {
            switch(count)
            {
            case 0:
                C4_EXPECT_EQ(ss.size(), n.size()-1);
                C4_EXPECT_EQ(ss.empty(), false);
                break;
            case 1:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                break;
            }
            ++count;
        }
        C4_EXPECT_EQ(count, 2);
        C4_EXPECT_EQ(count, n.num_splits(':'));
    }

    {
        S const n(":0123456:");
        C4_EXPECT_EQ(n.num_splits(':'), 3);
        typename S::size_type count = 0;
        for(auto &ss : n.split(':'))
        {
            switch(count)
            {
            case 0:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                break;
            case 1:
                C4_EXPECT_EQ(ss.size(), n.size()-2);
                C4_EXPECT_EQ(ss.empty(), false);
                break;
            case 2:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                break;
            }
            ++count;
        }
        C4_EXPECT_EQ(count, 3);
        C4_EXPECT_EQ(count, n.num_splits(':'));
    }

    {
        S const n(":");
        C4_EXPECT_EQ(n.num_splits(':'), 2);
        typename S::size_type count = 0;
        for(auto &ss : n.split(':'))
        {
            switch(count)
            {
            case 0:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                break;
            case 1:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                break;
            }
            ++count;
        }
        C4_EXPECT_EQ(count, 2);
        C4_EXPECT_EQ(count, n.num_splits(':'));
    }

    {
        S const n("01:23:45:67");
        C4_EXPECT_EQ(n.num_splits(':'), 4);
        typename S::size_type count = 0;
        for(auto &ss : n.split(':'))
        {
            switch(count)
            {
            case 0:
                C4_EXPECT_EQ(ss, "01");
                C4_EXPECT_NE(ss, "01:");
                C4_EXPECT_NE(ss, ":01:");
                C4_EXPECT_NE(ss, ":01");
                break;
            case 1:
                C4_EXPECT_EQ(ss, "23");
                C4_EXPECT_NE(ss, "23:");
                C4_EXPECT_NE(ss, ":23:");
                C4_EXPECT_NE(ss, ":23");
                break;
            case 2:
                C4_EXPECT_EQ(ss, "45");
                C4_EXPECT_NE(ss, "45:");
                C4_EXPECT_NE(ss, ":45:");
                C4_EXPECT_NE(ss, ":45");
                break;
            case 3:
                C4_EXPECT_EQ(ss, "67");
                C4_EXPECT_NE(ss, "67:");
                C4_EXPECT_NE(ss, ":67:");
                C4_EXPECT_NE(ss, ":67");
                break;
            }
            count++;
        }
        C4_EXPECT_EQ(count, 4);
        C4_EXPECT_EQ(count, n.num_splits(':'));
    }

    {
        const S n(":01:23:45:67:");
        C4_EXPECT_EQ(n.num_splits(':'), 6);
        typename S::size_type count = 0;
        for(auto &ss : n.split(':'))
        {
            switch(count)
            {
            case 0:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                break;
            case 1:
                C4_EXPECT_EQ(ss, "01");
                C4_EXPECT_NE(ss, "01:");
                C4_EXPECT_NE(ss, ":01:");
                C4_EXPECT_NE(ss, ":01");
                break;
            case 2:
                C4_EXPECT_EQ(ss, "23");
                C4_EXPECT_NE(ss, "23:");
                C4_EXPECT_NE(ss, ":23:");
                C4_EXPECT_NE(ss, ":23");
                break;
            case 3:
                C4_EXPECT_EQ(ss, "45");
                C4_EXPECT_NE(ss, "45:");
                C4_EXPECT_NE(ss, ":45:");
                C4_EXPECT_NE(ss, ":45");
                break;
            case 4:
                C4_EXPECT_EQ(ss, "67");
                C4_EXPECT_NE(ss, "67:");
                C4_EXPECT_NE(ss, ":67:");
                C4_EXPECT_NE(ss, ":67");
                break;
            case 5:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                break;
            }
            count++;
        }
        C4_EXPECT_EQ(count, 6);
        C4_EXPECT_EQ(count, n.num_splits(':'));
    }

    {
        const S n("::::01:23:45:67::::");
        C4_EXPECT_EQ(n.num_splits(':'), 12);
        typename S::size_type count = 0;
        for(auto &ss : n.split(':'))
        {
            switch(count)
            {
            case 0:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                C4_EXPECT_NE(ss, "::");
                break;
            case 1:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                C4_EXPECT_NE(ss, "::");
                break;
            case 2:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                C4_EXPECT_NE(ss, "::");
                break;
            case 3:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                C4_EXPECT_NE(ss, "::");
                break;
            case 4:
                C4_EXPECT_EQ(ss, "01");
                C4_EXPECT_NE(ss, "01:");
                C4_EXPECT_NE(ss, ":01:");
                C4_EXPECT_NE(ss, ":01");
                break;
            case 5:
                C4_EXPECT_EQ(ss, "23");
                C4_EXPECT_NE(ss, "23:");
                C4_EXPECT_NE(ss, ":23:");
                C4_EXPECT_NE(ss, ":23");
                break;
            case 6:
                C4_EXPECT_EQ(ss, "45");
                C4_EXPECT_NE(ss, "45:");
                C4_EXPECT_NE(ss, ":45:");
                C4_EXPECT_NE(ss, ":45");
                break;
            case 7:
                C4_EXPECT_EQ(ss, "67");
                C4_EXPECT_NE(ss, "67:");
                C4_EXPECT_NE(ss, ":67:");
                C4_EXPECT_NE(ss, ":67");
                break;
            case 8:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                C4_EXPECT_NE(ss, "::");
                break;
            case 9:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                C4_EXPECT_NE(ss, "::");
                break;
            case 10:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                C4_EXPECT_NE(ss, "::");
                break;
            case 11:
                C4_EXPECT_EQ(ss.size(), 0);
                C4_EXPECT_EQ(ss.empty(), true);
                C4_EXPECT_NE(ss, "::");
                break;
            }
            count++;
        }
        C4_EXPECT_EQ(count, 12);
        C4_EXPECT_EQ(count, n.num_splits(':'));
    }
}

template< class S >
void test_stringbase_erase()
{
    char b[] = "0123456789";
    S s(b);

    s.erase("9");
    C4_EXPECT_EQ(s, "012345678");
    s.erase("0");
    C4_EXPECT_EQ(s, "12345678");
    s.erase("7");
    C4_EXPECT_EQ(s, "1234568");
    s.erase("46");
    C4_EXPECT_EQ(s, "12358");
    s.erase("18");
    C4_EXPECT_EQ(s, "235");
    s.erase("523");
    C4_EXPECT_EQ(s, "");
    C4_EXPECT(s.empty());
}

template< class S, class SStream >
void test_stringbase_stream()
{
    SStream ss;
    ss << "n1 n2 n3 n4";
    // substrings will need a buffer to be set
    // (otherwise, we'd have a segmentation fault)
    // it cannot be a static string a la substring n1("buf1")
    // because these are not writeable
    typename S::char_type buf1[5] = {"buf1"};
    typename S::char_type buf2[5] = {"buf2"};
    typename S::char_type buf3[5] = {"buf3"};
    typename S::char_type buf4[5] = {"buf4"};
    S n1(buf1), n2(buf2), n3(buf3), n4(buf4);

    ss >> n1;
    C4_EXPECT_EQ(n1.size(), 2);
    C4_EXPECT_EQ(n1, "n1");
    C4_EXPECT_NE(n1, "n1 ");
    C4_EXPECT_NE(n1, " n1");

    ss >> n2;
    C4_EXPECT_EQ(n2.size(), 2);
    C4_EXPECT_EQ(n2, "n2");
    C4_EXPECT_NE(n2, "n2 ");
    C4_EXPECT_NE(n2, " n2");

    ss >> n3;
    C4_EXPECT_EQ(n3.size(), 2);
    C4_EXPECT_EQ(n3, "n3");
    C4_EXPECT_NE(n3, "n3 ");
    C4_EXPECT_NE(n3, " n3");

    ss >> n4;
    C4_EXPECT_EQ(n4.size(), 2);
    C4_EXPECT_EQ(n4, "n4");
    C4_EXPECT_NE(n4, "n4 ");
    C4_EXPECT_NE(n4, " n4");
}
template< class S >
void test_stringbase_hash()
{
    typename S::char_type *cn = "adfkusdfkjsdf2ekjsdfkjh";
    S r("adfkusdfkjsdf2ekjsdfkjh");
    S n(cn);
    std::hash< S > H;
    size_t hr = H(r);
    size_t hn = H(n);
    C4_EXPECT_EQ(hr, hn);
}
template< class S >
void test_stringbase_sum()
{
    char buf[64] = {'\0'}; // this test must work with substringrs
    S out(buf, sizeof(buf)-1);

#define _clout(noarg) \
    out.resize(0);\
    C4_EXPECT(out.empty())

    S l("l"), r("r");
    out = l + r;
    C4_EXPECT_EQ(out, "lr");
    _clout(.);

    S n0("0");
    S n1("1");
    S n2("2");
    S n3("3");
    S n4("4");
    S n5("5");
    S n6("6");
    S n7("7");
    S n8("8");
    S n9("9");

    out = n0 + n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9;
    C4_EXPECT_EQ(out, "0123456789");
    _clout(.);

    out = (n0 + n1) + (n2 + n3) + (n4 + n5) + (n6 + n7) + (n8 + n9);
    C4_EXPECT_EQ(out, "0123456789");
    _clout(.);

    out = (n0 + (n1 + (n2 + (n3 + (n4 + n5) + n6) + n7) + n8) + n9);
    C4_EXPECT_EQ(out, "0123456789");
    _clout(.);

    out = (n0 + (n1 + (n2 + (n3 + (n4 + (n5 + (n6 + (n7 + (n8 + n9)))))))));
    C4_EXPECT_EQ(out, "0123456789");
    _clout(.);

    out = (((((((((n0 + n1) + n2) + n3) + n4) + n5) + n6) + n7) + n8) + n9);
    C4_EXPECT_EQ(out, "0123456789");
    _clout(.);
}
template< class S >
void test_stringbase_dircat()
{
    char buf[64] = {'\0'}; // this test must work with substringrs
    S out(buf, sizeof(buf)-1);

    S l("l"), r("r");
    out = l / r;
    C4_EXPECT_EQ(out, "l/r");
    _clout(.);

    S n0("0");
    S n1("1");
    S n2("2");
    S n3("3");
    S n4("4");
    S n5("5");
    S n6("6");
    S n7("7");
    S n8("8");
    S n9("9");

    out = n0 / n1 / n2 / n3 / n4 / n5 / n6 / n7 / n8 / n9;
    C4_EXPECT_EQ(out, "0/1/2/3/4/5/6/7/8/9");
    _clout(.);

    out = (n0 / n1) / (n2 / n3) / (n4 / n5) / (n6 / n7) / (n8 / n9);
    C4_EXPECT_EQ(out, "0/1/2/3/4/5/6/7/8/9");
    _clout(.);

    out = (n0 / (n1 / (n2 / (n3 / (n4 / n5) / n6) / n7) / n8) / n9);
    C4_EXPECT_EQ(out, "0/1/2/3/4/5/6/7/8/9");
    _clout(.);

    out = (n0 / (n1 / (n2 / (n3 / (n4 / (n5 / (n6 / (n7 / (n8 / n9)))))))));
    C4_EXPECT_EQ(out, "0/1/2/3/4/5/6/7/8/9");
    _clout(.);

    out = (((((((((n0 / n1) / n2) / n3) / n4) / n5) / n6) / n7) / n8) / n9);
    C4_EXPECT_EQ(out, "0/1/2/3/4/5/6/7/8/9");
    _clout(.);
}
template< class S >
void test_stringbase_prepend()
{
    char buf[64] = {'\0'}; // this test must work with substringrs
    S out(buf, sizeof(buf)-1);
    out.clear();
    memset(buf, 0, sizeof(buf));

    S l("l"), r("r"), t("prepend");
    out.prepend(l);
    C4_EXPECT_EQ(out, "l");
    out.prepend(r);
    C4_EXPECT_EQ(out, "rl");
    out.prepend(l);
    C4_EXPECT_EQ(out, "lrl");
    out.prepend(r);
    C4_EXPECT_EQ(out, "rlrl");
    out.prepend(t);
    C4_EXPECT_EQ(out, "prependrlrl");
    out.prepend(t);
    C4_EXPECT_EQ(out, "prependprependrlrl");
}

template< class S >
void test_stringbase_append()
{
    char buf[64] = {'\0'}; // this test must work with substringrs
    S out(buf, sizeof(buf)-1);
    out.clear();
    memset(buf, 0, sizeof(buf));

    S l("l"), r("r");
    out.append(l);
    C4_EXPECT_EQ(out, "l");
    out.append(r);
    C4_EXPECT_EQ(out, "lr");
    out.append(l);
    C4_EXPECT_EQ(out, "lrl");
    out.append(r);
    C4_EXPECT_EQ(out, "lrlr");
    out.append("append");
    C4_EXPECT_EQ(out, "lrlrappend");
    out.append("append");
    C4_EXPECT_EQ(out, "lrlrappendappend");
}
template< class S >
void test_stringbase_append_dir()
{
    char buf[64] = {'\0'}; // this test must work with substringrs
    S out(buf, sizeof(buf)-1);
    out.clear();
    memset(buf, 0, sizeof(buf));

    S l("l"), r("r");
    out.pushr(l);
    C4_EXPECT_EQ(out, "/l");
    out.pushr(r);
    C4_EXPECT_EQ(out, "/l/r");
    out.pushr(l);
    C4_EXPECT_EQ(out, "/l/r/l");
    out.pushr(r);
    C4_EXPECT_EQ(out, "/l/r/l/r");
    out.pushr("append");
    C4_EXPECT_EQ(out, "/l/r/l/r/append");
    out.pushr("append");
    C4_EXPECT_EQ(out, "/l/r/l/r/append/append");

    out.clear();
    memset(buf, 0, sizeof(buf));
    out.pushr("a");
    C4_EXPECT_EQ(out, "/a");
    out.pushr("/");
    C4_EXPECT_EQ(out, "/a/");
    out.append("//////");
    C4_EXPECT_EQ(out, "/a///////");
    out.pushr("/b");
    C4_EXPECT_EQ(out, "/a/b");
    out.pushr("");
    C4_EXPECT_EQ(out, "/a/b/");
    out.pushr("////////c");
    C4_EXPECT_EQ(out, "/a/b/c");
    out.pushr("c////////");
    C4_EXPECT_EQ(out, "/a/b/c/c////////");
    out.pushr("////////d////////");
    C4_EXPECT_EQ(out, "/a/b/c/c/d////////");
}
template< class S >
void test_stringbase_prepend_dir()
{
    char buf[64] = {'\0'}; // this test must work with substringrs
    S out(buf, sizeof(buf)-1);
    out.clear();
    memset(buf, 0, sizeof(buf));

    S l("l"), r("r");
    out.pushl(l);
    C4_EXPECT_EQ(out, "l/");
    out.pushl(r);
    C4_EXPECT_EQ(out, "r/l/");
    out.pushl(l);
    C4_EXPECT_EQ(out, "l/r/l/");
    out.pushl(r);
    C4_EXPECT_EQ(out, "r/l/r/l/");
    out.pushl("prepend");
    C4_EXPECT_EQ(out, "prepend/r/l/r/l/");
    out.pushl("prepend");
    C4_EXPECT_EQ(out, "prepend/prepend/r/l/r/l/");

    out.clear();
    memset(buf, 0, sizeof(buf));
    out.pushl("a");
    C4_EXPECT_EQ(out, "a/");
    out.pushl("/");
    C4_EXPECT_EQ(out, "/a/");
    out.prepend("//////");
    C4_EXPECT_EQ(out, "///////a/");
    out.pushl("/b");
    C4_EXPECT_EQ(out, "/b/a/");
    out.pushl("");
    C4_EXPECT_EQ(out, "/b/a/");
    out.pushl("c////////");
    C4_EXPECT_EQ(out, "c/b/a/");
    out.pushl("/c////////");
    C4_EXPECT_EQ(out, "/c/c/b/a/");
    out.pushl("////////d////////");
    C4_EXPECT_EQ(out, "////////d/c/c/b/a/");
}


template< class S_ >
void test_string_vs_wstring()
{
/* there's a problem with the string lengths which prevents this from working with
 * substring/substringrs. Revisit this.

    char    rs[64]  =  "áaàaâaäaãaéeèeëeêeóoòoôoõoöoñnçcíiìiïi\0";
    wchar_t rws[64] = L"áaàaâaäaãaéeèeëeêeóoòoôoõoöoñnçcíiìiïi\0";
    char    rbuf [64]; memset(rbuf, 0, sizeof(rbuf)); memcpy(rbuf, rs, sizeof(rs));
    wchar_t wrbuf[64]; memset(wrbuf, 0, sizeof(wrbuf)); memcpy(wrbuf, rws, sizeof(rws));

    using S  = typename S_::template parameterized_string_type< char >;
    using WS = typename S_::template parameterized_string_type< wchar_t >;

    S   s( rs),  r( rbuf);
    WS ws(rws), wr(wrbuf);

    s2ws(s, &wr);
    C4_EXPECT_EQ(wr, ws);
    wr.clear();

    ws2s(ws, &r);
    C4_EXPECT_EQ(r, s);
    r.clear();

    s2ws(s, &wr);
    ws2s(wr, &r);
    C4_EXPECT_EQ(r, s);
    wr.clear();
    r.clear();

    ws2s(ws, &r);
    s2ws(r, &wr);
    C4_EXPECT_EQ(wr, ws);

    {
        auto awr = s2ws(s);
        C4_EXPECT_EQ(awr, ws);
    }

    {
        auto ar = ws2s(ws);
        C4_EXPECT_EQ(ar, s);
    }
    */
}

#undef _clout
#undef MTEST
#undef NTEST

#ifdef __clang__
#   pragma clang diagnostic pop
#elif defined(__GNUC__)
#   pragma GCC diagnostic pop
#endif

C4_END_NAMESPACE(c4)
