#include "sstream.hpp"
#include "c4/string.hpp"

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
template< class String, class T >
sstream< String >& operator<< (sstream< String >& ss, exvec3<T> const& v)
{
    ss.printp("({},{},{})", v.x, v.y, v.z);
    return ss;
}
template< class String, class T >
sstream< String >& operator>> (sstream< String >& ss, exvec3<T> & v)
{
    ss.scanp("({},{},{})", v.x, v.y, v.z);
    return ss;
}

//-----------------------------------------------------------------------------
template <class T>
struct RoundTripTest_c4string : public ::testing::Test
{
    sstream< c4::string > ss;
};
template <class T>
struct RoundTripTest_stdstring : public ::testing::Test
{
    sstream< std::string > ss;
};
TYPED_TEST_CASE_P(RoundTripTest_c4string);
TYPED_TEST_CASE_P(RoundTripTest_stdstring);

//-----------------------------------------------------------------------------
#define _testrtrip3(which, strtype)                                 \
                                                                    \
    which<strtype, TypeParam>(ss, 65, 66, 67);                      \
    ss.reset();                                                     \
                                                                    \
    which<strtype, TypeParam>(ss, 97, 98, 99);                      \
    ss.reset();                                                     \
                                                                    \
    which<strtype, TypeParam>(ss, 123, 124, 125);                   \
    ss.reset();                                                     \
                                                                    \
    which<strtype, exvec3<TypeParam>>(ss, {65,66,67},{68,69,70},{71,72,73}); \
    ss.reset();

//-----------------------------------------------------------------------------

template< class String, class T >
void do_round_trip_chevron(sstream<String> &ss, T const& val1, T const& val2, T const& val3)
{
    T v1, v2, v3;
    ss << val1 << ' ' << val2 << ' '<< val3;
    char c;
    ss >> v1 >> c >> v2 >> c >> v3;
    EXPECT_EQ(v1, val1);
    EXPECT_EQ(v2, val2);
    EXPECT_EQ(v3, val3);
}
TYPED_TEST_P(RoundTripTest_c4string, chevron)
{
    _testrtrip3(do_round_trip_chevron, c4::string);
}
TYPED_TEST_P(RoundTripTest_stdstring, chevron)
{
    _testrtrip3(do_round_trip_chevron, std::string);
}

//-----------------------------------------------------------------------------
template< class String, class T >
void do_round_trip_printp(sstream<String> &ss, T const& val1, T const& val2, T const& val3)
{
    T v1, v2, v3;
    ss.printp("{} {} {}", val1, val2, val3);
    ss.scanp("{} {} {}", v1, v2, v3);
    EXPECT_EQ(v1, val1);
    EXPECT_EQ(v2, val2);
    EXPECT_EQ(v3, val3);

    ss.reset(); v1 = v2 = v3 = {};
    ss.printp("{} aaaaaaaaa {} bbbb {} ccc", val1, val2, val3);
    ss.scanp("{} aaaaaaaaa {} bbbb {} ccc", v1, v2, v3);
    EXPECT_EQ(v1, val1);
    EXPECT_EQ(v2, val2);
    EXPECT_EQ(v3, val3);

    ss.reset(); v1 = v2 = v3 = {};
    ss.printp("{} aaaaaaaaa_{} bbbb_{} ccc", val1, val2, val3);
    ss.scanp("{} aaaaaaaaa_{} bbbb_{} ccc", v1, v2, v3);
    EXPECT_EQ(v1, val1);
    EXPECT_EQ(v2, val2);
    EXPECT_EQ(v3, val3);
}
TYPED_TEST_P(RoundTripTest_c4string, printp)
{
    _testrtrip3(do_round_trip_printp, c4::string);
}
TYPED_TEST_P(RoundTripTest_stdstring, printp)
{
    _testrtrip3(do_round_trip_printp, std::string);
}

//-----------------------------------------------------------------------------
template< class String, class T >
void do_round_trip_cat(sstream<String> &ss, T const& val1, T const& val2, T const& val3)
{
    T v1, v2, v3;
    ss.cat(val1, ' ', val2, ' ', val3);
    char c;
    ss.uncat(v1, c, v2, c, v3);
    EXPECT_EQ(v1, val1);
    EXPECT_EQ(v2, val2);
    EXPECT_EQ(v3, val3);
}
TYPED_TEST_P(RoundTripTest_c4string, cat)
{
    _testrtrip3(do_round_trip_cat, c4::string);
}
TYPED_TEST_P(RoundTripTest_stdstring, cat)
{
    _testrtrip3(do_round_trip_cat, std::string);
}

//-----------------------------------------------------------------------------
template< class String, class T >
void do_round_trip_catsep(sstream<String> &ss, T const& val1, T const& val2, T const& val3)
{
    T v1, v2, v3;
    ss.catsep(' ', val1, val2, val3);
    ss.uncatsep(' ', v1, v2, v3);
    EXPECT_EQ(v1, val1);
    EXPECT_EQ(v2, val2);
    EXPECT_EQ(v3, val3);
}
TYPED_TEST_P(RoundTripTest_c4string, catsep)
{
    _testrtrip3(do_round_trip_catsep, c4::string);
}
TYPED_TEST_P(RoundTripTest_stdstring, catsep)
{
    _testrtrip3(do_round_trip_catsep, std::string);
}

REGISTER_TYPED_TEST_CASE_P(RoundTripTest_c4string, chevron, printp, cat, catsep);
REGISTER_TYPED_TEST_CASE_P(RoundTripTest_stdstring, chevron, printp, cat, catsep);

using ScalarTypes = ::testing::Types<char, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t, float, double>;
INSTANTIATE_TYPED_TEST_CASE_P(sstream, RoundTripTest_c4string, ScalarTypes);
INSTANTIATE_TYPED_TEST_CASE_P(sstream, RoundTripTest_stdstring, ScalarTypes);

C4_END_NAMESPACE(c4)
