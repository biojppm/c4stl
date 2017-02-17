#include "c4/error.hpp"

#include <gtest/gtest.h>

C4_BEGIN_NAMESPACE(c4)

C4_BEGIN_HIDDEN_NAMESPACE
bool was_called = false;
void error_callback(const char *msg, size_t msg_sz)
{
    EXPECT_EQ(strncmp(msg, "bla bla", msg_sz), 0);
    was_called = true;
}
C4_END_HIDDEN_NAMESPACE

TEST(Error, scoped_callback)
{
    auto orig = get_error_callback();
    {
        auto tmp = ScopedErrorSettings(ON_ERROR_CALLBACK, error_callback);
        EXPECT_EQ(get_error_callback() == error_callback, true);
        C4_ERROR("bla bla");
        EXPECT_EQ(was_called, true);
    }
    EXPECT_EQ(get_error_callback() == orig, true);
}

C4_END_NAMESPACE(c4)
