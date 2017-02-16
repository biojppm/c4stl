#include "c4/span.hpp"

#include <gtest/gtest.h>

C4_BEGIN_NAMESPACE(c4)

TEST(span, default_init)
{
    span< int > s;
    EXPECT_EQ(s.size(), 0);
    EXPECT_EQ(s.capacity(), 0);
    EXPECT_EQ(s.data(), nullptr);
}

TEST(spanrs, default_init)
{
    spanrs< int > s;
    EXPECT_EQ(s.size(), 0);
    EXPECT_EQ(s.capacity(), 0);
    EXPECT_EQ(s.data(), nullptr);
}

TEST(span, empty_init)
{
    int arr[10];
    span< int > s(arr, 0);
    EXPECT_EQ(s.size(), 0);
    EXPECT_EQ(s.capacity(), 0);
    EXPECT_NE(s.data(), nullptr);
}

TEST(spanrs, empty_init)
{
    int arr[10];

    {
        spanrs< int > s(arr, 0);
        EXPECT_EQ(s.size(), 0);
        EXPECT_EQ(s.capacity(), 0);
        EXPECT_EQ(s.data(), arr);
    }

    {
        spanrs< int > s(arr, 0, C4_COUNTOF(arr));
        EXPECT_EQ(s.size(), 0);
        EXPECT_EQ(s.capacity(), 10);
        EXPECT_EQ(s.data(), arr);
    }
}

template< class SpanClass >
void test_subspan()
{
    int arr[10];
    SpanClass s(arr);
    C4_STATIC_ASSERT((std::is_same< decltype(s.subspan(0)), decltype(s) >::value));

    auto ss = s.subspan(0, 5);
    EXPECT_EQ(ss.size(), 5);
    EXPECT_EQ(ss.capacity(), 5);
    EXPECT_EQ(ss.data(), arr);

    ss = s.subspan(5);
    EXPECT_EQ(ss.size(), 5);
    EXPECT_EQ(ss.capacity(), 5);
    EXPECT_EQ(ss.data(), &arr[5]);
}

TEST(span, subspan)
{
    test_subspan< span< int > >();
}
TEST(spanrs, subspan)
{
    test_subspan< spanrs< int > >();
}


template< class T >
void test_fromArray()
{
    T arr1[10];
    T arr2[20];

    T a = 0;
    for(auto &v : arr1) { v = a; a += T(1); }
    for(auto &v : arr2) { v = a; a += T(1); }

    {
        span<T> s(arr1);
        EXPECT_EQ(s.size(), C4_COUNTOF(arr1));
        EXPECT_EQ(s.capacity(), C4_COUNTOF(arr1));
        EXPECT_EQ(s.data(), arr1);
    }

    {
        spanrs<T> s(arr1);
        EXPECT_EQ(s.size(), C4_COUNTOF(arr1));
        EXPECT_EQ(s.capacity(), C4_COUNTOF(arr1));
        EXPECT_EQ(s.data(), arr1);
    }

    {
        span<T> s = arr1;
        EXPECT_EQ(s.size(), C4_COUNTOF(arr1));
        EXPECT_EQ(s.capacity(), C4_COUNTOF(arr1));
        EXPECT_EQ(s.data(), arr1);
    }

    {
        spanrs<T> s = arr1;
        EXPECT_EQ(s.size(), C4_COUNTOF(arr1));
        EXPECT_EQ(s.capacity(), C4_COUNTOF(arr1));
        EXPECT_EQ(s.data(), arr1);
    }

    {
        span<T> s = arr1;
        EXPECT_EQ(s.size(), C4_COUNTOF(arr1));
        EXPECT_EQ(s.capacity(), C4_COUNTOF(arr1));
        EXPECT_EQ(s.data(), arr1);
        s = arr2;
        EXPECT_EQ(s.size(), C4_COUNTOF(arr2));
        EXPECT_EQ(s.capacity(), C4_COUNTOF(arr2));
        EXPECT_EQ(s.data(), arr2);
    }

    {
        spanrs<T> s = arr1;
        EXPECT_EQ(s.size(), C4_COUNTOF(arr1));
        EXPECT_EQ(s.capacity(), C4_COUNTOF(arr1));
        EXPECT_EQ(s.data(), arr1);
        s = arr2;
        EXPECT_EQ(s.size(), C4_COUNTOF(arr2));
        EXPECT_EQ(s.capacity(), C4_COUNTOF(arr2));
        EXPECT_EQ(s.data(), arr2);
    }
}

TEST(span, fromArray)
{
    test_fromArray< char >();
    test_fromArray< int >();
    test_fromArray< uint32_t >();
}

C4_END_NAMESPACE(c4)
