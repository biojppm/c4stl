#include "c4/span.hpp"

#include <gtest/gtest.h>

C4_BEGIN_NAMESPACE(c4)

//-----------------------------------------------------------------------------
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

TEST(etched_span, default_init)
{
    etched_span< int > s;
    EXPECT_EQ(s.size(), 0);
    EXPECT_EQ(s.capacity(), 0);
    EXPECT_EQ(s.data(), nullptr);
}

//-----------------------------------------------------------------------------
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

TEST(etched_span, empty_init)
{
    int arr[10];

    {
        etched_span< int > s(arr, 0);
        EXPECT_EQ(s.size(), 0);
        EXPECT_EQ(s.capacity(), 0);
        EXPECT_EQ(s.data(), arr);
        EXPECT_EQ(s.offset(), 0);
    }

    {
        etched_span< int > s(arr, 0, C4_COUNTOF(arr));
        EXPECT_EQ(s.size(), 0);
        EXPECT_EQ(s.capacity(), 10);
        EXPECT_EQ(s.data(), arr);
        EXPECT_EQ(s.offset(), 0);
    }
}

//-----------------------------------------------------------------------------
template< class SpanClass >
void test_fromArray()
{
    using T = typename SpanClass::value_type;
    T arr1[10];
    T arr2[20];

    T a = 0;
    for(auto &v : arr1) { v = a; a += T(1); }
    for(auto &v : arr2) { v = a; a += T(1); }

    {
        SpanClass s(arr1);
        EXPECT_EQ(s.size(), C4_COUNTOF(arr1));
        EXPECT_EQ(s.capacity(), C4_COUNTOF(arr1));
        EXPECT_EQ(s.data(), arr1);
    }

    {
        SpanClass s = arr1;
        EXPECT_EQ(s.size(), C4_COUNTOF(arr1));
        EXPECT_EQ(s.capacity(), C4_COUNTOF(arr1));
        EXPECT_EQ(s.data(), arr1);
    }

    {
        SpanClass s = arr1;
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
    test_fromArray< span<char> >();
    test_fromArray< span<int> >();
    test_fromArray< span<uint32_t> >();
}

TEST(spanrs, fromArray)
{
    test_fromArray< spanrs<char> >();
    test_fromArray< spanrs<int> >();
    test_fromArray< spanrs<uint32_t> >();
}

TEST(etched_span, fromArray)
{
    test_fromArray< etched_span<char> >();
    test_fromArray< etched_span<int> >();
    test_fromArray< etched_span<uint32_t> >();
}

//-----------------------------------------------------------------------------
TEST(span, subspan)
{
    int arr[10];
    span< int > s(arr);
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
TEST(spanrs, subspan)
{
    int arr[10];
    spanrs< int > s(arr);
    C4_STATIC_ASSERT((std::is_same< decltype(s.subspan(0)), decltype(s) >::value));

    auto ss = s.subspan(0, 5);
    EXPECT_EQ(ss.size(), 5);
    EXPECT_EQ(ss.capacity(), 10);
    EXPECT_EQ(ss.data(), arr);

    ss = s.subspan(5);
    EXPECT_EQ(ss.size(), 5);
    EXPECT_EQ(ss.capacity(), 5);
    EXPECT_EQ(ss.data(), &arr[5]);
}
TEST(etched_span, subspan)
{
    int arr[10];
    etched_span< int > s(arr);
    C4_STATIC_ASSERT((std::is_same< decltype(s.subspan(0)), decltype(s) >::value));

    auto ss = s.subspan(0, 5);
    EXPECT_EQ(ss.size(), 5);
    EXPECT_EQ(ss.capacity(), 10);
    EXPECT_EQ(ss.data(), arr);
    EXPECT_EQ(ss.offset(), 0);

    ss = ss.original();
    EXPECT_EQ(ss.size(), 10);
    EXPECT_EQ(ss.capacity(), 10);
    EXPECT_EQ(ss.data(), arr);
    EXPECT_EQ(ss.offset(), 0);

    ss = s.subspan(5);
    EXPECT_EQ(ss.size(), 5);
    EXPECT_EQ(ss.capacity(), 5);
    EXPECT_EQ(ss.data(), &arr[5]);
    EXPECT_EQ(ss.offset(), 5);

    ss = ss.original();
    EXPECT_EQ(ss.size(), 10);
    EXPECT_EQ(ss.capacity(), 10);
    EXPECT_EQ(ss.data(), arr);
    EXPECT_EQ(ss.offset(), 0);
}

//-----------------------------------------------------------------------------
TEST(span, range)
{
    int arr[10];
    span< int > s(arr);
    C4_STATIC_ASSERT((std::is_same< decltype(s.range(0)), decltype(s) >::value));

    auto ss = s.range(0, 5);
    EXPECT_EQ(ss.size(), 5);
    EXPECT_EQ(ss.capacity(), 5);
    EXPECT_EQ(ss.data(), arr);

    ss = s.range(5);
    EXPECT_EQ(ss.size(), 5);
    EXPECT_EQ(ss.capacity(), 5);
    EXPECT_EQ(ss.data(), &arr[5]);

    ss = s.range(5, 10);
    EXPECT_EQ(ss.size(), 5);
    EXPECT_EQ(ss.capacity(), 5);
    EXPECT_EQ(ss.data(), &arr[5]);
}
TEST(spanrs, range)
{
    int arr[10];
    spanrs< int > s(arr);
    C4_STATIC_ASSERT((std::is_same< decltype(s.range(0)), decltype(s) >::value));

    auto ss = s.range(0, 5);
    EXPECT_EQ(ss.size(), 5);
    EXPECT_EQ(ss.capacity(), 10);
    EXPECT_EQ(ss.data(), arr);

    ss = s.range(5);
    EXPECT_EQ(ss.size(), 5);
    EXPECT_EQ(ss.capacity(), 5);
    EXPECT_EQ(ss.data(), &arr[5]);

    ss = s.range(5, 10);
    EXPECT_EQ(ss.size(), 5);
    EXPECT_EQ(ss.capacity(), 5);
    EXPECT_EQ(ss.data(), &arr[5]);
}
TEST(etched_span, range)
{
    int arr[10];
    etched_span< int > s(arr);
    C4_STATIC_ASSERT((std::is_same< decltype(s.range(0)), decltype(s) >::value));

    auto ss = s.range(0, 5);
    EXPECT_EQ(ss.size(), 5);
    EXPECT_EQ(ss.capacity(), 10);
    EXPECT_EQ(ss.data(), arr);

    ss = ss.original();
    EXPECT_EQ(ss.size(), 10);
    EXPECT_EQ(ss.capacity(), 10);
    EXPECT_EQ(ss.data(), arr);

    ss = s.range(5);
    EXPECT_EQ(ss.size(), 5);
    EXPECT_EQ(ss.capacity(), 5);
    EXPECT_EQ(ss.data(), &arr[5]);

    ss = s.range(5, 10);
    EXPECT_EQ(ss.size(), 5);
    EXPECT_EQ(ss.capacity(), 5);
    EXPECT_EQ(ss.data(), &arr[5]);

    ss = ss.original();
    EXPECT_EQ(ss.size(), 10);
    EXPECT_EQ(ss.capacity(), 10);
    EXPECT_EQ(ss.data(), arr);
}

//-----------------------------------------------------------------------------
TEST(span, first)
{
    int arr[10];
    span< int > s(arr);
    C4_STATIC_ASSERT((std::is_same< decltype(s.first(1)), decltype(s) >::value));

    auto ss = s.first(0);
    EXPECT_EQ(ss.size(), 0);
    EXPECT_EQ(ss.capacity(), 0);
    EXPECT_EQ(ss.data(), arr);

    ss = s.first(5);
    EXPECT_EQ(ss.size(), 5);
    EXPECT_EQ(ss.capacity(), 5);
    EXPECT_EQ(ss.data(), arr);
}
TEST(spanrs, first)
{
    int arr[10];
    spanrs< int > s(arr);
    C4_STATIC_ASSERT((std::is_same< decltype(s.first(1)), decltype(s) >::value));

    auto ss = s.first(0);
    EXPECT_EQ(ss.size(), 0);
    EXPECT_EQ(ss.capacity(), 10);
    EXPECT_EQ(ss.data(), arr);

    ss = s.first(5);
    EXPECT_EQ(ss.size(), 5);
    EXPECT_EQ(ss.capacity(), 10);
    EXPECT_EQ(ss.data(), arr);
}
TEST(etched_span, first)
{
    int arr[10];
    etched_span< int > s(arr);
    C4_STATIC_ASSERT((std::is_same< decltype(s.first(1)), decltype(s) >::value));

    auto ss = s.first(0);
    EXPECT_EQ(ss.size(), 0);
    EXPECT_EQ(ss.capacity(), 10);
    EXPECT_EQ(ss.data(), arr);

    ss = ss.original();
    EXPECT_EQ(ss.size(), 10);
    EXPECT_EQ(ss.capacity(), 10);
    EXPECT_EQ(ss.data(), arr);

    ss = s.first(5);
    EXPECT_EQ(ss.size(), 5);
    EXPECT_EQ(ss.capacity(), 10);
    EXPECT_EQ(ss.data(), arr);

    ss = ss.original();
    EXPECT_EQ(ss.size(), 10);
    EXPECT_EQ(ss.capacity(), 10);
    EXPECT_EQ(ss.data(), arr);
}

//-----------------------------------------------------------------------------
TEST(span, last)
{
    int arr[10];
    span< int > s(arr);
    C4_STATIC_ASSERT((std::is_same< decltype(s.last(1)), decltype(s) >::value));

    auto ss = s.last(0);
    EXPECT_EQ(ss.size(), 0);
    EXPECT_EQ(ss.capacity(), 0);
    EXPECT_EQ(ss.data(), arr + s.size());

    ss = s.last(5);
    EXPECT_EQ(ss.size(), 5);
    EXPECT_EQ(ss.capacity(), 5);
    EXPECT_EQ(ss.data(), arr + 5);
}
TEST(spanrs, last)
{
    int arr[10];
    spanrs< int > s(arr);
    C4_STATIC_ASSERT((std::is_same< decltype(s.last(1)), decltype(s) >::value));

    auto ss = s.last(0);
    EXPECT_EQ(ss.size(), 0);
    EXPECT_EQ(ss.capacity(), 0);
    EXPECT_EQ(ss.data(), arr + s.size());

    ss = s.last(5);
    EXPECT_EQ(ss.size(), 5);
    EXPECT_EQ(ss.capacity(), 5);
    EXPECT_EQ(ss.data(), arr + 5);
}
TEST(etched_span, last)
{
    int arr[10];
    etched_span< int > s(arr);
    C4_STATIC_ASSERT((std::is_same< decltype(s.last(1)), decltype(s) >::value));

    auto ss = s.last(0);
    EXPECT_EQ(ss.size(), 0);
    EXPECT_EQ(ss.capacity(), 0);
    EXPECT_EQ(ss.data(), arr + s.size());

    ss = ss.original();
    EXPECT_EQ(ss.size(), 10);
    EXPECT_EQ(ss.capacity(), 10);
    EXPECT_EQ(ss.data(), arr);

    ss = s.last(5);
    EXPECT_EQ(ss.size(), 5);
    EXPECT_EQ(ss.capacity(), 5);
    EXPECT_EQ(ss.data(), arr + 5);

    ss = ss.original();
    EXPECT_EQ(ss.size(), 10);
    EXPECT_EQ(ss.capacity(), 10);
    EXPECT_EQ(ss.data(), arr);
}

//-----------------------------------------------------------------------------
TEST(span, rtrim)
{
    int arr[10];
    span< int > s(arr);
    auto ss = s;

    ss.rtrim(0);
    EXPECT_EQ(ss.size(), s.size());
    EXPECT_EQ(ss.capacity(), s.capacity());
    EXPECT_EQ(ss.data(), arr);

    ss.rtrim(5);
    EXPECT_EQ(ss.size(), s.size() - 5);
    EXPECT_EQ(ss.capacity(), s.capacity() - 5);
    EXPECT_EQ(ss.data(), arr);
}
TEST(spanrs, rtrim)
{
    int arr[10];
    spanrs< int > s(arr);
    auto ss = s;

    ss.rtrim(0);
    EXPECT_EQ(ss.size(), s.size());
    EXPECT_EQ(ss.capacity(), s.capacity());
    EXPECT_EQ(ss.data(), arr);

    ss.rtrim(5);
    EXPECT_EQ(ss.size(), s.size() - 5);
    EXPECT_EQ(ss.capacity(), s.capacity());
    EXPECT_EQ(ss.data(), arr);
}
TEST(etched_span, rtrim)
{
    int arr[10];
    etched_span< int > s(arr);
    auto ss = s;

    ss.rtrim(0);
    EXPECT_EQ(ss.size(), s.size());
    EXPECT_EQ(ss.capacity(), s.capacity());
    EXPECT_EQ(ss.data(), arr);

    ss = ss.original();
    EXPECT_EQ(ss.size(), 10);
    EXPECT_EQ(ss.capacity(), 10);
    EXPECT_EQ(ss.data(), arr);

    ss.rtrim(5);
    EXPECT_EQ(ss.size(), s.size() - 5);
    EXPECT_EQ(ss.capacity(), s.capacity());
    EXPECT_EQ(ss.data(), arr);

    ss = ss.original();
    EXPECT_EQ(ss.size(), 10);
    EXPECT_EQ(ss.capacity(), 10);
    EXPECT_EQ(ss.data(), arr);
}

//-----------------------------------------------------------------------------
TEST(span, ltrim)
{
    int arr[10];
    span< int > s(arr);
    auto ss = s;

    ss.ltrim(0);
    EXPECT_EQ(ss.size(), s.size());
    EXPECT_EQ(ss.capacity(), s.capacity());
    EXPECT_EQ(ss.data(), arr);

    ss.ltrim(5);
    EXPECT_EQ(ss.size(), s.size() - 5);
    EXPECT_EQ(ss.capacity(), s.capacity() - 5);
    EXPECT_EQ(ss.data(), arr + 5);
}
TEST(spanrs, ltrim)
{
    int arr[10];
    spanrs< int > s(arr);
    auto ss = s;

    ss.ltrim(0);
    EXPECT_EQ(ss.size(), s.size());
    EXPECT_EQ(ss.capacity(), ss.capacity());
    EXPECT_EQ(ss.data(), arr);

    ss.ltrim(5);
    EXPECT_EQ(ss.size(), s.size() - 5);
    EXPECT_EQ(ss.capacity(), s.size() - 5);
    EXPECT_EQ(ss.data(), arr + 5);
}
TEST(etched_span, ltrim)
{
    int arr[10];
    etched_span< int > s(arr);
    auto ss = s;

    ss.ltrim(0);
    EXPECT_EQ(ss.size(), s.size());
    EXPECT_EQ(ss.capacity(), ss.capacity());
    EXPECT_EQ(ss.data(), arr);

    ss = ss.original();
    EXPECT_EQ(ss.size(), 10);
    EXPECT_EQ(ss.capacity(), 10);
    EXPECT_EQ(ss.data(), arr);

    ss.ltrim(5);
    EXPECT_EQ(ss.size(), s.size() - 5);
    EXPECT_EQ(ss.capacity(), s.size() - 5);
    EXPECT_EQ(ss.data(), arr + 5);

    ss = ss.original();
    EXPECT_EQ(ss.size(), 10);
    EXPECT_EQ(ss.capacity(), 10);
    EXPECT_EQ(ss.data(), arr);
}

//-----------------------------------------------------------------------------
const char larrc[11] = "0123456789";
const char rarrc[11] = "1234567890";
const int larri[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
const int rarri[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
TEST(span_impl, eq)
{
    EXPECT_EQ(cspan  <char>(larrc), cspan  <char>(larrc));
    EXPECT_EQ(cspanrs<char>(larrc), cspan  <char>(larrc));
    EXPECT_EQ(cspan  <char>(larrc), cspanrs<char>(larrc));
    EXPECT_EQ(cspanrs<char>(larrc), cspanrs<char>(larrc));

    EXPECT_EQ(cspan  <int>(larri), cspan  <int>(larri));
    EXPECT_EQ(cspanrs<int>(larri), cspan  <int>(larri));
    EXPECT_EQ(cspan  <int>(larri), cspanrs<int>(larri));
    EXPECT_EQ(cspanrs<int>(larri), cspanrs<int>(larri));
}

TEST(span_impl, lt)
{
    EXPECT_LT(cspan  <char>(larrc), cspan  <char>(rarrc));
    EXPECT_LT(cspanrs<char>(larrc), cspan  <char>(rarrc));
    EXPECT_LT(cspan  <char>(larrc), cspanrs<char>(rarrc));
    EXPECT_LT(cspanrs<char>(larrc), cspanrs<char>(rarrc));

    EXPECT_LT(cspan  <int>(larri), cspan  <int>(rarri));
    EXPECT_LT(cspanrs<int>(larri), cspan  <int>(rarri));
    EXPECT_LT(cspan  <int>(larri), cspanrs<int>(rarri));
    EXPECT_LT(cspanrs<int>(larri), cspanrs<int>(rarri));
}
TEST(span_impl, gt)
{
    EXPECT_GT(cspan  <char>(rarrc), cspan  <char>(larrc));
    EXPECT_GT(cspan  <char>(rarrc), cspanrs<char>(larrc));
    EXPECT_GT(cspanrs<char>(rarrc), cspan  <char>(larrc));
    EXPECT_GT(cspanrs<char>(rarrc), cspanrs<char>(larrc));

    EXPECT_GT(cspan  <int>(rarri), cspan  <int>(larri));
    EXPECT_GT(cspan  <int>(rarri), cspanrs<int>(larri));
    EXPECT_GT(cspanrs<int>(rarri), cspan  <int>(larri));
    EXPECT_GT(cspanrs<int>(rarri), cspanrs<int>(larri));
}

TEST(span_impl, ge)
{
    EXPECT_GE(cspan  <char>(rarrc), cspan  <char>(larrc));
    EXPECT_GE(cspan  <char>(rarrc), cspanrs<char>(larrc));
    EXPECT_GE(cspanrs<char>(rarrc), cspan  <char>(larrc));
    EXPECT_GE(cspanrs<char>(rarrc), cspanrs<char>(larrc));
    EXPECT_GE(cspan  <char>(larrc), cspan  <char>(larrc));
    EXPECT_GE(cspan  <char>(larrc), cspanrs<char>(larrc));
    EXPECT_GE(cspanrs<char>(larrc), cspan  <char>(larrc));
    EXPECT_GE(cspanrs<char>(larrc), cspanrs<char>(larrc));
    EXPECT_GE(cspan  <int >(rarri), cspan  <int >(larri));
    EXPECT_GE(cspan  <int >(rarri), cspanrs<int >(larri));
    EXPECT_GE(cspanrs<int >(rarri), cspan  <int >(larri));
    EXPECT_GE(cspanrs<int >(rarri), cspanrs<int >(larri));
    EXPECT_GE(cspan  <int >(larri), cspan  <int >(larri));
    EXPECT_GE(cspan  <int >(larri), cspanrs<int >(larri));
    EXPECT_GE(cspanrs<int >(larri), cspan  <int >(larri));
    EXPECT_GE(cspanrs<int >(larri), cspanrs<int >(larri));
}
TEST(span_impl, le)
{
    EXPECT_LE(cspan  <char>(larrc), cspan  <char>(rarrc));
    EXPECT_LE(cspanrs<char>(larrc), cspan  <char>(rarrc));
    EXPECT_LE(cspan  <char>(larrc), cspanrs<char>(rarrc));
    EXPECT_LE(cspanrs<char>(larrc), cspanrs<char>(rarrc));
    EXPECT_LE(cspan  <char>(larrc), cspan  <char>(larrc));
    EXPECT_LE(cspanrs<char>(larrc), cspan  <char>(larrc));
    EXPECT_LE(cspan  <char>(larrc), cspanrs<char>(larrc));
    EXPECT_LE(cspanrs<char>(larrc), cspanrs<char>(larrc));
    EXPECT_LE(cspan  <int >(larri), cspan  <int >(rarri));
    EXPECT_LE(cspanrs<int >(larri), cspan  <int >(rarri));
    EXPECT_LE(cspan  <int >(larri), cspanrs<int >(rarri));
    EXPECT_LE(cspanrs<int >(larri), cspanrs<int >(rarri));
    EXPECT_LE(cspan  <int >(larri), cspan  <int >(larri));
    EXPECT_LE(cspanrs<int >(larri), cspan  <int >(larri));
    EXPECT_LE(cspan  <int >(larri), cspanrs<int >(larri));
    EXPECT_LE(cspanrs<int >(larri), cspanrs<int >(larri));
}

C4_END_NAMESPACE(c4)
