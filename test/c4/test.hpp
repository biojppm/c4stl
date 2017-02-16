#ifndef _C4_TEST_HPP_
#define _C4_TEST_HPP_

#include "c4/config.hpp"

#include <gtest/gtest.h>

#define C4_EXPECT(expr) C4_EXPECT_EQ(expr, true)

#define C4_EXPECT_NE(expr1, expr2) EXPECT_NE(expr1, expr2) << C4_PRETTY_FUNC << "\n"
#define C4_EXPECT_EQ(expr1, expr2) EXPECT_EQ(expr1, expr2) << C4_PRETTY_FUNC << "\n"
#define C4_EXPECT_LT(expr1, expr2) EXPECT_LT(expr1, expr2) << C4_PRETTY_FUNC << "\n"
#define C4_EXPECT_GT(expr1, expr2) EXPECT_GT(expr1, expr2) << C4_PRETTY_FUNC << "\n"
#define C4_EXPECT_GE(expr1, expr2) EXPECT_GE(expr1, expr2) << C4_PRETTY_FUNC << "\n"
#define C4_EXPECT_LE(expr1, expr2) EXPECT_LE(expr1, expr2) << C4_PRETTY_FUNC << "\n"

#define C4_EXPECT_STREQ(expr1, expr2) EXPECT_STREQ(expr1, expr2) << C4_PRETTY_FUNC << "\n"
#define C4_EXPECT_STRNE(expr1, expr2) EXPECT_STRNE(expr1, expr2) << C4_PRETTY_FUNC << "\n"

#define C4_EXPECT_STRCASEEQ(expr1, expr2) EXPECT_STRCASEEQ(expr1, expr2) << C4_PRETTY_FUNC << "\n"
#define C4_EXPECT_STRCASENE(expr1, expr2) EXPECT_STRCASENE(expr1, expr2) << C4_PRETTY_FUNC << "\n"



#define C4_TEST(ts, tn) TEST_F(::c4::TestFixture, ts##_##tn)


C4_BEGIN_NAMESPACE(c4)

class TestFixture : public ::testing::Test
{
protected:
    // You can remove any or all of the following functions if its body
    // is empty.

    // You can do set-up work for each test here.
    TestFixture() {}

    // You can do clean-up work that doesn't throw exceptions here.
    virtual ~TestFixture() {}

    // If the constructor and destructor are not enough for setting up
    // and cleaning up each test, you can define the following methods:

    // Code here will be called immediately after the constructor (right
    // before each test).
    virtual void SetUp() {}

    // Code here will be called immediately after each test (right
    // before the destructor).
    virtual void TearDown() {}

    // Objects declared here can be used by all tests in the test case for Foo.
};

C4_END_NAMESPACE(c4)

#endif // _C4_TEST_HPP_
