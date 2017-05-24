#ifndef _C4_STORAGE_RAW_HPP_
#define _C4_STORAGE_RAW_HPP_

/** @file raw.hpp Raw storage classes. */

#include "c4/config.hpp"
#include "c4/error.hpp"
#include "c4/ctor_dtor.hpp"
#include "c4/memory_util.hpp"
#include "c4/szconv.hpp"

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
 *   requires explicit calls to the function _raw_reserve(I currsz, I cap).
 *
 * - The elements contained in the raw storage are NOT automatically constructed
 *   or destroyed. The exception to this is in _raw_resize(), which adds empty
 *   space for a certain number of elements at, unless through .
 */

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

// forward declarations
template< class Storage, class TagType > struct raw_storage_traits;

template< class T, size_t N, class I=C4_SIZE_TYPE, I Alignment=alignof(T) >
struct raw_fixed;

template< class T, class I=C4_SIZE_TYPE, I Alignment=alignof(T), class Alloc=Allocator<T>, class GrowthPolicy=growth_default >
struct raw;

/** raw storage with inplace storage of up to N objects, thus saving an
 * allocation when the size is small. @ingroup raw_storage_classes */
template< class T, size_t N=16, class I=C4_SIZE_TYPE, I Alignment=alignof(T), class Alloc=Allocator<T>, class GrowthPolicy=growth_default >
struct raw_small; // = raw< T, I, Alignment, SmallAllocator<T, N, Alignment> >;

template< class T, size_t PageSize=256, class I=C4_SIZE_TYPE, I Alignment=alignof(T), class Alloc=Allocator<T> >
struct raw_paged;

/** raw paged with page size determined at runtime. @ingroup raw_storage_classes */
template< class T, class I=C4_SIZE_TYPE, I Alignment=alignof(T), class Alloc=Allocator<T> >
using raw_paged_rt = raw_paged< T, 0, I, Alignment, Alloc >;

//-----------------------------------------------------------------------------

struct contiguous_t {};
struct fixed_t {};
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
        paged = std::is_same< TagType, paged_t >::value,
        has_allocator = ! fixed
    };

    using util_type::destroy_n;
    using util_type::construct_n;
    using util_type::move_construct_n;
    using util_type::move_assign_n;
    using util_type::copy_construct_n;
    using util_type::copy_assign_n;
};


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

/** specialization of _raw_storage_traits for non-fixed contiguous containers.
 * @ingroup raw_storage_classes */
template< class Storage >
struct _raw_storage_traits< Storage, contiguous_t >
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

    union {
        alignas(Alignment) char _m_buf[N * sizeof(T)];
        alignas(Alignment) T m_arr[N];
    };

public:

    _c4_DEFINE_ARRAY_TYPES(T, I)
    using storage_traits = raw_storage_traits< raw_fixed, fixed_t >;

public:

    C4_ALWAYS_INLINE raw_fixed() {}
    C4_ALWAYS_INLINE ~raw_fixed() {}

    // copy and move operations are deleted, and must be implemented by the containers,
    // as this will involve knowledge over what elements are to copied or moved
    raw_fixed(raw_fixed const& that) = delete;
    raw_fixed(raw_fixed     && that) = delete;
    raw_fixed& operator=(raw_fixed const& that) = delete;
    raw_fixed& operator=(raw_fixed     && that) = delete;

    C4_ALWAYS_INLINE T      & operator[] (I i)       C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < N); return m_arr[i]; }
    C4_ALWAYS_INLINE T const& operator[] (I i) const C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < N); return m_arr[i]; }

    C4_ALWAYS_INLINE T      * data()       noexcept { return m_arr; }
    C4_ALWAYS_INLINE T const* data() const noexcept { return m_arr; }

    C4_ALWAYS_INLINE constexpr I capacity() const noexcept { return N; }

    C4_ALWAYS_INLINE constexpr static I next_capacity(I cap) noexcept { return N; }
    C4_ALWAYS_INLINE constexpr static I  max_capacity()      noexcept { return N; }

public:

    iterator       _raw_iterator(I id)       noexcept { return m_arr + id; }
    const_iterator _raw_iterator(I id) const noexcept { return m_arr + id; }

public:

    void _raw_reserve(I cap) const C4_NOEXCEPT_A
    {
        C4_ASSERT(cap <= N);
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

    _c4_DEFINE_ARRAY_TYPES(T, I)
    using storage_traits = raw_storage_traits< raw, contiguous_t >;

    using allocator_type = Alloc;
    using allocator_traits = std::allocator_traits< Alloc >;
    using growth_policy = GrowthPolicy;

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

    C4_ALWAYS_INLINE static I max_capacity() noexcept { return std::numeric_limits< I >::max() - 1; }
    C4_ALWAYS_INLINE I next_capacity(I desired) const noexcept
    {
        return GrowthPolicy::next_size(sizeof(T), m_capacity, desired);
    }

public:

    iterator       _raw_iterator(I id)       noexcept { return m_ptr + id; }
    const_iterator _raw_iterator(I id) const noexcept { return m_ptr + id; }

public:

    void _raw_reserve(I curr, I cap) C4_NOEXCEPT_A
    {
        C4_ASSERT(curr <= cap);
        T *tmp = nullptr;
        if(cap != m_capacity && cap != 0)
        {
            tmp = m_allocator.allocate(cap, Alignment);
        }
        if(m_ptr)
        {
            if(tmp)
            {
                c4::move_construct_n(tmp, m_ptr, curr);
            }
            m_allocator.deallocate(m_ptr, m_capacity, Alignment);
        }
        m_capacity = cap;
        m_ptr = tmp;
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

/** raw contiguous storage with in-place storage for a small number of objects
 * @tparam N the number of objects
 * @ingroup raw_storage_classes */
template< class T, size_t N_, class I, I Alignment, class Alloc, class GrowthPolicy >
struct raw_small
{

    C4_STATIC_ASSERT(sizeof(T) == alignof(T));

    union {
        union {
            alignas(Alignment) T    m_arr[N_];
            alignas(Alignment) char m_buf[N_ * sizeof(T)];
        };
        T * m_ptr;
    };

    I     m_capacity;
    Alloc m_allocator;

public:

    _c4_DEFINE_ARRAY_TYPES(T, I)
    using storage_traits = raw_storage_traits< raw_small, contiguous_t >;

    using allocator_type = Alloc;
    using allocator_traits = std::allocator_traits< Alloc >;
    using growth_policy = GrowthPolicy;

    C4_STATIC_ASSERT(N_ < (size_t)std::numeric_limits< I >::max());
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

    C4_ALWAYS_INLINE T      & operator[] (I i)       C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < m_capacity); return m_capacity <= N ? m_arr[i] : m_ptr[i]; }
    C4_ALWAYS_INLINE T const& operator[] (I i) const C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < m_capacity); return m_capacity <= N ? m_arr[i] : m_ptr[i]; }

    C4_ALWAYS_INLINE T      * data()       noexcept { return m_capacity <= N ? m_arr : m_ptr; }
    C4_ALWAYS_INLINE T const* data() const noexcept { return m_capacity <= N ? m_arr : m_ptr; }

    C4_ALWAYS_INLINE I capacity() const noexcept { return m_capacity; }

    C4_ALWAYS_INLINE static I max_capacity() noexcept { return std::numeric_limits< I >::max() - 1; }
    C4_ALWAYS_INLINE I next_capacity(I desired) const noexcept
    {
        return GrowthPolicy::next_size(sizeof(T), m_capacity, desired);
    }

public:

    iterator       _raw_iterator(I id)       noexcept { return m_capacity <= N ? m_arr + id : m_ptr + id; }
    const_iterator _raw_iterator(I id) const noexcept { return m_capacity <= N ? m_arr + id : m_ptr + id; }

public:

    void _raw_reserve(I curr, I cap) C4_NOEXCEPT_A
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

template< class T, class I >
struct default_page_size
{
    enum : I { value = 256 };
};
template< class T >
struct default_page_size< T, uint8_t >
{
    enum : uint8_t { value = 128 };
};
template< class T >
struct default_page_size< T, int8_t >
{
    enum : int8_t { value = 64 };
};

//-----------------------------------------------------------------------------

// utility defines, undefined below
#define _c4this static_cast< RawPaged* >(this)
#define _c4cthis static_cast< RawPaged const* >(this)

/* a crtp base for paged storage */
template< class T, class I, I Alignment, class RawPaged >
struct _raw_paged_crtp
{

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

public:

    void _raw_resize(I cap);

public:

    template< class U >
    class iterator_impl : public std::iterator< std::bidirectional_iterator_tag, U >
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
        iterator_impl& operator++ (int) noexcept { iterator_impl it = *this; ++i; return    it; }

        iterator_impl& operator-- (   ) noexcept {                           --i; return *this; }
        iterator_impl& operator-- (int) noexcept { iterator_impl it = *this; --i; return    it; }

        bool operator== (iterator_impl const& that) const noexcept { return i == that.i && this_ == that.this_; }
        bool operator!= (iterator_impl const& that) const noexcept { return i != that.i || this_ != that.this_; }

    };

    using iterator = iterator_impl< T* >;
    using const_iterator = iterator_impl< T const* >;

    iterator       _raw_iterator(I id)       noexcept { return       iterator(this, id); }
    const_iterator _raw_iterator(I id) const noexcept { return const_iterator(this, id); }

public:

    template< class ...Args >
    void _raw_construct_n(I first, I n, Args&&... args);
    void _raw_destroy_n(I first, I n);

    void _raw_move_construct_n(RawPaged const& src, I first, I n);
    void _raw_copy_construct_n(RawPaged const& src, I first, I n);

    void _raw_move_assign_n(RawPaged const& src, I first, I n);
    void _raw_copy_assign_n(RawPaged const& src, I first, I n);
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
        _c4this->m_num_pages = 0;
        return;
    }

    // number of pages: round up to the next multiple of ps
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


template< class T, class I, I Alignment, class RawPaged >
template< class ...Args >
C4_ALWAYS_INLINE void
_raw_paged_crtp< T, I, Alignment, RawPaged >::
_raw_construct_n(I first, I n, Args&&... args)
{
    const I pg_sz = _c4this->page_size();
    I first_id = _c4this->_raw_id(first);
    I count = 0; // num elms handled so far
    I pg = _c4this->_raw_pg(first);
    while(count < n)
    {
        C4_ASSERT(pg <= _c4this->_raw_pg(first + n));
        const I missing = n - count;
        I pn = pg_sz - first_id;
        pn = pn < missing ? pn : missing;  // num elms to handle in this page
        c4::construct_n(_c4this->m_pages[pg] + first_id, pn, std::forward< Args >(args)...);
        pg++;
        first_id = 0;
        count += pn;
    }
}

template< class T, class I, I Alignment, class RawPaged >
C4_ALWAYS_INLINE void
_raw_paged_crtp< T, I, Alignment, RawPaged >::
_raw_destroy_n(I first, I n)
{
    C4_NOT_IMPLEMENTED();
}

template< class T, class I, I Alignment, class RawPaged >
C4_ALWAYS_INLINE void
_raw_paged_crtp< T, I, Alignment, RawPaged >::
_raw_move_construct_n(RawPaged const& src, I first, I n)
{
    C4_NOT_IMPLEMENTED();
}

template< class T, class I, I Alignment, class RawPaged >
C4_ALWAYS_INLINE void
_raw_paged_crtp< T, I, Alignment, RawPaged >::
_raw_copy_construct_n(RawPaged const& src, I first, I n)
{
    C4_NOT_IMPLEMENTED();
}

template< class T, class I, I Alignment, class RawPaged >
C4_ALWAYS_INLINE void
_raw_paged_crtp< T, I, Alignment, RawPaged >::
_raw_move_assign_n(RawPaged const& src, I first, I n)
{
    C4_NOT_IMPLEMENTED();
}

template< class T, class I, I Alignment, class RawPaged >
C4_ALWAYS_INLINE void
_raw_paged_crtp< T, I, Alignment, RawPaged >::
_raw_copy_assign_n(RawPaged const& src, I first, I n)
{
    C4_NOT_IMPLEMENTED();
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

    enum : I {
        //! id mask: all the bits up to PageSize. Use to extract the position of an index within a page.
        _raw_idmask = I(PageSize) - I(1),
        //! page lsb: the number of bits by bits complementary to PageSize. Use to extract the page of an index.
        _raw_pglsb  = lsb11< I, PageSize >::value
    };

    constexpr static I _raw_pg(I const i) { return i >> _raw_pglsb; }
    constexpr static I _raw_id(I const i) { return i &  _raw_idmask; }

    T    **m_pages;      //< array containing the pages
    I      m_num_pages;  //< number of current pages in the array
    Alloc  m_allocator;

public:

    _c4_DEFINE_ARRAY_TYPES_WITHOUT_ITERATOR(T, I)
    using storage_traits = raw_storage_traits< raw_paged, paged_t >;
    using allocator_type = Alloc;
    using allocator_traits = std::allocator_traits< Alloc >;

public:

    raw_paged() : raw_paged(0) {}
    raw_paged(Alloc const& a) : raw_paged(0, a) {}

    raw_paged(I cap) : m_pages(nullptr), m_num_pages(0), m_allocator()
    {
        crtp_base::_raw_resize(cap);
    }
    raw_paged(I cap, Alloc const& a) : m_pages(nullptr), m_num_pages(0), m_allocator(a)
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
        C4_XASSERT(pg >= 0 && pg < m_num_pages);
        C4_XASSERT(id >= 0 && id < PageSize);
        return m_pages[pg][id];
    }
    C4_ALWAYS_INLINE T const& operator[] (I i) const C4_NOEXCEPT_X
    {
        C4_XASSERT(i < crtp_base::capacity());
        I pg = i >> _raw_pglsb;
        I id = i & _raw_idmask;
        C4_XASSERT(pg >= 0 && pg < m_num_pages);
        C4_XASSERT(id >= 0 && id < PageSize);
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
struct raw_paged< T, 0, I, Alignment, Alloc > : public _raw_paged_crtp< T, I, Alignment, raw_paged<T, 0, I, Alignment, Alloc > >
{
    using crtp_base = _raw_paged_crtp< T, I, Alignment, raw_paged<T, 0, I, Alignment, Alloc > >;
    static_assert(std::is_integral< I >::value, "");

    C4_CONSTEXPR14 I _raw_pg(const I i) const { return i >> m_page_lsb; }
    C4_CONSTEXPR14 I _raw_id(const I i) const { return i &  m_id_mask; }

    T    **m_pages;       ///< array containing the pages
    I      m_num_pages;   ///< number of current pages in the array
    I      m_id_mask;     ///< page size - 1: cannot be changed after construction.
    I      m_page_lsb;    ///< least significant bit of the page size: cannot be changed after construction.
    Alloc  m_allocator;

public:

    _c4_DEFINE_ARRAY_TYPES(T, I)
    using storage_traits = raw_storage_traits< raw_paged, paged_t >;
    using allocator_type = Alloc;
    using allocator_traits = std::allocator_traits< Alloc >;

public:

    raw_paged() : raw_paged(szconv<I>(0), default_page_size<T,I>::value) {}
    raw_paged(Alloc const& a) : raw_paged(szconv<I>(0), default_page_size<T,I>::value, a) {}

    raw_paged(I cap) : raw_paged(cap, default_page_size<T,I>::value) {}
    raw_paged(I cap, Alloc const& a) : raw_paged(cap, default_page_size<T,I>::value, a) {}

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
        C4_XASSERT(pg >= 0 && pg < m_num_pages);
        C4_XASSERT(id >= 0 && id < m_id_mask + 1);
        return m_pages[pg][id];
    }
    C4_ALWAYS_INLINE T const& operator[] (I i) const C4_NOEXCEPT_X
    {
        C4_XASSERT(i < crtp_base::capacity());
        I pg = i >> m_page_lsb;
        I id = i & m_id_mask;
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
