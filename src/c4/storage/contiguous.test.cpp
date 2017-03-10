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


template <class T> using fixed_size8 = container_test< fixed_size< T, 8 >, T >;
TYPED_TEST_CASE_P(fixed_size8);

#define C4_DEFINE_CONTAINER_TEST(test_name, ctor_args, ...)             \
    TYPED_TEST_P(fixed_size8, test_name)                                \
{                                                                       \
    auto c = this->construct_container ctor_args;                       \
    using cont = typename TestFixture::container_type;                  \
    test_##test_name< cont, TypeParam >(c, ## __VA_ARGS__);             \
}


//-----------------------------------------------------------------------------
template <class Cont, class T>
void test_ctor_aggregate(Cont const&c, std::initializer_list<T> il)
{
    Cont fs(aggregate, il);
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
    span< T > ss = fs;
    EXPECT_EQ(ss.begin(), fs.begin());
    EXPECT_EQ(ss.end(),   fs.end());
    EXPECT_EQ(ss.data(),  fs.data());
    EXPECT_EQ(ss.size(),  fs.size());
}
C4_DEFINE_CONTAINER_TEST(span_conversion, ());

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

REGISTER_TYPED_TEST_CASE_P(fixed_size8, ctor_aggregate, subspan, span_conversion);

INSTANTIATE_TYPED_TEST_CASE_P(containers, fixed_size8, c4::archetypes::containees);

C4_END_NAMESPACE(stg)
C4_END_NAMESPACE(c4)
