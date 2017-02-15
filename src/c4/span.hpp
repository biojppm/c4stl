#ifndef _C4_SPAN_HPP_
#define _C4_SPAN_HPP_

#include "c4/config.hpp"
#include "c4/storage/base.hpp"
#include "c4/error.hpp"

/** @todo add a paged_span */

C4_BEGIN_NAMESPACE(c4)

template< class T, class I = uint32_t > class span;
template< class T, class I = uint32_t > class spanrs;

template< class T, class I > using cspan   = span< const T, I >;
template< class T, class I > using cspanrs = spanrs< const T, I >;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template< class T, class I, class SpanImpl >
class _span_crtp
{
#define _c4ptr ((SpanImpl*)this)->m_ptr
#define _c4sz ((SpanImpl*)this)->m_size

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

    C4_ALWAYS_INLINE T&          front()       C4_NOEXCEPT { C4_XASSERT(!empty()); return _c4ptr[0]; }
    C4_ALWAYS_INLINE fastcref<T> front() const C4_NOEXCEPT { C4_XASSERT(!empty()); return _c4ptr[0]; }

    C4_ALWAYS_INLINE T&          back()       C4_NOEXCEPT { C4_XASSERT(!empty()); return _c4ptr[_c4sz - 1]; }
    C4_ALWAYS_INLINE fastcref<T> back() const C4_NOEXCEPT { C4_XASSERT(!empty()); return _c4ptr[_c4sz - 1]; }

    C4_ALWAYS_INLINE T&          operator[] (I i)       C4_NOEXCEPT { C4_XASSERT(i >= 0 && i < _c4sz); return _c4ptr[i]; }
    C4_ALWAYS_INLINE fastcref<T> operator[] (I i) const C4_NOEXCEPT { C4_XASSERT(i >= 0 && i < _c4sz); return _c4ptr[i]; }

    C4_ALWAYS_INLINE void ltrim(I n) C4_NOEXCEPT { C4_XASSERT(n >= 0 && n < _c4sz); _c4ptr += n; }
    C4_ALWAYS_INLINE void rtrim(I n) C4_NOEXCEPT { C4_XASSERT(n >= 0 && n < _c4sz); _c4sz -= n; }

    C4_ALWAYS_INLINE SpanImpl subspan(I first) const C4_NOEXCEPT
    {
        C4_XASSERT(first >= 0 && first < _c4sz);
        return SpanImpl::_select(_c4ptr + first, _c4sz - first);
    }
    C4_ALWAYS_INLINE SpanImpl subspan(I first, I num = I(-1)) const C4_NOEXCEPT
    {
        C4_XASSERT(first >= 0 && first < _c4sz);
        num = num == I(-1) ? _c4sz - num : num;
        C4_XASSERT(first + num >= 0 && first + num < _c4sz);
        return SpanImpl::_select(_c4ptr + first, num);
    }

    C4_ALWAYS_INLINE SpanImpl first(I num) const C4_NOEXCEPT
    {
        C4_XASSERT(num >= 0 && num < _c4sz);
        return SpanImpl::_select(_c4ptr, num);
    }
    C4_ALWAYS_INLINE SpanImpl last(I num) const C4_NOEXCEPT
    {
        C4_XASSERT(num >= 0 && num < _c4sz);
        return SpanImpl::_select(_c4ptr + _c4sz - num, num);
    }

#undef _c4ptr
#undef _c4sz
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template< class T, class I >
class span : public _span_crtp<T, I, span<T, I>>
{
    friend class _span_crtp<T, I, span<T, I>>;

    T *m_ptr;
    I m_size;

public:

    _c4_DEFINE_ARRAY_TYPES(T, I)

    C4_ALWAYS_INLINE operator span< const T, I > () const { return span< const T, I >((T const*)m_ptr, m_size); }

    C4_ALWAYS_INLINE span() : m_ptr{nullptr}, m_size{0} {}
    C4_ALWAYS_INLINE span(T *p, I sz) : m_ptr{p}, m_size{sz} {}
    template< size_t N >
    C4_ALWAYS_INLINE span(T (&arr)[N]) : m_ptr{arr}, m_size{N} {}

    span(span const&) = default;
    span(span     &&) = default;

    span& operator= (span const&) = default;
    span& operator= (span     &&) = default;

public:

    C4_ALWAYS_INLINE span _select(T *p, I sz) const { return span(p, sz); }

    C4_ALWAYS_INLINE I capacity() const noexcept { return m_size; }

    C4_ALWAYS_INLINE I resize(I sz) C4_NOEXCEPT
    {
        C4_XASSERT(sz <= m_size);
        m_size = sz;
    }
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** a span resizeable up to a capacity */
template< class T, class I >
class spanrs : public _span_crtp<T, I, spanrs<T, I>>
{
    friend class _span_crtp<T, I, span<T, I>>;

    T *m_ptr;
    I m_size;
    I m_capacity;

public:

    _c4_DEFINE_ARRAY_TYPES(T, I)

    C4_ALWAYS_INLINE operator spanrs< const T, I > () const { return spanrs< const T, I >((T const*)m_ptr, m_size, m_capacity); }
    C4_ALWAYS_INLINE operator span< T, I > () const { return span< T, I >(m_ptr, m_size); }
    C4_ALWAYS_INLINE operator span< const T, I > () const { return span< const T, I >((T const*)m_ptr, m_size); }

public:

    C4_ALWAYS_INLINE spanrs() : m_ptr{nullptr}, m_size{0}, m_capacity{0} {}
    C4_ALWAYS_INLINE spanrs(T *p, I sz) : m_ptr{p}, m_size{sz}, m_capacity{sz} {}
    C4_ALWAYS_INLINE spanrs(T *p, I sz, I cap) : m_ptr{p}, m_size{sz}, m_capacity{sz} {}
    template< size_t N >
    C4_ALWAYS_INLINE spanrs(T (&arr)[N]) : m_ptr{arr}, m_size{N}, m_capacity{N} {}

    spanrs(spanrs const&) = default;
    spanrs(spanrs     &&) = default;

    spanrs& operator= (spanrs const&) = default;
    spanrs& operator= (spanrs     &&) = default;

public:

    C4_ALWAYS_INLINE span _select(T *p, I sz) const { return spanrs(p, sz, m_capacity); }

    C4_ALWAYS_INLINE I capacity() const noexcept { return m_capacity; }

    C4_ALWAYS_INLINE I resize(I sz) C4_NOEXCEPT
    {
        C4_XASSERT(sz <= m_capacity);
        m_size = sz;
    }

};

C4_END_NAMESPACE(c4)

#endif /* _C4_SPAN_HPP_ */
