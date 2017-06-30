#ifndef _C4_STORAGE_RAW_HPP_
#define _C4_STORAGE_RAW_HPP_

/** @file raw.hpp Raw storage classes. */

#include "c4/config.hpp"
#include "c4/error.hpp"
#include "c4/ctor_dtor.hpp"
#include "c4/memory_util.hpp"
#include "c4/szconv.hpp"

#include "c4/storage/growth.hpp"

#include <limits>
#include <memory>
#include <tuple>

C4_BEGIN_NAMESPACE(c4)

// external forward declarations
template< class T > class Allocator;

C4_BEGIN_NAMESPACE(stg)

struct growth_default;

/** @defgroup raw_storage_classes Raw storage classes
 *
 * These classes are a building block for the several flavours of contiguous
 * containers. They offer a convenient way to hold a number of contiguous
 * objects via the index-based [] operator. raw_paged does not lay out its
 * elements contiguously in memory, but its elements are paged, so it still
 * offers a contiguous index range with constant-time access. Note the following:
 *
 * - The memory used by these objects is automatically freed. Allocation
 *   requires proper construction or alternatively explicit calls to the
 *   function _raw_reserve(I currsz, I cap).
 *
 * - The elements contained in the raw storage are NOT automatically constructed
 *   or destroyed. The exception to this is in _raw_resize(), which adds empty
 *   space for a certain number of elements at, unless through .
 */

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template< class T, class I >
struct default_page_size
{
    enum : I {
        value = 256,
        max = ((std::numeric_limits< I >::max() >> 1) + 1),
    };
};
template< class T >
struct default_page_size< T, uint8_t >
{
    enum : uint8_t {
        value = 64,
        max = 128,
    };
};
template< class T >
struct default_page_size< T, int8_t >
{
    enum : int8_t {
        value = 32,
        max = 64,
    };
};

//-----------------------------------------------------------------------------
template< class T, class I >
struct default_small_size
{
    enum : I { value = 16 };
};

//-----------------------------------------------------------------------------


/** returns the maximum size that a container can have when there is a npos
 * marker.
 * @warning assumes npos==-1 */
template< class Storage >
C4_CONSTEXPR14 C4_ALWAYS_INLINE size_t raw_max_size_with_npos()
{
    using I = typename Storage::size_type;
    size_t mc = Storage::max_capacity();
    if(mc == std::numeric_limits< I >::max() && std::is_unsigned< I >::value)
    {
        mc -= 1;
    }
    return mc;
}

//-----------------------------------------------------------------------------
/** since the page size is a power of two, the max capacity is simply the
 * maximum of the size type */
template< class I >
C4_ALWAYS_INLINE constexpr size_t raw_max_capacity() noexcept
{
    return size_t(std::numeric_limits< I >::max());
}

//-----------------------------------------------------------------------------

#define _C4_FOREACH_ARR(soa, member, macro)                             \
{                                                                       \
    size_t C4_XCAT(_foreach_dummy, __LINE__)[] = {                      \
        (                                                               \
         macro(std::get<Indices>(soa).member, Indices)                  \
         C4_COMMA                                                       \
         size_t(0)                                                      \
        )...                                                            \
    };                                                                  \
    C4_UNUSED(C4_XCAT(_foreach_dummy, __LINE__));                       \
}

#define _C4_FOREACH_ARR_LR(lsoa, rsoa, member, macro)                   \
{                                                                       \
    size_t C4_XCAT(_foreach_dummy_lr, __LINE__)[] = {                   \
        (                                                               \
         macro(std::get<Indices>(lsoa).member, std::get<Indices>(rsoa).member, Indices) \
         C4_COMMA                                                       \
         size_t(0)                                                      \
        )...                                                            \
    };                                                                  \
    C4_UNUSED(C4_XCAT(_foreach_dummy_lr, __LINE__));                    \
}

//-----------------------------------------------------------------------------

// forward declarations
template< class Storage, class TagType > struct raw_storage_traits;

/** raw contiguous storage with fixed (at compile time) capacity
 * @ingroup raw_storage_classes
 * @todo make the Alignment a size_t */
template< class T, size_t N, class I=C4_SIZE_TYPE, I Alignment=alignof(T) >
struct raw_fixed;

template< class T, size_t N, class I=C4_SIZE_TYPE, I Alignment=alignof(T) >
struct raw_fixed_soa;

/** @todo make the Alignment a size_t */
template< class T, class I=C4_SIZE_TYPE, I Alignment=alignof(T), class Alloc=Allocator<T>, class GrowthPolicy=growth_default >
struct raw;

/** @todo make the Alignment a size_t */
template< class T, class I=C4_SIZE_TYPE, I Alignment=alignof(T), class Alloc=Allocator<T>, class GrowthPolicy=growth_default >
struct raw_soa;

/** raw storage with inplace storage of up to N objects, thus saving an
 * allocation when the size is small. @ingroup raw_storage_classes
 * @todo make N the second parameter (like raw_fixed or std::array)
 * @todo make the Alignment a size_t */
template< class T, class I=C4_SIZE_TYPE, size_t N=default_small_size<T,I>::value, I Alignment=alignof(T), class Alloc=Allocator<T>, class GrowthPolicy=growth_default >
struct raw_small;

/** @todo make the Alignment a size_t */
template< class T, class I=C4_SIZE_TYPE, size_t PageSize=default_page_size<T, I>::value, I Alignment=alignof(T), class Alloc=Allocator<T> >
struct raw_paged;

/** raw paged with page size determined at runtime. @ingroup raw_storage_classes */
template< class T, class I=C4_SIZE_TYPE, I Alignment=alignof(T), class Alloc=Allocator<T> >
using raw_paged_rt = raw_paged< T, I, 0, Alignment, Alloc >;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template< class Storage >
struct tmp_storage : public Storage
{
    C4_CONSTEXPR14 C4_ALWAYS_INLINE operator bool() const { return !this->empty(); }
};
/** a class for storage policies that do not need transfer when grown or shrinked */
template< class T, class I >
struct _NoTmpStorage
{
    constexpr C4_ALWAYS_INLINE operator bool () const { return false; }
    C4_ALWAYS_INLINE T       & operator[] (I /*i*/)       { C4_NEVER_REACH(); return *(T*)nullptr; }
    C4_ALWAYS_INLINE T const & operator[] (I /*i*/) const { C4_NEVER_REACH(); return *(T*)nullptr; }
};
/** fixed storage cannot be grown or shrinked */
template< class T, size_t N, class I, I Alignment >
struct tmp_storage< raw_fixed< T, N, I, Alignment > > : public _NoTmpStorage< T, I >
{
};
template< class T, size_t N, class I, I Alignment >
struct tmp_storage< raw_fixed_soa< T, N, I, Alignment > > : public _NoTmpStorage< T, I >
{
};
/** paged storage does not need copying of the pages when grown or shrinked */
template< class T, class I, size_t PageSize, I Alignment, class Alloc >
struct tmp_storage< raw_paged< T, I, PageSize, Alignment, Alloc > > : public _NoTmpStorage< T, I >
{
};
/** small storage always uses a temporary pointer for tmp storage, so it
 * is treated just the same as tmp storage for pointer-only. */
template< class T, class I, size_t N, I Alignment, class Alloc, class GrowthPolicy >
struct tmp_storage< raw_small< T, I, N, Alignment, Alloc, GrowthPolicy > > : public tmp_storage< raw< T, I, Alignment, Alloc, GrowthPolicy > >
{
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

struct contiguous_t {};
struct fixed_t {};
struct small_t {};
struct paged_t {};

struct contiguous_soa_t {};
struct fixed_soa_t {};
struct small_soa_t {};
struct paged_soa_t {};

template< class Storage, class TagType >
struct _raw_storage_traits;

/** Type traits for raw storage classes.
 * @ingroup raw_storage_classes */
template< class Storage, class TagType >
struct raw_storage_traits : public _raw_storage_traits< Storage, TagType >
{

    enum {
        contiguous     = std::is_same< TagType, contiguous_t     >::value,
        contiguous_soa = std::is_same< TagType, contiguous_soa_t >::value,
        fixed          = std::is_same< TagType, fixed_t          >::value,
        fixed_soa      = std::is_same< TagType, fixed_soa_t      >::value,
        small          = std::is_same< TagType, small_t          >::value,
        small_soa      = std::is_same< TagType, small_soa_t      >::value,
        paged          = std::is_same< TagType, paged_t          >::value,
        paged_soa      = std::is_same< TagType, paged_soa_t      >::value,
        uses_allocator = ( ! fixed) && ( ! fixed_soa),
    };

    using _impl_type = _raw_storage_traits< Storage, TagType >;

    using tag_type = TagType;
    using storage_type = Storage;

    using _impl_type::destroy_n;
    using _impl_type::construct_n;
    using _impl_type::move_construct_n;
    using _impl_type::move_assign_n;
    using _impl_type::copy_construct_n;
    using _impl_type::copy_assign_n;
};

//-----------------------------------------------------------------------------

template< class Storage >
struct _raw_storage_calls_use_data_only
{
    using I = typename Storage::size_type;

    template< class ...Args >
    C4_ALWAYS_INLINE static void construct_n(Storage& dest, I first, I n, Args&&... args)
    {
        c4::construct_n(dest.data() + first, n, std::forward< Args >(args)...);
    }

    C4_ALWAYS_INLINE static void destroy_n(Storage& dest, I first, I n)
    {
        c4::destroy_n(dest.data() + first, n);
    }

    C4_ALWAYS_INLINE static void move_construct_n(Storage& dest, Storage& src, I first, I n)
    {
        c4::move_construct_n(dest.data() + first, src.data() + first, n);
    }

    C4_ALWAYS_INLINE static void move_assign_n(Storage& dest, Storage& src, I first, I n)
    {
        c4::move_assign_n(dest.data() + first, src.data() + first, n);
    }

    C4_ALWAYS_INLINE static void copy_construct_n(Storage& dest, Storage const& src, I first, I n)
    {
        c4::copy_construct_n(dest.data() + first, src.data() + first, n);
    }

    C4_ALWAYS_INLINE static void copy_assign_n(Storage& dest, Storage const& src, I first, I n)
    {
        c4::copy_assign_n(dest.data() + first, src.data() + first, n);
    }
};

template< class Storage >
struct _raw_storage_calls_use_allocator_and_data
{
    using I = typename Storage::size_type;
    using allocator_traits = typename Storage::allocator_traits;

    /// @todo use alloc_traits here and in the other functions
    template< class ...Args >
    C4_ALWAYS_INLINE static void construct_n(Storage& dest, I first, I n, Args&&... args)
    {
        dest.m_cap_n_alloc.alloc().construct_n(dest.data() + first, n, std::forward< Args >(args)...);
    }

    C4_ALWAYS_INLINE static void destroy_n(Storage& dest, I first, I n)
    {
        dest.m_cap_n_alloc.alloc().destroy_n(dest.data() + first, n);
    }

    C4_ALWAYS_INLINE static void move_construct_n(Storage& dest, Storage& src, I first, I n)
    {
        c4::move_construct_n(dest.data() + first, src.data() + first, n);
    }

    C4_ALWAYS_INLINE static void copy_construct_n(Storage& dest, Storage& src, I first, I n)
    {
        c4::copy_construct_n(dest.data() + first, src.data() + first, n);
    }

    C4_ALWAYS_INLINE static void move_assign_n(Storage& dest, Storage const& src, I first, I n)
    {
        c4::move_assign_n(dest.data() + first, src.data() + first, n);
    }

    C4_ALWAYS_INLINE static void copy_assign_n(Storage& dest, Storage const& src, I first, I n)
    {
        c4::copy_assign_n(dest.data() + first, src.data() + first, n);
    }
};

template< class Storage >
struct _raw_storage_calls_forward_to_storage
{
    using I = typename Storage::size_type;

    template< class ...Args >
    C4_ALWAYS_INLINE static void construct_n(Storage& dest, I first, I n, Args&&... args)
    {
        dest._raw_construct_n(first, n, std::forward< Args >(args)...);
    }

    C4_ALWAYS_INLINE static void destroy_n(Storage& dest, I first, I n)
    {
        dest._raw_destroy_n(first, n);
    }

    C4_ALWAYS_INLINE static void move_construct_n(Storage& dest, Storage& src, I first, I n)
    {
        dest._raw_move_construct_n(src, first, n);
    }

    C4_ALWAYS_INLINE static void move_assign_n(Storage& dest, Storage& src, I first, I n)
    {
        dest._raw_move_assign_n(src, first, n);
    }

    C4_ALWAYS_INLINE static void copy_construct_n(Storage& dest, Storage const& src, I first, I n)
    {
        dest._raw_copy_construct_n(src, first, n);
    }

    C4_ALWAYS_INLINE static void copy_assign_n(Storage& dest, Storage const& src, I first, I n)
    {
        dest._raw_copy_assign_n(src, first, n);
    }
};

//-----------------------------------------------------------------------------

/** specialization of _raw_storage_traits for fixed storage containers.
 * @ingroup raw_storage_classes */
template< class Storage >
struct _raw_storage_traits< Storage, fixed_t > : public _raw_storage_calls_use_data_only< Storage >
{
};

template< class Storage >
struct _raw_storage_traits< Storage, fixed_soa_t > : public _raw_storage_calls_forward_to_storage< Storage >
{
};

/** specialization of _raw_storage_traits for non-fixed contiguous containers.
 * @ingroup raw_storage_classes */
template< class Storage >
struct _raw_storage_traits< Storage, contiguous_t > : public _raw_storage_calls_use_allocator_and_data< Storage >
{
};
template< class Storage >
struct _raw_storage_traits< Storage, contiguous_soa_t > : public _raw_storage_calls_forward_to_storage< Storage >
{
};

/** specialization of _raw_storage_traits for small contiguous containers.
 * @ingroup raw_storage_classes */
template< class Storage >
struct _raw_storage_traits< Storage, small_t > : public _raw_storage_calls_use_allocator_and_data< Storage >
{
};

/** specialization of _raw_storage_traits for paged storage containers.
 * @ingroup raw_storage_classes */
template< class Storage >
struct _raw_storage_traits< Storage, paged_t > : public _raw_storage_calls_forward_to_storage< Storage >
{
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/** pair of value and allocator. Makes use of the empty base class optimization to
 * compress the allocator when it has no members */
template< class I, class Alloc >
struct valnalloc : public Alloc
{
    I m_value;

    valnalloc(I v) : Alloc(), m_value(v) {}
    valnalloc(I v, Alloc const& a) : Alloc(a), m_value(v) {}
    valnalloc(Alloc const& a) : Alloc(a), m_value(0) {}

    C4_ALWAYS_INLINE C4_CONSTEXPR14 I          & value()       { return m_value; }
    C4_ALWAYS_INLINE C4_CONSTEXPR14 I     const& value() const { return m_value; }
    C4_ALWAYS_INLINE C4_CONSTEXPR14 Alloc      & alloc()       { return static_cast< Alloc      & >(*this); }
    C4_ALWAYS_INLINE C4_CONSTEXPR14 Alloc const& alloc() const { return static_cast< Alloc const& >(*this); }
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/** for instantiating raw storage with structure of arrays */
template< class... SoaTypes >
struct soa
{
};

template< class T, size_t N, size_t Alignment=alignof(T) >
struct mem_fixed
{
    static_assert(Alignment >= alignof(T), "bad alignment value");
    /** the union with the char buffer is needed to prevent auto-construction
     * of the elements in m_arr */
    union {
        alignas(Alignment) T     m_arr[N];
        alignas(Alignment) char  m_buf[N * sizeof(T)];
    };

    C4_ALWAYS_INLINE mem_fixed() {}
    C4_ALWAYS_INLINE ~mem_fixed() {}
};

template< class T, size_t N, size_t Alignment >
struct mem_small
{
    static_assert(Alignment >= alignof(T), "bad alignment value");

#ifdef __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wnested-anon-types" // warning: anonymous types declared in an anonymous union are an extension
#endif

    union {
        /** the union with the char buffer is needed to prevent
         * auto-construction of the elements in m_arr */
        union {
            alignas(Alignment) T    m_arr[N];
            alignas(Alignment) char m_buf[N * sizeof(T)];
        };
        T * m_ptr;
    };

#ifdef __clang__
#   pragma clang diagnostic pop
#endif

    C4_ALWAYS_INLINE mem_small() : m_ptr(nullptr) {}
    C4_ALWAYS_INLINE mem_small(T *p) : m_ptr(p) {}
    C4_ALWAYS_INLINE ~mem_small() {}
};

template< class T >
struct mem_raw
{
    T *m_ptr;

    C4_ALWAYS_INLINE mem_raw() : m_ptr(nullptr) {}
    C4_ALWAYS_INLINE mem_raw(T *p) : m_ptr(p) {}
    C4_ALWAYS_INLINE ~mem_raw() {}
};

template< class T >
struct mem_paged
{
    T **m_pages;

    C4_ALWAYS_INLINE mem_paged() : m_pages(nullptr) {}
    C4_ALWAYS_INLINE mem_paged(T **p) : m_pages(p) {}
    C4_ALWAYS_INLINE ~mem_paged() {}
};


template< class T, size_t Cap >                struct ___natvis_util       { T*  p; };
template< class T, size_t PgSz, size_t NumPg > struct ___natvis_util_paged { T** p; };

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template< class T, size_t N, class I, I Alignment >
struct raw_fixed : public mem_fixed< T, N, Alignment >
{

    C4_STATIC_ASSERT(N <= (size_t)std::numeric_limits< I >::max());

public:

    _c4_DEFINE_ARRAY_TYPES(T, I);
    using allocator_type = void;

    using storage_traits = raw_storage_traits< raw_fixed, fixed_t >;

    template< class U >
    using rebind_type = raw_fixed<U, N, I, alignof(U)>;

    using tmp_type = tmp_storage< raw_fixed >;

    enum : I { arr_size = static_cast<I>(N), num_arrays = 1, };

public:

    C4_ALWAYS_INLINE raw_fixed() {}
    C4_ALWAYS_INLINE ~raw_fixed() {}

    C4_ALWAYS_INLINE raw_fixed(I cap) { C4_UNUSED(cap); C4_ASSERT(cap <= (I)N); }

    // copy and move operations are deleted, and must be implemented by the containers,
    // as this will involve knowledge over what elements are to copied or moved
    C4_NO_COPY_OR_MOVE(raw_fixed);

    C4_ALWAYS_INLINE T      & operator[] (I i)       C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < (I)N); return this->m_arr[i]; }
    C4_ALWAYS_INLINE T const& operator[] (I i) const C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < (I)N); return this->m_arr[i]; }

    C4_ALWAYS_INLINE T      * data()       noexcept { return this->m_arr; }
    C4_ALWAYS_INLINE T const* data() const noexcept { return this->m_arr; }

    C4_ALWAYS_INLINE constexpr I capacity() const noexcept { return (I)N; }
    C4_ALWAYS_INLINE constexpr bool empty() const noexcept { return false; }

    C4_ALWAYS_INLINE constexpr static size_t max_capacity() noexcept { return N; }
    C4_ALWAYS_INLINE C4_CONSTEXPR14 static size_t next_capacity(size_t cap) C4_NOEXCEPT_A
    {
        C4_UNUSED(cap);
        C4_ASSERT(cap <= (size_t)N);
        return N;
    }

public:

    iterator       _raw_iterator(I id)       noexcept { return this->m_arr + id; }
    const_iterator _raw_iterator(I id) const noexcept { return this->m_arr + id; }

public:

    /** assume the curr size is zero */
    void _raw_reserve(I cap)
    {
        _raw_reserve(0, cap);
    }

    void _raw_reserve(I currsz, I cap) const
    {
        C4_UNUSED(currsz);
        C4_UNUSED(cap);
        C4_ASSERT(cap <= (I)N);
    }

    void _raw_reserve_allocate(I cap, tmp_type * /*tmp*/)
    {
        C4_UNUSED(cap);
        C4_ASSERT(cap <= (I)N);
    }
    void _raw_reserve_replace(I /*tmpsz*/, tmp_type * /*tmp*/)
    {
        C4_NEVER_REACH();
    }

    C4_ALWAYS_INLINE void _raw_resize(I pos, I prevsz, I nextsz);
    C4_ALWAYS_INLINE void _raw_make_room(I pos, I prevsz, I more);
    C4_ALWAYS_INLINE void _raw_destroy_room(I pos, I prevsz, I less);

};

//-----------------------------------------------------------------------------

template< class T, size_t N, class I, I Alignment, class IndexSequence >
struct raw_fixed_soa_impl;

/** raw fixed storage for structure-of-arrays. this is a work in progress */
template< class... SoaTypes, size_t N, class I, I Alignment, size_t... Indices >
struct raw_fixed_soa_impl< soa<SoaTypes...>, N, I, Alignment, index_sequence<Indices...>() >
{
    template< class U > struct maxalign { enum { value = (Alignment > alignof(U) ? Alignment : alignof(U)) }; };
    template< class U > using arr_type = mem_fixed< U, N, maxalign<U>::value >;

    std::tuple< arr_type<SoaTypes>... > m_soa;

    C4_STATIC_ASSERT(N <= (size_t)std::numeric_limits< I >::max());

public:

    _c4_DEFINE_TUPLE_ARRAY_TYPES(SoaTypes, I);
    using allocator_type = void;

    using storage_traits = raw_storage_traits< raw_fixed_soa_impl, fixed_soa_t >;

    template< class U >
    using rebind_type = raw_fixed_soa_impl<U, N, I, maxalign<U>::value, index_sequence<Indices...>()>;

    template< I n > using nth_type = typename std::tuple_element< n, std::tuple<SoaTypes...> >::type;
    using tuple_type = std::tuple< arr_type<SoaTypes>... >;

    using tmp_type = tmp_storage< raw_fixed_soa_impl >;

    enum : I {
        arr_size = static_cast<I>(N),
        num_arrays = static_cast<I>(sizeof...(SoaTypes))
    };

public:

    C4_ALWAYS_INLINE ~raw_fixed_soa_impl() {}
    C4_ALWAYS_INLINE raw_fixed_soa_impl() {}
    C4_ALWAYS_INLINE raw_fixed_soa_impl(I cap) { C4_UNUSED(cap); C4_ASSERT(cap <= (I)N); }

    // copy and move operations are deleted, and must be implemented by the containers,
    // as this will involve knowledge over what elements are to copied or moved
    C4_NO_COPY_OR_MOVE(raw_fixed_soa_impl);

public:

    C4_ALWAYS_INLINE constexpr I capacity() const noexcept { return (I)N; }
    C4_ALWAYS_INLINE constexpr bool empty() const noexcept { return false; }

    C4_ALWAYS_INLINE constexpr static size_t max_capacity() noexcept { return N; }
    C4_ALWAYS_INLINE C4_CONSTEXPR14 static size_t next_capacity(size_t cap) C4_NOEXCEPT_A
    {
        C4_UNUSED(cap);
        C4_ASSERT(cap <= (size_t)N);
        return N;
    }

public:

    // n defaults to 0 so that one-typed tuples can be used without an explicit index
    template< I n=0 > C4_ALWAYS_INLINE nth_type<n>      * data()       { C4_STATIC_ASSERT(n < num_arrays); return std::get<n>(m_soa).m_arr; }
    template< I n=0 > C4_ALWAYS_INLINE nth_type<n> const* data() const { C4_STATIC_ASSERT(n < num_arrays); return std::get<n>(m_soa).m_arr; }

    template< I n=0 > C4_ALWAYS_INLINE nth_type<n>      & operator[] (I i)       C4_NOEXCEPT_X { C4_STATIC_ASSERT(n < num_arrays); C4_XASSERT(i >= 0 && i < (I)N); return std::get<n>(m_soa).m_arr[i]; }
    template< I n=0 > C4_ALWAYS_INLINE nth_type<n> const& operator[] (I i) const C4_NOEXCEPT_X { C4_STATIC_ASSERT(n < num_arrays); C4_XASSERT(i >= 0 && i < (I)N); return std::get<n>(m_soa).m_arr[i]; }
    template< I n=0 > C4_ALWAYS_INLINE nth_type<n>      & get        (I i)       C4_NOEXCEPT_X { C4_STATIC_ASSERT(n < num_arrays); C4_XASSERT(i >= 0 && i < (I)N); return std::get<n>(m_soa).m_arr[i]; }
    template< I n=0 > C4_ALWAYS_INLINE nth_type<n> const& get        (I i) const C4_NOEXCEPT_X { C4_STATIC_ASSERT(n < num_arrays); C4_XASSERT(i >= 0 && i < (I)N); return std::get<n>(m_soa).m_arr[i]; }

    template< I n=0 > C4_ALWAYS_INLINE nth_type<n>      * _raw_iterator(I id)       noexcept { C4_STATIC_ASSERT(n < num_arrays); return std::get<n>(m_soa).m_arr + id; }
    template< I n=0 > C4_ALWAYS_INLINE nth_type<n> const* _raw_iterator(I id) const noexcept { C4_STATIC_ASSERT(n < num_arrays); return std::get<n>(m_soa).m_arr + id; }

public:

    /** assume the curr size is zero */
    void _raw_reserve(I cap)
    {
        _raw_reserve(0, cap);
    }

    void _raw_reserve(I currsz, I cap) const
    {
        C4_UNUSED(currsz);
        C4_UNUSED(cap);
        C4_ASSERT(cap <= (I)N);
    }

    void _raw_reserve_allocate(I cap, tmp_type * /*tmp*/)
    {
        C4_UNUSED(cap);
        C4_ASSERT(cap <= (I)N);
    }
    void _raw_reserve_replace(I /*tmpsz*/, tmp_type * /*tmp*/)
    {
        C4_NEVER_REACH();
    }

    C4_ALWAYS_INLINE void _raw_resize(I pos, I prevsz, I nextsz);
    C4_ALWAYS_INLINE void _raw_make_room(I pos, I prevsz, I more);
    C4_ALWAYS_INLINE void _raw_destroy_room(I pos, I prevsz, I less);

public:

    /** passes args to all arrays */
    template< class ...Args >
    void _raw_construct_n(I first, I n, Args const&... args)
    {
        #define _c4mcr(arr, i) c4::construct_n(arr + first, n, args...)
        _C4_FOREACH_ARR(m_soa, m_arr, _c4mcr)
        #undef _c4mcr
    }

    /** passes each tuple element to the elements of its corresponding array */
    template< class ...Args >
    void _raw_construct_n(I first, I n, std::tuple< Args... > const& args)
    {
        static_assert(sizeof...(args) == sizeof...(SoaTypes), "incompatible number of arguments");
        #define _c4mcr(arr, i) c4::construct_n(arr + first, n, std::forward(std::get< i >(args)))
        _C4_FOREACH_ARR(m_soa, m_arr, _c4mcr)
        #undef _c4mcr
    }

    void _raw_destroy_n(I first, I n)
    {
        #define _c4mcr(arr, i) c4::destroy_n(arr + first, n)
        _C4_FOREACH_ARR(m_soa, m_arr, _c4mcr)
        #undef _c4mcr
    }

    void _raw_move_construct_n(raw_fixed_soa_impl& src, I first, I n)
    {
        #define _c4mcr(larr, rarr, i) c4::move_construct_n(larr + first, rarr + first, n)
        _C4_FOREACH_ARR_LR(m_soa, src.m_soa, m_arr, _c4mcr)
        #undef _c4mcr
    }

    void _raw_move_assign_n(raw_fixed_soa_impl& src, I first, I n)
    {
        #define _c4mcr(larr, rarr, i) c4::move_assign_n(larr + first, rarr + first, n)
        _C4_FOREACH_ARR_LR(m_soa, src.m_soa, m_arr, _c4mcr)
        #undef _c4mcr
    }

    void _raw_copy_construct_n(raw_fixed_soa_impl const& src, I first, I n)
    {
        #define _c4mcr(larr, rarr, i) c4::copy_construct_n(larr + first, rarr + first, n)
        _C4_FOREACH_ARR_LR(m_soa, src.m_soa, m_arr, _c4mcr)
        #undef _c4mcr
    }

    void _raw_copy_assign_n(raw_fixed_soa_impl const& src, I first, I n)
    {
        #define _c4mcr(larr, rarr, i) c4::copy_assign_n(larr + first, rarr + first, n)
        _C4_FOREACH_ARR_LR(m_soa, src.m_soa, m_arr, _c4mcr)
        #undef _c4mcr
    }

};

/** implement raw_fixed_soa for a single array */
template< class T, size_t N, class I, I Alignment >
struct raw_fixed_soa
    :
        public raw_fixed_soa_impl< soa<T>, N, I, Alignment, index_sequence<0>() >
{
    using _impl_type = raw_fixed_soa_impl< soa<T>, N, I, Alignment, index_sequence<0>() >;
    using _impl_type::_impl_type;
};

/** implement raw_fixed_soa for multiple arrays */
template< class... SoaTypes, size_t N, class I, I Alignment >
struct raw_fixed_soa< soa<SoaTypes...>, N, I, Alignment >
    :
        public raw_fixed_soa_impl< soa<SoaTypes...>, N, I, Alignment, index_sequence_for<SoaTypes...>() >
{
    using _impl_type = raw_fixed_soa_impl< soa<SoaTypes...>, N, I, Alignment, index_sequence_for<SoaTypes...>() >;
    using _impl_type::_impl_type;
};

//-----------------------------------------------------------------------------

/** Resize the buffer at pos, so that prevsz increases to nextsz;
 *  when growing, ___adds to the right___ of pos; when
 *  shrinking, ___removes to the left___ of pos. If growing, the capacity
 *  will increase to the value obtained with the growth policy; if shrinking,
 *  the capacity will stay the same. Use _raw_reserve() to diminish the
 *  capacity.
 *
 *  @param pos the position from which room is to be created (to the right)
 *         or destroyed (to the left)
 *  @param prevsz the previous size
 *  @param nextsz the next size
 *  @see _raw_make_room
 *  @see _raw_destroy_room
 */
template< class T, size_t N, class I, I Alignment >
void raw_fixed< T, N, I, Alignment >::_raw_resize(I pos, I prevsz, I nextsz)
{
    if(nextsz > prevsz) // grow to the right of pos
    {
        _raw_make_room(pos, prevsz, nextsz-prevsz);
    }
    else if(nextsz < prevsz) // shrink to the left of pos
    {
        _raw_destroy_room(pos, prevsz, prevsz-nextsz);
    }
}

/** Resize the buffer at pos, so that prevsz increases to nextsz;
 *  when growing, ___adds to the right___ of pos; when
 *  shrinking, ___removes to the left___ of pos. If growing, the capacity
 *  will increase to the value obtained with the growth policy; if shrinking,
 *  the capacity will stay the same. Use _raw_reserve() to diminish the
 *  capacity.
 *
 *  @param pos the position from which room is to be created (to the right)
 *         or destroyed (to the left)
 *  @param prevsz the previous size
 *  @param nextsz the next size
 *  @see _raw_make_room
 *  @see _raw_destroy_room
 */
template< class... SoaTypes, size_t N, class I, I Alignment, size_t... Indices >
void raw_fixed_soa_impl< soa<SoaTypes...>, N, I, Alignment, std::index_sequence<Indices...>() >::
_raw_resize(I pos, I prevsz, I nextsz)
{
    if(nextsz > prevsz) // grow to the right of pos
    {
        _raw_make_room(pos, prevsz, nextsz-prevsz);
    }
    else if(nextsz < prevsz) // shrink to the left of pos
    {
        _raw_destroy_room(pos, prevsz, prevsz-nextsz);
    }
}

//-----------------------------------------------------------------------------
/** grow to the right of pos
 @code
 pos: 0 1 2 3 4 5 6 7 8
 val: A B C D E F . . .

 _raw_make_room(3, 6, 3)

 pos: 0 1 2 3 4 5 6 7 8
 val: A B C . . . D E F
 @endcode
 */
template< class T, size_t N, class I, I Alignment >
void raw_fixed< T, N, I, Alignment >::_raw_make_room(I pos, I prevsz, I more)
{
    C4_ASSERT(prevsz >= 0 && prevsz < N);
    C4_ASSERT(more   >= 0 && more   < N);
    C4_ASSERT(pos    >= 0 && pos    < N);
    C4_ASSERT(prevsz+more >= 0 && prevsz+more < N);
    C4_ASSERT(pos <= prevsz);
    c4::make_room(this->m_arr + pos, prevsz - pos, more);
}

/** grow to the right of pos
 @code
 pos: 0 1 2 3 4 5 6 7 8
 val: A B C D E F . . .

 _raw_make_room(3, 6, 3)

 pos: 0 1 2 3 4 5 6 7 8
 val: A B C . . . D E F
 @endcode
 */
template< class... SoaTypes, size_t N, class I, I Alignment, size_t... Indices >
void raw_fixed_soa_impl< soa<SoaTypes...>, N, I, Alignment, std::index_sequence<Indices...>() >::
_raw_make_room(I pos, I prevsz, I more)
{
    C4_ASSERT(prevsz >= 0 && prevsz < N);
    C4_ASSERT(more   >= 0 && more   < N);
    C4_ASSERT(pos    >= 0 && pos    < N);
    C4_ASSERT(prevsz+more >= 0 && prevsz+more < N);
    C4_ASSERT(pos <= prevsz);
    #define _c4mcr(arr, i) c4::make_room(arr + pos, prevsz - pos, more)
    _C4_FOREACH_ARR(m_soa, m_arr, _c4mcr)
    #undef _c4mcr
}

//-----------------------------------------------------------------------------
/** shrink to the left of pos
 @code
 pos: 0 1 2 3 4 5 6 7 8
 val: A B C D E F G H I

 _raw_destroy_room(6, 9, 3)

 pos: 0 1 2 3 4 5
 val: A B C G H I
 @endcode
 */
template< class T, size_t N, class I, I Alignment >
void raw_fixed< T, N, I, Alignment >::_raw_destroy_room(I pos, I prevsz, I less)
{
    C4_ASSERT(prevsz >= 0 && prevsz < N);
    C4_ASSERT(pos    >= 0 && pos    < N);
    C4_ASSERT(pos <= prevsz);
    C4_ASSERT(pos >= less);
    I delta = pos - less;
    c4::destroy_room(this->m_arr + delta, prevsz - delta, less);
}

/** shrink to the left of pos
 @code
 pos: 0 1 2 3 4 5 6 7 8
 val: A B C D E F G H I

 _raw_destroy_room(6, 9, 3)

 pos: 0 1 2 3 4 5
 val: A B C G H I
 @endcode
 */
template< class... SoaTypes, size_t N, class I, I Alignment, size_t... Indices >
void raw_fixed_soa_impl< soa<SoaTypes...>, N, I, Alignment, std::index_sequence<Indices...>() >::
_raw_destroy_room(I pos, I prevsz, I less)
{
    C4_ASSERT(prevsz >= 0 && prevsz < N);
    C4_ASSERT(pos    >= 0 && pos    < N);
    C4_ASSERT(pos <= prevsz);
    C4_ASSERT(pos >= less);
    I delta = pos - less;
    #define _c4mcr(arr, i) c4::destroy_room(arr + delta, prevsz - delta, less)
    _C4_FOREACH_ARR(m_soa, m_arr, _c4mcr)
    #undef _c4mcr
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/** raw contiguous storage with variable capacity
 * @ingroup raw_storage_classes */
template< class T, class I, I Alignment, class Alloc, class GrowthPolicy >
struct raw : public mem_raw< T >
{

    // m_ptr is brought in by the base class

    valnalloc<I, Alloc> m_cap_n_alloc;

    enum : I { num_arrays = 1, };

public:

    _c4_DEFINE_ARRAY_TYPES(T, I);
    using storage_traits = raw_storage_traits< raw, contiguous_t >;

    using allocator_type = Alloc;
    using allocator_traits = std::allocator_traits< Alloc >;
    template< class U >
    using rebind_alloc = typename allocator_traits::template rebind_alloc< U >;
    using growth_policy = GrowthPolicy;

    template< class U >
    using rebind_type = raw<U, I, alignof(U), rebind_alloc<U>, GrowthPolicy>;

    using tmp_type = tmp_storage< raw >;

public:

    raw() : raw(0) {}
    raw(Alloc const& a) : raw(0, a) {}

    raw(I cap) : mem_raw<T>(nullptr), m_cap_n_alloc(0) { _raw_reserve(0, cap); }
    raw(I cap, Alloc const& a) : mem_raw<T>(nullptr), m_cap_n_alloc(0, a) { _raw_reserve(0, cap); }

    ~raw()
    {
        _raw_reserve(0, 0);
    }

    // copy and move operations are deleted, and must be implemented by the containers,
    // as this will involve knowledge over what elements are to be copied or moved
    C4_NO_COPY_OR_MOVE(raw);

public:

    C4_ALWAYS_INLINE T      & operator[] (I i)       C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < m_cap_n_alloc.m_value); return this->m_ptr[i]; }
    C4_ALWAYS_INLINE T const& operator[] (I i) const C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < m_cap_n_alloc.m_value); return this->m_ptr[i]; }

    C4_ALWAYS_INLINE T      * data()       noexcept { return this->m_ptr; }
    C4_ALWAYS_INLINE T const* data() const noexcept { return this->m_ptr; }

    C4_ALWAYS_INLINE I capacity() const noexcept { return m_cap_n_alloc.m_value; }
    C4_ALWAYS_INLINE bool empty() const noexcept { return m_cap_n_alloc.m_value == 0; }

    C4_ALWAYS_INLINE constexpr static size_t max_capacity() noexcept { return raw_max_capacity< I >(); }
    C4_ALWAYS_INLINE constexpr size_t next_capacity(size_t desired) const noexcept
    {
        return GrowthPolicy::next_size(sizeof(T), m_cap_n_alloc.m_value, desired);
    }

public:

    iterator       _raw_iterator(I id)       noexcept { return this->m_ptr + id; }
    const_iterator _raw_iterator(I id) const noexcept { return this->m_ptr + id; }

public:

    C4_ALWAYS_INLINE Alloc      & allocator()       { return m_cap_n_alloc.alloc(); }
    C4_ALWAYS_INLINE Alloc const& allocator() const { return m_cap_n_alloc.alloc(); }

    void _raw_reserve(I cap);
    void _raw_reserve(I currsz, I cap);

    void _raw_reserve_allocate(I cap, tmp_type *tmp);
    void _raw_reserve_replace(I /*tmpsz*/, tmp_type *tmp);

    void _raw_resize(I pos, I prev, I next);
    void _raw_make_room(I pos, I prevsz, I more);
    void _raw_destroy_room(I pos, I prevsz, I less);

};


//-----------------------------------------------------------------------------

template< class T, class I, I Alignment, class Alloc, class GrowthPolicy, class IndexSequence >
struct raw_soa_impl;

/** raw contiguous storage with variable capacity for structure of arrays.
 * @ingroup raw_storage_classes */
template< class... SoaTypes, class I, I Alignment, class Alloc, class GrowthPolicy, size_t... Indices >
struct raw_soa_impl< soa<SoaTypes...>, I, Alignment, Alloc, GrowthPolicy, index_sequence<Indices...>() >
{
    template< class U > struct maxalign { enum { value = (Alignment > alignof(U) ? Alignment : alignof(U)) }; };
    template< class U > using arr_type = mem_raw< U >;

    std::tuple< arr_type<SoaTypes>... > m_soa;

    valnalloc<I, Alloc> m_cap_n_alloc;

public:

    _c4_DEFINE_TUPLE_ARRAY_TYPES(SoaTypes, I);
    using storage_traits = raw_storage_traits< raw_soa_impl, contiguous_soa_t >;

    using allocator_type = Alloc;
    using allocator_traits = std::allocator_traits< Alloc >;
    template< class U >
    using rebind_alloc = typename allocator_traits::template rebind_alloc< U >;
    using growth_policy = GrowthPolicy;

    template< class U >
    using rebind_type = raw_soa_impl<U, I, maxalign<U>::value, rebind_alloc<U>, GrowthPolicy, index_sequence<Indices...>()>;

    template< I n > using nth_type = typename std::tuple_element< n, std::tuple<SoaTypes...> >::type;
    template< I n > using nth_allocator_type = rebind_alloc< nth_type<n> >;
    template< I n > using nth_alignment = maxalign< nth_type<n> >;
    using tuple_type = std::tuple< arr_type<SoaTypes>... >;

    using tmp_type = tmp_storage< raw_soa_impl >;

    enum : I { num_arrays = sizeof...(SoaTypes), };

public:

    raw_soa_impl() : raw_soa_impl(0) {}
    raw_soa_impl(Alloc const& a) : raw_soa_impl(0, a) {}

    raw_soa_impl(I cap) : m_soa(), m_cap_n_alloc(0) { C4_ASSERT(std::get<0>(m_soa).m_ptr == nullptr); _raw_reserve(0, cap); }
    raw_soa_impl(I cap, Alloc const& a) : m_soa(), m_cap_n_alloc(0, a) { C4_ASSERT(std::get<0>(m_soa).m_ptr == nullptr); _raw_reserve(0, cap); }

    ~raw_soa_impl()
    {
        _raw_reserve(0, 0);
    }

    // copy and move operations are deleted, and must be implemented by the containers,
    // as this will involve knowledge over what elements are to be copied or moved
    C4_NO_COPY_OR_MOVE(raw_soa_impl);

public:

    C4_ALWAYS_INLINE Alloc      & allocator()       { return m_cap_n_alloc.alloc(); }
    C4_ALWAYS_INLINE Alloc const& allocator() const { return m_cap_n_alloc.alloc(); }
    template< I n=0 > C4_ALWAYS_INLINE nth_allocator_type<n> nth_allocator() const { return nth_allocator_type<n>(m_cap_n_alloc.alloc()); }

    C4_ALWAYS_INLINE I capacity() const noexcept { return m_cap_n_alloc.m_value; }
    C4_ALWAYS_INLINE bool empty() const noexcept { return m_cap_n_alloc.m_value == 0; }

    C4_ALWAYS_INLINE constexpr static size_t max_capacity() noexcept { return raw_max_capacity< I >(); }
    C4_ALWAYS_INLINE constexpr size_t next_capacity(size_t desired) const noexcept
    {
        return GrowthPolicy::next_size(sizeof(nth_type<0>), m_cap_n_alloc.m_value, desired);
    }

public:

    template< I n=0 > C4_ALWAYS_INLINE nth_type<n>      * data()       noexcept { return std::get<n>(m_soa).m_ptr; }
    template< I n=0 > C4_ALWAYS_INLINE nth_type<n> const* data() const noexcept { return std::get<n>(m_soa).m_ptr; }

    template< I n=0 > C4_ALWAYS_INLINE nth_type<n>      & operator[] (I i)       C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < m_cap_n_alloc.m_value); return std::get<n>(m_soa).m_ptr[i]; }
    template< I n=0 > C4_ALWAYS_INLINE nth_type<n> const& operator[] (I i) const C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < m_cap_n_alloc.m_value); return std::get<n>(m_soa).m_ptr[i]; }
    template< I n=0 > C4_ALWAYS_INLINE nth_type<n>      & get        (I i)       C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < m_cap_n_alloc.m_value); return std::get<n>(m_soa).m_ptr[i]; }
    template< I n=0 > C4_ALWAYS_INLINE nth_type<n> const& get        (I i) const C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < m_cap_n_alloc.m_value); return std::get<n>(m_soa).m_ptr[i]; }

    template< I n=0 > C4_ALWAYS_INLINE nth_type<n>      * _raw_iterator(I id)       noexcept { return this->m_ptr + id; }
    template< I n=0 > C4_ALWAYS_INLINE nth_type<n> const* _raw_iterator(I id) const noexcept { return this->m_ptr + id; }

public:

    void _raw_reserve(I cap);

    void _raw_reserve(I currsz, I cap);
    void _raw_reserve_allocate(I cap, tmp_type *tmp);
    void _raw_reserve_replace(I tmpsz, tmp_type *tmp);

    void _raw_resize(I pos, I prev, I next);
    void _raw_make_room(I pos, I prevsz, I more);
    void _raw_destroy_room(I pos, I prevsz, I less);

public:

    template< I n > void _do_raw_reserve(I currsz, I cap);
    template< I n > void _do_raw_reserve_allocate(I cap, tmp_type *tmp);
    template< I n > void _do_raw_reserve_replace(I tmpsz, tmp_type *tmp);

    template< I n > void _do_raw_resize(I pos, I prev, I next);
    template< I n > void _do_raw_make_room(I pos, I prevsz, I more);
    template< I n > void _do_raw_destroy_room(I pos, I prevsz, I less);

public:

    /** passes args to all arrays */
    template< class ...Args >
    void _raw_construct_n(I first, I n, Args const&... args)
    {
        #define _c4mcr(arr, i) c4::construct_n(arr + first, n, args...)
        _C4_FOREACH_ARR(m_soa, m_ptr, _c4mcr)
        #undef _c4mcr
    }

    /** passes each tuple element to the elements of its corresponding array */
    template< class ...Args >
    void _raw_construct_n(I first, I n, std::tuple< Args... > const& args)
    {
        static_assert(sizeof...(args) == sizeof...(SoaTypes), "incompatible number of arguments");
        #define _c4mcr(arr, i) c4::construct_n(arr + first, n, std::forward(std::get< i >(args)))
        _C4_FOREACH_ARR(m_soa, m_ptr, _c4mcr)
        #undef _c4mcr
    }

    void _raw_destroy_n(I first, I n)
    {
        #define _c4mcr(arr, i) c4::destroy_n(arr + first, n)
        _C4_FOREACH_ARR(m_soa, m_ptr, _c4mcr)
        #undef _c4mcr
    }

    void _raw_move_construct_n(raw_soa_impl& src, I first, I n)
    {
        #define _c4mcr(larr, rarr, i) c4::move_construct_n(larr + first, rarr + first, n)
        _C4_FOREACH_ARR_LR(m_soa, src.m_soa, m_ptr, _c4mcr)
        #undef _c4mcr
    }

    void _raw_move_assign_n(raw_soa_impl& src, I first, I n)
    {
        #define _c4mcr(larr, rarr, i) c4::move_assign_n(larr + first, rarr + first, n)
        _C4_FOREACH_ARR_LR(m_soa, src.m_soa, m_ptr, _c4mcr)
        #undef _c4mcr
    }

    void _raw_copy_construct_n(raw_soa_impl const& src, I first, I n)
    {
        #define _c4mcr(larr, rarr, i) c4::copy_construct_n(larr + first, rarr + first, n)
        _C4_FOREACH_ARR_LR(m_soa, src.m_soa, m_ptr, _c4mcr)
        #undef _c4mcr
    }

    void _raw_copy_assign_n(raw_soa_impl const& src, I first, I n)
    {
        #define _c4mcr(larr, rarr, i) c4::copy_assign_n(larr + first, rarr + first, n)
        _C4_FOREACH_ARR_LR(m_soa, src.m_soa, m_ptr, _c4mcr)
        #undef _c4mcr
    }
};


/** implement raw_fixed_soa for a single array */
template< class T, class I, I Alignment, class Alloc, class GrowthPolicy >
struct raw_soa
    :
        public raw_soa_impl< soa<T>, I, Alignment, Alloc, GrowthPolicy, index_sequence<0>() >
{
    using _impl_type = raw_soa_impl< soa<T>, I, Alignment, Alloc, GrowthPolicy, index_sequence<0>() >;
    using _impl_type::_impl_type;
};

/** implement raw_fixed_soa for multiple arrays */
template< class... SoaTypes, class I, I Alignment, class Alloc, class GrowthPolicy >
struct raw_soa< soa<SoaTypes...>, I, Alignment, Alloc, GrowthPolicy >
    :
        public raw_soa_impl< soa<SoaTypes...>, I, Alignment, Alloc, GrowthPolicy, index_sequence_for<SoaTypes...>() >
{
    using _impl_type = raw_soa_impl< soa<SoaTypes...>, I, Alignment, Alloc, GrowthPolicy, index_sequence_for<SoaTypes...>() >;
    using _impl_type::_impl_type;
};


//-----------------------------------------------------------------------------
/** assume the curr size is zero */
template< class T, class I, I Alignment, class Alloc, class GrowthPolicy >
void raw< T, I, Alignment, Alloc, GrowthPolicy >::_raw_reserve(I cap)
{
    _raw_reserve(0, cap);
}

/** assume the curr size is zero */
template< class... SoaTypes, class I, I Alignment, class Alloc, class GrowthPolicy, size_t... Indices >
void raw_soa_impl< soa<SoaTypes...>, I, Alignment, Alloc, GrowthPolicy, index_sequence<Indices...>() >::
_raw_reserve(I cap)
{
    _raw_reserve(0, cap);
}

//-----------------------------------------------------------------------------
template< class T, class I, I Alignment, class Alloc, class GrowthPolicy >
void raw< T, I, Alignment, Alloc, GrowthPolicy >::_raw_reserve(I currsz, I cap)
{
    C4_ASSERT(currsz <= cap);
    T *tmp = nullptr;
    if(cap != m_cap_n_alloc.m_value && cap != 0)
    {
        tmp = m_cap_n_alloc.alloc().allocate(cap, Alignment);
    }
    if(this->m_ptr)
    {
        if(tmp)
        {
            c4::move_construct_n(tmp, this->m_ptr, currsz);
        }
        m_cap_n_alloc.alloc().deallocate(this->m_ptr, m_cap_n_alloc.m_value, Alignment);
    }
    this->m_ptr = tmp;
    m_cap_n_alloc.m_value = cap;
}

template< class... SoaTypes, class I, I Alignment, class Alloc, class GrowthPolicy, size_t... Indices >
template< I n >
void raw_soa_impl< soa<SoaTypes...>, I, Alignment, Alloc, GrowthPolicy, index_sequence<Indices...>() >::
_do_raw_reserve(I currsz, I cap)
{
    nth_type<n> *  tmp = nullptr;
    nth_type<n> *& ptr = std::get<n>(m_soa).m_ptr;
    auto al = nth_allocator<n>();
    if(cap != m_cap_n_alloc.m_value && cap != 0)
    {
        tmp = al.allocate(cap, nth_alignment<n>::value);
    }
    if(ptr)
    {
        if(tmp)
        {
            c4::move_construct_n(tmp, ptr, currsz);
        }
        al.deallocate(ptr, m_cap_n_alloc.m_value, nth_alignment<n>::value);
    }
    ptr = tmp;
}
template< class... SoaTypes, class I, I Alignment, class Alloc, class GrowthPolicy, size_t... Indices >
void raw_soa_impl< soa<SoaTypes...>, I, Alignment, Alloc, GrowthPolicy, index_sequence<Indices...>() >::
_raw_reserve(I currsz, I cap)
{
    C4_ASSERT(currsz <= cap);
    #define _c4mcr(arr, i) _do_raw_reserve<i>(currsz, cap)
    _C4_FOREACH_ARR(m_soa, m_ptr, _c4mcr)
    #undef _c4mcr
    m_cap_n_alloc.m_value = cap;
}

//-----------------------------------------------------------------------------
template< class T, class I, I Alignment, class Alloc, class GrowthPolicy >
void raw< T, I, Alignment, Alloc, GrowthPolicy >::_raw_reserve_allocate(I cap, tmp_type *tmp)
{
    C4_ASSERT(currsz <= cap);
    T *t = nullptr;
    if(cap != m_cap_n_alloc.m_value && cap != 0)
    {
        t = m_cap_n_alloc.alloc().allocate(cap, Alignment);
    }
    tmp->m_ptr = t;
    tmp->m_cap_n_alloc.m_value = cap;
}

template< class... SoaTypes, class I, I Alignment, class Alloc, class GrowthPolicy, size_t... Indices >
template< I n >
void raw_soa_impl< soa<SoaTypes...>, I, Alignment, Alloc, GrowthPolicy, index_sequence<Indices...>() >::
_do_raw_reserve_allocate(I cap, tmp_type *tmp)
{
    nth_type<n> *t = nullptr;
    if(cap != m_cap_n_alloc.m_value && cap != 0)
    {
        t = nth_allocator<n>().allocate(cap, nth_alignment<n>::value);
    }
    std::get<n>(tmp->m_soa).m_ptr = t;
}
template< class... SoaTypes, class I, I Alignment, class Alloc, class GrowthPolicy, size_t... Indices >
void raw_soa_impl< soa<SoaTypes...>, I, Alignment, Alloc, GrowthPolicy, index_sequence<Indices...>() >::
_raw_reserve_allocate(I cap, tmp_type *tmp)
{
    C4_ASSERT(currsz <= cap);
    #define _c4mcr(arr, i) _do_raw_reserve_allocate<i>(cap, tmp)
    _C4_FOREACH_ARR(m_soa, m_ptr, _c4mcr)
    #undef _c4mcr
    tmp->m_cap_n_alloc.m_value = cap;
}

//-----------------------------------------------------------------------------
template< class T, class I, I Alignment, class Alloc, class GrowthPolicy >
void raw< T, I, Alignment, Alloc, GrowthPolicy >::_raw_reserve_replace(I /*tmpsz*/, tmp_type *tmp)
{
    if(this->m_ptr)
    {
        m_cap_n_alloc.alloc().deallocate(this->m_ptr, m_cap_n_alloc.m_value, Alignment);
    }
    m_cap_n_alloc.m_value = tmp->m_cap_n_alloc.m_value;
    this->m_ptr = tmp->m_ptr;
    tmp->m_ptr = nullptr;
    tmp->m_cap_n_alloc.m_value = 0;
}

template< class... SoaTypes, class I, I Alignment, class Alloc, class GrowthPolicy, size_t... Indices >
template< I n >
void raw_soa_impl< soa<SoaTypes...>, I, Alignment, Alloc, GrowthPolicy, index_sequence<Indices...>() >::
_do_raw_reserve_replace(I /*tmpsz*/, tmp_type *tmp)
{
    nth_type<n> *& tmpptr = std::get<n>(tmp->m_soa).m_ptr;
    nth_type<n> *& ptr = std::get<n>(m_soa).m_ptr;
    if(ptr)
    {
        nth_allocator<n>().deallocate(ptr, m_cap_n_alloc.m_value, nth_alignment<n>::value);
    }
    ptr = tmpptr;
    tmpptr = nullptr;
}
template< class... SoaTypes, class I, I Alignment, class Alloc, class GrowthPolicy, size_t... Indices >
void raw_soa_impl< soa<SoaTypes...>, I, Alignment, Alloc, GrowthPolicy, index_sequence<Indices...>() >::
_raw_reserve_replace(I /*tmpsz*/, tmp_type *tmp)
{
    #define _c4mcr(arr, i) _do_raw_reserve_replace<i>(tmpsz, tmp)
    _C4_FOREACH_ARR(m_soa, m_ptr, _c4mcr)
    #undef _c4mcr
    m_cap_n_alloc.m_value = tmp->m_cap_n_alloc.m_value;
    tmp->m_cap_n_alloc.m_value = 0;
}

//-----------------------------------------------------------------------------
/** Resize the buffer at pos, so that prevsz increases to nextsz;
 *  when growing, ___adds to the right___ of pos; when
 *  shrinking, ___removes to the left___ of pos. If growing, the capacity
 *  will increase to the value obtained with the growth policy; if shrinking,
 *  the capacity will stay the same. Use _raw_reserve() to diminish the
 *  capacity.
 *
 *  @param pos the position from which room is to be created (to the right)
 *         or destroyed (to the left)
 *  @param prevsz the previous size
 *  @param nextsz the next size
 *  @see _raw_make_room
 *  @see _raw_destroy_room
 */
template< class T, class I, I Alignment, class Alloc, class GrowthPolicy >
void raw< T, I, Alignment, Alloc, GrowthPolicy >::_raw_resize(I pos, I prevsz, I nextsz)
{
    C4_ASSERT(nextsz >= 0 && nextsz < m_cap_n_alloc.m_value);
    C4_ASSERT(prevsz >= 0 && prevsz < m_cap_n_alloc.m_value);
    C4_ASSERT(pos    >= 0 && pos    < m_cap_n_alloc.m_value);
    if(nextsz > prevsz)
    {
        _raw_make_room(pos, prevsz, nextsz-prevsz);
    }
    else if(nextsz < prevsz)
    {
        _raw_destroy_room(pos, prevsz, prevsz-nextsz);
    }
}

/** Resize the buffer at pos, so that prevsz increases to nextsz;
 *  when growing, ___adds to the right___ of pos; when
 *  shrinking, ___removes to the left___ of pos. If growing, the capacity
 *  will increase to the value obtained with the growth policy; if shrinking,
 *  the capacity will stay the same. Use _raw_reserve() to diminish the
 *  capacity.
 *
 *  @param pos the position from which room is to be created (to the right)
 *         or destroyed (to the left)
 *  @param prevsz the previous size
 *  @param nextsz the next size
 *  @see _raw_make_room
 *  @see _raw_destroy_room
 */
template< class... SoaTypes, class I, I Alignment, class Alloc, class GrowthPolicy, size_t... Indices >
void raw_soa_impl< soa<SoaTypes...>, I, Alignment, Alloc, GrowthPolicy, index_sequence<Indices...>() >::
_raw_resize(I pos, I prevsz, I nextsz)
{
    C4_ASSERT(nextsz >= 0 && nextsz < m_cap_n_alloc.m_value);
    C4_ASSERT(prevsz >= 0 && prevsz < m_cap_n_alloc.m_value);
    C4_ASSERT(pos    >= 0 && pos    < m_cap_n_alloc.m_value);
    if(nextsz > prevsz)
    {
        _raw_make_room(pos, prevsz, nextsz-prevsz);
    }
    else if(nextsz < prevsz)
    {
        _raw_destroy_room(pos, prevsz, prevsz-nextsz);
    }
}

//-----------------------------------------------------------------------------
/** grow to the right of pos
 @code
 pos: 0 1 2 3 4 5 6 7 8
 val: A B C D E F . . .

 _raw_make_room(3, 6, 3)

 pos: 0 1 2 3 4 5 6 7 8
 val: A B C . . . D E F
 @endcode
 */
template< class T, class I, I Alignment, class Alloc, class GrowthPolicy >
void raw< T, I, Alignment, Alloc, GrowthPolicy >::_raw_make_room(I pos, I prevsz, I more)
{
    C4_ASSERT(prevsz >= 0 && prevsz < m_cap_n_alloc.m_value);
    C4_ASSERT(more   >= 0 && more   < m_cap_n_alloc.m_value);
    C4_ASSERT(pos    >= 0 && pos    < m_cap_n_alloc.m_value);
    C4_ASSERT(prevsz+more >= 0 && prevsz+more < m_cap_n_alloc.m_value);
    C4_ASSERT(pos <= prevsz);
    I nextsz = prevsz + more;
    if(nextsz <= m_cap_n_alloc.m_value)
    {
        c4::make_room(this->m_ptr + pos, prevsz - pos, more);
    }
    else
    {
        I real_next = next_capacity(nextsz);
        T* tmp = m_cap_n_alloc.alloc().allocate(real_next, Alignment);
        if(this->m_ptr)
        {
            c4::make_room(tmp, this->m_ptr, prevsz, more, pos);
            m_cap_n_alloc.alloc().deallocate(this->m_ptr, m_cap_n_alloc.m_value, Alignment);
        }
        else
        {
            C4_ASSERT(prevsz == 0);
        }
        this->m_ptr = tmp;
        m_cap_n_alloc.m_value = real_next;
    }
}

/** grow to the right of pos
 @code
 pos: 0 1 2 3 4 5 6 7 8
 val: A B C D E F . . .

 _raw_make_room(3, 6, 3)

 pos: 0 1 2 3 4 5 6 7 8
 val: A B C . . . D E F
 @endcode
 */
template< class... SoaTypes, class I, I Alignment, class Alloc, class GrowthPolicy, size_t... Indices >
template< I n >
void raw_soa_impl< soa<SoaTypes...>, I, Alignment, Alloc, GrowthPolicy, index_sequence<Indices...>() >::
_do_raw_make_room(I pos, I prevsz, I more)
{
    nth_type<n> *& ptr = std::get<n>(m_soa).m_ptr;
    I nextsz = prevsz + more;
    if(nextsz <= m_cap_n_alloc.m_value)
    {
        c4::make_room(ptr + pos, prevsz - pos, more);
    }
    else
    {
        I real_next = next_capacity(nextsz);
        nth_type<n>* tmp = nth_allocator<n>().allocate(real_next, nth_alignment<n>::value);
        if(ptr)
        {
            c4::make_room(tmp, ptr, prevsz, more, pos);
            nth_allocator<n>().deallocate(ptr, m_cap_n_alloc.m_value, nth_alignment<n>::value);
        }
        else
        {
            C4_ASSERT(prevsz == 0);
        }
        ptr = tmp;
    }
}
template< class... SoaTypes, class I, I Alignment, class Alloc, class GrowthPolicy, size_t... Indices >
void raw_soa_impl< soa<SoaTypes...>, I, Alignment, Alloc, GrowthPolicy, index_sequence<Indices...>() >::
_raw_make_room(I pos, I prevsz, I more)
{
    C4_ASSERT(prevsz >= 0 && prevsz < m_cap_n_alloc.m_value);
    C4_ASSERT(more   >= 0 && more   < m_cap_n_alloc.m_value);
    C4_ASSERT(pos    >= 0 && pos    < m_cap_n_alloc.m_value);
    C4_ASSERT(prevsz+more >= 0 && prevsz+more < m_cap_n_alloc.m_value);
    C4_ASSERT(pos <= prevsz);
    #define _c4mcr(arr, i) _do_raw_make_room<i>(pos, prevsz, more)
    _C4_FOREACH_ARR(m_soa, m_ptr, _c4mcr)
    #undef _c4mcr
    m_cap_n_alloc.m_value = next_capacity(prevsz + more);
}

//-----------------------------------------------------------------------------
/** shrink to the left of pos
 @code
 pos: 0 1 2 3 4 5 6 7 8
 val: A B C D E F G H I

 _raw_destroy_room(6, 9, 3)

 pos: 0 1 2 3 4 5 6 7 8
 val: A B C G H I . . .
 @endcode
 */
template< class T, class I, I Alignment, class Alloc, class GrowthPolicy >
void raw< T, I, Alignment, Alloc, GrowthPolicy >::_raw_destroy_room(I pos, I prevsz, I less)
{
    C4_ASSERT(prevsz >= 0 && prevsz < m_cap_n_alloc.m_value);
    C4_ASSERT(pos    >= 0 && pos    < m_cap_n_alloc.m_value);
    C4_ASSERT(pos    <= prevsz);
    C4_ASSERT(pos    >= less);
    I delta = pos - less;
    c4::destroy_room(this->m_ptr + delta, prevsz - delta, less);
}
/** shrink to the left of pos
 @code
 pos: 0 1 2 3 4 5 6 7 8
 val: A B C D E F G H I

 _raw_destroy_room(6, 9, 3)

 pos: 0 1 2 3 4 5 6 7 8
 val: A B C G H I . . .
 @endcode
 */
template< class... SoaTypes, class I, I Alignment, class Alloc, class GrowthPolicy, size_t... Indices >
void raw_soa_impl< soa<SoaTypes...>, I, Alignment, Alloc, GrowthPolicy, index_sequence<Indices...>() >::
_raw_destroy_room(I pos, I prevsz, I less)
{
    C4_ASSERT(prevsz >= 0 && prevsz < m_cap_n_alloc.m_value);
    C4_ASSERT(pos    >= 0 && pos    < m_cap_n_alloc.m_value);
    C4_ASSERT(pos    <= prevsz);
    C4_ASSERT(pos    >= less);
    I delta = pos - less;
    #define _c4mcr(arr, i) c4::destroy_room(arr + delta, prevsz - delta, less)
    _C4_FOREACH_ARR(m_soa, m_ptr, _c4mcr)
    #undef _c4mcr
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/** raw contiguous storage with in-place room for a small number of objects
 * @tparam N the number of objects
 * @ingroup raw_storage_classes */
template< class T, class I, size_t N_, I Alignment, class Alloc, class GrowthPolicy >
struct raw_small : public mem_small< T, N_, Alignment >
{

    C4_STATIC_ASSERT(N_ <= (size_t)std::numeric_limits< I >::max());
    //C4_STATIC_ASSERT(sizeof(T) == alignof(T));  // not sure if this is needed

    // the data members are brought by the base class
    using mem_type = mem_small< T, N_, Alignment >;

    valnalloc<I,Alloc> m_cap_n_alloc;

public:

    _c4_DEFINE_ARRAY_TYPES(T, I);
    using storage_traits = raw_storage_traits< raw_small, small_t >;

    using allocator_type = Alloc;
    using allocator_traits = std::allocator_traits< Alloc >;
    template< class U >
    using rebind_alloc = typename allocator_traits::template rebind_alloc< U >;
    using growth_policy = GrowthPolicy;

    template< class U >
    using rebind_type = raw_small<U, I, N_, alignof(U), rebind_alloc<U>, GrowthPolicy>;

    using tmp_type = tmp_storage< raw_small >;

    enum : I { array_size = (I)N_, N = (I)N_, num_arrays = 1, };

public:

    raw_small() : raw_small(0) {}
    raw_small(Alloc const& a) : raw_small(0, a) {}

    raw_small(I cap) : mem_type(nullptr), m_cap_n_alloc(N) { _raw_reserve(0, cap); }
    raw_small(I cap, Alloc const& a) : mem_type(nullptr), m_cap_n_alloc(N, a) { _raw_reserve(0, cap); }

    ~raw_small()
    {
        _raw_reserve(0, 0);
    }

    // copy and move operations are deleted, and must be implemented by the containers,
    // as this will involve knowledge over what elements are to copied or moved
    C4_NO_COPY_OR_MOVE(raw_small);

    // gcc is triggering a false positive here when compiling in release mode:
    // error: array subscript is below array bounds [-Werror=array-bounds].
    // probably this is due to moving a return branch into the assert.
    // see https://gcc.gnu.org/ml/gcc/2009-09/msg00270.html for a similar example
    C4_ALWAYS_INLINE T      & operator[] (I i)       C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < m_cap_n_alloc.m_value); return C4_LIKELY(i >= 0 && m_cap_n_alloc.m_value <= N) ? this->m_arr[i] : this->m_ptr[i]; }
    C4_ALWAYS_INLINE T const& operator[] (I i) const C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < m_cap_n_alloc.m_value); return C4_LIKELY(i >= 0 && m_cap_n_alloc.m_value <= N) ? this->m_arr[i] : this->m_ptr[i]; }

    C4_ALWAYS_INLINE T      * data()       noexcept { return m_cap_n_alloc.m_value <= N ? this->m_arr : this->m_ptr; }
    C4_ALWAYS_INLINE T const* data() const noexcept { return m_cap_n_alloc.m_value <= N ? this->m_arr : this->m_ptr; }

    C4_ALWAYS_INLINE I capacity() const noexcept { return m_cap_n_alloc.m_value; }
    C4_ALWAYS_INLINE bool empty() const noexcept { return m_cap_n_alloc.m_value == 0; }
    C4_ALWAYS_INLINE bool is_small() const noexcept { return m_cap_n_alloc.m_value <= N; }

    C4_ALWAYS_INLINE constexpr static size_t max_capacity() noexcept { return raw_max_capacity< I >(); }
    C4_ALWAYS_INLINE constexpr size_t next_capacity(size_t desired) const noexcept
    {
        return GrowthPolicy::next_size(sizeof(T), m_cap_n_alloc.m_value, desired);
    }

public:

    iterator       _raw_iterator(I id)       noexcept { return m_cap_n_alloc.m_value <= N ? this->m_arr + id : this->m_ptr + id; }
    const_iterator _raw_iterator(I id) const noexcept { return m_cap_n_alloc.m_value <= N ? this->m_arr + id : this->m_ptr + id; }

public:

    /** assume the curr size is zero */
    void _raw_reserve(I cap) { _raw_reserve(0, cap); }

    void _raw_reserve(I currsz, I cap);

    void _raw_reserve_allocate(I cap, tmp_type *tmp);
    void _raw_reserve_replace(I tmpsz, tmp_type *tmp);

    void _raw_resize(I pos, I prev, I next);
};

//-----------------------------------------------------------------------------

/** raw small storage for structure-of-arrays. this is a work in progress */
template< class... SoaTypes, class I, size_t N_, I Alignment, class Alloc, class GrowthPolicy >
struct raw_small< soa< SoaTypes... >, I, N_, Alignment, Alloc, GrowthPolicy >
{
    template< class U > using arr_type = mem_small< U, N_, (Alignment > alignof(U) ? Alignment : alignof(U)) >;

    std::tuple< arr_type<SoaTypes>... > m_soa;
    valnalloc< I, Alloc >               m_cap_n_alloc;
};


//-----------------------------------------------------------------------------

template< class T, class I, size_t N_, I Alignment, class Alloc, class GrowthPolicy >
void raw_small<T, I, N_, Alignment, Alloc, GrowthPolicy >::
_raw_reserve(I currsz, I cap)
{
    C4_ASSERT(currsz <= cap);
    T *tmp = nullptr;
    if(cap == m_cap_n_alloc.m_value) return;
    if(cap <= N)
    {
        if(m_cap_n_alloc.m_value <= N)
        {
            return; // nothing to do
        }
        else
        {
            tmp = this->m_arr; // move the storage to the array
        }
    }
    else
    {
        // since here we know that cap != m_cap_n_alloc.m_value and that cap
        // is larger than the array, we'll always need a new buffer
        tmp = m_cap_n_alloc.alloc().allocate(cap, Alignment);
    }
    if(m_cap_n_alloc.m_value)
    {
        if(m_cap_n_alloc.m_value <= N)
        {
            C4_ASSERT(tmp != this->m_arr);
            c4::move_construct_n(tmp, this->m_arr, currsz);
        }
        else
        {
            c4::move_construct_n(tmp, this->m_ptr, currsz);
            m_cap_n_alloc.alloc().deallocate(this->m_ptr, m_cap_n_alloc.m_value, Alignment);
        }
    }
    m_cap_n_alloc.m_value = cap;
    this->m_ptr = tmp;
}


//-----------------------------------------------------------------------------
template< class T, class I, size_t N_, I Alignment, class Alloc, class GrowthPolicy >
void raw_small<T, I, N_, Alignment, Alloc, GrowthPolicy >::
_raw_reserve_allocate(I cap, tmp_type *tmp)
{
    tmp->m_cap_n_alloc.m_value = 0;
    tmp->m_ptr = 0;
    if(cap == m_cap_n_alloc.m_value)
    {
        return;
    }
    else if(cap < N)
    {
        if(m_cap_n_alloc.m_value <= N)
        {
            return;
        }
        else
        {
            // move the storage to the array - this requires a temporary buffer
            tmp->m_cap_n_alloc.m_value = cap;
            tmp->m_ptr = m_cap_n_alloc.alloc().allocate(cap, Alignment);
        }
    }
    else
    {
        tmp->m_cap_n_alloc.m_value = cap;
        tmp->m_ptr = m_cap_n_alloc.alloc().allocate(cap, Alignment);
    }
}


//-----------------------------------------------------------------------------

template< class T, class I, size_t N_, I Alignment, class Alloc, class GrowthPolicy >
void raw_small<T, I, N_, Alignment, Alloc, GrowthPolicy >::
_raw_reserve_replace(I tmpsz, tmp_type *tmp)
{
    C4_ASSERT(*tmp);
    if(tmp->m_cap_n_alloc.m_value <= N)
    {
        // moving the storage to the array requires a temporary buffer.
        c4::move_construct_n(this->m_arr, tmp->m_ptr, tmpsz);
        m_cap_n_alloc.alloc().deallocate(tmp->m_ptr, tmp->m_cap_n_alloc.m_value);
    }
    else
    {
        this->m_ptr = tmp->m_ptr;
    }
    m_cap_n_alloc.m_value = tmp->m_cap_n_alloc.m_value;
    tmp->m_ptr = nullptr;
    tmp->m_cap_n_alloc.m_value = 0;
}


//-----------------------------------------------------------------------------
/** Resize the buffer at pos, so that the previous size increases to the
 *  value of next; when growing, ___adds to the right___ of pos; when
 *  shrinking, ___removes to the left___ of pos. If growing, the capacity
 *  will increase to the value obtained with the growth policy; if shrinking,
 *  the capacity will stay the same. Use _raw_reserve() to diminish the
 *  capacity.
 *
 *  @param pos the position from which room is to be created (to the right)
 *         or destroyed (to the left)
 *  @param prev the previous size
 *  @param next the next size */

template< class T, class I, size_t N_, I Alignment, class Alloc, class GrowthPolicy >
void raw_small<T, I, N_, Alignment, Alloc, GrowthPolicy >::
_raw_resize(I pos, I prev, I next)
{
    C4_ASSERT(next >= 0 && next < m_cap_n_alloc.m_value);
    C4_ASSERT(prev >= 0 && prev < m_cap_n_alloc.m_value);
    C4_ASSERT(pos  >= 0 && pos  < m_cap_n_alloc.m_value);
    if(next > prev)
    {
        if(m_cap_n_alloc.m_value <= N && next <= N)
        {
            c4::make_room(this->m_arr + pos, prev - pos, next - prev);
        }
        else
        {
            C4_ASSERT(next > N);
            if(next <= m_cap_n_alloc.m_value)
            {
                c4::make_room(this->m_ptr + pos, prev - pos, next - prev);
            }
            else
            {
                I cap = next_capacity(next);
                T* tmp = m_cap_n_alloc.alloc().allocate(cap, Alignment);
                if(m_cap_n_alloc.m_value <= N)
                {
                    c4::make_room(tmp, this->m_arr, prev, next - prev, pos);
                }
                else if(m_cap_n_alloc.m_value > N)
                {
                    c4::make_room(tmp, this->m_ptr, prev, next - prev, pos);
                    m_cap_n_alloc.alloc().deallocate(this->m_ptr, m_cap_n_alloc.m_value, Alignment);
                }
                this->m_ptr = tmp;
                m_cap_n_alloc.m_value = cap;
            }
        }
    }
    else if(next < prev)
    {
        I delta = prev - next;
        C4_ASSERT(pos > delta);
        c4::destroy_room(data() + pos - delta, prev - pos, delta);
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

// utility defines, undefined below
#define _c4this static_cast< RawPaged* >(this)
#define _c4cthis static_cast< RawPaged const* >(this)

/* a crtp base for paged storage */
template< class T, class I, I Alignment, class RawPaged >
struct _raw_paged_crtp : public mem_paged< T >
{
public:

    using tmp_type = tmp_storage< RawPaged >;

    _raw_paged_crtp(T **p) : mem_paged< T >(p) {}

    enum : I { num_arrays = 1, };

public:

    C4_ALWAYS_INLINE T& operator[] (I i) C4_NOEXCEPT_X
    {
        C4_XASSERT(i < capacity());
        const I pg = i >> _c4cthis->m_page_lsb;
        const I id = i & _c4cthis->m_id_mask;
        C4_XASSERT(pg >= 0 && pg < _c4cthis->m_numpages_n_alloc.m_value);
        C4_XASSERT(id >= 0 && id < _c4cthis->m_id_mask + 1);
        return this->m_pages[pg][id];
    }
    C4_ALWAYS_INLINE T const& operator[] (I i) const C4_NOEXCEPT_X
    {
        C4_XASSERT(i < capacity());
        const I pg = i >> _c4cthis->m_page_lsb;
        const I id = i & _c4cthis->m_id_mask;
        C4_XASSERT(pg >= 0 && pg < _c4cthis->m_numpages_n_alloc.m_value);
        C4_XASSERT(id >= 0 && id < _c4cthis->m_id_mask + 1);
        return this->m_pages[pg][id];
    }

    C4_ALWAYS_INLINE bool empty() const noexcept { return _c4cthis->m_capacity == 0; }

    /** since the page size is a power of two, the max capacity is simply the
     * maximum of the size type */
    C4_ALWAYS_INLINE constexpr static size_t max_capacity() noexcept { return raw_max_capacity< I >(); }
    C4_ALWAYS_INLINE C4_CONSTEXPR14 size_t next_capacity(size_t desired) const C4_NOEXCEPT_A
    {
        const size_t ps = _c4cthis->page_size();
        const size_t np = (desired + ps - 1) / ps;
        const size_t cap = np * ps;
        C4_ASSERT(cap >= desired);
        return cap;
    }

    C4_ALWAYS_INLINE C4_CONSTEXPR14 I num_pages() const noexcept { return _c4cthis->m_numpages_n_alloc.m_value; }
    C4_ALWAYS_INLINE C4_CONSTEXPR14 I capacity() const noexcept { return _c4cthis->m_numpages_n_alloc.m_value * _c4cthis->page_size(); }

public:

    void _raw_reserve(I cap)
    {
        _raw_reserve(0, cap);
    }
    void _raw_reserve(I currsz, I cap);

    void _raw_reserve_allocate(I cap, tmp_type *tmp);
    void _raw_reserve_replace(I /*tmpsz*/, tmp_type *tmp) { C4_XASSERT(!(*tmp)); /* nothing to do */ }

public:

    template< class U >
    class iterator_impl : public std::iterator< std::random_access_iterator_tag, U >
    {
        RawPaged *this_;
        I i;

    public:

        using value_type = U;
        using size_type = I;

        iterator_impl(RawPaged *rp, I i_) : this_(rp), i(i_) {}

        U& operator*  () const noexcept { return  (*this_)[i]; }
        U* operator-> () const noexcept { return &(*this_)[i]; }

        iterator_impl& operator++ (   ) noexcept {                           ++i; return *this; }
        iterator_impl  operator++ (int) noexcept { iterator_impl it = *this; ++i; return    it; }

        iterator_impl& operator-- (   ) noexcept {                           --i; return *this; }
        iterator_impl  operator-- (int) noexcept { iterator_impl it = *this; --i; return    it; }

        bool operator== (iterator_impl const& that) const noexcept { return i == that.i && this_ == that.this_; }
        bool operator!= (iterator_impl const& that) const noexcept { return i != that.i || this_ != that.this_; }

    };

    using iterator = iterator_impl< T* >;
    using const_iterator = iterator_impl< T const* >;

    iterator       _raw_iterator(I id)       noexcept { return       iterator(this, id); }
    const_iterator _raw_iterator(I id) const noexcept { return const_iterator(this, id); }

public:

    template< class ...Args >
    void _raw_construct_n(I first, I n, Args const&... args)
    {
        _process_pages(&_raw_paged_crtp::_raw_construct_n_handler< Args... >, _c4this->m_pages, first, n, args...);
    }

    template< class ...Args >
    void _raw_construct_n(T **pages, I first, I n, Args const&... args)
    {
        _process_pages(&_raw_paged_crtp::_raw_construct_n_handler< Args... >, pages, first, n, args...);
    }

    template< class ...Args >
    C4_ALWAYS_INLINE static void _raw_construct_n_handler(T *ptr, I n, Args const&... args)
    {
        c4::construct_n(ptr, n, args...);
    }

public:

    void _raw_destroy_n(I first, I n)
    {
        _process_pages(&_raw_paged_crtp::_raw_destroy_n_handler, _c4this->m_pages, first, n);
    }

    void _raw_destroy_n(T **pages, I first, I n)
    {
        _process_pages(&_raw_paged_crtp::_raw_destroy_n_handler, pages, first, n);
    }

    C4_ALWAYS_INLINE static void _raw_destroy_n_handler(T *ptr, I n)
    {
        c4::destroy_n(ptr, n);
    }

public:

    void _raw_move_construct_n(RawPaged & src, I first, I n)
    {
        _process_pages(&_raw_paged_crtp::_raw_move_construct_n_handler, _c4this->m_pages, first, src.m_pages, first, n);
    }

    void _raw_move_construct_n(RawPaged & src, I first_this, I first_that, I n)
    {
        _process_pages(&_raw_paged_crtp::_raw_move_construct_n_handler, _c4this->m_pages, first_this, src.m_pages, first_that, n);
    }

    void _raw_move_construct_n(T ** pages, I first_this, T ** that, I first_that, I n)
    {
        _process_pages(&_raw_paged_crtp::_raw_move_construct_n_handler, pages, first_this, that, first_that, n);
    }

    C4_ALWAYS_INLINE static void _raw_move_construct_n_handler(T *dst, T *src, I n)
    {
        c4::move_construct_n(dst, src, n);
    }

public:

    void _raw_copy_construct_n(RawPaged const& /*src*/, I /*first*/, I /*n*/) { C4_NOT_IMPLEMENTED(); }

public:

    void _raw_move_assign_n(RawPaged      & /*src*/, I /*first*/, I /*n*/) { C4_NOT_IMPLEMENTED(); }

public:

    void _raw_copy_assign_n(RawPaged const& /*src*/, I /*first*/, I /*n*/) { C4_NOT_IMPLEMENTED(); }

public:

    template< class Function, class ...Args >
    void _process_pages(Function handler, T **pages, I first, I n, Args... args);

    template< class Function, class ...Args >
    void _process_pages(Function handler, T **pages, I first_this, T **other, I first_that, I n, Args... args);

};

template< class T, class I, I Alignment, class RawPaged >
template< class Function, class ...Args >
void _raw_paged_crtp< T, I, Alignment, RawPaged >::
_process_pages(Function handler, T **pages, I first_id, I n, Args... args)
{
    I pg = _c4cthis->_raw_pg(first_id);
    const I last_pg = _c4cthis->_raw_pg(first_id + n);
    first_id = _c4cthis->_raw_id(first_id);
    I count = 0; // num elms handled so far
    while(count < n)
    {
        C4_ASSERT(pg <= last_pg); C4_UNUSED(last_pg);
        const I remaining = n - count;
        I pn = _c4this->page_size() - first_id;
        pn = pn < remaining ? pn : remaining;  // num elms to handle in this page
        handler(pages[pg] + first_id, pn, args...);
        ++pg;
        first_id = 0;
        count += pn;
    }
}

/** the page size of other is the same! */
template< class T, class I, I Alignment, class RawPaged >
template< class Function, class ...Args >
void _raw_paged_crtp< T, I, Alignment, RawPaged >::
_process_pages(Function handler, T **pages, I first_id_this, T **other, I first_id_that, I n, Args... args)
{
    I pg = _c4this->_raw_pg(first_id_this);
    const I last_pg = _c4cthis->_raw_pg(first_id_this + n);
    first_id_this = _c4this->_raw_id(first_id_this);
    first_id_that = _c4this->_raw_id(first_id_that);
    I count = 0; // num elms handled so far
    while(count < n)
    {
        C4_ASSERT(pg <= last_pg); C4_UNUSED(last_pg);
        const I remaining = n - count;
        I pnthis = _c4this->page_size() - first_id_this;
        I pnthat = _c4this->page_size() - first_id_that;
        pnthis = pnthis < remaining ? pnthis : remaining;  // num elms to handle in this page
        pnthat = pnthat < remaining ? pnthat : remaining;  // num elms to handle in this page
        if(pnthis == pnthat)
        {
            handler(pages[pg] + first_id_this, other[pg] + first_id_that, pnthis, args...);
        }
        else
        {
            // it's complicated, needs to be implemented. merely a placeholder.
            C4_NOT_IMPLEMENTED();
        }
        ++pg;
        first_id_this = 0;
        first_id_that = 0;
        count += pnthis;
    }
}

template< class T, class I, I Alignment, class RawPaged >
void _raw_paged_crtp< T, I, Alignment, RawPaged >::_raw_reserve_allocate(I cap, tmp_type *tmp)
{
    C4_XASSERT(!(*tmp));
    const I ps = _c4cthis->page_size();
    const I np = szconv<I>(size_t(cap + ps - 1) / size_t(ps));

    if(np == _c4this->m_numpages_n_alloc.m_value) return;

    auto at = _c4this->m_numpages_n_alloc.alloc().template rebound< T* >();
    T** tmp_pages = at.allocate(np);
    if(np > _c4this->m_numpages_n_alloc.m_value) // more pages (grow)
    {
        for(I i = 0; i < _c4this->m_numpages_n_alloc.m_value; ++i)
        {
            tmp_pages[i] = _c4this->m_pages[i];
        }
        for(I i = _c4this->m_numpages_n_alloc.m_value; i < np; ++i)
        {
            tmp_pages[i] = _c4this->m_numpages_n_alloc.alloc().allocate(ps, Alignment);
        }
    }
    else // less pages (shrink)
    {
        for(I i = 0; i < np; ++i)
        {
            tmp_pages[i] = _c4this->m_pages[i];
        }
        for(I i = np; i < _c4this->m_numpages_n_alloc.m_value; ++i)
        {
            _c4this->m_numpages_n_alloc.alloc().deallocate(_c4this->m_pages[i], ps, Alignment);
        }
    }
    at.deallocate(_c4this->m_pages, _c4this->m_numpages_n_alloc.m_value);
    _c4this->m_pages = tmp_pages;
    _c4this->m_numpages_n_alloc.m_value = np;
}

template< class T, class I, I Alignment, class RawPaged >
void _raw_paged_crtp< T, I, Alignment, RawPaged >::_raw_reserve(I currsz, I cap)
{
    auto at = _c4this->m_numpages_n_alloc.alloc().template rebound< T* >();
    const I ps = _c4this->page_size();
    if(cap == 0)
    {
        _c4this->_raw_destroy_n(0, currsz);
        for(I i = 0; i < _c4this->m_numpages_n_alloc.m_value; ++i)
        {
            _c4this->m_numpages_n_alloc.alloc().deallocate(_c4this->m_pages[i], ps, Alignment);
        }
        at.deallocate(_c4this->m_pages, _c4this->m_numpages_n_alloc.m_value);
        _c4this->m_pages = nullptr;
        _c4this->m_numpages_n_alloc.m_value = 0;
        return;
    }

    // number of pages: round up to the next multiple of ps
    const I np = (cap + ps - 1) / ps;
    C4_ASSERT(np * ps >= cap);

    if(np == _c4this->m_numpages_n_alloc.m_value)
    {
        return;
    }

    T ** tmp = at.allocate(np);
    for(I i = 0; i < np; ++i)
    {
        tmp[i] = _c4this->m_numpages_n_alloc.alloc().allocate(ps, Alignment);
    }
    if(_c4this->m_pages)
    {
        if(currsz > 0)
        {
            I num_to_move = currsz;
            if(cap < currsz)
            {
                _c4this->_raw_destroy_n(cap, currsz);
                num_to_move = cap;
            }
            _raw_move_construct_n(tmp, 0, _c4this->m_pages, 0, num_to_move);
        }
        for(I i = 0; i < _c4this->m_numpages_n_alloc.m_value; ++i)
        {
            _c4this->m_numpages_n_alloc.alloc().deallocate(_c4this->m_pages[i], ps, Alignment);
        }
        at.deallocate(_c4this->m_pages, _c4this->m_numpages_n_alloc.m_value);
    }
    _c4this->m_numpages_n_alloc.m_value = np;
    _c4this->m_pages = tmp;
}


#undef _c4this
#undef _c4cthis

//-----------------------------------------------------------------------------
/** raw paged storage, allocatable. This is NOT a contiguous storage structure.
  * However, it does behave as such, offering a O(1) [] operator with contiguous
  * range indices. This is useful for minimizing allocations and data copies in
  * dynamic array-based containers like flat_list or split_list.
  *
  * @tparam T the stored type
  * @tparam PageSize The page size, in number of elements (ie, not in bytes). Must be a power of two.
  * @tparam I the size type. Must be integral
  * @tparam Alignment the alignment at which the stored type should be aligned
  * @tparam Alloc an allocator
  *
  * @ingroup raw_storage_classes */
template< class T, class I, size_t PageSize, I Alignment, class Alloc >
struct raw_paged : public _raw_paged_crtp< T, I, Alignment, raw_paged<T, I, PageSize, Alignment, Alloc > >
{
    using crtp_base = _raw_paged_crtp< T, I, Alignment, raw_paged<T, I, PageSize, Alignment, Alloc > >;

    static_assert(std::is_integral< I >::value, "I must be an integral type");
    static_assert(PageSize > 1, "PageSize must be > 1");
    static_assert((PageSize & (PageSize - 1)) == 0, "PageSize must be a power of two");
    static_assert(PageSize <= std::numeric_limits< I >::max(), "PageSize overflow");

    enum : I {
        /** id mask: all the bits up to PageSize. Use to extract the position
         * of an index within a page. Despite being an enum value, the name
         * starts with the m_ prefix to allow for compatibility with code for
         * raw_paged_rt, the dynamically-sized version of this class. */
        m_id_mask = I(PageSize) - I(1),
        /** page lsb: the number of bits complementary to PageSize. Use to
         * extract the page of an index. Despite being an enum value, the
         * name starts with the m_ prefix to allow for compatibility with
         * code for raw_paged_rt, the dynamically-sized version of this
         * class. */
        m_page_lsb = lsb11< I, PageSize >::value
    };

    constexpr static I _raw_pg(I const i) { return i >> m_page_lsb; } ///< get the page index
    constexpr static I _raw_id(I const i) { return i &  m_id_mask; }  ///< get the index within the page

    // the pages array is brought by the base class
    valnalloc<I, Alloc> m_numpages_n_alloc;  ///< number of current pages in the array + allocator (for empty base class optimization)

public:

    _c4_DEFINE_ARRAY_TYPES_WITHOUT_ITERATOR(T, I);
    using storage_traits = raw_storage_traits< raw_paged, paged_t >;
    using allocator_type = Alloc;
    using allocator_traits = std::allocator_traits< Alloc >;
    template< class U >
    using rebind_alloc = typename allocator_traits::template rebind_alloc< U >;

    template< class U >
    using rebind_type = raw_paged<U, I, PageSize, alignof(U), rebind_alloc<U>>;

    enum : I { fixed_page_size = PageSize };

public:

    raw_paged() : raw_paged(0) {}
    raw_paged(Alloc const& a) : raw_paged(0, a) {}

    raw_paged(I cap) : crtp_base(nullptr), m_numpages_n_alloc(0)
    {
        crtp_base::_raw_reserve(0, cap);
    }
    raw_paged(I cap, Alloc const& a) : crtp_base(nullptr), m_numpages_n_alloc(0, a)
    {
        crtp_base::_raw_reserve(0, cap);
    }

    ~raw_paged()
    {
        crtp_base::_raw_reserve(0, 0);
    }

    // copy and move operations are deleted, and must be implemented by the containers,
    // as this will involve knowledge over what elements are to copied or moved
    C4_NO_COPY_OR_MOVE(raw_paged);

    C4_ALWAYS_INLINE static constexpr I page_size() noexcept { return PageSize; }

    template< class RawPagedContainer >
    friend void test_raw_page_addressing(RawPagedContainer const& rp);

};

//-----------------------------------------------------------------------------

/** specialization of raw_paged for dynamic (set at run time) page size.
 * @ingroup raw_storage_classes */
template< class T, class I, I Alignment, class Alloc >
struct raw_paged< T, I, 0, Alignment, Alloc > : public _raw_paged_crtp< T, I, Alignment, raw_paged<T, I, 0, Alignment, Alloc > >
{
    using crtp_base = _raw_paged_crtp< T, I, Alignment, raw_paged<T, I, 0, Alignment, Alloc > >;
    static_assert(std::is_integral< I >::value, "");

    C4_CONSTEXPR14 I _raw_pg(const I i) const { return i >> m_page_lsb; }
    C4_CONSTEXPR14 I _raw_id(const I i) const { return i &  m_id_mask; }

    valnalloc<I, Alloc> m_numpages_n_alloc;  ///< number of current pages in the array + allocator (for empty base class optimization)
    I      m_id_mask;     ///< page size - 1: cannot be changed after construction.
    I      m_page_lsb;    ///< least significant bit of the page size: cannot be changed after construction.

public:

    _c4_DEFINE_ARRAY_TYPES(T, I);
    using storage_traits = raw_storage_traits< raw_paged, paged_t >;
    using allocator_type = Alloc;
    using allocator_traits = std::allocator_traits< Alloc >;
    template< class U >
    using rebind_alloc = typename allocator_traits::template rebind_alloc< U >;

    template< class U >
    using rebind_type = raw_paged<U, I, 0, Alignment, rebind_alloc<U>>;

    enum : I { fixed_page_size = 0 };

public:

    raw_paged() : raw_paged(szconv<I>(0), default_page_size<T,I>::value) {}
    raw_paged(Alloc const& a) : raw_paged(szconv<I>(0), default_page_size<T,I>::value, a) {}

    raw_paged(I cap) : raw_paged(cap, default_page_size<T,I>::value) {}
    raw_paged(I cap, Alloc const& a) : raw_paged(cap, default_page_size<T,I>::value, a) {}

    raw_paged(I cap, I page_sz) : crtp_base(nullptr), m_numpages_n_alloc(0), m_id_mask(page_sz - 1), m_page_lsb(lsb(page_sz))
    {
        C4_ASSERT_MSG(page_sz > 1, "page_sz=%zu", (size_t)page_sz);
        C4_ASSERT_MSG((page_sz & (page_sz - 1)) == 0, "page size must be a power of two. page_sz=%zu", (size_t)page_sz);
        crtp_base::_raw_reserve(0, cap);
    }
    raw_paged(I cap, I page_sz, Alloc const& a) : crtp_base(nullptr), m_numpages_n_alloc(0, a), m_id_mask(page_sz - 1), m_page_lsb(lsb(page_sz))
    {
        C4_ASSERT_MSG(page_sz > 1, "page_sz=%zu", (size_t)page_sz);
        C4_ASSERT_MSG((page_sz & (page_sz - 1)) == 0, "page size must be a power of two. page_sz=%zu", (size_t)page_sz);
        crtp_base::_raw_reserve(0, cap);
    }

    ~raw_paged()
    {
        crtp_base::_raw_reserve(0, 0);
    }

    // copy and move operations are deleted, and must be implemented by the containers,
    // as this will involve knowledge over what elements are to copied or moved
    C4_NO_COPY_OR_MOVE(raw_paged);

    C4_ALWAYS_INLINE I page_size() const noexcept { return m_id_mask + 1; }

    template< class RawPagedContainer >
    friend void test_raw_page_addressing(RawPagedContainer const& rp);

};


/** raw paged storage for structure-of-arrays. This is a work-in-progress. */
template< class... SoaTypes, class I, size_t PageSize, I Alignment, class Alloc >
struct raw_paged< soa< SoaTypes... >, I, PageSize, Alignment, Alloc >
{
    std::tuple< mem_paged<SoaTypes>... > m_soa;
    valnalloc< I, Alloc >                m_numpages_n_alloc;
};
template< class... SoaTypes, class I, I Alignment, class Alloc >
struct raw_paged< soa< SoaTypes... >, I, 0, Alignment, Alloc >
{
    std::tuple< mem_paged<SoaTypes>... > m_soa;
    valnalloc< I, Alloc >                m_numpages_n_alloc;
    I m_id_mask;     ///< page size - 1: cannot be changed after construction.
    I m_page_lsb;    ///< least significant bit of the page size: cannot be changed after construction.
};

C4_END_NAMESPACE(stg)
C4_END_NAMESPACE(c4)

#endif // _C4_STORAGE_RAW_HPP_
