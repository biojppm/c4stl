#ifndef _C4_LIST_TEST_HPP
#define _C4_LIST_TEST_HPP

#include "c4/list.hpp"

#include "c4/test.hpp"
#include "c4/archetypes.hpp"

C4_BEGIN_NAMESPACE(c4)

template< class List >
void list_test0_ctor_empty()
{
    List li;
    EXPECT_TRUE(li.empty());
    EXPECT_EQ(li.size(), 0);
    EXPECT_EQ(li.capacity(), 0);
    EXPECT_EQ(li.begin(), li.end());
    EXPECT_EQ(std::distance(li.begin(), li.end()), 0);
}

template< class List >
void list_test0_ctor_with_capacity()
{
    List li(c4::with_capacity, 5);
    EXPECT_TRUE(li.empty());
    EXPECT_EQ(li.size(), 0);
    EXPECT_GE(li.capacity(), (typename List::size_type)5);
    EXPECT_EQ(li.begin(), li.end());
    EXPECT_EQ(std::distance(li.begin(), li.end()), 0);
}

template< class List >
void list_test0_ctor_with_initlist()
{
    using T = typename List::value_type;
    using I = typename List::size_type;

    std::initializer_list< T > il = c4::archetypes::archetype_proto< T >::il();
    List li(c4::aggregate, il);
    EXPECT_FALSE(li.empty());
    EXPECT_EQ(li.size(), il.size());
    EXPECT_GE(li.capacity(), (typename List::size_type)il.size());
    EXPECT_NE(li.begin(), li.end());
    EXPECT_EQ(std::distance(li.begin(), li.end()), il.size());

    int pos = 0;
    for(auto const& v : li)
    {
        auto const& ref = c4::archetypes::archetype_proto< T >::get(pos++);
        EXPECT_EQ(v, ref);
    }
}

#define _C4_TEST_LIST_BASIC_TESTS(listtestname, listtype)       \
TEST(listtestname, ctor_empty)                                  \
{                                                               \
    list_test0_ctor_empty< listtype >();                        \
}                                                               \
TEST(listtestname, ctor_empty_const)                            \
{                                                               \
    list_test0_ctor_empty< const listtype >();                  \
}                                                               \
TEST(listtestname, ctor_with_capacity)                          \
{                                                               \
    list_test0_ctor_with_capacity< listtype >();                \
}                                                               \
TEST(listtestname, ctor_with_initlist)                          \
{                                                               \
    list_test0_ctor_with_initlist< listtype >();                \
}

#define _C4_CALL_LIST_TESTS(list_type_name, list_type, containee_type_name, containee_type, sztype_name, sztype) \
_C4_TEST_LIST_BASIC_TESTS                                               \
(                                                                       \
    list_type_name##__##containee_type_name##__##sztype_name,           \
    list_type< containee_type C4_COMMA sztype >                         \
)

#define _C4_CALL_LIST_TESTS_FOR_ALL_SIZE_TYPES(list_tyname, list_ty, containee_type_name, containee_type) \
_C4_CALL_LIST_TESTS(list_tyname, list_ty, containee_type_name, containee_type, uint64_t, uint64_t) \
_C4_CALL_LIST_TESTS(list_tyname, list_ty, containee_type_name, containee_type,  int64_t,  int64_t) \
_C4_CALL_LIST_TESTS(list_tyname, list_ty, containee_type_name, containee_type, uint32_t, uint32_t) \
_C4_CALL_LIST_TESTS(list_tyname, list_ty, containee_type_name, containee_type,  int32_t,  int32_t) \
_C4_CALL_LIST_TESTS(list_tyname, list_ty, containee_type_name, containee_type, uint16_t, uint16_t) \
_C4_CALL_LIST_TESTS(list_tyname, list_ty, containee_type_name, containee_type,  int16_t,  int16_t) \
_C4_CALL_LIST_TESTS(list_tyname, list_ty, containee_type_name, containee_type,  uint8_t,  uint8_t) \
_C4_CALL_LIST_TESTS(list_tyname, list_ty, containee_type_name, containee_type,   int8_t,   int8_t)

#define _C4_CALL_FLAT_LIST_TESTS_ADAPTOR(tyname, ty) \
_C4_CALL_LIST_TESTS_FOR_ALL_SIZE_TYPES(flat_list, flat_list, tyname, ty)

#define _C4_CALL_SPLIT_LIST_TESTS_ADAPTOR(tyname, ty) \
_C4_CALL_LIST_TESTS_FOR_ALL_SIZE_TYPES(split_list, split_list, tyname, ty)

#define _C4_CALL_FLAT_FWD_LIST_TESTS_ADAPTOR(tyname, ty) \
_C4_CALL_LIST_TESTS_FOR_ALL_SIZE_TYPES(flat_fwd_list, flat_fwd_list, tyname, ty)

#define _C4_CALL_SPLIT_FWD_LIST_TESTS_ADAPTOR(tyname, ty) \
_C4_CALL_LIST_TESTS_FOR_ALL_SIZE_TYPES(split_fwd_list, split_fwd_list, tyname, ty)

C4_END_NAMESPACE(c4)

#endif // _C4_LIST_TEST_HPP
