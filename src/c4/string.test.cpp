#include "c4/string.test.hpp"
#include "c4/sstream.hpp"
#include "c4/string.hpp"
#include "c4/hash.hpp"


C4_BEGIN_NAMESPACE(c4)

/** this function was made a friend to have access to member names */
void test_string_member_alignment()
{
    EXPECT_EQ(offsetof(c4::string, m_str     ), offsetof(c4::substring  , m_str     ));
    EXPECT_EQ(offsetof(c4::string, m_str     ), offsetof(c4::substringrs, m_str     ));
    EXPECT_EQ(offsetof(c4::string, m_size    ), offsetof(c4::substring  , m_size    ));
    EXPECT_EQ(offsetof(c4::string, m_size    ), offsetof(c4::substringrs, m_size    ));
    EXPECT_EQ(offsetof(c4::string, m_capacity), offsetof(c4::substringrs, m_capacity));
}

C4_BEGIN_HIDDEN_NAMESPACE

TEST(StringConversion, member_alignment)
{
    test_string_member_alignment();
}

_C4_TEST_STRINGBASE_DERIVED_NON_RESIZEABLE(substring)
_C4_TEST_STRINGBASE_DERIVED(substringrs)
_C4_TEST_STRINGBASE_DERIVED(string)

TEST(StringTrimOverflow, trim)
{
    using tstring = basic_string< char, uint8_t >;
    // assign a 255 char 
    tstring s(' ', 254); // (+1 for the null character)
}

TEST(StringInterop, StringVsSubstring)
{
    string s, t, r;
    s = "b a ba";
    t = "supercallifragilisticexpialidocious";

    {
        substring ss = s;
        C4_EXPECT_EQ(s, ss);
        C4_EXPECT_EQ(s.data(), ss.data());
        r = ss;
        C4_EXPECT_EQ(r, ss);
        C4_EXPECT_EQ(r, s);
        C4_EXPECT_NE(r.data(), ss.data());
        C4_EXPECT_NE(r.data(), s.data());
    }

    {
        substring ss = t;
        C4_EXPECT_EQ(t, ss);
        C4_EXPECT_EQ(t.data(), ss.data());
        r = ss;
        C4_EXPECT_EQ(r, ss);
        C4_EXPECT_EQ(r, t);
        C4_EXPECT_NE(r.data(), ss.data());
        C4_EXPECT_NE(r.data(), s.data());
    }

    {
    }
}

TEST(string, ctor)
{
    {
        string s;
        C4_EXPECT_EQ(s.empty(), true);
    }

    {
        const char *n = "asdkasdjkasd";
        const string s2(n);
        C4_EXPECT_EQ(s2.empty(), false);
        C4_EXPECT_EQ(s2, n); // string contents are the same
        C4_EXPECT_NE(s2.c_str(), n); // pointers are not the same

        const string s3(n, size_t(3));
        C4_EXPECT_EQ(s3.empty(), false);
        C4_EXPECT_EQ(s3.size(), 3);
        C4_EXPECT_EQ(s3, "asd"); // string contents are the same
        C4_EXPECT_NE(s3.c_str(), n); // pointers are not the same
    }
}

TEST(string, appendzz)
{
    string s1, s2 = "s2", s3 = "s3";

    s1 = ".";
    s1 += s2;
    s1 += s3;
    C4_EXPECT_EQ(s1, ".s2s3");

    s1.append("s4");
    C4_EXPECT_EQ(s1, ".s2s3s4");

    s1 = s2 + s3;
    C4_EXPECT_EQ(s1, "s2s3");

    auto ss0 = s3.substr(0,1);
    auto ss1 = s3.substr(1,1);
    s1 = s2 + s3 + ss0 + ss1 + ss0 + ss1;;
    C4_EXPECT_EQ(s1, "s2s3s3s3");
}


C4_END_HIDDEN_NAMESPACE
C4_END_NAMESPACE(c4)
