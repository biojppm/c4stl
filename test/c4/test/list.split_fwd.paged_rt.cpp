#include "c4/test/list.hpp"
#include "c4/libtest/supprwarn_push.hpp"

C4_BEGIN_NAMESPACE(c4)

TEST(split_fwd_list, ilist)
{
    split_fwd_list<int> li(c4::aggregate, {0, 1, 2, 3, 4});
    EXPECT_FALSE(li.empty());
    EXPECT_EQ(li.size(), 5);
    EXPECT_EQ(li.capacity(), li.m_elm.page_size());
    EXPECT_EQ(li.elm(0), 0);
    EXPECT_EQ(li.elm(1), 1);
    EXPECT_EQ(li.elm(2), 2);
    EXPECT_EQ(li.elm(3), 3);
    EXPECT_EQ(li.elm(4), 4);
}

#define _c4adaptor(tyname, ty) \
    _C4_CALL_SPLIT_FWD_LIST_TESTS_FOR_STORAGE(tyname, ty, raw_paged_rt, _C4_RAW_PAGED_RT)

CALL_FOR_CONTAINEE_ARCHETYPES(_c4adaptor)

C4_END_NAMESPACE(c4)

#include "c4/libtest/supprwarn_pop.hpp"
