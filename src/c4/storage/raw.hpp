#ifndef _C4_STORAGE_RAW_HPP_
#define _C4_STORAGE_RAW_HPP_

/** @file raw.hpp Raw storage classes. */

#include "c4/config.hpp"
#include "c4/error.hpp"
#include "c4/ctor_dtor.hpp"

#include <limits>
#include <memory>

C4_BEGIN_NAMESPACE(c4)

// external forward declarations
template< class T > class Allocator;
template< class T, size_t N, size_t Alignment > class SmallAllocator;

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
 *   requires explicit calls to the function _raw_resize(I n).
 * - The elements contained in the raw storage are NOT automatically constructed
 *   or destroyed.
 */

// forward declarations
template< class Storage > struct raw_storage_traits;
template< class Storage, class... > struct raw_storage_util;

template< class T, size_t N, class I = C4_SIZE_TYPE, I Alignment = alignof(T) >
struct raw_fixed;

template< class T, class I = C4_SIZE_TYPE, I Alignment = alignof(T), class Alloc = Allocator<T>, class GrowthPolicy = growth_default >
struct raw;

template< class T, size_t PageSize = 256, class I = C4_SIZE_TYPE, I Alignment = alignof(T), class Alloc = Allocator<T> >
struct raw_paged;

/** raw paged with page size determined at runtime. @ingroup raw_storage_classes */
template< class T, class I = C4_SIZE_TYPE, I Alignment = alignof(T), class Alloc = Allocator<T> >
using raw_paged_rt = raw_paged< T, 0, I, Alignment, Alloc >;

/** raw storage with inplace storage of up to N objects, thus saving an
 * allocation when the size is small. @ingroup raw_storage_classes */
template< class T, size_t N, class I = C4_SIZE_TYPE, I Alignment = alignof(T), class Alloc = Allocator<T>, class GrowthPolicy = growth_default >
using raw_small = raw< T, I, Alignment, SmallAllocator<T, N, Alignment> >;

//-----------------------------------------------------------------------------
/** Type traits for raw storage classes.
 * @ingroup raw_storage_classes */
template< class Storage >
struct raw_storage_traits
{
    using storage_type = Storage;
    using value_type = typename Storage::value_type;
    using size_type = typename Storage::size_type;
    using util_type = raw_storage_util< Storage >;

    enum {
        fixed = false,
        contiguous = true,
        with_allocator = ! fixed,
        contiguous_with_allocator = contiguous && ( ! fixed)
    };
};

/** @ingroup raw_storage_classes */
#define C4_DEFINE_STORAGE_TRAITS(type, is_fixed, is_contiguous, ...) \
template< __VA_ARGS__ >                                              \
struct raw_storage_traits< type >                                    \
{                                                                    \
    using storage_type = type;                                       \
    using value_type = typename T::value_type;                       \
    using size_type = typename T::size_type;                         \
    using util_type = raw_storage_util< type >;                      \
                                                                     \
    enum {                                                           \
        fixed = is_fixed,                                            \
        contiguous = is_contiguous,                                  \
        with_allocator = ! fixed,                                    \
        contiguous_with_allocator = contiguous && ( ! fixed)         \
    };                                                               \
}


//-----------------------------------------------------------------------------
/** Utility class which uses SFINAE to dispatch construction/destruction
 * to the appropriate functions. This class is meant to be used by container
 * implementations to aid in object management.
 * @ingroup raw_storage_classes */
template< class Storage, class... >
struct raw_storage_util;

// utility define, undefined below
#define _c4_rsu_require(what) typename std::enable_if< what, void >::type

/** specialization of raw_storage_util for fixed storage containers.
 * @ingroup raw_storage_classes */
template< class Storage >
struct raw_storage_util< Storage, _c4_rsu_require(raw_storage_traits< Storage >::fixed) >
{
    using storage_type = Storage;
    using traits_type = raw_storage_traits< Storage >;

    using T = typename raw_storage_traits< Storage >::value_type;
    using I = typename raw_storage_traits< Storage >::size_type;

    template< class ...Args >
    C4_ALWAYS_INLINE static void construct(Storage& dest, I first, I n, Args&&... args)
    {
        c4::construct_n(dest.data() + first, n, std::forward< Args >(args)...);
    }

    C4_ALWAYS_INLINE static void destroy(Storage& dest, I first, I n)
    {
        c4::destroy_n(dest.data() + first, n);
    }

    C4_ALWAYS_INLINE static void move_construct(Storage& dest, Storage const& src, I first, I n)
    {
        c4::move_construct_n(dest.data() + first, src.data() + first, n);
    }

    C4_ALWAYS_INLINE static void construct(Storage& dest, Storage const& src, I first, I n)
    {
        c4::copy_construct_n(dest.data() + first, src.data() + first, n);
    }

    C4_ALWAYS_INLINE static void move_assign(Storage& dest, Storage const& src, I first, I n)
    {
        c4::move_assign_n(dest.data() + first, src.data() + first, n);
    }

    C4_ALWAYS_INLINE static void copy_assign(Storage& dest, Storage const& src, I first, I n)
    {
        c4::copy_assign_n(dest.data() + first, src.data() + first, n);
    }
};

/** specialization of raw_storage_util for dynamic storage containers.
 * @ingroup raw_storage_classes */
template< class Storage >
struct raw_storage_util< Storage, _c4_rsu_require(raw_storage_traits< Storage >::contiguous_with_allocator) >
{
    using storage_type = Storage;
    using traits_type = raw_storage_traits< Storage >;
    using alloc_traits = typename Storage::allocator_traits;

    using T = typename raw_storage_traits< Storage >::value_type;
    using I = typename raw_storage_traits< Storage >::size_type;

    template< class ...Args >
    C4_ALWAYS_INLINE static void construct(Storage& dest, I first, I n, Args&&... args)
    {
        /// @todo use alloc_traits here and in the other functions
        dest.m_allocator.construct_n(dest.data() + first, n, std::forward< Args >(args)...);
    }

    C4_ALWAYS_INLINE static void destroy(Storage& dest, I first, I n)
    {
        dest.m_allocator.destroy_n(dest.data() + first, n);
    }

    C4_ALWAYS_INLINE static void move_construct(Storage& dest, Storage const& src, I first, I n)
    {
        c4::move_construct_n(dest.data() + first, src.data() + first, n);
    }

    C4_ALWAYS_INLINE static void construct(Storage& dest, Storage const& src, I first, I n)
    {
        c4::copy_construct_n(dest.data() + first, src.data() + first, n);
    }

    C4_ALWAYS_INLINE static void move_assign(Storage& dest, Storage const& src, I first, I n)
    {
        c4::move_assign_n(dest.data() + first, src.data() + first, n);
    }

    C4_ALWAYS_INLINE static void copy_assign(Storage& dest, Storage const& src, I first, I n)
    {
        c4::copy_assign_n(dest.data() + first, src.data() + first, n);
    }
};


/** specialization of raw_storage_util for non-contiguous storage containers.
 * @ingroup raw_storage_classes */
template< class Storage >
struct raw_storage_util< Storage, _c4_rsu_require( ! raw_storage_traits< Storage >::contiguous) >
{
    using storage_type = Storage;
    using traits_type = raw_storage_traits< Storage >;

    using T = typename raw_storage_traits< Storage >::value_type;
    using I = typename raw_storage_traits< Storage >::size_type;

    template< class ...Args >
    C4_ALWAYS_INLINE static void construct(Storage& dest, I first, I n, Args&&... args)
    {
        dest._raw_construct(first, n, std::forward< Args >(args)...);
    }

    C4_ALWAYS_INLINE static void destroy(Storage& dest, I first, I n)
    {
        dest._raw_destroy(first, n);
    }

    C4_ALWAYS_INLINE static void move_construct(Storage& dest, Storage const& src, I first, I n)
    {
        dest._raw_move_construct(src, first, n);
    }

    C4_ALWAYS_INLINE static void copy_construct(Storage& dest, Storage const& src, I first, I n)
    {
        dest._raw_copy_construct(src, first, n);
    }

    C4_ALWAYS_INLINE static void move_assign(Storage& dest, Storage const& src, I first, I n)
    {
        dest._raw_move_assign(src, first, n);
    }

    C4_ALWAYS_INLINE static void copy_assign(Storage& dest, Storage const& src, I first, I n)
    {
        dest._raw_copy_assign(src, first, n);
    }
};

#undef _c4_rsu_require


//-----------------------------------------------------------------------------

C4_DEFINE_STORAGE_TRAITS(raw_fixed< T C4_COMMA N C4_COMMA I C4_COMMA Alignment >,
                         true, // fixed
                         true, // contiguous
                         class T, size_t N, class I, I Alignment);

/** raw storage with capacity fixed at compile time
 * @ingroup raw_storage_classes */
template< class T, size_t N, class I, I Alignment >
struct raw_fixed
{

    union {
        alignas(Alignment) char _m_buf[N * sizeof(T)];
        alignas(Alignment) T m_ptr[N];
    };

    _c4_DEFINE_ARRAY_TYPES(T, I)

    C4_ALWAYS_INLINE raw_fixed() {}
    C4_ALWAYS_INLINE ~raw_fixed() {}

    C4_ALWAYS_INLINE T      & operator[] (I i)       noexcept { return m_ptr[i]; }
    C4_ALWAYS_INLINE T const& operator[] (I i) const noexcept { return m_ptr[i]; }

    C4_ALWAYS_INLINE T      * data()       noexcept { return m_ptr; }
    C4_ALWAYS_INLINE T const* data() const noexcept { return m_ptr; }

    C4_ALWAYS_INLINE constexpr I  max_capacity()      const noexcept { return N; }
    C4_ALWAYS_INLINE constexpr I      capacity()      const noexcept { return N; }
    C4_ALWAYS_INLINE constexpr I next_capacity(I cap) const noexcept { return N; }

    void _raw_resize(I n) const C4_NOEXCEPT_A { C4_ASSERT(n <= N); }
};

//-----------------------------------------------------------------------------

C4_DEFINE_STORAGE_TRAITS(raw< T C4_COMMA I C4_COMMA Alignment C4_COMMA Alloc C4_COMMA GrowthPolicy >,
                         false, // fixed
                         true,  // contiguous
                         class T, class I, I Alignment, class Alloc, class GrowthPolicy);

/** raw storage with variable size and capacity: allocatable, contiguous
 * @ingroup raw_storage_classes */
template< class T, class I, I Alignment, class Alloc, class GrowthPolicy >
struct raw
{

    T*    m_ptr;
    I     m_capacity;
    Alloc m_allocator;

    _c4_DEFINE_ARRAY_TYPES(T, I)

    using allocator_type = Alloc;
    using allocator_traits = std::allocator_traits< Alloc >;

    raw() : raw(0) {}
    raw(Alloc const& a) : raw(0, a) {}

    raw(I cap) : m_ptr(nullptr), m_capacity(0), m_allocator() { _raw_resize(cap); }
    raw(I cap, Alloc const& a) : m_ptr(nullptr), m_capacity(0), m_allocator(a) { _raw_resize(cap); }

    ~raw()
    {
        _raw_resize(0);
    }

    // copy and move operations are deleted, and must be implemented by the containers,
    // as this will involve knowledge over what elements are to copied or moved
    raw(raw const& that) = delete;
    raw(raw     && that) = delete;
    raw& operator=(raw const& that) = delete;
    raw& operator=(raw     && that) = delete;

    C4_ALWAYS_INLINE           T      & operator[] (I i)       noexcept { return m_ptr[i]; }
    C4_ALWAYS_INLINE constexpr T const& operator[] (I i) const noexcept { return m_ptr[i]; }

    C4_ALWAYS_INLINE T      * data()       noexcept { return m_ptr; }
    C4_ALWAYS_INLINE T const* data() const noexcept { return m_ptr; }

    C4_ALWAYS_INLINE I max_capacity() const noexcept { return std::numeric_limits< I >::max() - 1; }
    C4_ALWAYS_INLINE I capacity() const noexcept { return m_capacity; }
    C4_ALWAYS_INLINE I next_capacity(I desired) const
    {
        return GrowthPolicy::next_size(sizeof(T), m_capacity, desired);
    }

    void _raw_resize(I cap)
    {
        if(m_ptr)
        {
            m_allocator.deallocate(m_ptr, m_capacity, Alignment);
            m_ptr = nullptr;
        }
        if(cap > 0)
        {
            m_ptr = m_allocator.allocate(cap, Alignment);
        }
        m_capacity = cap;
    }

};


//-----------------------------------------------------------------------------

C4_DEFINE_STORAGE_TRAITS(raw_paged< T C4_COMMA PageSize C4_COMMA I C4_COMMA Alignment C4_COMMA Alloc >,
                         false, // fixed
                         false, // contiguous
                         class T, size_t PageSize, class I, I Alignment, class Alloc);

template< class T, class I, I Alignment, class RawPaged >
struct _raw_paged_crtp
{
#define _c4this static_cast< RawPaged* >(this)
#define _c4cthis static_cast< RawPaged const* >(this)

    C4_ALWAYS_INLINE static constexpr I max_capacity() noexcept { return std::numeric_limits< I >::max() - 1; }
    C4_ALWAYS_INLINE C4_CONSTEXPR14 I num_pages() const noexcept { return _c4cthis->m_num_pages; }
    C4_ALWAYS_INLINE C4_CONSTEXPR14 I capacity() const noexcept { return _c4cthis->m_num_pages * _c4cthis->page_size(); }
    C4_ALWAYS_INLINE C4_CONSTEXPR14 I next_capacity(I desired) const C4_NOEXCEPT_A
    {
        I cap = capacity();
        I np = desired / _c4cthis->m_num_pages;
        cap = np * _c4cthis->m_num_pages;
        C4_ASSERT(cap >= desired);
        return cap;
    }

protected:

    void _raw_resize(I cap);
};

template< class T, class I, I Alignment, class RawPaged >
void _raw_paged_crtp< T, I, Alignment, RawPaged >::_raw_resize(I cap)
{
    auto at = _c4this->m_allocator.template rebound< T* >();
    const I ps = _c4this->page_size();
    if(cap == 0)
    {
        for(I i = 0; i < _c4this->m_num_pages; ++i)
        {
            _c4this->m_allocator.deallocate(_c4this->m_pages[i], ps, Alignment);
        }
        at.deallocate(_c4this->m_pages, _c4this->m_num_pages);
        _c4this->m_pages = nullptr;
        return;
    }

    // http://stackoverflow.com/questions/17005364/dividing-two-integers-and-rounding-up-the-result-without-using-floating-pont
    const I np = (cap + ps - 1) / ps;
    C4_ASSERT(np * ps >= cap);
    if(np >= _c4this->m_num_pages)
    {
        _c4this->m_pages = at.reallocate(_c4this->m_pages, _c4this->m_num_pages, np);
        for(I i = _c4this->m_num_pages; i < np; ++i)
        {
            _c4this->m_pages[i] = _c4this->m_allocator.allocate(ps, Alignment);
        }
    }
    else
    {
        for(I i = np; i < _c4this->m_num_pages; ++i)
        {
            _c4this->m_allocator.deallocate(_c4this->m_pages[i], ps, Alignment);
        }
        _c4this->m_pages = at.reallocate(_c4this->m_pages, _c4this->m_num_pages, np);
    }
    _c4this->m_num_pages = np;
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
template< class T, size_t PageSize, class I, I Alignment, class Alloc >
struct raw_paged : public _raw_paged_crtp< T, I, Alignment, raw_paged<T, PageSize, I, Alignment, Alloc > >
{
    using crtp_base = _raw_paged_crtp< T, I, Alignment, raw_paged<T, PageSize, I, Alignment, Alloc > >;

    static_assert(PageSize > 1, "PageSize must be > 1");
    static_assert((PageSize & (PageSize - 1)) == 0, "PageSize must be a power of two");
    static_assert(std::is_integral< I >::value, "");

    //! id mask: all the bits up to PageSize. Use to extract the position of an index within a page.
    constexpr static const I _raw_idmask = I(PageSize) - I(1);

    //! page lsb: the number of bits by bits complementary to PageSize. Use to extract the page of an index.
    constexpr static const I _raw_pglsb  = lsb11< I, PageSize >::value;

    T    **m_pages;      //< array containing the pages
    I      m_num_pages;  //< number of current pages in the array
    Alloc  m_allocator;

public:

    _c4_DEFINE_ARRAY_TYPES(T, I)
    using allocator_type = Alloc;
    using allocator_traits = std::allocator_traits< Alloc >;

public:

    raw_paged() : raw_paged(0) {}
    raw_paged(Alloc const& a) : raw_paged(0, a) {}

    raw_paged(I cap) : m_pages(nullptr), m_num_pages(0), m_allocator()
    {
        crtp_base::_raw_resize(cap);
    }
    raw_paged(I cap, I page_sz, Alloc const& a) : m_pages(nullptr), m_num_pages(0), m_allocator(a)
    {
        crtp_base::_raw_resize(cap);
    }

    ~raw_paged()
    {
        crtp_base::_raw_resize(0);
    }

    // copy and move operations are deleted, and must be implemented by the containers,
    // as this will involve knowledge over what elements are to copied or moved
    raw_paged(raw_paged const& that) = delete;
    raw_paged(raw_paged     && that) = delete;
    raw_paged& operator=(raw_paged const& that) = delete;
    raw_paged& operator=(raw_paged     && that) = delete;

    C4_ALWAYS_INLINE T& operator[] (I i) C4_NOEXCEPT_X
    {
        C4_XASSERT(i < crtp_base::capacity());
        I pg = i >> _raw_pglsb;
        I id = i & _raw_idmask;
        C4_XASSERT(pg < m_num_pages);
        C4_XASSERT(id < PageSize);
        return m_pages[pg][id];
    }
    C4_ALWAYS_INLINE T const& operator[] (I i) const C4_NOEXCEPT_X
    {
        C4_XASSERT(i < crtp_base::capacity());
        I pg = i >> _raw_pglsb;
        I id = i & _raw_idmask;
        C4_XASSERT(pg < m_num_pages);
        C4_XASSERT(id < PageSize);
        return m_pages[pg][id];
    }

    C4_ALWAYS_INLINE static constexpr I page_size() noexcept { return PageSize; }

    template< class RawPagedContainer >
    friend void test_raw_page_addressing(RawPagedContainer const& rp);

};

//-----------------------------------------------------------------------------

/** specialization of raw_paged allowing the page size to be set at run time.
 * @ingroup raw_storage_classes */
template< class T, class I, I Alignment, class Alloc >
struct raw_paged< T, 0, I, Alignment, Alloc > : public _raw_paged_crtp< T, I, Alignment, raw_paged<T, 0, I, Alignment, Alloc > >
{
    using crtp_base = _raw_paged_crtp< T, I, Alignment, raw_paged<T, 0, I, Alignment, Alloc > >;
    static_assert(std::is_integral< I >::value, "");

    T    **m_pages;       ///< array containing the pages
    I      m_num_pages;   ///< number of current pages in the array
    I      m_id_mask;     ///< page size: cannot be changed after construction.
    I      m_page_lsb;    ///< least significant bit of the page size
    Alloc  m_allocator;

public:

    _c4_DEFINE_ARRAY_TYPES(T, I)
    using allocator_type = Alloc;
    using allocator_traits = std::allocator_traits< Alloc >;

public:

    raw_paged() : raw_paged(0, 256) {}
    raw_paged(Alloc const& a) : raw_paged(0, 256, a) {}

    raw_paged(I cap) : raw_paged(cap, 256) {}
    raw_paged(I cap, Alloc const& a) : raw_paged(cap, 256, a) {}

    raw_paged(I cap, I page_sz) : m_pages(nullptr), m_num_pages(0), m_id_mask(page_sz - 1), m_page_lsb(lsb(page_sz)), m_allocator()
    {
        C4_ASSERT(page_sz > 1);
        C4_ASSERT_MSG((page_sz & (page_sz - 1)) == 0, "page size must be a power of two");
        crtp_base::_raw_resize(cap);
    }
    raw_paged(I cap, I page_sz, Alloc const& a) : m_pages(nullptr), m_num_pages(0), m_id_mask(page_sz - 1), m_page_lsb(lsb(page_sz)), m_allocator(a)
    {
        C4_ASSERT(page_sz > 1);
        C4_ASSERT_MSG((page_sz & (page_sz - 1)) == 0, "page size must be a power of two");
        crtp_base::_raw_resize(cap);
    }

    ~raw_paged()
    {
        crtp_base::_raw_resize(0);
    }

    // copy and move operations are deleted, and must be implemented by the containers,
    // as this will involve knowledge over what elements are to copied or moved
    raw_paged(raw_paged const& that) = delete;
    raw_paged(raw_paged     && that) = delete;
    raw_paged& operator=(raw_paged const& that) = delete;
    raw_paged& operator=(raw_paged     && that) = delete;

    C4_ALWAYS_INLINE T& operator[] (I i) C4_NOEXCEPT_X
    {
        C4_XASSERT(i < crtp_base::capacity());
        I pg = i >> m_page_lsb;
        I id = i & m_id_mask;
        C4_XASSERT(pg < m_num_pages);
        C4_XASSERT(id < m_id_mask + 1);
        return m_pages[pg][id];
    }
    C4_ALWAYS_INLINE T const& operator[] (I i) const C4_NOEXCEPT_X
    {
        C4_XASSERT(i < crtp_base::capacity());
        I pg = i >> m_page_lsb;
        I id = i & m_id_mask;
        C4_XASSERT(pg < m_num_pages);
        C4_XASSERT(id < m_id_mask + 1);
        return m_pages[pg][id];
    }

    C4_ALWAYS_INLINE I page_size() const noexcept { return m_id_mask + 1; }

    template< class RawPagedContainer >
    friend void test_raw_page_addressing(RawPagedContainer const& rp);

};

C4_END_NAMESPACE(stg)
C4_END_NAMESPACE(c4)

#endif // _C4_STORAGE_RAW_HPP_
