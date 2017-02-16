#include "c4/error.hpp"

#include <gtest/gtest.h>

C4_BEGIN_NAMESPACE(c4)

C4_BEGIN_HIDDEN_NAMESPACE
bool was_called = false;
void error_callback()
{
    was_called = true;
}
C4_END_HIDDEN_NAMESPACE

TEST(Error, scoped_callback)
{
    auto orig = get_error_callback();
    {
        auto tmp = ScopedErrorCallback(error_callback);
        EXPECT_EQ(get_error_callback() == error_callback, true);
        C4_ERROR("bla bla");
        EXPECT_EQ(was_called, true);
    }
    EXPECT_EQ(get_error_callback() == orig, true);
}

C4_END_NAMESPACE(c4)
