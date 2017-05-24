#include "c4/list.hpp"

#include "c4/test.hpp"
#include "c4/archetypes.hpp"

C4_BEGIN_NAMESPACE(c4)

template< class List >
void list_test0_empty_ctor()
{
    List li;
    EXPECT_EQ(li.size(), 0);
    EXPECT_EQ(li.capacity(), 0);
    EXPECT_EQ(li.begin(), li.end());
    EXPECT_EQ(std::distance(li.begin(), li.end()), 0);
}

#define _C4_TEST_LIST_BASIC_TESTS(listtestname, listtype)       \
TEST(listtestname, empty_ctor)                                  \
{                                                               \
    list_test0_empty_ctor< listtype >();                        \
}                                                               \
TEST(listtestname, empty_ctor_const)                            \
{                                                               \
    list_test0_empty_ctor< const listtype >();                  \
}

#define _C4_LIST_TESTS(list_type_name, list_type, containee_type_name, containee_type, sztype_name, sztype) \
_C4_TEST_LIST_BASIC_TESTS                                               \
(                                                                       \
    list_type_name##_##containee_type_name##_##sztype_name,             \
    list_type< containee_type C4_COMMA sztype >                         \
)

#define _c4_call_lists_with_size_type(tyname, ty, size_type_name, size_type) \
_C4_LIST_TESTS(flat_list     , flat_list     , tyname, ty, size_type_name, size_type) \
_C4_LIST_TESTS(split_list    , split_list    , tyname, ty, size_type_name, size_type) \
_C4_LIST_TESTS(split_fwd_list, split_fwd_list, tyname, ty, size_type_name, size_type) \
_C4_LIST_TESTS(flat_fwd_list , split_fwd_list, tyname, ty, size_type_name, size_type)

#define _c4_adaptor(tyname, ty) _c4_call_lists_with_size_type(tyname, ty, uint64_t, uint64_t)
CALL_FOR_CONTAINEE_ARCHETYPES(_c4_adaptor)
#undef _c4_adaptor

#define _c4_adaptor(tyname, ty) _c4_call_lists_with_size_type(tyname, ty, int64_t, int64_t)
CALL_FOR_CONTAINEE_ARCHETYPES(_c4_adaptor)
#undef _c4_adaptor

#define _c4_adaptor(tyname, ty) _c4_call_lists_with_size_type(tyname, ty, uint32_t, uint32_t)
CALL_FOR_CONTAINEE_ARCHETYPES(_c4_adaptor)
#undef _c4_adaptor

#define _c4_adaptor(tyname, ty) _c4_call_lists_with_size_type(tyname, ty, int32_t, int32_t)
CALL_FOR_CONTAINEE_ARCHETYPES(_c4_adaptor)
#undef _c4_adaptor

#define _c4_adaptor(tyname, ty) _c4_call_lists_with_size_type(tyname, ty, uint16_t, uint16_t)
CALL_FOR_CONTAINEE_ARCHETYPES(_c4_adaptor)
#undef _c4_adaptor

#define _c4_adaptor(tyname, ty) _c4_call_lists_with_size_type(tyname, ty, int16_t, int16_t)
CALL_FOR_CONTAINEE_ARCHETYPES(_c4_adaptor)
#undef _c4_adaptor

#define _c4_adaptor(tyname, ty) _c4_call_lists_with_size_type(tyname, ty, uint8_t, uint8_t)
CALL_FOR_CONTAINEE_ARCHETYPES(_c4_adaptor)
#undef _c4_adaptor

#define _c4_adaptor(tyname, ty) _c4_call_lists_with_size_type(tyname, ty, int8_t, int8_t)
CALL_FOR_CONTAINEE_ARCHETYPES(_c4_adaptor)
#undef _c4_adaptor

C4_END_NAMESPACE(c4)
