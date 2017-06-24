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
/** since the page size is a power of two, the max capacity is simply the
 * maximum of the size type */
template< class I >
C4_ALWAYS_INLINE constexpr size_t raw_max_capacity() noexcept
{
    return size_t(std::numeric_limits< I >::max());
}

//-----------------------------------------------------------------------------

// forward declarations
template< class Storage, class TagType > struct raw_storage_traits;

/** @todo make the Alignment a size_t */
template< class T, size_t N, class I=C4_SIZE_TYPE, I Alignment=alignof(T) >
struct raw_fixed;

/** @todo make the Alignment a size_t */
template< class T, class I=C4_SIZE_TYPE, I Alignment=alignof(T), class Alloc=Allocator<T>, class GrowthPolicy=growth_default >
struct raw;

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

struct contiguous_t {};
struct fixed_t {};
struct small_t {};
struct paged_t {};

template< class Storage, class TagType >
struct _raw_storage_traits;

/** Type traits for raw storage classes.
 * @ingroup raw_storage_classes */
template< class Storage, class TagType >
struct raw_storage_traits : public _raw_storage_traits< Storage, TagType >
{
    using storage_type = Storage;
    using value_type = typename Storage::value_type;
    using size_type = typename Storage::size_type;
    using tag_type = TagType;
    using util_type = _raw_storage_traits< Storage, TagType >;

    enum {
        contiguous = std::is_same< TagType, contiguous_t >::value,
        fixed = std::is_same< TagType, fixed_t >::value,
        small = std::is_same< TagType, small_t >::value,
        paged = std::is_same< TagType, paged_t >::value,
        uses_allocator = ! fixed
    };

    using util_type::destroy_n;
    using util_type::construct_n;
    using util_type::move_construct_n;
    using util_type::move_assign_n;
    using util_type::copy_construct_n;
    using util_type::copy_assign_n;
};


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

/** specialization of _raw_storage_traits for fixed storage containers.
 * @ingroup raw_storage_classes */
template< class Storage >
struct _raw_storage_traits< Storage, fixed_t >
{
    using storage_type = Storage;

    using T = typename Storage::value_type;
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

    C4_ALWAYS_INLINE static void move_construct_n(Storage& dest, Storage const& src, I first, I n)
    {
        c4::move_construct_n(dest.data() + first, src.data() + first, n);
    }

    C4_ALWAYS_INLINE static void copy_construct_n(Storage& dest, Storage const& src, I first, I n)
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
struct _common_contiguous_traits
{
    using storage_type = Storage;

    using T = typename Storage::value_type;
    using I = typename Storage::size_type;

    /// @todo use alloc_traits here and in the other functions
    template< class ...Args >
    C4_ALWAYS_INLINE static void construct_n(Storage& dest, I first, I n, Args&&... args)
    {
        dest.m_allocator.construct_n(dest.data() + first, n, std::forward< Args >(args)...);
    }

    C4_ALWAYS_INLINE static void destroy_n(Storage& dest, I first, I n)
    {
        dest.m_allocator.destroy_n(dest.data() + first, n);
    }

    C4_ALWAYS_INLINE static void move_construct_n(Storage& dest, Storage const& src, I first, I n)
    {
        c4::move_construct_n(dest.data() + first, src.data() + first, n);
    }

    C4_ALWAYS_INLINE static void copy_construct_n(Storage& dest, Storage const& src, I first, I n)
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

/** specialization of _raw_storage_traits for non-fixed contiguous containers.
 * @ingroup raw_storage_classes */
template< class Storage >
struct _raw_storage_traits< Storage, contiguous_t > : public _common_contiguous_traits< Storage >
{
};
/** specialization of _raw_storage_traits for small contiguous containers.
 * @ingroup raw_storage_classes */
template< class Storage >
struct _raw_storage_traits< Storage, small_t > : public _common_contiguous_traits< Storage >
{
    using storage_type = Storage;
};


/** specialization of _raw_storage_traits for paged storage containers.
 * @ingroup raw_storage_classes */
template< class Storage >
struct _raw_storage_traits< Storage, paged_t >
{
    using storage_type = Storage;

    using T = typename Storage::value_type;
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

    C4_ALWAYS_INLINE static void move_construct_n(Storage& dest, Storage const& src, I first, I n)
    {
        dest._raw_move_construct_n(src, first, n);
    }

    C4_ALWAYS_INLINE static void copy_construct_n(Storage& dest, Storage const& src, I first, I n)
    {
        dest._raw_copy_construct_n(src, first, n);
    }

    C4_ALWAYS_INLINE static void move_assign_n(Storage& dest, Storage const& src, I first, I n)
    {
        dest._raw_move_assign_n(src, first, n);
    }

    C4_ALWAYS_INLINE static void copy_assign_n(Storage& dest, Storage const& src, I first, I n)
    {
        dest._raw_copy_assign_n(src, first, n);
    }
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/** raw contiguous storage with fixed (at compile time) capacity
 * @ingroup raw_storage_classes */
template< class T, size_t N, class I, I Alignment >
struct raw_fixed
{

    C4_STATIC_ASSERT(N <= (size_t)std::numeric_limits< I >::max());

    /** the union with the char buffer is needed to prevent auto-construction
     * of the elements in m_arr */
    union {
        alignas(Alignment) char _m_buf[N * sizeof(T)];
        alignas(Alignment) T m_arr[N];
    };

public:

    _c4_DEFINE_ARRAY_TYPES(T, I);
    using allocator_type = void;

    using storage_traits = raw_storage_traits< raw_fixed, fixed_t >;

    template< class U >
    using rebind_type = raw_fixed<U, N, I, alignof(U)>;

public:

    C4_ALWAYS_INLINE raw_fixed() {}
    C4_ALWAYS_INLINE ~raw_fixed() {}

    C4_ALWAYS_INLINE raw_fixed(I cap) { C4_UNUSED(cap); C4_ASSERT(cap <= (I)N); }

    // copy and move operations are deleted, and must be implemented by the containers,
    // as this will involve knowledge over what elements are to copied or moved
    raw_fixed(raw_fixed const& that) = delete;
    raw_fixed(raw_fixed     && that) = delete;
    raw_fixed& operator=(raw_fixed const& that) = delete;
    raw_fixed& operator=(raw_fixed     && that) = delete;

    C4_ALWAYS_INLINE T      & operator[] (I i)       C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < (I)N); return m_arr[i]; }
    C4_ALWAYS_INLINE T const& operator[] (I i) const C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < (I)N); return m_arr[i]; }

    C4_ALWAYS_INLINE T      * data()       noexcept { return m_arr; }
    C4_ALWAYS_INLINE T const* data() const noexcept { return m_arr; }

    C4_ALWAYS_INLINE constexpr I capacity() const noexcept { return (I)N; }

    C4_ALWAYS_INLINE constexpr static size_t max_capacity() noexcept { return N; }
    C4_ALWAYS_INLINE C4_CONSTEXPR14 static size_t next_capacity(size_t cap) C4_NOEXCEPT_A
    {
        C4_UNUSED(cap);
        C4_ASSERT(cap <= (I)N);
        return N;
    }

public:

    iterator       _raw_iterator(I id)       noexcept { return m_arr + id; }
    const_iterator _raw_iterator(I id) const noexcept { return m_arr + id; }

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

    struct tmp_storage
    {
    };
    void _raw_reserve_allocate(I cap, tmp_storage *)
    {
        C4_ASSERT(cap <= (I)N);
    }
    void _raw_reserve_replace(tmp_storage *tmp)
    {
    }

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
    void _raw_resize(I pos, I prev, I next)
    {
        C4_ASSERT(next >= 0 && next < N);
        C4_ASSERT(prev >= 0 && prev < N);
        C4_ASSERT(pos  >= 0 && pos  < N);
        if(next > prev)
        {
            c4::make_room(m_arr + pos, prev - pos, next - prev);
        }
        else if(next < prev)
        {
            I delta = prev - next;
            C4_ASSERT(pos > delta);
            c4::destroy_room(m_arr + pos - delta, prev - pos, delta);
        }
    }

};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/** raw contiguous storage with variable capacity
 * @ingroup raw_storage_classes */
template< class T, class I, I Alignment, class Alloc, class GrowthPolicy >
struct raw
{

    T*    m_ptr;
    I     m_capacity;
    Alloc m_allocator;

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

public:

    raw() : raw(0) {}
    raw(Alloc const& a) : raw(0, a) {}

    raw(I cap) : m_ptr(nullptr), m_capacity(0), m_allocator() { _raw_reserve(0, cap); }
    raw(I cap, Alloc const& a) : m_ptr(nullptr), m_capacity(0), m_allocator(a) { _raw_reserve(0, cap); }

    ~raw()
    {
        _raw_reserve(0, 0);
    }

    // copy and move operations are deleted, and must be implemented by the containers,
    // as this will involve knowledge over what elements are to be copied or moved
    raw(raw const& that) = delete;
    raw(raw     && that) = delete;
    raw& operator=(raw const& that) = delete;
    raw& operator=(raw     && that) = delete;

public:

    C4_ALWAYS_INLINE T      & operator[] (I i)       C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < m_capacity); return m_ptr[i]; }
    C4_ALWAYS_INLINE T const& operator[] (I i) const C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < m_capacity); return m_ptr[i]; }

    C4_ALWAYS_INLINE T      * data()       noexcept { return m_ptr; }
    C4_ALWAYS_INLINE T const* data() const noexcept { return m_ptr; }

    C4_ALWAYS_INLINE I capacity() const noexcept { return m_capacity; }

    C4_ALWAYS_INLINE constexpr static size_t max_capacity() noexcept { return raw_max_capacity< I >(); }
    C4_ALWAYS_INLINE constexpr size_t next_capacity(size_t desired) const noexcept
    {
        return GrowthPolicy::next_size(sizeof(T), m_capacity, desired);
    }

public:

    iterator       _raw_iterator(I id)       noexcept { return m_ptr + id; }
    const_iterator _raw_iterator(I id) const noexcept { return m_ptr + id; }

public:

    /** assume the curr size is zero */
    void _raw_reserve(I cap)
    {
        _raw_reserve(0, cap);
    }

    void _raw_reserve(I currsz, I cap)
    {
        C4_ASSERT(currsz <= cap);
        T *tmp = nullptr;
        if(cap != m_capacity && cap != 0)
        {
            tmp = m_allocator.allocate(cap, Alignment);
        }
        if(m_ptr)
        {
            if(tmp)
            {
                c4::move_construct_n(tmp, m_ptr, currsz);
            }
            m_allocator.deallocate(m_ptr, m_capacity, Alignment);
        }
        m_capacity = cap;
        m_ptr = tmp;
    }

    struct tmp_storage : public raw
    {
    };
    void _raw_reserve_allocate(I cap, tmp_storage *tmp)
    {
        T *t = nullptr;
        if(cap != m_capacity && cap != 0)
        {
            t = m_allocator.allocate(cap, Alignment);
        }
        tmp->m_capacity = cap;
        tmp->m_ptr = t;
    }
    void _raw_reserve_replace(tmp_storage *tmp)
    {
        if(m_ptr)
        {
            m_allocator.deallocate(m_ptr, m_capacity, Alignment);
        }
        m_capacity = tmp->m_capacity;
        m_ptr = tmp->m_ptr;
        tmp->m_ptr = nullptr;
        tmp->m_capacity = 0;
    }

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
    void _raw_resize(I pos, I prev, I next)
    {
        C4_ASSERT(next >= 0 && next < m_capacity);
        C4_ASSERT(prev >= 0 && prev < m_capacity);
        C4_ASSERT(pos  >= 0 && pos  < m_capacity);
        if(next > prev)
        {
            if(next <= m_capacity)
            {
                c4::make_room(m_ptr + pos, prev - pos, next - prev);
            }
            else
            {
                m_capacity = next_capacity(next);
                T* tmp = m_allocator.allocate(m_capacity, Alignment);
                if(m_ptr)
                {
                    c4::make_room(tmp, m_ptr, prev, next - prev, pos);
                    m_allocator.deallocate(m_ptr, m_capacity, Alignment);
                }
                else
                {
                    C4_ASSERT(prev == 0);
                }
                m_ptr = tmp;
            }
        }
        else if(next < prev)
        {
            I delta = prev - next;
            C4_ASSERT(pos > delta);
            c4::destroy_room(m_ptr + pos - delta, prev - pos, delta);
        }
    }

};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/** raw contiguous storage with in-place room for a small number of objects
 * @tparam N the number of objects
 * @ingroup raw_storage_classes */
template< class T, class I, size_t N_, I Alignment, class Alloc, class GrowthPolicy >
struct raw_small
{

    C4_STATIC_ASSERT(N_ <= (size_t)std::numeric_limits< I >::max());
    //C4_STATIC_ASSERT(sizeof(T) == alignof(T));  // not sure if this is needed

#ifdef __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wnested-anon-types" // warning: anonymous types declared in an anonymous union are an extension
#endif

    union {
        /** the union with the char buffer is needed to prevent
         * auto-construction of the elements in m_arr */
        union {
            alignas(Alignment) T    m_arr[N_];
            alignas(Alignment) char m_buf[N_ * sizeof(T)];
        };
        T * m_ptr;
    };

    I     m_capacity;
    Alloc m_allocator;

#ifdef __clang__
#   pragma clang diagnostic pop
#endif

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

    enum : I { array_size = (I)N_, N = (I)N_ };

public:

    raw_small() : raw_small(0) {}
    raw_small(Alloc const& a) : raw_small(0, a) {}

    raw_small(I cap) : m_ptr(nullptr), m_capacity(N), m_allocator() { _raw_reserve(0, cap); }
    raw_small(I cap, Alloc const& a) : m_ptr(nullptr), m_capacity(N), m_allocator(a) { _raw_reserve(0, cap); }

    ~raw_small()
    {
        _raw_reserve(0, 0);
    }

    // copy and move operations are deleted, and must be implemented by the containers,
    // as this will involve knowledge over what elements are to copied or moved
    raw_small(raw_small const& that) = delete;
    raw_small(raw_small     && that) = delete;
    raw_small& operator=(raw_small const& that) = delete;
    raw_small& operator=(raw_small     && that) = delete;

    // gcc is triggering a false positive here when compiling in release mode:
    // error: array subscript is below array bounds [-Werror=array-bounds].
    // probably this is due to moving a return branch into the assert.
    // see https://gcc.gnu.org/ml/gcc/2009-09/msg00270.html for a similar example
    C4_ALWAYS_INLINE T      & operator[] (I i)       C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < m_capacity); return C4_LIKELY(i >= 0 && m_capacity <= N) ? m_arr[i] : m_ptr[i]; }
    C4_ALWAYS_INLINE T const& operator[] (I i) const C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < m_capacity); return C4_LIKELY(i >= 0 && m_capacity <= N) ? m_arr[i] : m_ptr[i]; }

    C4_ALWAYS_INLINE T      * data()       noexcept { return m_capacity <= N ? m_arr : m_ptr; }
    C4_ALWAYS_INLINE T const* data() const noexcept { return m_capacity <= N ? m_arr : m_ptr; }

    C4_ALWAYS_INLINE I capacity() const noexcept { return m_capacity; }
    C4_ALWAYS_INLINE bool is_small() const noexcept { return m_capacity <= N; }

    C4_ALWAYS_INLINE constexpr static size_t max_capacity() noexcept { return raw_max_capacity< I >(); }
    C4_ALWAYS_INLINE constexpr size_t next_capacity(size_t desired) const noexcept
    {
        return GrowthPolicy::next_size(sizeof(T), m_capacity, desired);
    }

public:

    iterator       _raw_iterator(I id)       noexcept { return m_capacity <= N ? m_arr + id : m_ptr + id; }
    const_iterator _raw_iterator(I id) const noexcept { return m_capacity <= N ? m_arr + id : m_ptr + id; }

public:

    /** assume the curr size is zero */
    void _raw_reserve(I cap)
    {
        _raw_reserve(0, cap);
    }

    void _raw_reserve(I curr, I cap)
    {
        C4_ASSERT(curr <= cap);
        T *tmp = nullptr;
        if(cap == m_capacity) return;
        if(cap <= N)
        {
            if(m_capacity <= N)
            {
                return; // nothing to do
            }
            else
            {
                tmp = m_arr; // move the storage to the array
            }
        }
        else
        {
            // since here we know that cap != m_capacity and that cap
            // is larger than the array, we'll always need a new buffer
            tmp = m_allocator.allocate(cap, Alignment);
        }
        if(m_capacity)
        {
            if(m_capacity <= N)
            {
                C4_ASSERT(tmp != m_arr);
                c4::move_construct_n(tmp, m_arr, curr);
            }
            else
            {
                c4::move_construct_n(tmp, m_ptr, curr);
                m_allocator.deallocate(m_ptr, m_capacity, Alignment);
            }
        }
        m_capacity = cap;
        m_ptr = tmp;
    }

    struct tmp_storage : public raw_small
    {
    };
    void _raw_reserve_allocate(I cap, tmp_storage *tmp)
    {
        C4_NOT_IMPLEMENTED();
    }
    void _raw_reserve_replace(tmp_storage *tmp)
    {
        C4_NOT_IMPLEMENTED();
    }

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
    void _raw_resize(I pos, I prev, I next)
    {
        C4_ASSERT(next >= 0 && next < m_capacity);
        C4_ASSERT(prev >= 0 && prev < m_capacity);
        C4_ASSERT(pos  >= 0 && pos  < m_capacity);
        if(next > prev)
        {
            if(m_capacity <= N && next <= N)
            {
                c4::make_room(m_arr + pos, prev - pos, next - prev);
            }
            else
            {
                C4_ASSERT(next > N);
                if(next <= m_capacity)
                {
                    c4::make_room(m_ptr + pos, prev - pos, next - prev);
                }
                else
                {
                    I cap = next_capacity(next);
                    T* tmp = m_allocator.allocate(cap, Alignment);
                    if(m_capacity <= N)
                    {
                        c4::make_room(tmp, m_arr, prev, next - prev, pos);
                    }
                    else if(m_capacity > N)
                    {
                        c4::make_room(tmp, m_ptr, prev, next - prev, pos);
                        m_allocator.deallocate(m_ptr, m_capacity, Alignment);
                    }
                    m_ptr = tmp;
                    m_capacity = cap;
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

};

//-----------------------------------------------------------------------------

// utility defines, undefined below
#define _c4this static_cast< RawPaged* >(this)
#define _c4cthis static_cast< RawPaged const* >(this)

/* a crtp base for paged storage */
template< class T, class I, I Alignment, class RawPaged >
struct _raw_paged_crtp
{

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

    C4_ALWAYS_INLINE C4_CONSTEXPR14 I num_pages() const noexcept { return _c4cthis->m_num_pages; }
    C4_ALWAYS_INLINE C4_CONSTEXPR14 I capacity() const noexcept { return _c4cthis->m_num_pages * _c4cthis->page_size(); }

public:

    void _raw_reserve(I cap)
    {
        _raw_reserve(0, cap);
    }
    void _raw_reserve(I currsz, I cap);

    struct tmp_storage : public RawPaged
    {
    };
    void _raw_reserve_allocate(I cap, tmp_storage *tmp)
    {
        C4_NOT_IMPLEMENTED();
    }
    void _raw_reserve_replace(tmp_storage *tmp)
    {
        C4_NOT_IMPLEMENTED();
    }

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

        U& operator*  () const C4_NOEXCEPT_X { return  (*this_)[i]; }
        U* operator-> () const C4_NOEXCEPT_X { return &(*this_)[i]; }

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
void _raw_paged_crtp< T, I, Alignment, RawPaged >::_raw_reserve(I currsz, I cap)
{
    auto at = _c4this->m_allocator.template rebound< T* >();
    const I ps = _c4this->page_size();
    if(cap == 0)
    {
        _c4this->_raw_destroy_n(0, currsz);
        for(I i = 0; i < _c4this->m_num_pages; ++i)
        {
            _c4this->m_allocator.deallocate(_c4this->m_pages[i], ps, Alignment);
        }
        at.deallocate(_c4this->m_pages, _c4this->m_num_pages);
        _c4this->m_pages = nullptr;
        _c4this->m_num_pages = 0;
        return;
    }

    // number of pages: round up to the next multiple of ps
    const I np = (cap + ps - 1) / ps;
    C4_ASSERT(np * ps >= cap);

    if(np == _c4this->m_num_pages)
    {
        return;
    }

    T ** tmp = at.allocate(np);
    for(I i = 0; i < np; ++i)
    {
        tmp[i] = _c4this->m_allocator.allocate(ps, Alignment);
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
        for(I i = 0; i < _c4this->m_num_pages; ++i)
        {
            _c4this->m_allocator.deallocate(_c4this->m_pages[i], ps, Alignment);
        }
        at.deallocate(_c4this->m_pages, _c4this->m_num_pages);
    }
    _c4this->m_num_pages = np;
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

    constexpr static I _raw_pg(I const i) { return i >> m_page_lsb; }
    constexpr static I _raw_id(I const i) { return i &  m_id_mask; }

    T    **m_pages;      //< array containing the pages
    I      m_num_pages;  //< number of current pages in the array
    Alloc  m_allocator;

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

    raw_paged(I cap) : m_pages(nullptr), m_num_pages(0), m_allocator()
    {
        crtp_base::_raw_reserve(0, cap);
    }
    raw_paged(I cap, Alloc const& a) : m_pages(nullptr), m_num_pages(0), m_allocator(a)
    {
        crtp_base::_raw_reserve(0, cap);
    }

    ~raw_paged()
    {
        crtp_base::_raw_reserve(0, 0);
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
        const I pg = i >> m_page_lsb;
        const I id = i & m_id_mask;
        C4_XASSERT(pg >= 0 && pg < m_num_pages);
        C4_XASSERT(id >= 0 && id < (I)PageSize);
        return m_pages[pg][id];
    }
    C4_ALWAYS_INLINE T const& operator[] (I i) const C4_NOEXCEPT_X
    {
        C4_XASSERT(i < crtp_base::capacity());
        const I pg = i >> m_page_lsb;
        const I id = i & m_id_mask;
        C4_XASSERT(pg >= 0 && pg < m_num_pages);
        C4_XASSERT(id >= 0 && id < (I)PageSize);
        return m_pages[pg][id];
    }

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

    T    **m_pages;       ///< array containing the pages
    I      m_num_pages;   ///< number of current pages in the array
    I      m_id_mask;     ///< page size - 1: cannot be changed after construction.
    I      m_page_lsb;    ///< least significant bit of the page size: cannot be changed after construction.
    Alloc  m_allocator;

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

    raw_paged(I cap, I page_sz) : m_pages(nullptr), m_num_pages(0), m_id_mask(page_sz - 1), m_page_lsb(lsb(page_sz)), m_allocator()
    {
        C4_ASSERT_MSG(page_sz > 1, "page_sz=%zu", (size_t)page_sz);
        C4_ASSERT_MSG((page_sz & (page_sz - 1)) == 0, "page size must be a power of two. page_sz=%zu", (size_t)page_sz);
        crtp_base::_raw_reserve(0, cap);
    }
    raw_paged(I cap, I page_sz, Alloc const& a) : m_pages(nullptr), m_num_pages(0), m_id_mask(page_sz - 1), m_page_lsb(lsb(page_sz)), m_allocator(a)
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
    raw_paged(raw_paged const& that) = delete;
    raw_paged(raw_paged     && that) = delete;
    raw_paged& operator=(raw_paged const& that) = delete;
    raw_paged& operator=(raw_paged     && that) = delete;

    C4_ALWAYS_INLINE T& operator[] (I i) C4_NOEXCEPT_X
    {
        C4_XASSERT(i < crtp_base::capacity());
        const I pg = i >> m_page_lsb;
        const I id = i & m_id_mask;
        C4_XASSERT(pg >= 0 && pg < m_num_pages);
        C4_XASSERT(id >= 0 && id < m_id_mask + 1);
        return m_pages[pg][id];
    }
    C4_ALWAYS_INLINE T const& operator[] (I i) const C4_NOEXCEPT_X
    {
        C4_XASSERT(i < crtp_base::capacity());
        const I pg = i >> m_page_lsb;
        const I id = i & m_id_mask;
        C4_XASSERT(pg >= 0 && pg < m_num_pages);
        C4_XASSERT(id >= 0 && id < m_id_mask + 1);
        return m_pages[pg][id];
    }

    C4_ALWAYS_INLINE I page_size() const noexcept { return m_id_mask + 1; }

    template< class RawPagedContainer >
    friend void test_raw_page_addressing(RawPagedContainer const& rp);

};

C4_END_NAMESPACE(stg)
C4_END_NAMESPACE(c4)

#endif // _C4_STORAGE_RAW_HPP_
