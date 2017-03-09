#ifndef _C4_STORAGE_CONTIGUOUS_HPP_
#define _C4_STORAGE_CONTIGUOUS_HPP_

#include "c4/config.hpp"
#include "c4/storage/raw.hpp"

C4_BEGIN_NAMESPACE(c4)

// forward declarations
template< class T, class I > class span;

template< class T, class I >
using cspan = span< const T, I >;

// some tag types

/** a tag type for initializing the containers with variadic arguments a la
 * initializer_list, minus the initializer_list overload problems.
 * @see */
struct aggregate_t {};
constexpr const aggregate_t aggregate{};

/** a tag type for specifying the initial capacity of allocatable contiguous storage */
struct with_capacity_t {};
constexpr const with_capacity_t with_capacity{};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

C4_BEGIN_NAMESPACE(stg)

// forward declarations
template< class T, size_t N, class I = C4_SIZE_TYPE, I Alignment = alignof(T) >
class fixed_size;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#define _c4this  static_cast< Storage      * >(this)
#define _c4cthis static_cast< Storage const* >(this)

/** CRTP base for non-resizeable contiguous storage */
template< class T, class I, class Storage >
struct _ctg_crtp
{

    _c4_DEFINE_ARRAY_TYPES(T, I)

    C4_ALWAYS_INLINE constexpr I type_size() const { return sizeof(T); }
    C4_ALWAYS_INLINE           I byte_size() const { return _c4this->size() * sizeof(T); }

public:

    using span_type = span< T, I >;
    using cspan_type = cspan< T, I >;

    C4_ALWAYS_INLINE operator  span_type ()       { return  span_type(_c4this->data(), _c4this->size()); }
    C4_ALWAYS_INLINE operator cspan_type () const { return cspan_type(_c4this->data(), _c4this->size()); }

     span_type subspan(I first = 0)       {  span_type s = *this; return s.subspan(first); }
    cspan_type subspan(I first = 0) const { cspan_type s = *this; return s.subspan(first); }

     span_type subspan(I first, I num)       {  span_type s = *this; return s.subspan(first, num); }
    cspan_type subspan(I first, I num) const { cspan_type s = *this; return s.subspan(first, num); }

public:

    C4_ALWAYS_INLINE bool is_valid_iterator(const_iterator it) const noexcept
    {
        return it >= _c4cthis->data() && it <= _c4cthis->data() + _c4cthis->size();
    }
    C4_ALWAYS_INLINE bool is_valid_index(I i) const noexcept
    {
        return i >= 0 && i < _c4cthis->size();
    }

};

#undef _c4this
#undef _c4cthis


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/** contiguous storage, fixed size+capacity (cannot resize down). */
template< class T, size_t N, class I, I Alignment >
class fixed_size : public _ctg_crtp< T, I, fixed_size< T, N, I, Alignment > >
{
public:

    alignas(Alignment) T m_arr[N];

public:

    C4_STATIC_ASSERT(N <= std::numeric_limits< I >::max());

    using base_type = _ctg_crtp< T, I, fixed_size< T, N, I, Alignment > >;
    friend class _ctg_crtp< T, I, fixed_size< T, N, I, Alignment > >;

public:

    _c4_DEFINE_ARRAY_TYPES(T, I)

    enum { alignment = Alignment };

public:

    fixed_size() {}

    fixed_size (cspan< T, I > const& v) { C4_ASSERT(v.size() == N); c4::copy_construct_n(m_arr, v.data(), v.size()); }
    void assign(cspan< T, I > const& v) { C4_ASSERT(v.size() == N); c4::copy_assign_n(m_arr, v.data(), v.size()); }

    fixed_size (aggregate_t, std::initializer_list< T > il) { C4_ASSERT(il.size() == N); c4::copy_construct_n(m_arr, il.begin(), il.size()); }
    void assign(aggregate_t, std::initializer_list< T > il) { C4_ASSERT(il.size() == N); c4::copy_assign_n(m_arr, il.begin(), il.size()); }

    fixed_size (T const (&v)[N]) { c4::copy_construct_n(m_arr, v, N); }
    void assign(T const (&v)[N]) { c4::copy_assign_n(m_arr, v, N); }

    fixed_size (T const* v, I sz) { C4_ASSERT(sz == N); c4::copy_construct_n(m_arr, v, sz); }
    void assign(T const* v, I sz) { C4_ASSERT(sz == N); c4::copy_assign_n(m_arr, v, sz); }

    /** @warning do NOT pass in std::move() arguments. */
    template< class... Args > fixed_size(Args&&... args)
    {
        c4::construct_n(m_arr, std::forward< Args >(args)...);
    }
    /** @warning do NOT pass in std::move() arguments. */
    template< class... Args > void assign(Args&&... args)
    {
        c4::destroy_n(m_arr);
        c4::construct_n(m_arr, std::forward< Args >(args)...);
    }

public:

    C4_ALWAYS_INLINE constexpr bool empty()    const noexcept { return false; }
    C4_ALWAYS_INLINE constexpr I    size()     const noexcept { return N; }
    C4_ALWAYS_INLINE constexpr I    max_size() const noexcept { return N; }
    C4_ALWAYS_INLINE constexpr I    capacity() const noexcept { return N; }

    C4_ALWAYS_INLINE                 iterator  begin()       noexcept { return m_arr; }
    C4_ALWAYS_INLINE constexpr const_iterator  begin() const noexcept { return m_arr; }
    C4_ALWAYS_INLINE constexpr const_iterator cbegin() const noexcept { return m_arr; }

    C4_ALWAYS_INLINE                 iterator  end()       noexcept { return m_arr + I(N); }
    C4_ALWAYS_INLINE constexpr const_iterator  end() const noexcept { return m_arr + I(N); }
    C4_ALWAYS_INLINE constexpr const_iterator cend() const noexcept { return m_arr + I(N); }

    C4_ALWAYS_INLINE           T      & front()       noexcept { return m_arr[0]; }
    C4_ALWAYS_INLINE constexpr T const& front() const noexcept { return m_arr[0]; }

    C4_ALWAYS_INLINE           T      & back()       noexcept { return m_arr[I(N) - 1]; }
    C4_ALWAYS_INLINE constexpr T const& back() const noexcept { return m_arr[I(N) - 1]; }

    C4_ALWAYS_INLINE           T      * data()       noexcept { return m_arr; }
    C4_ALWAYS_INLINE constexpr T const* data() const noexcept { return m_arr; }

    C4_ALWAYS_INLINE                T      & operator[] (I i)       C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < I(N)); return m_arr[i]; }
    C4_ALWAYS_INLINE C4_CONSTEXPR14 T const& operator[] (I i) const C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < I(N)); return m_arr[i]; }

    C4_ALWAYS_INLINE void fill(T const& v)
    {
        c4::copy_assign_n(m_arr, v, N);
    }

};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

C4_END_NAMESPACE(stg)
C4_END_NAMESPACE(c4)

#endif // _C4_STORAGE_CONTIGUOUS_HPP_
