#include "sstream.hpp"

#include <gtest/gtest.h>
#include <iostream>

C4_BEGIN_NAMESPACE(c4)

struct exvec3
{
    int x, y, z;
    bool operator== (exvec3 const& that) const
    {
        return x == that.x && y == that.y && z == that.z;
    }
};
sstream& operator<< (sstream& ss, exvec3 const& v)
{
    ss.printp("({},{},{})", v.x, v.y, v.z);
    return ss;
}
sstream& operator>> (sstream& ss, exvec3 & v)
{
    ss.scanp("({},{},{})", v.x, v.y, v.z);
    return ss;
}

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
template< class T >
void round_trip(sstream &ss, T const& val1, T const& val2, T const& val3)
{
    ss.reset();
    do_round_trip_chevron< T >(ss, val1, val2, val3);
    ss.reset();
    do_round_trip_printp< T >(ss, val1, val2, val3);
    ss.reset();
    do_round_trip_cat< T >(ss, val1, val2, val3);
}

TEST(sstream, round_trip_chevron)
{
    sstream ss;

    round_trip<int>(ss, 0, 10, 20);
    round_trip<int>(ss, 1238123, 876823, 324787);
    round_trip<exvec3>(ss, {1,2,3},{4,5,6},{7,8,9});
}

C4_END_NAMESPACE(c4)
