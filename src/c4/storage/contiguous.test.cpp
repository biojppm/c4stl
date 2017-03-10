#include "c4/storage/contiguous.hpp"
#include "c4/test.hpp"
#include "c4/archetypes.hpp"

C4_BEGIN_NAMESPACE(c4)
C4_BEGIN_NAMESPACE(stg)

TYPED_TEST_P(container_test, ctor_aggregate)
{
    auto il = this->getil();
    fixed_size< TypeParam, 8 > fs(aggregate, il);

    EXPECT_EQ(fs.size(), il.size());

    for(int i = 0, e = fs.size(); i < e; ++i)
    {
        EXPECT_EQ(fs[i], *(il.begin() + i));
    }
}

TYPED_TEST_P(container_test, subspan)
{
    auto il = this->getil();
    fixed_size< TypeParam, 8 > fs(aggregate, this->getil());

    EXPECT_EQ(fs.size(), il.size());

    auto s = fs.subspan();
    EXPECT_EQ(s.begin(), fs.begin());
    EXPECT_EQ(s.end(),   fs.end());
    EXPECT_EQ(s.data(),  fs.data());
    EXPECT_EQ(s.size(),  fs.size());

    span< TypeParam > ss = fs;
    EXPECT_EQ(ss.begin(), fs.begin());
    EXPECT_EQ(ss.end(),   fs.end());
    EXPECT_EQ(ss.data(),  fs.data());
    EXPECT_EQ(ss.size(),  fs.size());
}

REGISTER_TYPED_TEST_CASE_P(container_test, ctor_aggregate, subspan);

INSTANTIATE_TYPED_TEST_CASE_P(fixed_size8, container_test, c4::archetypes::containees);

C4_END_NAMESPACE(stg)
C4_END_NAMESPACE(c4)
