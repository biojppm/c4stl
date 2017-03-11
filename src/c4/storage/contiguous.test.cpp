#include "c4/storage/contiguous.hpp"
#include "c4/test.hpp"
#include "c4/archetypes.hpp"

C4_BEGIN_NAMESPACE(c4)
C4_BEGIN_NAMESPACE(stg)

template <class Container, class T>
struct container_test : public ::testing::Test
{
    C4_ASSERT_SAME_TYPE(T, typename Container::value_type);

    using container_type = Container;
    using atype = T;

    static std::initializer_list<atype> getil()
    {
        auto fn = &archetypes::archetype_proto<T>::get;
        static const std::initializer_list< atype > il{
            fn(0), fn(1), fn(2), fn(3),
            fn(4), fn(5), fn(6), fn(7)
        };
        return il;
    }

    template< class... CtorArgs >
    Container construct_container(CtorArgs&& ...args)
    {
        Container c(std::forward<CtorArgs>(args)...);
        return c;
    }

};

//-----------------------------------------------------------------------------
template <class T> using test_fixed_size = container_test< fixed_size< T, 8 >, T >;
TYPED_TEST_CASE_P(test_fixed_size);

#define C4_DEFINE_CONTAINER_TEST(test_name, ctor_args, ...) \
TYPED_TEST_P(test_fixed_size, test_name)                    \
{                                                           \
    using cont = typename TestFixture::container_type;      \
    using T = typename cont::value_type;                    \
    auto c = cont ctor_args;                                \
    test_##test_name< cont, TypeParam >(c, ## __VA_ARGS__); \
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template <class Cont, class T>
void test_ctor_empty(Cont const& fs)
{
}
C4_DEFINE_CONTAINER_TEST(ctor_empty, ());

//-----------------------------------------------------------------------------
template <class Cont, class T>
void test_ctor_copy(Cont const& fs)
{
    Cont cp(fs);
}
C4_DEFINE_CONTAINER_TEST(ctor_copy, ());

//-----------------------------------------------------------------------------
template <class Cont, class T>
void test_ctor_move(Cont const& fs)
{
    Cont cp(fs);
    Cont mv(std::move(cp));
}
C4_DEFINE_CONTAINER_TEST(ctor_move, ());

//-----------------------------------------------------------------------------
template <class Cont, class T>
void test_ctor_copy_span(Cont const& fs)
{
    cspan< T, typename Cont::size_type > s = fs;
    Cont c(s);
}
C4_DEFINE_CONTAINER_TEST(ctor_copy_span, ());

//-----------------------------------------------------------------------------
template <class Cont, class T>
void test_ctor_aggregate(Cont const& fs, std::initializer_list<T> il)
{
    EXPECT_EQ(fs.size(), il.size());
    for(typename Cont::size_type i = 0, e = fs.size(); i < e; ++i)
    {
        EXPECT_EQ(fs[i], *(il.begin() + i));
    }
}
C4_DEFINE_CONTAINER_TEST(ctor_aggregate, (aggregate, this->getil()), this->getil());

//-----------------------------------------------------------------------------
template <class Cont, class T>
void test_subspan(Cont &fs)
{
    auto s = fs.subspan();
    EXPECT_EQ(s.begin(), fs.begin());
    EXPECT_EQ(s.end(),   fs.end());
    EXPECT_EQ(s.data(),  fs.data());
    EXPECT_EQ(s.size(),  fs.size());
}
C4_DEFINE_CONTAINER_TEST(subspan, ());


//-----------------------------------------------------------------------------
template <class Cont, class T>
void test_span_conversion(Cont &fs)
{
    cspan< T, typename Cont::size_type > ss = fs;
    EXPECT_EQ(ss.begin(), fs.begin());
    EXPECT_EQ(ss.end(),   fs.end());
    EXPECT_EQ(ss.data(),  fs.data());
    EXPECT_EQ(ss.size(),  fs.size());
}
C4_DEFINE_CONTAINER_TEST(span_conversion, ());


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

REGISTER_TYPED_TEST_CASE_P(test_fixed_size,
                           ctor_empty, ctor_copy, ctor_move, ctor_copy_span, ctor_aggregate,
                           subspan, span_conversion);

INSTANTIATE_TYPED_TEST_CASE_P(containers, test_fixed_size, c4::archetypes::containees);

C4_END_NAMESPACE(stg)
C4_END_NAMESPACE(c4)
