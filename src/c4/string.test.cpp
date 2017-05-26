#include "c4/string.test.hpp"
#include "c4/sstream.hpp"
#include "c4/string.hpp"
#include "c4/hash.hpp"


C4_BEGIN_NAMESPACE(c4)

C4_BEGIN_HIDDEN_NAMESPACE

_C4_TEST_STRINGBASE_DERIVED(substringrs, substringrs)
_C4_TEST_STRINGBASE_DERIVED(string, string)
_C4_TEST_STRINGBASE_DERIVED(text, text)
//WIP..._C4_TEST_STRINGBASE_DERIVED(small_string_u8 , basic_small_string< char C4_COMMA uint8_t  >)
//WIP..._C4_TEST_STRINGBASE_DERIVED(small_string_u16, basic_small_string< char C4_COMMA uint16_t >)
//WIP..._C4_TEST_STRINGBASE_DERIVED(small_string_u32, basic_small_string< char C4_COMMA uint32_t >)
//WIP..._C4_TEST_STRINGBASE_DERIVED(small_string_u64, basic_small_string< char C4_COMMA uint64_t >)
//WIP..._C4_TEST_STRINGBASE_DERIVED(small_string_i8 , basic_small_string< char C4_COMMA int8_t   >)
//WIP..._C4_TEST_STRINGBASE_DERIVED(small_string_i16, basic_small_string< char C4_COMMA int16_t  >)
//WIP..._C4_TEST_STRINGBASE_DERIVED(small_string_i32, basic_small_string< char C4_COMMA int32_t  >)
//WIP..._C4_TEST_STRINGBASE_DERIVED(small_string_i64, basic_small_string< char C4_COMMA int64_t  >)

TEST(StringTrimOverflow, trim)
{
    using tstring = basic_text< char, uint8_t >;
    // assign a 255 char
    tstring s(' ', 254); // (+1 for the null character)
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
