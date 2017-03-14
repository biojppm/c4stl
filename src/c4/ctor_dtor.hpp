#ifndef _C4_CTOR_DTOR_H_
#define _C4_CTOR_DTOR_H_

#include "c4/preprocessor.hpp"
#include "c4/language.hpp"
#include "c4/memory_util.hpp"
#include "c4/error.hpp"

#include <type_traits>
#include <utility> // std::forward

/** @file ctor_dtor.hpp object construction and destruction facilities.
 * Some of these are not yet available in C++11. */

C4_BEGIN_NAMESPACE(c4)

#define _C4REQUIRE(cond) \
C4_ALWAYS_INLINE typename std::enable_if< cond, void >::type

/** default-construct an object, trivial version */
template< class U > _C4REQUIRE(std::is_trivially_default_constructible< U >::value)
construct(U* ptr) noexcept
{
    memset(ptr, 0, sizeof(U));
}
/** default-construct an object, non-trivial version */
template< class U > _C4REQUIRE( ! std::is_trivially_default_constructible< U >::value)
construct(U* ptr) noexcept
{
    new (ptr) U();
}

/** default-construct n objects, trivial version */
template< class U, class I > _C4REQUIRE(std::is_trivially_default_constructible< U >::value)
construct_n(U* ptr, I n) noexcept
{
    memset(ptr, 0, n * sizeof(U));
}
/** default-construct n objects, non-trivial version */
template< class U, class I > _C4REQUIRE( ! std::is_trivially_default_constructible< U >::value)
construct_n(U* ptr, I n) noexcept
{
    for(I i = 0; i < n; ++i)
    {
        new (ptr+i) U();
    }
}

template< class U, class ...Args >
inline void construct(U* ptr, Args&&... args)
{
    new ((void*)ptr) U(std::forward< Args >(args)...);
}
template< class U, class I, class ...Args >
inline void construct_n(U* ptr, I n, Args&&... args)
{
    for(I i = 0; i < n; ++i)
    {
        new ((void*)(ptr + i)) U(args...);
    }
}

//-----------------------------------------------------------------------------
// copy-construct

template< class U > _C4REQUIRE(std::is_trivially_copy_constructible< U >::value)
copy_construct(U* dst, U const* src) noexcept
{
    memcpy(dst, src, sizeof(U));
}
template< class U > _C4REQUIRE( ! std::is_trivially_copy_constructible< U >::value)
copy_construct(U* dst, U const* src)
{
    new (dst) U(*src);
}
template< class U, class I > _C4REQUIRE(std::is_trivially_copy_constructible< U >::value)
copy_construct_n(U* dst, U const* src, I n) noexcept
{
    memcpy(dst, src, n * sizeof(U));
}
template< class U, class I > _C4REQUIRE( ! std::is_trivially_copy_constructible< U >::value)
copy_construct_n(U* dst, U const* src, I n)
{
    for(I i = 0; i < n; ++i)
    {
        new (dst + i) U(*(src + i));
    }
}

template< class U > _C4REQUIRE(std::is_scalar< U >::value)
copy_construct(U* dst, U src) noexcept // pass by value for scalar types
{
    *dst = src;
}
template< class U > _C4REQUIRE( ! std::is_scalar< U >::value)
copy_construct(U* dst, U const& src) // pass by reference for non-scalar types
{
    new (dst) U(src);
}
template< class U, class I > _C4REQUIRE(std::is_scalar< U >::value)
copy_construct_n(U* dst, U src, I n) noexcept // pass by value for scalar types
{
    for(I i = 0; i < n; ++i)
    {
        dst[i] = src;
    }
}
template< class U, class I > _C4REQUIRE( ! std::is_scalar< U >::value)
copy_construct_n(U* dst, U const& src, I n) // pass by reference for non-scalar types
{
    for(I i = 0; i < n; ++i)
    {
        new (dst + i) U(src);
    }
}

template< class U, size_t N >
C4_ALWAYS_INLINE void copy_construct(U (&dst)[N], U const (&src)[N]) noexcept
{
    copy_construct_n(dst, src, N);
}

//-----------------------------------------------------------------------------
// copy-assign

template< class U > _C4REQUIRE(std::is_trivially_copy_assignable< U >::value)
copy_assign(U* dst, U const* src) noexcept
{
    memcpy(dst, src, sizeof(U));
}
template< class U > _C4REQUIRE( ! std::is_trivially_copy_assignable< U >::value)
copy_assign(U* dst, U const* src) noexcept
{
    *dst = *src;
}
template< class U, class I > _C4REQUIRE(std::is_trivially_copy_assignable< U >::value)
copy_assign_n(U* dst, U const* src, I n) noexcept
{
    memcpy(dst, src, n * sizeof(U));
}
template< class U, class I > _C4REQUIRE( ! std::is_trivially_copy_assignable< U >::value)
copy_assign_n(U* dst, U const* src, I n) noexcept
{
    for(I i = 0; i < n; ++i)
    {
        dst[i] = src[i];
    }
}

template< class U > _C4REQUIRE(std::is_scalar< U >::value)
copy_assign(U* dst, U src) noexcept // pass by value for scalar types
{
    *dst = src;
}
template< class U > _C4REQUIRE( ! std::is_scalar< U >::value)
copy_assign(U* dst, U const& src) noexcept // pass by reference for non-scalar types
{
    *dst = src;
}
template< class U, class I > _C4REQUIRE(std::is_scalar< U >::value)
copy_assign_n(U* dst, U src, I n) noexcept // pass by value for scalar types
{
    for(I i = 0; i < n; ++i)
    {
        dst[i] = src;
    }
}
template< class U, class I > _C4REQUIRE( ! std::is_scalar< U >::value)
copy_assign_n(U* dst, U const& src, I n) noexcept // pass by reference for non-scalar types
{
    for(I i = 0; i < n; ++i)
    {
        dst[i] = src;
    }
}

template< class U, size_t N >
C4_ALWAYS_INLINE void copy_assign(U (&dst)[N], U const (&src)[N]) noexcept
{
    copy_assign_n(dst, src, N);
}

//-----------------------------------------------------------------------------
// move-construct

template< class U > _C4REQUIRE(std::is_trivially_move_constructible< U >::value)
move_construct(U* dst, U* src) noexcept
{
    memcpy(dst, src, sizeof(U));
}
template< class U > _C4REQUIRE( ! std::is_trivially_move_constructible< U >::value)
move_construct(U* dst, U* src) noexcept
{
    new (dst) U(std::move(*src));
}
template< class U, class I > _C4REQUIRE(std::is_trivially_move_constructible< U >::value)
move_construct_n(U* dst, U* src, I n) noexcept
{
    memcpy(dst, src, n * sizeof(U));
}
template< class U, class I > _C4REQUIRE( ! std::is_trivially_move_constructible< U >::value)
move_construct_n(U* dst, U* src, I n) noexcept
{
    for(I i = 0; i < n; ++i)
    {
        new (dst + i) U(std::move(src[i]));
    }
}

//-----------------------------------------------------------------------------
// move-assign

template< class U > _C4REQUIRE(std::is_trivially_move_assignable< U >::value)
move_assign(U* dst, U* src) noexcept
{
    memcpy(dst, src, sizeof(U));
}
template< class U > _C4REQUIRE( ! std::is_trivially_move_assignable< U >::value)
move_assign(U* dst, U* src) noexcept
{
    *dst = std::move(*src);
}
template< class U, class I > _C4REQUIRE(std::is_trivially_move_assignable< U >::value)
move_assign_n(U* dst, U* src, I n) noexcept
{
    memcpy(dst, src, n * sizeof(U));
}
template< class U, class I > _C4REQUIRE( ! std::is_trivially_move_assignable< U >::value)
move_assign_n(U* dst, U* src, I n) noexcept
{
    for(I i = 0; i < n; ++i)
    {
        *(dst + i) = std::move(*(src + i));
    }
}

//-----------------------------------------------------------------------------
// destroy

template< class U > _C4REQUIRE(std::is_trivially_destructible< U >::value)
destroy(U* ptr) noexcept
{
    C4_UNUSED(ptr); // nothing to do
}
template< class U > _C4REQUIRE( ! std::is_trivially_destructible< U >::value)
destroy(U* ptr) noexcept
{
    ptr->~U();
}
template< class U, class I > _C4REQUIRE(std::is_trivially_destructible< U >::value)
destroy_n(U* ptr, I n) noexcept
{
    C4_UNUSED(ptr);
    C4_UNUSED(n); // nothing to do
}
template< class U, class I > _C4REQUIRE( ! std::is_trivially_destructible< U >::value)
destroy_n(U* ptr, I n) noexcept
{
    for(I i = 0; i < n; ++i)
    {
        ptr[i].~U();
    }
}

//-----------------------------------------------------------------------------
/** makes room at the beginning of buf, which has a current size of n */
template< class U, class I > _C4REQUIRE(std::is_trivially_move_constructible< U >::value)
make_room(U *buf, I n, I room) noexcept
{
    if(room >= n)
    {
        memcpy(buf + room, buf, n * sizeof(U));
    }
    else
    {
        memmove(buf + room, buf, n * sizeof(U));
    }
}
/** makes room at the beginning of buf, which has a current size of n */
template< class U, class I > _C4REQUIRE( ! std::is_trivially_move_constructible< U >::value)
make_room(U *buf, I n, I room) C4_NOEXCEPT_A
{
    C4_ASSERT(room >= 0);
    if(room >= n)
    {
        for(I i = 0; i < n; ++i)
        {
            new (buf + (i + room)) U(std::move(buf[i]));
        }
    }
    else
    {
        for(I i = 0; i < n; ++i)
        {
            I w = n-1 - i; // do a backwards loop
            new (buf + (w + room)) U(std::move(buf[w]));
        }
    }
}

template< class U, class I > _C4REQUIRE(std::is_trivially_move_constructible< U >::value)
make_room(U *dst, U const* src, I n, I room, I pos) C4_NOEXCEPT_A
{
    C4_ASSERT(pos < n);
    memcpy(dst, src, pos * sizeof(U));
    memcpy(dst + room + pos, src + pos, (n - pos) * sizeof(U));
}
template< class U, class I > _C4REQUIRE( ! std::is_trivially_move_constructible< U >::value)
make_room(U *dst, U const* src, I n, I room, I pos)
{
    for(I i = 0; i < pos; ++i)
    {
        new (dst + i) U(std::move(src[i]));
    }
    src += pos;
    dst += room + pos;
    for(I i = 0, e = n - pos; i < e; ++i)
    {
        new (dst + i) U(std::move(src[i]));
    }
}

//-----------------------------------------------------------------------------
/** destroys room at the beginning of buf, which has a current size of n */
template< class U, class I > _C4REQUIRE(std::is_trivially_move_constructible< U >::value)
destroy_room(U *buf, I n, I room) noexcept
{
    C4_ASSERT(room <= n);
    if(room >= n)
    {
        C4_UNUSED(buf); // nothing to do
    }
    else
    {
        memmove(buf, buf + room, room * sizeof(U));
    }
}
/** destroys room at the beginning of buf, which has a current size of n */
template< class U, class I > _C4REQUIRE( ! std::is_trivially_move_constructible< U >::value)
destroy_room(U *buf, I n, I room)
{
    if(room < n)
    {
        for(I i = 0, e = n - room; i < e; ++i)
        {
            buf[i] = std::move(buf[i + room]);
        }
    }
    else
    {
        for(I i = 0; i < n; ++i)
        {
            buf[i].~U();
        }
    }
}

template< class U, class I > _C4REQUIRE(std::is_trivially_move_constructible< U >::value)
destroy_room(U *dst, U const* src, I n, I room, I pos) noexcept
{
    if(C4_UNLIKELY(n < room)) return;
    memcpy(dst, src, pos * sizeof(U));
    memcpy(dst + pos, src + room + pos, (n - pos - room) * sizeof(U));
}
template< class U, class I > _C4REQUIRE( ! std::is_trivially_move_constructible< U >::value)
destroy_room(U *dst, U const* src, I n, I room, I pos)
{
    if(C4_UNLIKELY(n < room)) return;
    for(I i = 0; i < pos; ++i)
    {
        new (dst + i) U(std::move(src[i]));
    }
    src += room + pos;
    dst += pos;
    for(I i = 0, e = n - pos - room; i < e; ++i)
    {
        new (dst + i) U(std::move(src[i]));
    }
}

C4_END_NAMESPACE(c4)

#undef _C4REQUIRE

#endif /* _C4_CTOR_DTOR_H_ */
