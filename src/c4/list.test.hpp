#ifndef _C4_LIST_TEST_HPP
#define _C4_LIST_TEST_HPP

#include "c4/list.hpp"

#include "c4/test.hpp"
#include "c4/archetypes.hpp"

C4_BEGIN_NAMESPACE(c4)

#define _C4_DEFINE_LIST_TEST_TYPES(List) \
    using T = typename List::value_type;\
    using I = typename List::size_type;\
    using CT = Counting< T >;\
    using proto = c4::archetypes::archetype_proto< T >;\
    using C ## List = typename List::container_type< CT >;\
    using iltype = std::initializer_list< T >;\
    using ciltype = std::initializer_list< CT >;

template< class List >
void list_test0_ctor_empty()
{
    _C4_DEFINE_LIST_TEST_TYPES(List);

    auto cch = CT::check_ctors_dtors(0, 0);
    {
        CList li;
        EXPECT_TRUE(li.empty());
        EXPECT_EQ(li.size(), 0);
        EXPECT_EQ(li.capacity(), 0);
        EXPECT_EQ(li.begin(), li.end());
        EXPECT_EQ(std::distance(li.begin(), li.end()), 0);
    }
}

template< class List >
void list_test0_ctor_with_capacity()
{
    _C4_DEFINE_LIST_TEST_TYPES(List);

    auto cch = CT::check_ctors_dtors(0, 0);
    {
        CList li(c4::with_capacity, 5);
        EXPECT_TRUE(li.empty());
        EXPECT_EQ(li.size(), 0);
        EXPECT_GE(li.capacity(), (typename List::size_type)5);
        EXPECT_EQ(li.begin(), li.end());
        EXPECT_EQ(std::distance(li.begin(), li.end()), 0);
    }
}

template< class List >
void list_test0_ctor_with_initlist()
{
    _C4_DEFINE_LIST_TEST_TYPES(List);

    ciltype il = proto::cil();
    {
        auto cpch = CT::check_copy_ctors(il.size());
        auto dtch = CT::check_dtors(il.size());
        {
            CList li(c4::aggregate, il);
            EXPECT_FALSE(li.empty());
            EXPECT_EQ(li.size(), il.size());
            EXPECT_GE(li.capacity(), (typename List::size_type)il.size());
            EXPECT_NE(li.begin(), li.end());
            EXPECT_EQ(std::distance(li.begin(), li.end()), il.size());

            int pos = 0;
            for(auto const& v : li)
            {
                auto const& ref = proto::get(pos++);
                EXPECT_EQ(v, ref);
            }
        }
    }
}

template< class List >
void list_test0_push_back_copy()
{
    _C4_DEFINE_LIST_TEST_TYPES(List);

    ciltype il = proto::cil();
    {
        auto cpch = CT::check_copy_ctors(il.size());
        auto dtch = CT::check_dtors(il.size());
        {
            CList li;
            for(auto const& elm : il)
            {
                li.push_back(elm);
            }

            EXPECT_FALSE(li.empty());
            EXPECT_EQ(li.size(), il.size());
            EXPECT_GE(li.capacity(), (typename List::size_type)il.size());
            EXPECT_NE(li.begin(), li.end());
            EXPECT_EQ(std::distance(li.begin(), li.end()), il.size());

            int pos = 0;
            for(auto const& v : li)
            {
                auto const& ref = proto::get(pos++);
                EXPECT_EQ(v, ref);
            }
        }
    }
}

template< class List >
void list_test0_push_back_move()
{
    _C4_DEFINE_LIST_TEST_TYPES(List);

    ciltype il = proto::cil();
    {
        auto cpch = CT::check_move_ctors(il.size());
        auto dtch = CT::check_dtors(2 * il.size()); // 1 movedfrom + 1 movedto
        {
            CList li;
            for(auto const& elm : il)
            {
                auto tmp = elm;
                li.push_back(std::move(tmp));
            }

            EXPECT_FALSE(li.empty());
            EXPECT_EQ(li.size(), il.size());
            EXPECT_GE(li.capacity(), (typename List::size_type)il.size());
            EXPECT_NE(li.begin(), li.end());
            EXPECT_EQ(std::distance(li.begin(), li.end()), il.size());

            int pos = 0;
            for(auto const& v : li)
            {
                auto const& ref = proto::get(pos++);
                EXPECT_EQ(v, ref);
            }
        }
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
}                                                               \
TEST(listtestname, push_back_copy)                              \
{                                                               \
    list_test0_push_back_copy< listtype >();                    \
}                                                               \
TEST(listtestname, push_back_move)                              \
{                                                               \
    list_test0_push_back_move< listtype >();                    \
}                                                               \


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
