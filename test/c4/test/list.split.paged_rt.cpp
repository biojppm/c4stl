// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "c4/test/list.hpp"

C4_BEGIN_NAMESPACE(c4)

TEST(split_list, ilist)
{
    split_list<int> li(c4::aggregate, {0, 1, 2, 3, 4});
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
    _C4_CALL_SPLIT_LIST_TESTS_FOR_STORAGE(tyname, ty, raw_paged_rt, _C4_RAW_PAGED_RT)

CALL_FOR_CONTAINEE_ARCHETYPES(_c4adaptor)

C4_END_NAMESPACE(c4)
