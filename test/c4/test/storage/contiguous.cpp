#include "c4/storage/contiguous.hpp"
#include "c4/test.hpp"
#include "c4/libtest/archetypes.hpp"

#ifdef __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif

C4_BEGIN_NAMESPACE(c4)
C4_BEGIN_NAMESPACE(stg)

template <class Container>
struct container_test : public ::testing::Test
{
    using T = typename Container::value_type;
    using container_type = Container;

    static std::initializer_list<T> getil()
    {
        auto fn = &archetypes::archetype_proto<T>::get;
        static const std::initializer_list<T> il{
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
template <class T> using test_fixed_size = container_test<fixed_size<T, 8>>;
TYPED_TEST_CASE_P(test_fixed_size);

template <class T> using test_fixed_capacity = container_test<fixed_capacity<T, 8>>;
TYPED_TEST_CASE_P(test_fixed_capacity);


//-----------------------------------------------------------------------------
#define C4_DEFINE_FIXED_CONTAINER_TEST(test_name, ctor_args, ...)   \
TYPED_TEST_P(test_fixed_size, test_name)                    \
{                                                           \
    using cont = typename TestFixture::container_type;      \
    using T = typename cont::value_type;                    \
    auto c = cont ctor_args;                                \
    test_##test_name<cont, TypeParam>(c, ## __VA_ARGS__);   \
}

#define C4_DEFINE_DYNAMIC_CONTAINER_TEST(test_name, ctor_args, ...) \
TYPED_TEST_P(test_fixed_capacity, test_name)                    \
{                                                           \
    using cont = typename TestFixture::container_type;      \
    using T = typename cont::value_type;                    \
    auto c = cont ctor_args;                                \
    test_##test_name<cont, TypeParam>(c, ## __VA_ARGS__);   \
}

#define C4_DEFINE_CONTAINER_TEST(test_name, ctor_args, ...) \
    C4_DEFINE_FIXED_CONTAINER_TEST(test_name, ctor_args, ## __VA_ARGS__);  \
    C4_DEFINE_DYNAMIC_CONTAINER_TEST(test_name, ctor_args, ## __VA_ARGS__);

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template <class Cont, class T>
void test_contiguous_class_layout(Cont const& fs)
{
    using raw_storage_type = typename Cont::raw_storage_type;
    using base_crtp_type = typename Cont::base_crtp;

    {
        contiguous< T, size_t, raw_fixed< T, 8 > > fn;
        contiguous< T, size_t, raw<T> > dn;
        contiguous< T, size_t, raw_paged<T> > pn;
    }
}
C4_DEFINE_DYNAMIC_CONTAINER_TEST(contiguous_class_layout, ());

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
template <template<class, class> class Span, class Cont, class T>
void test_span_type_conversion(Cont &fs)
{
    Span< T, typename Cont::size_type > ss = fs;
    EXPECT_EQ(ss.begin(), fs.begin());
    EXPECT_EQ(ss.end(),   fs.end());
    EXPECT_EQ(ss.data(),  fs.data());
    EXPECT_EQ(ss.size(),  fs.size());

    Span< T, typename Cont::size_type > css = fs;
    EXPECT_EQ(css.begin(), fs.begin());
    EXPECT_EQ(css.end(),   fs.end());
    EXPECT_EQ(css.data(),  fs.data());
    EXPECT_EQ(css.size(),  fs.size());

    Cont e;
    if(e.empty())
    {
        span< T, typename Cont::size_type > es = e;
        EXPECT_TRUE(es.empty());
        EXPECT_EQ(es.begin(), e.begin());
        EXPECT_EQ(es.end(),   e.end());
        EXPECT_EQ(es.data(),  e.data());
        EXPECT_EQ(es.size(),  e.size());
    }
}
template <class Cont, class T>
void test_span_conversion(Cont &fs)
{
    test_span_type_conversion< span, Cont, T >(fs);
}
C4_DEFINE_CONTAINER_TEST(span_conversion, ());

//-----------------------------------------------------------------------------
template <class Cont, class T>
void test_spanrs_conversion(Cont &fs)
{
    test_span_type_conversion< spanrs, Cont, T >(fs);

    spanrs< T, typename Cont::size_type > ss = fs;
    EXPECT_EQ(ss.capacity(), fs.capacity());
}
C4_DEFINE_CONTAINER_TEST(spanrs_conversion, ());

//-----------------------------------------------------------------------------
template <class Cont, class T>
void test_etched_span_conversion(Cont &fs)
{
    test_span_type_conversion< etched_span, Cont, T >(fs);

    etched_span< T, typename Cont::size_type > ss = fs;
    EXPECT_EQ(ss.capacity(), fs.capacity());
    EXPECT_EQ(ss.offset(), 0);
}
C4_DEFINE_CONTAINER_TEST(etched_span_conversion, ());

//-----------------------------------------------------------------------------
template <class Cont, class T>
void test_get_span(Cont &fs)
{
    auto s = fs.get_span();
    EXPECT_EQ(s.begin(), fs.begin());
    EXPECT_EQ(s.end(),   fs.end());
    EXPECT_EQ(s.data(),  fs.data());
    EXPECT_EQ(s.size(),  fs.size());

    C4_ASSERT(fs.size() > 4);

    s = fs.get_span(2);
    EXPECT_EQ(s.begin(), fs.begin() + 2);
    EXPECT_EQ(fs.end() - s.end(), 0);
    EXPECT_EQ(s.data(),  fs.data() + 2);
    EXPECT_EQ(s.size(),  fs.size() - 2);
    EXPECT_EQ(s.capacity(), fs.capacity() - 2);

    s = fs.get_span(2, fs.size() - 4);
    EXPECT_EQ(s.begin(), fs.begin() + 2);
    EXPECT_EQ(fs.end() - s.end(), 2);
    EXPECT_EQ(s.data(),  fs.data() + 2);
    EXPECT_EQ(s.size(),  fs.size() - 4);
    EXPECT_EQ(s.capacity(), fs.capacity() - 4);

    s = fs.get_span(0, fs.size() - 2);
    EXPECT_EQ(s.begin(), fs.begin());
    EXPECT_EQ(fs.end() - s.end(), 2);
    EXPECT_EQ(s.data(),  fs.data());
    EXPECT_EQ(s.size(),  fs.size() - 2);
    EXPECT_EQ(s.capacity(), fs.capacity() - 2);
}
C4_DEFINE_CONTAINER_TEST(get_span, ());


//-----------------------------------------------------------------------------
template <class Cont, class T>
void test_get_spanrs(Cont &fs)
{
    auto s = fs.get_spanrs();
    EXPECT_EQ(s.begin(), fs.begin());
    EXPECT_EQ(s.end(),   fs.end());
    EXPECT_EQ(s.data(),  fs.data());
    EXPECT_EQ(s.size(),  fs.size());

    C4_ASSERT(fs.size() > 4);

    s = fs.get_spanrs(2);
    EXPECT_EQ(s.begin(), fs.begin() + 2);
    EXPECT_EQ(fs.end() - s.end(), 0);
    EXPECT_EQ(s.end(),   fs.end());
    EXPECT_EQ(s.data(),  fs.data() + 2);
    EXPECT_EQ(s.size(),  fs.size() - 2);
    EXPECT_EQ(s.capacity(), fs.capacity() - 2);

    s = fs.get_spanrs(2, fs.size() - 4);
    EXPECT_EQ(s.begin(), fs.begin() + 2);
    EXPECT_EQ(fs.end() - s.end(), 2);
    EXPECT_EQ(s.end(),   fs.end() - 2);
    EXPECT_EQ(s.data(),  fs.data() + 2);
    EXPECT_EQ(s.size(),  fs.size() - 4);
    EXPECT_EQ(s.capacity(), fs.capacity() - 2);

    s = fs.get_spanrs(0, fs.size() - 2);
    EXPECT_EQ(s.begin(), fs.begin());
    EXPECT_EQ(fs.end() - s.end(), 2);
    EXPECT_EQ(s.data(),  fs.data());
    EXPECT_EQ(s.size(),  fs.size() - 2);
    EXPECT_EQ(s.capacity(), fs.capacity());
}
C4_DEFINE_CONTAINER_TEST(get_spanrs, ());


//-----------------------------------------------------------------------------
template <class Cont, class T>
void test_get_etched_span(Cont &fs)
{
    auto s = fs.get_etched_span();
    EXPECT_EQ(s.begin(), fs.begin());
    EXPECT_EQ(s.end(),   fs.end());
    EXPECT_EQ(s.data(),  fs.data());
    EXPECT_EQ(s.size(),  fs.size());
    EXPECT_EQ(s.offset(), 0);

    C4_ASSERT(fs.size() > 4);

    s = fs.get_etched_span(2);
    EXPECT_EQ(s.begin(), fs.begin() + 2);
    EXPECT_EQ(fs.end() - s.end(), 0);
    EXPECT_EQ(s.data(),  fs.data() + 2);
    EXPECT_EQ(s.size(),  fs.size() - 2);
    EXPECT_EQ(s.capacity(), fs.capacity() - 2);
    EXPECT_EQ(s.offset(), 2);

    s = fs.get_etched_span(2, fs.size() - 4);
    EXPECT_EQ(s.begin(), fs.begin() + 2);
    EXPECT_EQ(fs.end() - s.end(), 2);
    EXPECT_EQ(s.data(),  fs.data() + 2);
    EXPECT_EQ(s.size(),  fs.size() - 4);
    EXPECT_EQ(s.capacity(), fs.capacity() - 2);
    EXPECT_EQ(s.offset(), 2);

    s = fs.get_etched_span(0, fs.size() - 2);
    EXPECT_EQ(s.begin(), fs.begin());
    EXPECT_EQ(fs.end() - s.end(), 2);
    EXPECT_EQ(s.data(),  fs.data());
    EXPECT_EQ(s.size(),  fs.size() - 2);
    EXPECT_EQ(s.capacity(), fs.capacity());
    EXPECT_EQ(s.offset(), 0);
}
C4_DEFINE_CONTAINER_TEST(get_etched_span, ());



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

REGISTER_TYPED_TEST_CASE_P(test_fixed_size,
                           ctor_empty, ctor_copy, ctor_move, ctor_copy_span, ctor_aggregate,
                           get_span, span_conversion,
                           get_spanrs, spanrs_conversion,
                           get_etched_span, etched_span_conversion);

REGISTER_TYPED_TEST_CASE_P(test_fixed_capacity,
                           contiguous_class_layout,
                           ctor_empty, ctor_copy, ctor_move, ctor_copy_span, ctor_aggregate,
                           get_span, span_conversion,
                           get_spanrs, spanrs_conversion,
                           get_etched_span, etched_span_conversion);

INSTANTIATE_TYPED_TEST_CASE_P(containers, test_fixed_size, c4::archetypes::containees);
//INSTANTIATE_TYPED_TEST_CASE_P(containers, test_fixed_capacity, c4::archetypes::containees);

C4_END_NAMESPACE(stg)
C4_END_NAMESPACE(c4)
