#ifndef _C4_TYPES_HPP_
#define _C4_TYPES_HPP_

#include <stdint.h>
#include <stddef.h>
#include <type_traits>

/** @file types.hpp basic types, and utility macros and traits for types.
 * @ingroup basic_headers */

C4_BEGIN_NAMESPACE(c4)

using i8 = int8_t;
using u8 = uint8_t;
using i16 = int16_t;
using u16 = uint16_t;
using i32 = int32_t;
using u32 = uint32_t;
using i64 = int64_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

using ssize_t = std::make_signed< size_t >::type;

//--------------------------------------------------

// some tag types

/** a tag type for initializing the containers with variadic arguments a la
 * initializer_list, minus the initializer_list overload problems.
 * @see */
struct aggregate_t {};
/** @see aggregate_t */
constexpr const aggregate_t aggregate{};

/** a tag type for specifying the initial capacity of allocatable contiguous storage */
struct with_capacity_t {};
/** @see with_capacity_t */
constexpr const with_capacity_t with_capacity{};

/** a tag type which can be used to disambiguate in variadic template overloads */
struct varargs_t {};
/** a tag variable which can be used to disambiguate in variadic template overloads */
constexpr const varargs_t varargs{};

//--------------------------------------------------

/** whether a value should be used in place of a const-reference in argument passing. */
template< class T >
struct cref_uses_val
{
    enum { value = (
    std::is_scalar< T >::value
    ||
    (std::is_pod< T >::value && sizeof(T) <= sizeof(size_t))) };
};
/** utility macro to override the default behaviour for c4::fastcref<T>
 @see fastcref */
#define C4_CREF_USES_VAL(T) \
template<>                  \
struct cref_uses_val< T >   \
{                           \
    enum { value = true };  \
};

/** Whether to use pass-by-value or pass-by-const-reference in a function argument
 * or return type. */
template< class T >
using fastcref = typename std::conditional< c4::cref_uses_val< T >::value, T, T const& >::type;

//--------------------------------------------------

/** Just what its name says. Useful sometimes as a default empty policy class. */
struct EmptyStruct
{
    template< class... T > EmptyStruct(T && ...){}
};

/** Just what its name says. Useful sometimes as a default policy class to
 * be inherited from. */
struct EmptyStructVirtual
{
    virtual ~EmptyStructVirtual() = default;
    template< class... T > EmptyStructVirtual(T && ...){}
};


/** */
template< class T >
struct inheritfrom : public T {};

//--------------------------------------------------
// Utilities to make a class obey size restrictions (eg, min size or size multiple of).
// DirectX usually makes this restriction with uniform buffers.
// This is also useful for padding to prevent false-sharing.

/** how many bytes must be added to size such that the result is at least minsize? */
C4_ALWAYS_INLINE constexpr size_t min_remainder(size_t size, size_t minsize)
{
    return size < minsize ? minsize-size : 0;
}

/** how many bytes must be added to size such that the result is a multiple of multipleof?  */
C4_ALWAYS_INLINE constexpr size_t mult_remainder(size_t size, size_t multipleof)
{
    return (((size % multipleof) != 0) ? (multipleof-(size % multipleof)) : 0);
}

/** force the following class to be tightly packed.
 * @see http://stackoverflow.com/questions/21092415/force-c-structure-to-pack-tightly */
#pragma pack(push, 1)
/** pad a class with more bytes at the end. */
template< class T, size_t BytesToPadAtEnd >
struct Padded : public T
{
    using T::T;
public:
    char ___c4padspace___[BytesToPadAtEnd];
};
#pragma pack(pop)
/** When the padding argument is 0, we cannot declare the char[] array. */
template< class T >
struct Padded< T, 0 > : public T
{
    using T::T;
};

/** make T have a size which is at least Min bytes */
template< class T, size_t Min >
using MinSized = Padded< T, min_remainder(sizeof(T), Min) >;

/** make T have a size which is a multiple of Mult bytes */
template< class T, size_t Mult >
using MultSized = Padded< T, mult_remainder(sizeof(T), Mult) >;

/** make T have a size which is simultaneously:
 *  -bigger or equal than Min
 *  -a multiple of Mult */
template< class T, size_t Min, size_t Mult >
using MinMultSized = MultSized< MinSized< T, Min >, Mult >;

/** make T be suitable for use as a uniform buffer. (at least with DirectX). */
template< class T >
using UbufSized = MinMultSized< T, 64, 16 >;

//-----------------------------------------------------------------------------

/** SFINAE. use this macro to enable a template function overload
based on a compile-time condition.
@code
// define an overload for a non-pod type
template< class T, C4_REQUIRE_T(std::is_pod< T >::value) >
void foo() { std::cout << "pod type\n"; }

// define an overload for a non-pod type
template< class T, C4_REQUIRE_T(!std::is_pod< T >::value) >
void foo() { std::cout << "nonpod type\n"; }

struct non_pod
{
    non_pod() : name("asdfkjhasdkjh") {}
    const char *name;
};

int main()
{
    foo< float >(); // prints "pod type"
    foo< non_pod >(); // prints "nonpod type"
}
@endcode */
#define C4_REQUIRE_T(cond) typename std::enable_if< cond, bool >::type* = nullptr

/** enable_if for a return type
 * @see C4_REQUIRE_T */
#define C4_REQUIRE_R(cond, type_) typename std::enable_if< cond, type_ >::type

//-----------------------------------------------------------------------------
/** declare a traits class telling whether a type provides a member typedef */
#define C4_DEFINE_HAS_TYPEDEF(member_typedef)                   \
template< typename T >                                          \
struct has_##stype                                              \
{                                                               \
private:                                                        \
                                                                \
    typedef char                      yes;                      \
    typedef struct { char array[2]; } no;                       \
                                                                \
    template< typename C >                                      \
    static yes _test(typename C::member_typedef*);              \
                                                                \
    template< typename C >                                      \
    static no  _test(...);                                      \
                                                                \
public:                                                         \
                                                                \
    enum { value = (sizeof(_test< T >(0)) == sizeof(yes)) };    \
                                                                \
}

//-----------------------------------------------------------------------------
/** declare a traits class telling whether a type provides a method */
#define C4_DEFINE_HAS_METHOD(ret_type, method_name, const_qualifier, ...) \
template< typename T >                                                  \
struct has_##method_name##_method                                       \
{                                                                       \
private:                                                                \
                                                                        \
    typedef char                      &yes;                             \
    typedef struct { char array[2]; } &no;                              \
                                                                        \
    template< typename C >                                              \
    static yes _test                                                    \
    (                                                                   \
        C const_qualifier* v,                                           \
        typename std::enable_if                                         \
        <                                                               \
            std::is_same< decltype(v->method_name(__VA_ARGS__)), ret_type >::value \
            ,                                                           \
            void /* this is defined only if the bool above is true. */  \
                 /* so when it fails, SFINAE is triggered */            \
        >                                                               \
        ::type*                                                         \
    );                                                                  \
                                                                        \
    template< typename C >                                              \
    static no _test(...);                                               \
                                                                        \
public:                                                                 \
                                                                        \
    enum { value = (sizeof(_test< T >((typename std::remove_reference< T >::type*)0, 0)) == sizeof(yes)) }; \
                                                                        \
};

//-----------------------------------------------------------------------------
#define _c4_DEFINE_ARRAY_TYPES_WITHOUT_ITERATOR(T, I)       \
                                                            \
    using value_type = T;                                   \
    using size_type = I;                                    \
    using ssize_type = typename std::make_signed<I>::type;  \
                                                            \
    using pointer = T*;                                     \
    using const_pointer = T const*;                         \
                                                            \
    using reference = T&;                                   \
    using const_reference = T const&;                       \
                                                            \
    using difference_type = ptrdiff_t;


#define _c4_DEFINE_ARRAY_TYPES(T, I)                                    \
                                                                        \
    _c4_DEFINE_ARRAY_TYPES_WITHOUT_ITERATOR(T, I);                      \
                                                                        \
    using iterator = T*;                                                \
    using const_iterator = T const*;                                    \
                                                                        \
    using reverse_iterator = std::reverse_iterator< T* >;               \
    using const_reverse_iterator = std::reverse_iterator< T const* >;


//-----------------------------------------------------------------------------
// http://stackoverflow.com/questions/10821380/is-t-an-instance-of-a-template-in-c
template< template < typename... > class X, typename    T > struct is_instance_of_tpl             : std::false_type {};
template< template < typename... > class X, typename... Y > struct is_instance_of_tpl<X, X<Y...>> : std::true_type {};

//-----------------------------------------------------------------------------
// A template parameter pack is mass-forwardable if
// all of its types are mass-forwardable...
template< class T, class ...Args >
struct is_mass_forwardable : public std::conditional<
    is_mass_forwardable< T >::value && is_mass_forwardable< Args... >::value,
    std::true_type, std::false_type
    >::type
{};
// ... and a type is mass-forwardable if:
template< class T >
struct is_mass_forwardable<T> : public std::conditional<
    (
        !std::is_rvalue_reference<T>::value ||
        (
            std::is_trivially_move_constructible<typename std::remove_reference<T>::type>::value &&
            std::is_trivially_move_assignable<typename std::remove_reference<T>::type>::value
        )
    ),
    std::true_type,
    std::false_type
    >::type
{};

C4_END_NAMESPACE(c4)

#endif /* _C4_TYPES_HPP_ */
