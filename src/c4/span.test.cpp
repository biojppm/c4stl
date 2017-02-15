#include "c4/span.hpp"

#include <gtest/gtest.h>

C4_BEGIN_NAMESPACE(c4)

TEST(span, defaultinit)
{
    span< int > s;
    EXPECT_EQ(s.size(), 0);
    EXPECT_EQ(s.capacity(), 0);
    EXPECT_EQ(s.data(), nullptr);
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
        span<T> s = arr1;
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
}

TEST(span, fromArray)
{
    test_fromArray< int >();
    test_fromArray< uint32_t >();
}

C4_END_NAMESPACE(c4)
