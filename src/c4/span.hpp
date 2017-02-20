#ifndef _C4_SPAN_HPP_
#define _C4_SPAN_HPP_

#include "c4/config.hpp"
#include "c4/error.hpp"

/** @todo add a paged_span */

C4_BEGIN_NAMESPACE(c4)

template< class T, class I = uint32_t > class span;
template< class T, class I = uint32_t > class spanrs;

template< class T, class I = uint32_t > using cspan   = span< const T, I >;
template< class T, class I = uint32_t > using cspanrs = spanrs< const T, I >;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** a crtp base for implementing span */
template< class T, class I, class SpanImpl >
class _span_crtp
{
#define _c4this ((SpanImpl*)this)
#define _c4ptr  ((SpanImpl*)this)->m_ptr
#define _c4sz   ((SpanImpl*)this)->m_size

public:

    _c4_DEFINE_ARRAY_TYPES(T, I);

public:

    C4_ALWAYS_INLINE constexpr I value_size() const noexcept { return sizeof(T); }
    C4_ALWAYS_INLINE constexpr I elm_size  () const noexcept { return sizeof(T); }
    C4_ALWAYS_INLINE constexpr I type_size () const noexcept { return sizeof(T); }
    C4_ALWAYS_INLINE           I byte_size () const noexcept { return _c4sz*sizeof(T); }

    C4_ALWAYS_INLINE bool empty()    const noexcept { return _c4sz == 0; }
    C4_ALWAYS_INLINE I    size()     const noexcept { return _c4sz; }
    //C4_ALWAYS_INLINE I    capacity() const noexcept { return _c4sz; } // this must be defined by impl classes

    C4_ALWAYS_INLINE void clear() noexcept { _c4sz = 0; }

    C4_ALWAYS_INLINE T      * data()       noexcept { return _c4ptr; }
    C4_ALWAYS_INLINE T const* data() const noexcept { return _c4ptr; }

    C4_ALWAYS_INLINE       iterator  begin()       noexcept { return _c4ptr; }
    C4_ALWAYS_INLINE const_iterator  begin() const noexcept { return _c4ptr; }
    C4_ALWAYS_INLINE const_iterator cbegin() const noexcept { return _c4ptr; }

    C4_ALWAYS_INLINE       iterator  end()       noexcept { return _c4ptr + _c4sz; }
    C4_ALWAYS_INLINE const_iterator  end() const noexcept { return _c4ptr + _c4sz; }
    C4_ALWAYS_INLINE const_iterator cend() const noexcept { return _c4ptr + _c4sz; }

    C4_ALWAYS_INLINE T&          front()       C4_NOEXCEPT_X { C4_XASSERT(!empty()); return _c4ptr[0]; }
    C4_ALWAYS_INLINE fastcref<T> front() const C4_NOEXCEPT_X { C4_XASSERT(!empty()); return _c4ptr[0]; }

    C4_ALWAYS_INLINE T&          back()       C4_NOEXCEPT_X { C4_XASSERT(!empty()); return _c4ptr[_c4sz - 1]; }
    C4_ALWAYS_INLINE fastcref<T> back() const C4_NOEXCEPT_X { C4_XASSERT(!empty()); return _c4ptr[_c4sz - 1]; }

    C4_ALWAYS_INLINE T&          operator[] (I i)       C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < _c4sz); return _c4ptr[i]; }
    C4_ALWAYS_INLINE fastcref<T> operator[] (I i) const C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < _c4sz); return _c4ptr[i]; }

    C4_ALWAYS_INLINE SpanImpl subspan(I first) const C4_NOEXCEPT_X
    {
        C4_XASSERT(first >= 0 && first < _c4sz);
        return _c4this->_select(_c4ptr + first, _c4sz - first);
    }
    C4_ALWAYS_INLINE SpanImpl subspan(I first, I num) const C4_NOEXCEPT_X
    {
        C4_XASSERT(first >= 0 && first < _c4sz);
        C4_XASSERT(first + num >= 0 && first + num < _c4sz);
        return _c4this->_select(_c4ptr + first, num);
    }
    C4_ALWAYS_INLINE SpanImpl range(I first, I last) const C4_NOEXCEPT_X
    {
        C4_XASSERT(first >= 0 && first < _c4sz);
        C4_XASSERT(last >= 0 && last < _c4sz);
        return _c4this->_select(_c4ptr + first, last - first);
    }

    C4_ALWAYS_INLINE SpanImpl first(I num) const C4_NOEXCEPT_X
    {
        C4_XASSERT(num >= 0 && num < _c4sz);
        return _c4this->_select(_c4ptr, num);
    }
    C4_ALWAYS_INLINE SpanImpl last(I num) const C4_NOEXCEPT_X
    {
        C4_XASSERT(num >= 0 && num < _c4sz);
        return _c4this->_select(_c4ptr + _c4sz - num, num);
    }

    C4_ALWAYS_INLINE bool same_span(_span_crtp const& that) const noexcept
    {
        return size() == that.size() && data() == that.data();
    }
    template< class I2, class Impl2 >
    C4_ALWAYS_INLINE bool same_span(_span_crtp< T, I2, Impl2 > const& that) const noexcept
    {
        return size() == that.size() && data() == that.data();
    }

#undef _c4this
#undef _c4ptr
#undef _c4sz
};

//-----------------------------------------------------------------------------
template <class T, class Il, class Ir, class _Impll, class _Implr>
inline constexpr bool operator==
(
    const _span_crtp<T, Il, _Impll>& l,
    const _span_crtp<T, Ir, _Implr>& r
)
{
#if C4_CPP >= 14
    return std::equal(l.begin(), l.end(), r.begin(), r.end());
#else
    return l.same_span(r) && std::equal(l.begin(), l.end(), r.begin());
#endif
}

template <class T, class Il, class Ir, class _Impll, class _Implr>
inline constexpr bool operator!=
(
    const _span_crtp<T, Il, _Impll>& l,
    const _span_crtp<T, Ir, _Implr>& r
)
{
    return ! (l == r);
}

//-----------------------------------------------------------------------------
template <class T, class Il, class Ir, class _Impll, class _Implr>
inline constexpr bool operator<
(
    const _span_crtp<T, Il, _Impll>& l,
    const _span_crtp<T, Ir, _Implr>& r
)
{
    return std::lexicographical_compare(l.begin(), l.end(), r.begin(), r.end());
}

template <class T, class Il, class Ir, class _Impll, class _Implr>
inline constexpr bool operator<=
(
    const _span_crtp<T, Il, _Impll>& l,
    const _span_crtp<T, Ir, _Implr>& r
)
{
    return ! (l > r);
}

//-----------------------------------------------------------------------------
template <class T, class Il, class Ir, class _Impll, class _Implr>
inline constexpr bool operator>
(
    const _span_crtp<T, Il, _Impll>& l,
    const _span_crtp<T, Ir, _Implr>& r
)
{
    return r < l;
}

//-----------------------------------------------------------------------------
template <class T, class Il, class Ir, class _Impll, class _Implr>
inline constexpr bool operator>=
(
    const _span_crtp<T, Il, _Impll>& l,
    const _span_crtp<T, Ir, _Implr>& r
)
{
    return !(l < r);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template< class T, class I >
class span : public _span_crtp<T, I, span<T, I>>
{
    friend class _span_crtp<T, I, span<T, I>>;

    T *m_ptr;
    I m_size;

    C4_ALWAYS_INLINE span _select(T *p, I sz) const { return span(p, sz); }

public:

    _c4_DEFINE_ARRAY_TYPES(T, I)

    C4_ALWAYS_INLINE operator span< const T, I > () const noexcept { return span< const T, I >((T const*)m_ptr, m_size); }

public:

    C4_ALWAYS_INLINE span() noexcept : m_ptr{nullptr}, m_size{0} {}
    C4_ALWAYS_INLINE span(T *p, I sz) noexcept : m_ptr{p}, m_size{sz} {}
    template< size_t N >
    C4_ALWAYS_INLINE span(T (&arr)[N]) noexcept : m_ptr{arr}, m_size{N} {}

    span(span const&) = default;
    span(span     &&) = default;

    span& operator= (span const&) = default;
    span& operator= (span     &&) = default;

public:

    C4_ALWAYS_INLINE I capacity() const noexcept { return m_size; }

    C4_ALWAYS_INLINE I resize(I sz) C4_NOEXCEPT_A
    {
        C4_XASSERT(sz <= m_size);
        m_size = sz;
    }

    C4_ALWAYS_INLINE void rtrim(I n) C4_NOEXCEPT_X { C4_XASSERT(n >= 0 && n < m_size); m_size -= n; }
    C4_ALWAYS_INLINE void ltrim(I n) C4_NOEXCEPT_X { C4_XASSERT(n >= 0 && n < m_size); m_size -= n; m_ptr += n; }

};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** a span resizeable up to a capacity */
template< class T, class I >
class spanrs : public _span_crtp<T, I, spanrs<T, I>>
{
    friend class _span_crtp<T, I, spanrs<T, I>>;

    T *m_ptr;
    I m_size;
    I m_capacity;
    
    C4_ALWAYS_INLINE spanrs _select(T *p, I sz) const noexcept { return spanrs(p, sz, m_capacity - (p - m_ptr)); }

public:

    _c4_DEFINE_ARRAY_TYPES(T, I)

    C4_ALWAYS_INLINE operator spanrs< const T, I > () const noexcept { return spanrs< const T, I >((T const*)m_ptr, m_size, m_capacity); }
    C4_ALWAYS_INLINE operator span< T, I > () const noexcept { return span< T, I >(m_ptr, m_size); }

public:

    C4_ALWAYS_INLINE spanrs() noexcept : m_ptr{nullptr}, m_size{0}, m_capacity{0} {}
    C4_ALWAYS_INLINE spanrs(T *p, I sz) noexcept : m_ptr{p}, m_size{sz}, m_capacity{sz} {}
    C4_ALWAYS_INLINE spanrs(T *p, I sz, I cap) noexcept : m_ptr{p}, m_size{sz}, m_capacity{cap} {}
    template< size_t N >
    C4_ALWAYS_INLINE spanrs(T (&arr)[N]) noexcept : m_ptr{arr}, m_size{N}, m_capacity{N} {}

    spanrs(spanrs const&) = default;
    spanrs(spanrs     &&) = default;

    spanrs& operator= (spanrs const&) = default;
    spanrs& operator= (spanrs     &&) = default;

public:

    C4_ALWAYS_INLINE I capacity() const noexcept { return m_capacity; }

    C4_ALWAYS_INLINE I resize(I sz) C4_NOEXCEPT_A
    {
        C4_ASSERT(sz <= m_capacity);
        m_size = sz;
    }

    C4_ALWAYS_INLINE void rtrim(I n) C4_NOEXCEPT_X { C4_XASSERT(n >= 0 && n < m_size); m_size -= n; }
    C4_ALWAYS_INLINE void ltrim(I n) C4_NOEXCEPT_X { C4_XASSERT(n >= 0 && n < m_size); m_size -= n; m_ptr += n; m_capacity -= n; }

};

C4_END_NAMESPACE(c4)

#endif /* _C4_SPAN_HPP_ */
