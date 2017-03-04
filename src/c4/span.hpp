#ifndef _C4_SPAN_HPP_
#define _C4_SPAN_HPP_

#include "c4/config.hpp"
#include "c4/error.hpp"
#include "c4/szconv.hpp"

#include <algorithm>

C4_BEGIN_NAMESPACE(c4)

/** @defgroup span_classes Span classes
 *
 * A span is a non-owning range of elements contiguously stored in memory.
 * Unlike STL's array_view, the span allows write-access to its members.
 *
 * To obtain subspans from a span, the following const member functions
 * are available:
 *  - subspan(first, num)
 *  - range(first, last)
 *  - first(num)
 *  - last(num)
 *
 * A span can also be resized via the following non-const member functions:
 *  - resize(sz)
 *  - ltrim(num)
 *  - rtrim(num)
*/

template< class T, class I=C4_SIZE_TYPE > class span;
template< class T, class I=C4_SIZE_TYPE > class spanrs;
template< class T, class I=C4_SIZE_TYPE > class etched_span;

template< class T, class I=C4_SIZE_TYPE > using cspan   = span< const T, I >; ///< @ingroup span_classes
template< class T, class I=C4_SIZE_TYPE > using cspanrs = spanrs< const T, I >; ///< @ingroup span_classes
template< class T, class I=C4_SIZE_TYPE > using cetched_span = etched_span< const T, I >; ///< @ingroup span_classes

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** a crtp base for implementing span */
template< class T, class I, class SpanImpl >
class _span_crtp
{
// some utility defines, undefined at the end of this class
#define _c4this  ((SpanImpl      *)this)
#define _c4cthis ((SpanImpl const*)this)
#define _c4ptr   ((SpanImpl      *)this)->m_ptr
#define _c4cptr  ((SpanImpl const*)this)->m_ptr
#define _c4sz    ((SpanImpl      *)this)->m_size
#define _c4csz   ((SpanImpl const*)this)->m_size

public:

    _c4_DEFINE_ARRAY_TYPES(T, I);

public:

    C4_ALWAYS_INLINE constexpr I value_size() const noexcept { return sizeof(T); }
    C4_ALWAYS_INLINE constexpr I elm_size  () const noexcept { return sizeof(T); }
    C4_ALWAYS_INLINE constexpr I type_size () const noexcept { return sizeof(T); }
    C4_ALWAYS_INLINE           I byte_size () const noexcept { return _c4csz*sizeof(T); }

    C4_ALWAYS_INLINE bool empty()    const noexcept { return _c4csz == 0; }
    C4_ALWAYS_INLINE I    size()     const noexcept { return _c4csz; }
    //C4_ALWAYS_INLINE I    capacity() const noexcept { return _c4sz; } // this must be defined by impl classes

    C4_ALWAYS_INLINE void clear() noexcept { _c4sz = 0; }

    C4_ALWAYS_INLINE T      * data()       noexcept { return _c4ptr; }
    C4_ALWAYS_INLINE T const* data() const noexcept { return _c4cptr; }

    C4_ALWAYS_INLINE       iterator  begin()       noexcept { return _c4ptr; }
    C4_ALWAYS_INLINE const_iterator  begin() const noexcept { return _c4cptr; }
    C4_ALWAYS_INLINE const_iterator cbegin() const noexcept { return _c4cptr; }

    C4_ALWAYS_INLINE       iterator  end()       noexcept { return _c4ptr  + _c4sz; }
    C4_ALWAYS_INLINE const_iterator  end() const noexcept { return _c4cptr + _c4csz; }
    C4_ALWAYS_INLINE const_iterator cend() const noexcept { return _c4cptr + _c4csz; }

    C4_ALWAYS_INLINE T      & front()       C4_NOEXCEPT_X { C4_XASSERT(!empty()); return _c4ptr [0]; }
    C4_ALWAYS_INLINE T const& front() const C4_NOEXCEPT_X { C4_XASSERT(!empty()); return _c4cptr[0]; }

    C4_ALWAYS_INLINE T      & back()       C4_NOEXCEPT_X { C4_XASSERT(!empty()); return _c4ptr [_c4sz  - 1]; }
    C4_ALWAYS_INLINE T const& back() const C4_NOEXCEPT_X { C4_XASSERT(!empty()); return _c4cptr[_c4csz - 1]; }

    C4_ALWAYS_INLINE T      & operator[] (I i)       C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < _c4sz ); return _c4ptr [i]; }
    C4_ALWAYS_INLINE T const& operator[] (I i) const C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < _c4csz); return _c4cptr[i]; }

    C4_ALWAYS_INLINE SpanImpl subspan(I first, I num) const C4_NOEXCEPT_X
    {
        C4_XASSERT(first >= 0 && first < _c4csz);
        C4_XASSERT(first + num >= 0 && first + num < _c4csz);
        return _c4cthis->_select(_c4cptr + first, num);
    }
    C4_ALWAYS_INLINE SpanImpl subspan(I first) const C4_NOEXCEPT_X ///< goes up until the end of the span
    {
        C4_XASSERT(first >= 0 && first < _c4csz);
        return _c4cthis->_select(_c4cptr + first, _c4csz - first);
    }

    C4_ALWAYS_INLINE SpanImpl range(I first, I last) const C4_NOEXCEPT_X ///< last element is NOT included
    {
        C4_XASSERT(first >= 0 && first < _c4csz);
        C4_XASSERT(last >= 0 && last <= _c4csz);
        C4_XASSERT(last >= first);
        return _c4cthis->_select(_c4cptr + first, last - first);
    }
    C4_ALWAYS_INLINE SpanImpl range(I first) const C4_NOEXCEPT_X ///< goes up until the end of the span
    {
        C4_XASSERT(first >= 0 && first < _c4csz);
        return _c4cthis->_select(_c4cptr + first, _c4csz - first);
    }

    C4_ALWAYS_INLINE SpanImpl first(I num) const C4_NOEXCEPT_X ///< get the first num elements, starting at 0
    {
        C4_XASSERT(num >= 0 && num < _c4csz);
        return _c4cthis->_select(_c4cptr, num);
    }
    C4_ALWAYS_INLINE SpanImpl last(I num) const C4_NOEXCEPT_X ///< get the last num elements, starting at size()-num
    {
        C4_XASSERT(num >= 0 && num < _c4csz);
        return _c4cthis->_select(_c4cptr + _c4csz - num, num);
    }

    C4_ALWAYS_INLINE bool same_span(_span_crtp const& that) const noexcept
    {
        return size() == that.size() && data() == that.data();
    }
    template< class I2, class Impl2 >
    C4_ALWAYS_INLINE bool same_span(_span_crtp< T, I2, Impl2 > const& that) const noexcept
    {
        I tsz = szconv< I >(that.size()); // asserts that the size does not overflow
        return size() == tsz && data() == that.data();
    }

#undef _c4this
#undef _c4cthis
#undef _c4ptr
#undef _c4cptr
#undef _c4sz
#undef _c4csz
};

//-----------------------------------------------------------------------------
template <class T, class Il, class Ir, class _Impll, class _Implr>
inline constexpr bool operator==
(
    _span_crtp<T, Il, _Impll> const& l,
    _span_crtp<T, Ir, _Implr> const& r
)
{
#if C4_CPP >= 14
    return std::equal(l.begin(), l.end(), r.begin(), r.end());
#else
    return l.same_span(r) || std::equal(l.begin(), l.end(), r.begin());
#endif
}

template <class T, class Il, class Ir, class _Impll, class _Implr>
inline constexpr bool operator!=
(
    _span_crtp<T, Il, _Impll> const& l,
    _span_crtp<T, Ir, _Implr> const& r
)
{
    return ! (l == r);
}

//-----------------------------------------------------------------------------
template <class T, class Il, class Ir, class _Impll, class _Implr>
inline constexpr bool operator<
(
    _span_crtp<T, Il, _Impll> const& l,
    _span_crtp<T, Ir, _Implr> const& r
)
{
    return std::lexicographical_compare(l.begin(), l.end(), r.begin(), r.end());
}

template <class T, class Il, class Ir, class _Impll, class _Implr>
inline constexpr bool operator<=
(
    _span_crtp<T, Il, _Impll> const& l,
    _span_crtp<T, Ir, _Implr> const& r
)
{
    return ! (l > r);
}

//-----------------------------------------------------------------------------
template <class T, class Il, class Ir, class _Impll, class _Implr>
inline constexpr bool operator>
(
    _span_crtp<T, Il, _Impll> const& l,
    _span_crtp<T, Ir, _Implr> const& r
)
{
    return r < l;
}

//-----------------------------------------------------------------------------
template <class T, class Il, class Ir, class _Impll, class _Implr>
inline constexpr bool operator>=
(
    _span_crtp<T, Il, _Impll> const& l,
    _span_crtp<T, Ir, _Implr> const& r
)
{
    return ! (l < r);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** A span of elements contiguously stored in memory.
 * @ingroup span_classes */
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

    C4_ALWAYS_INLINE void resize(I sz) C4_NOEXCEPT_A
    {
        C4_XASSERT(sz <= m_size);
        m_size = sz;
    }

    C4_ALWAYS_INLINE void rtrim(I n) C4_NOEXCEPT_A { C4_ASSERT(n >= 0 && n < m_size); m_size -= n; }
    C4_ALWAYS_INLINE void ltrim(I n) C4_NOEXCEPT_A { C4_ASSERT(n >= 0 && n < m_size); m_size -= n; m_ptr += n; }

};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** A span resizeable up to a capacity. Subselection or resizing will keep
 * the original provided it starts at begin(). If subselection or resizing
 * change the pointer, then the original capacity information will be lost.
 *
 * Thus, resizing via resize() and ltrim() and subselecting via first()
 * or any of subspan() or range() when starting from the beginning will keep
 * the original capacity. OTOH, using last(), or any of subspan() or range()
 * with an offset from the start will remove from capacity by the corresponding
 * offset. If this is undesired, then consider using etched_span.
 *
 * @ingroup span_classes
 */
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

    C4_ALWAYS_INLINE void rtrim(I n) C4_NOEXCEPT_A { C4_ASSERT(n >= 0 && n < m_size); m_size -= n; }
    C4_ALWAYS_INLINE void ltrim(I n) C4_NOEXCEPT_A { C4_ASSERT(n >= 0 && n < m_size); m_size -= n; m_ptr += n; m_capacity -= n; }

};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** A span which always retains the information about the original range it
 * was taken from. The resizing methods resize(), ltrim(), rtrim() as well
 * as the subselection methods subspan(), range(), first() and last() can be
 * used at will without loosing the original span, which can always
 * be recovered by calling original().
 *
 * @ingroup span_classes
 */
template< class T, class I >
class etched_span : public _span_crtp<T, I, etched_span<T, I>>
{
    friend class _span_crtp<T, I, etched_span<T, I>>;

    T *m_ptr;     ///< the current ptr. the original ptr is (m_ptr - m_offset).
    I m_size;     ///< the current size. the original size is unrecoverable.
    I m_capacity; ///< the current capacity. the original capacity is (m_capacity + m_offset).
    I m_offset;   ///< the offset of the current m_ptr to the start of the original memory block.

    C4_ALWAYS_INLINE etched_span _select(T *p, I sz) const noexcept
    {
        auto delta = p - m_ptr;
        return etched_span(p, sz, m_capacity - delta, m_offset + delta);
    }

public:

    _c4_DEFINE_ARRAY_TYPES(T, I)

    C4_ALWAYS_INLINE operator span< T, I > () const noexcept { return span< T, I >(m_ptr, m_size); }
    C4_ALWAYS_INLINE operator spanrs< T, I > () const noexcept { return spanrs< T, I >(m_ptr, m_size, m_capacity); }
    C4_ALWAYS_INLINE operator etched_span< const T, I > () const noexcept { return etched_span< const T, I >(m_ptr, m_size, m_capacity, m_offset); }

public:

    C4_ALWAYS_INLINE etched_span() noexcept : m_ptr{nullptr}, m_size{0}, m_capacity{0}, m_offset{0} {}
    C4_ALWAYS_INLINE etched_span(T *p, I sz) noexcept : m_ptr{p}, m_size{sz}, m_capacity{sz}, m_offset{0} {}
    C4_ALWAYS_INLINE etched_span(T *p, I sz, I cap) noexcept : m_ptr{p}, m_size{sz}, m_capacity{cap}, m_offset{0} {}
    C4_ALWAYS_INLINE etched_span(T *p, I sz, I cap, I offs) noexcept : m_ptr{p}, m_size{sz}, m_capacity{cap}, m_offset{offs} {}
    template< size_t N >
    C4_ALWAYS_INLINE etched_span(T (&arr)[N]) noexcept : m_ptr{arr}, m_size{N}, m_capacity{N}, m_offset{0} {}

    etched_span(etched_span const&) = default;
    etched_span(etched_span     &&) = default;

    etched_span& operator= (etched_span const&) = default;
    etched_span& operator= (etched_span     &&) = default;

public:

    C4_ALWAYS_INLINE I offset() const noexcept { return m_offset; }
    C4_ALWAYS_INLINE I capacity() const noexcept { return m_capacity; }

    C4_ALWAYS_INLINE void resize(I sz) C4_NOEXCEPT_A { C4_ASSERT(sz <= m_capacity); m_size = sz; }

    C4_ALWAYS_INLINE void rtrim(I n) C4_NOEXCEPT_A { C4_ASSERT(n >= 0 && n < m_size); m_size -= n; }
    C4_ALWAYS_INLINE void ltrim(I n) C4_NOEXCEPT_A { C4_ASSERT(n >= 0 && n < m_size); m_size -= n; m_ptr += n; m_offset += n; m_capacity -= n; }

    /** recover the original span as an etched_span */
    C4_ALWAYS_INLINE etched_span original() const
    {
        return etched_span(m_ptr - m_offset, m_capacity + m_offset, m_capacity + m_offset, 0);
    }
    /** recover the original span as a different span type. Example: spanrs<...> orig = s.original< spanrs >(); */
    template< template< class, class > class OtherSpanType >
    C4_ALWAYS_INLINE OtherSpanType< T, I > original()
    {
        return OtherSpanType< T, I >(m_ptr - m_offset, m_capacity + m_offset);
    }
};

C4_END_NAMESPACE(c4)

#endif /* _C4_SPAN_HPP_ */
