#include "c4/test.hpp"

C4_BEGIN_NAMESPACE(c4)
C4_BEGIN_NAMESPACE(stg)

template< class RawStorage >
struct test_raw_storage
{

    void test_ctor()
    {
        {
            RawStorage rs;
            EXPECT_EQ(rs.empty(), true);
        }
    }
};


C4_END_NAMESPACE(stg)
C4_END_NAMESPACE(c4)
