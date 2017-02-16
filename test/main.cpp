#include <gtest/gtest.h>

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int stat = RUN_ALL_TESTS();
    return stat;
}
