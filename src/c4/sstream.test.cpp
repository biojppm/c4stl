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
    using char_type = typename sstream< String >::char_type;
    ss.printp(C4_TXTTY("({},{},{})", char_type), v.x, v.y, v.z);
    return ss;
}
template< class String, class T >
sstream< String >& operator>> (sstream< String >& ss, exvec3<T> & v)
{
    using char_type = typename sstream< String >::char_type;
    ss.scanp(C4_TXTTY("({},{},{})", char_type), v.x, v.y, v.z);
    return ss;
}

//-----------------------------------------------------------------------------
template <class T>
struct RoundTripTest_c4string : public ::testing::Test
{
    sstream< c4::string > ss;
};
template <class T>
struct RoundTripTest_c4wstring : public ::testing::Test
{
    sstream< c4::wstring > ss;
};
template <class T>
struct RoundTripTest_c4substring : public ::testing::Test
{
    c4::string str;
    sstream< c4::substring > ss;
    RoundTripTest_c4substring() : str(512), ss(str.data(), str.size())
    {
    }
};
template <class T>
struct RoundTripTest_c4wsubstring : public ::testing::Test
{
    c4::wstring str;
    sstream< c4::wsubstring > ss;
    RoundTripTest_c4wsubstring() : str(512), ss(str.data(), str.size())
    {
    }
};
template <class T>
struct RoundTripTest_stdstring : public ::testing::Test
{
    sstream< std::string > ss;
};
template <class T>
struct RoundTripTest_stdwstring : public ::testing::Test
{
    sstream< std::wstring > ss;
};
TYPED_TEST_CASE_P(RoundTripTest_c4string);
TYPED_TEST_CASE_P(RoundTripTest_c4wstring);
TYPED_TEST_CASE_P(RoundTripTest_c4substring);
TYPED_TEST_CASE_P(RoundTripTest_c4wsubstring);
TYPED_TEST_CASE_P(RoundTripTest_stdstring);
TYPED_TEST_CASE_P(RoundTripTest_stdwstring);

//-----------------------------------------------------------------------------
#define _testrtrip3(which, strtype)                                 \
                                                                    \
    which<strtype, TypeParam>(this->ss, 65, 66, 67);                      \
    this->ss.reset();                                                     \
                                                                    \
    which<strtype, TypeParam>(this->ss, 97, 98, 99);                      \
    this->ss.reset();                                                     \
                                                                    \
    which<strtype, TypeParam>(this->ss, 123, 124, 125);                   \
    this->ss.reset();                                                     \
                                                                    \
    which<strtype, exvec3<TypeParam>>(this->ss, {65,66,67},{68,69,70},{71,72,73}); \
    this->ss.reset();

//-----------------------------------------------------------------------------

template< class String, class T >
void do_round_trip_chevron(sstream<String> &ss, T const& val1, T const& val2, T const& val3)
{
    using char_type = typename sstream< String >::char_type;
    T v1, v2, v3;
    ss << val1 << char_type(' ') << val2 << char_type(' ') << val3;
    char_type c;
    ss >> v1 >> c >> v2 >> c >> v3;
    EXPECT_EQ(v1, val1) << C4_PRETTY_FUNC;
    EXPECT_EQ(v2, val2) << C4_PRETTY_FUNC;
    EXPECT_EQ(v3, val3) << C4_PRETTY_FUNC;
}
TYPED_TEST_P(RoundTripTest_c4string, chevron)
{
    _testrtrip3(do_round_trip_chevron, c4::string);
}
TYPED_TEST_P(RoundTripTest_c4wstring, chevron)
{
    _testrtrip3(do_round_trip_chevron, c4::wstring);
}
TYPED_TEST_P(RoundTripTest_c4substring, chevron)
{
    _testrtrip3(do_round_trip_chevron, c4::substring);
}
TYPED_TEST_P(RoundTripTest_c4wsubstring, chevron)
{
    _testrtrip3(do_round_trip_chevron, c4::wsubstring);
}
TYPED_TEST_P(RoundTripTest_stdstring, chevron)
{
    _testrtrip3(do_round_trip_chevron, std::string);
}
TYPED_TEST_P(RoundTripTest_stdwstring, chevron)
{
    _testrtrip3(do_round_trip_chevron, std::wstring);
}

//-----------------------------------------------------------------------------
template< class String, class T >
void do_round_trip_printp(sstream<String> &ss, T const& val1, T const& val2, T const& val3)
{
    using char_type = typename sstream< String >::char_type;

    T v1, v2, v3;
    ss.printp(C4_TXTTY("{} {} {}", char_type), val1, val2, val3);
    ss.scanp(C4_TXTTY("{} {} {}", char_type), v1, v2, v3);
    EXPECT_EQ(v1, val1);
    EXPECT_EQ(v2, val2);
    EXPECT_EQ(v3, val3);

    ss.reset(); v1 = v2 = v3 = {};
    ss.printp(C4_TXTTY("{} aaaaaaaaa {} bbbb {} ccc", char_type), val1, val2, val3);
    ss.scanp(C4_TXTTY("{} aaaaaaaaa {} bbbb {} ccc", char_type), v1, v2, v3);
    EXPECT_EQ(v1, val1);
    EXPECT_EQ(v2, val2);
    EXPECT_EQ(v3, val3);

    ss.reset(); v1 = v2 = v3 = {};
    ss.printp(C4_TXTTY("{} aaaaaaaaa_{} bbbb_{} ccc", char_type), val1, val2, val3);
    ss.scanp(C4_TXTTY("{} aaaaaaaaa_{} bbbb_{} ccc", char_type), v1, v2, v3);
    EXPECT_EQ(v1, val1);
    EXPECT_EQ(v2, val2);
    EXPECT_EQ(v3, val3);
}
TYPED_TEST_P(RoundTripTest_c4string, printp)
{
    _testrtrip3(do_round_trip_printp, c4::string);
}
TYPED_TEST_P(RoundTripTest_c4wstring, printp)
{
    _testrtrip3(do_round_trip_printp, c4::wstring);
}
TYPED_TEST_P(RoundTripTest_c4substring, printp)
{
    _testrtrip3(do_round_trip_printp, c4::substring);
}
TYPED_TEST_P(RoundTripTest_c4wsubstring, printp)
{
    _testrtrip3(do_round_trip_printp, c4::wsubstring);
}
TYPED_TEST_P(RoundTripTest_stdstring, printp)
{
    _testrtrip3(do_round_trip_printp, std::string);
}
TYPED_TEST_P(RoundTripTest_stdwstring, printp)
{
    _testrtrip3(do_round_trip_printp, std::wstring);
}

//-----------------------------------------------------------------------------
template< class String, class T >
void do_round_trip_cat(sstream<String> &ss, T const& val1, T const& val2, T const& val3)
{
    using char_type = typename sstream< String >::char_type;
    T v1, v2, v3;
    ss.cat(val1, char_type(' '), val2, char_type(' '), val3);
    char_type c;
    ss.uncat(v1, c, v2, c, v3);
    EXPECT_EQ(v1, val1);
    EXPECT_EQ(v2, val2);
    EXPECT_EQ(v3, val3);
}
TYPED_TEST_P(RoundTripTest_c4string, cat)
{
    _testrtrip3(do_round_trip_cat, c4::string);
}
TYPED_TEST_P(RoundTripTest_c4wstring, cat)
{
    _testrtrip3(do_round_trip_cat, c4::wstring);
}
TYPED_TEST_P(RoundTripTest_c4substring, cat)
{
    _testrtrip3(do_round_trip_cat, c4::substring);
}
TYPED_TEST_P(RoundTripTest_c4wsubstring, cat)
{
    _testrtrip3(do_round_trip_cat, c4::wsubstring);
}
TYPED_TEST_P(RoundTripTest_stdstring, cat)
{
    _testrtrip3(do_round_trip_cat, std::string);
}
TYPED_TEST_P(RoundTripTest_stdwstring, cat)
{
    _testrtrip3(do_round_trip_cat, std::wstring);
}


//-----------------------------------------------------------------------------
template< class String, class T >
void do_round_trip_catsep(sstream<String> &ss, T const& val1, T const& val2, T const& val3)
{
    using char_type = typename sstream< String >::char_type;
    T v1, v2, v3;
    ss.catsep(char_type(' '), val1, val2, val3);
    ss.uncatsep(char_type(' '), v1, v2, v3);
    EXPECT_EQ(v1, val1);
    EXPECT_EQ(v2, val2);
    EXPECT_EQ(v3, val3);
}
TYPED_TEST_P(RoundTripTest_c4string, catsep)
{
    _testrtrip3(do_round_trip_catsep, c4::string);
}
TYPED_TEST_P(RoundTripTest_c4wstring, catsep)
{
    _testrtrip3(do_round_trip_catsep, c4::wstring);
}
TYPED_TEST_P(RoundTripTest_c4substring, catsep)
{
    _testrtrip3(do_round_trip_catsep, c4::substring);
}
TYPED_TEST_P(RoundTripTest_c4wsubstring, catsep)
{
    _testrtrip3(do_round_trip_catsep, c4::wsubstring);
}
TYPED_TEST_P(RoundTripTest_stdstring, catsep)
{
    _testrtrip3(do_round_trip_catsep, std::string);
}
TYPED_TEST_P(RoundTripTest_stdwstring, catsep)
{
    _testrtrip3(do_round_trip_catsep, std::wstring);
}

REGISTER_TYPED_TEST_CASE_P(RoundTripTest_c4string, chevron, printp, cat, catsep);
REGISTER_TYPED_TEST_CASE_P(RoundTripTest_c4wstring, chevron, printp, cat, catsep);
REGISTER_TYPED_TEST_CASE_P(RoundTripTest_c4substring, chevron, printp, cat, catsep);
REGISTER_TYPED_TEST_CASE_P(RoundTripTest_c4wsubstring, chevron, printp, cat, catsep);
REGISTER_TYPED_TEST_CASE_P(RoundTripTest_stdstring, chevron, printp, cat, catsep);
REGISTER_TYPED_TEST_CASE_P(RoundTripTest_stdwstring, chevron, printp, cat, catsep);

using ScalarTypes = ::testing::Types<char, wchar_t, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t, float, double>;

INSTANTIATE_TYPED_TEST_CASE_P(sstream, RoundTripTest_c4string, ScalarTypes);
INSTANTIATE_TYPED_TEST_CASE_P(sstream, RoundTripTest_c4wstring, ScalarTypes);
INSTANTIATE_TYPED_TEST_CASE_P(sstream, RoundTripTest_c4substring, ScalarTypes);
INSTANTIATE_TYPED_TEST_CASE_P(sstream, RoundTripTest_c4wsubstring, ScalarTypes);
INSTANTIATE_TYPED_TEST_CASE_P(sstream, RoundTripTest_stdstring, ScalarTypes);
INSTANTIATE_TYPED_TEST_CASE_P(sstream, RoundTripTest_stdwstring, ScalarTypes);

C4_END_NAMESPACE(c4)
