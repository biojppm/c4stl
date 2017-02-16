#include "sstream.hpp"

#include <gtest/gtest.h>
#include <iostream>

C4_BEGIN_NAMESPACE(c4)

template< class T >
struct exvec3
{
    T x, y, z;
    bool operator== (exvec3 const& that) const
    {
        return x == that.x && y == that.y && z == that.z;
    }
};
template< class T >
sstream& operator<< (sstream& ss, exvec3<T> const& v)
{
    ss.printp("({},{},{})", v.x, v.y, v.z);
    return ss;
}
template< class T >
sstream& operator>> (sstream& ss, exvec3<T> & v)
{
    ss.scanp("({},{},{})", v.x, v.y, v.z);
    return ss;
}

//-----------------------------------------------------------------------------
using ScalarTypes = ::testing::Types<char, i8, u8, i16, u16, i32, u32, i64, u64, float, double>;
template <class T>
struct RoundTripTest : public ::testing::Test
{
    sstream ss;
};
TYPED_TEST_CASE_P(RoundTripTest);

//-----------------------------------------------------------------------------
#define _testrtrip3(which)                                  \
    which<TypeParam>(ss, 0, 10, 20);                        \
    ss.reset();                                             \
                                                            \
    which<TypeParam>(ss, 21, 22, 23);                       \
    ss.reset();                                             \
                                                            \
    which<TypeParam>(ss, 123, 124, 125);                    \
    ss.reset();                                             \
                                                            \
    which<exvec3<TypeParam>>(ss, {1,2,3},{4,5,6},{7,8,9});  \
    ss.reset();

//-----------------------------------------------------------------------------

template< class T >
void do_round_trip_chevron(sstream &ss, T const& val1, T const& val2, T const& val3)
{
    T v1, v2, v3;
    ss << val1 << ' ' << val2 << ' '<< val3;
    char c;
    ss >> v1 >> c >> v2 >> c >> v3;
    EXPECT_EQ(v1, val1);
    EXPECT_EQ(v2, val2);
    EXPECT_EQ(v3, val3);
}
TYPED_TEST_P(RoundTripTest, chevron)
{
    _testrtrip3(do_round_trip_chevron);
}

//-----------------------------------------------------------------------------
template< class T >
void do_round_trip_printp(sstream &ss, T const& val1, T const& val2, T const& val3)
{
    T v1, v2, v3;
    ss.printp("{} {} {}", val1, val2, val3);
    ss.scanp("{} {} {}", v1, v2, v3);
    EXPECT_EQ(v1, val1);
    EXPECT_EQ(v2, val2);
    EXPECT_EQ(v3, val3);

    ss.reset();
    ss.printp("{} aaaaaaaaa {} bbbb {} ccc", val1, val2, val3);
    ss.scanp("{} aaaaaaaaa {} bbbb {} ccc", v1, v2, v3);
    EXPECT_EQ(v1, val1);
    EXPECT_EQ(v2, val2);
    EXPECT_EQ(v3, val3);

    ss.reset();
    ss.printp("{}aaaaaaaaa{}bbbb{}ccc", val1, val2, val3);
    ss.scanp("{}aaaaaaaaa{}bbbb{}ccc", v1, v2, v3);
    EXPECT_EQ(v1, val1);
    EXPECT_EQ(v2, val2);
    EXPECT_EQ(v3, val3);
}
TYPED_TEST_P(RoundTripTest, printp)
{
    _testrtrip3(do_round_trip_printp);
}

//-----------------------------------------------------------------------------
template< class T >
void do_round_trip_cat(sstream &ss, T const& val1, T const& val2, T const& val3)
{
    T v1, v2, v3;
    ss.cat(val1, ' ', val2, ' ', val3);
    char c;
    ss.uncat(v1, c, v2, c, v3);
    EXPECT_EQ(v1, val1);
    EXPECT_EQ(v2, val2);
    EXPECT_EQ(v3, val3);
}
TYPED_TEST_P(RoundTripTest, cat)
{
    _testrtrip3(do_round_trip_cat);
}

//-----------------------------------------------------------------------------
template< class T >
void do_round_trip_catsep(sstream &ss, T const& val1, T const& val2, T const& val3)
{
    T v1, v2, v3;
    ss.catsep(' ', val1, val2, val3);
    ss.uncatsep(' ', v1, v2, v3);
    EXPECT_EQ(v1, val1);
    EXPECT_EQ(v2, val2);
    EXPECT_EQ(v3, val3);
}
TYPED_TEST_P(RoundTripTest, catsep)
{
    _testrtrip3(do_round_trip_catsep);
}

REGISTER_TYPED_TEST_CASE_P(RoundTripTest, chevron, printp, cat, catsep);

INSTANTIATE_TYPED_TEST_CASE_P(sstream, RoundTripTest, ScalarTypes);

C4_END_NAMESPACE(c4)
