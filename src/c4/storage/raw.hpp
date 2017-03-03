#ifndef _C4_STORAGE_RAW_HPP_
#define _C4_STORAGE_RAW_HPP_

#include "c4/config.hpp"
#include "c4/error.hpp"

C4_BEGIN_NAMESPACE(c4)
C4_BEGIN_NAMESPACE(stg)

/** @defgroup growth_policies
*/

/** @defgroup raw_storage_classes Raw storage classes
 *
 * These classes are a building block for the several flavours of
 * contiguous containers. They offer a convenient way to hold a
 * number of (almost-)contiguous objects (via the index-based [] operator).
 *
 *     - The memory used by these objects is NOT automatically allocated or\n
 *       freed. Use the protected methods _raw_reserve(I n), _raw_trim(I n)
 *     - The elements in the raw storage are NOT automatically constructed\n
 *       or destroyed.
 */
template< class Storage > struct raw_storage_traits;
template< class Storage > struct raw_storage_util;

template< class T, size_t N, class I = C4_SIZE_TYPE, I Alignment = alignof(T) > struct raw_fixed;
template<
    class T,
    class I = C4_SIZE_TYPE,
    I Alignment = alignof(T),
    class Alloc = Allocator< T >,
    class GrowthType = growth_default
>
struct raw

//-----------------------------------------------------------------------------
/** Type traits for raw storage classes.
 * @ingroup raw_storage_classes */
template< class Storage >
struct raw_storage_traits
{
    using storage_type = Storage;
    using value_type = typename Storage::value_type;
    using size_type = typename Storage::size_type;

    constexpr static const bool fixed = false;
    constexpr static const bool contiguous = false;
};

/** @ingroup raw_storage_classes */
#define C4_DEFINE_STORAGE_TRAITS(type, is_fixed, is_contiguous, ...)    \
    template< __VA_ARGS__ >                                             \
    struct raw_storage_traits< type >                                   \
    {                                                                   \
        using storage_type = T;                                         \
        using value_type = typename T::value_type;                      \
        using size_type = typename T::size_type;                        \
                                                                        \
        constexpr static const bool fixed = is_fixed;                   \
        constexpr static const bool contiguous = is_contiguous;         \
    }


//-----------------------------------------------------------------------------
/** Utility class which uses SFINAE to dispatch construction/destruction
 * to the appropriate functions. This class is meant to be used by container
 * implementations to aid in object management.
 * @ingroup raw_storage_classes */
template< class Storage >
struct raw_storage_util : public raw_storage_traits< Storage >
{
    using contiguous = raw_storage_traits< Storage >::contiguous;
    using T = typename raw_storage_traits< Storage >::value_type;
    using I = typename raw_storage_traits< Storage >::size_type;

#define _c4require(what) C4_ALWAYS_INLINE static typename std::enable_if< what, void >::type

    template< class ...Args >
    _c4require(contiguous) construct(T& dest, I first, I n, Args&&... args)
    {
        dest.m_allocator.construct_n(dest.data() + first, n, std::forward< Args >(args)...);
    }
    template< class ...Args >
    _c4require( ! contiguous) construct(T& dest, I first, I n, Args&&... args)
    {
        dest._raw_construct(first, n, std::forward< Args >(args)...);
    }

    _c4require(contiguous) destroy(T& dest, I first, I n)
    {
        dest.m_allocator.destroy_n(dest.data() + first, n);
    }
    _c4require( ! contiguous) destroy(T& dest, I first, I n)
    {
        dest._raw_destroy(first, n);
    }

    _c4require(contiguous) move_construct(T& dest, T const& src, I first, I n)
    {
        c4::move_construct_n(dest.data() + first, src.data() + first, n);
    }
    _c4require( ! contiguous) move_construct(T& dest, T const& src, I first, I n)
    {
        dest._raw_move_construct(src, first, n);
    }

    _c4require(contiguous) copy_construct(T& dest, T const& src, I first, I n)
    {
        c4::copy_construct_n(dest.data() + first, src.data() + first, n);
    }
    _c4require( ! contiguous) copy_construct(T& dest, T const& src, I first, I n)
    {
        dest._raw_copy_construct(src, first, n);
    }

    _c4require(contiguous) move_assign(T& dest, T const& src, I first, I n)
    {
        c4::move_assign_n(dest.data() + first, src.data() + first, n);
    }
    _c4require( ! contiguous) move_assign(T& dest, T const& src, I first, I n)
    {
        dest._raw_move_assign(src, first, n);
    }

    _c4require(contiguous) copy_assign(T& dest, T const& src, I first, I n)
    {
        c4::copy_assign_n(dest.data() + first, src.data() + first, n);
    }
    _c4require( ! contiguous) copy_assign(T& dest, T const& src, I first, I n)
    {
        dest._raw_copy_assign(src, first, n);
    }


#undef _c4require
};


//-----------------------------------------------------------------------------
/** raw storage with fixed capacity
 * @ingroup raw_storage_classes */
template< class T, size_t N, class I, I Alignment >
struct raw_fixed
{

    union {
        alignas(Alignment) char _m_buf[N * sizeof(T)];
        alignas(Alignment) T m_ptr[N];
    };

    _c4_DEFINE_ARRAY_TYPES(T, I)

    C4_ALWAYS_INLINE T      & operator[] (I i)       noexcept { return m_ptr[i]; }
    C4_ALWAYS_INLINE T const& operator[] (I i) const noexcept { return m_ptr[i]; }

    C4_ALWAYS_INLINE T      * data()       noexcept { return m_ptr; }
    C4_ALWAYS_INLINE T const* data() const noexcept { return m_ptr; }

    C4_ALWAYS_INLINE constexpr I  max_capacity()      const noexcept { return N; }
    C4_ALWAYS_INLINE constexpr I      capacity()      const noexcept { return N; }
    C4_ALWAYS_INLINE constexpr I next_capacity(I cap) const noexcept { return N; }

    void _raw_reserve(I n) const C4_NOEXCEPT_A { C4_ASSERT(n <= N); }
    void _raw_trim   (I n) const C4_NOEXCEPT_A { C4_ASSERT(n <= N); }
};

C4_DEFINE_STORAGE_TRAITS(raw_fixed< T, N, I, Alignment >,
                         true, // fixed
                         true, // contiguous
                         class T, size_t N, class I, I Alignment);

//-----------------------------------------------------------------------------

/** raw storage with variable size and capacity: allocatable, contiguous
 * @ingroup raw_storage_classes */
template
<
    class T,
    class I = uint32_t,
    I Alignment = alignof(T),
    class Alloc = Allocator< T >,
    class GrowthType = growth_default
>
struct raw
{

    T*    m_ptr;
    I     m_capacity;
    Alloc m_allocator;

    _c4_DEFINE_ARRAY_TYPES(T, I)

    using allocator_type = Alloc;

    raw() : m_ptr(nullptr), m_capacity(0), m_allocator() {}
    raw(Alloc const& a) : m_ptr(nullptr), m_capacity(0), m_allocator(a) {}
    ~raw()
    {
        C4_ASSERT_MSG(m_ptr == nullptr, "the container using this did not free the storage");
    }

    /** @todo implement this */
    raw(raw const& that) = delete;
    raw(raw     && that) = default;

    /** @todo implement this */
    raw& operator=(raw const& that) = delete;
    raw& operator=(raw     && that) = default;

    C4_ALWAYS_INLINE T& operator[] (I i) { return m_ptr[i]; }
    C4_ALWAYS_INLINE constexpr fastcref<T> operator[] (I i) const { return m_ptr[i]; }

    C4_ALWAYS_INLINE T      * data()       noexcept { return m_ptr; }
    C4_ALWAYS_INLINE T const* data() const noexcept { return m_ptr; }

    C4_ALWAYS_INLINE I max_capacity() const noexcept { return std::numeric_limits< I >::max() - 1; }
    C4_ALWAYS_INLINE I capacity() const noexcept { return m_capacity; }
    C4_ALWAYS_INLINE I next_capacity(I desired) const
    {
        return GrowthType::next_size(m_capacity, desired);
    }

    void _raw_reserve(I cap)
    {

    }

    void _raw_trim(I cap)
    {

    }
};

C4_DEFINE_STORAGE_TRAITS(raw< T, I, Alignment, Alloc, GrowthType >,
                         false, // fixed
                         true,  // contiguous
                         class T, class I, I Alignment, class Alloc, class GrowthType);


//-----------------------------------------------------------------------------
/** raw storage: allocatable and paged. This is NOT a contiguous storage structure.
  * However, it does behave as such, offering the [] operator with contiguous
  * range indices. This is useful for minimizing allocations and data copies in
  * dynamic array-based containers as flat_list.
  *
  * @ingroup raw_storage_classes
  * @todo add a raw structure with a runtime-determined page size */
template
<
    class T,
    size_t PageSize,      //< The page size. Must be a power of two.
    class I = uint32_t,
    I Alignment = alignof(T),
    class Alloc = Allocator< T >
>
struct raw_paged
{
    static_assert(PageSize > 1, "PageSize must be > 1")
    static_assert(PageSize & (PageSize - 1) == 0, "PageSize must be a power of two")

    //! id mask: all the bits up to PageSize. Use to extract the position of an index within a page.
    constexpr static const I _raw_idmask = (I)PageSize - 1;

    //! page mask: bits complementary to PageSize. Use to extract the page of an index.
    constexpr static const I _raw_pgmask = ~ ((I)PageSize - 1); //< all the bits

    T    **m_pages;      //< array containing the pages
    I      m_num_pages;  //< number of current pages in the array
    Alloc  m_allocator;

public:

    _c4_DEFINE_ARRAY_TYPES(T, I)
    using allocator_type = Alloc;

    raw_paged() : m_pages(nullptr), m_num_pages(0), m_allocator() {}
    raw_paged(Alloc const& a) : m_pages(nullptr), m_num_pages(0), m_allocator(a) {}

    raw_paged(raw_paged const& that) = delete;
    raw_paged(raw_paged     && that) = default;
    raw_paged& operator=(raw_paged const& that) = delete;
    raw_paged& operator=(raw_paged     && that) = default;

    C4_ALWAYS_INLINE T& operator[] (I i)
    {
        C4_XASSERT(i < capacity());
        I pg = i & _raw_pgmask;
        I id = i & _raw_idmask;
        C4_XASSERT(pg < m_num_pages);
        C4_XASSERT(id < PageSize);
        return m_pages[pg][id];
    }
    C4_ALWAYS_INLINE constexpr fastcref<T> operator[] (I i) const
    {
        C4_XASSERT(i < capacity());
        I pg = i & _raw_pgmask;
        I id = i & _raw_id;
        C4_XASSERT(pg < m_num_pages);
        C4_XASSERT(id < PageSize);
        return m_pages[pg][id];
    }

    C4_ALWAYS_INLINE I max_capacity() const noexcept { return std::numeric_limits< I >::max() - 1; }
    C4_ALWAYS_INLINE I capacity() const noexcept { return m_num_pages * PageSize; }
    C4_ALWAYS_INLINE I next_capacity(I desired) const
    {
        I cap = capacity();
        if(desired < cap) return cap;
        I np = desired / PageSize;
        I cap = np * PageSize;
        return cap;
    }

protected:

    void _raw_reserve(I cap)
    {
        if(cap <= capacity()) return;
        I np = cap / PageSize;
        C4_XASSERT(np * PageSize >= capacity());
        if(np > m_num_pages)
        {
            auto at = m_allocator.rebound< T* >();
            m_pages = at.reallocate(m_pages, m_num_pages, np);
            for(I i = m_num_pages; i < np; ++i)
            {
                m_pages[i] = m_allocator.allocate(PageSize, Alignment);
            }
        }
        m_num_pages = np;
    }
    void _raw_trim(I to)
    {
        if(m_pages == nullptr) return;
        I np = to / PageSize;
        if(np >= m_num_pages) return;
        for(I i = np; i < m_num_pages; ++i)
        {
            m_allocator.deallocate(m_pages[i], PageSize, Alignment);
        }
        auto at = m_allocator.rebound< T* >();
        if(to == 0)
        {
            at.deallocate(m_pages, m_num_pages);
            m_pages = nullptr;
        }
        else
        {
            m_pages = at.reallocate(m_pages, m_num_pages, np);
        }
        m_num_pages = np;
    }
    void _raw_copy(raw_paged const& that, I first, I num)
    {
        C4_NOT_IMPLEMENTED();
    }
};


C4_DEFINE_STORAGE_TRAITS(raw_paged< T, I, PageSize, Alignment, Alloc >,
                         false, // fixed
                         false, // contiguous
                         true, // paged
                         class T, class I, I PageSize, I Alignment, class Alloc);

C4_END_NAMESPACE(stg)
C4_END_NAMESPACE(c4)

#endif // _C4_STORAGE_RAW_HPP_
