#include "c4/test.hpp"

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int stat = RUN_ALL_TESTS();
    C4_LOGP("tests finished with status {}\n", stat);
    return stat;
}
