
#include "c4/test/string.hpp"
#include "c4/sstream.hpp"
#include "c4/string.hpp"
#include "c4/hash.hpp"


C4_BEGIN_NAMESPACE(c4)

/** this function was made a friend to have access to member names */
void test_string_member_alignment()
{
    EXPECT_EQ(offsetof(c4::text, m_str     ), offsetof(c4::substring   , m_str     ));
    EXPECT_EQ(offsetof(c4::text, m_str     ), offsetof(c4::substringrs , m_str     ));
    EXPECT_EQ(offsetof(c4::text, m_size    ), offsetof(c4::substring   , m_size    ));
    EXPECT_EQ(offsetof(c4::text, m_size    ), offsetof(c4::substringrs , m_size    ));
    EXPECT_EQ(offsetof(c4::text, m_capacity), offsetof(c4::substringrs , m_capacity));
    EXPECT_EQ(offsetof(c4::text, m_str     ), offsetof(c4::csubstring  , m_str     ));
    EXPECT_EQ(offsetof(c4::text, m_str     ), offsetof(c4::csubstringrs, m_str     ));
    EXPECT_EQ(offsetof(c4::text, m_size    ), offsetof(c4::csubstring  , m_size    ));
    EXPECT_EQ(offsetof(c4::text, m_size    ), offsetof(c4::csubstringrs, m_size    ));
    EXPECT_EQ(offsetof(c4::text, m_capacity), offsetof(c4::csubstringrs, m_capacity));

    EXPECT_EQ(offsetof(c4::wtext, m_str     ), offsetof(c4::wsubstring  , m_str     ));
    EXPECT_EQ(offsetof(c4::wtext, m_str     ), offsetof(c4::wsubstringrs, m_str     ));
    EXPECT_EQ(offsetof(c4::wtext, m_size    ), offsetof(c4::wsubstring  , m_size    ));
    EXPECT_EQ(offsetof(c4::wtext, m_size    ), offsetof(c4::wsubstringrs, m_size    ));
    EXPECT_EQ(offsetof(c4::wtext, m_capacity), offsetof(c4::wsubstringrs, m_capacity));

    EXPECT_EQ(offsetof(c4::wtext, m_str     ), offsetof(c4::cwsubstring  , m_str     ));
    EXPECT_EQ(offsetof(c4::wtext, m_str     ), offsetof(c4::cwsubstringrs, m_str     ));
    EXPECT_EQ(offsetof(c4::wtext, m_size    ), offsetof(c4::cwsubstring  , m_size    ));
    EXPECT_EQ(offsetof(c4::wtext, m_size    ), offsetof(c4::cwsubstringrs, m_size    ));
    EXPECT_EQ(offsetof(c4::wtext, m_capacity), offsetof(c4::cwsubstringrs, m_capacity));
}

template< class C, class I >
void test_small_string_flag_alignment()
{
    basic_string< C, I > ss;
    EXPECT_EQ((char*)&ss.m_short.flag_n_sz, (char*)&ss.m_long.flag_n_sz);

    ss.m_short.flag_n_sz = 0;
    ss.m_long.flag_n_sz = 0;
    EXPECT_EQ(ss.m_short.flag_n_sz, ss.m_long.flag_n_sz);

    ss.m_short.flag_n_sz |= 1;
    EXPECT_EQ(ss.m_short.flag_n_sz & 1, ss.m_long.flag_n_sz & 1);
    EXPECT_EQ(ss.m_short.flag_n_sz, ss.m_long.flag_n_sz);

    ss.m_short.flag_n_sz &= ~1;
    EXPECT_EQ(ss.m_short.flag_n_sz & 1, ss.m_long.flag_n_sz & 1);
    EXPECT_EQ(ss.m_short.flag_n_sz, ss.m_long.flag_n_sz);

    ss.m_long.flag_n_sz |= 1;
    EXPECT_EQ(ss.m_short.flag_n_sz & 1, ss.m_long.flag_n_sz & 1);
    EXPECT_EQ(ss.m_short.flag_n_sz, ss.m_long.flag_n_sz);

    ss.m_long.flag_n_sz &= ~1;
    EXPECT_EQ(ss.m_short.flag_n_sz & 1, ss.m_long.flag_n_sz & 1);
    EXPECT_EQ(ss.m_short.flag_n_sz, ss.m_long.flag_n_sz);
}

C4_BEGIN_HIDDEN_NAMESPACE

TEST(StringInterop, member_alignment)
{
    test_string_member_alignment();
}

TEST(StringInterop, to_span)
{
    span< char > sp;
    c4::string str("asdkljasdkja");
    sp = str;
    C4_EXPECT_EQ(sp.data(), str.data());
    C4_EXPECT_EQ(sp.size(), str.size());
}

TEST(StringInterop, small_string_flag_alignment)
{
    test_small_string_flag_alignment< char, int8_t >();
    test_small_string_flag_alignment< char, int16_t >();
    test_small_string_flag_alignment< char, int32_t >();
    test_small_string_flag_alignment< char, int64_t >();
    test_small_string_flag_alignment< char, uint8_t >();
    test_small_string_flag_alignment< char, uint16_t >();
    test_small_string_flag_alignment< char, uint32_t >();
    test_small_string_flag_alignment< char, uint64_t >();

    test_small_string_flag_alignment< wchar_t, int8_t >();
    test_small_string_flag_alignment< wchar_t, int16_t >();
    test_small_string_flag_alignment< wchar_t, int32_t >();
    test_small_string_flag_alignment< wchar_t, int64_t >();
    test_small_string_flag_alignment< wchar_t, uint8_t >();
    test_small_string_flag_alignment< wchar_t, uint16_t >();
    test_small_string_flag_alignment< wchar_t, uint32_t >();
    test_small_string_flag_alignment< wchar_t, uint64_t >();
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

C4_END_HIDDEN_NAMESPACE
C4_END_NAMESPACE(c4)
