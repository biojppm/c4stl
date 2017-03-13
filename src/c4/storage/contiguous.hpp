#ifndef _C4_STORAGE_CONTIGUOUS_HPP_
#define _C4_STORAGE_CONTIGUOUS_HPP_

#include "c4/config.hpp"
#include "c4/storage/raw.hpp"
#include "c4/szconv.hpp"
#include "c4/span.hpp"

C4_BEGIN_NAMESPACE(c4)

// forward declarations
template< class T, class I > class span;
template< class T, class I > class spanrs;
template< class T, class I > class etched_span;

template< class T, class I > using cspan = span< const T, I >;
template< class T, class I > using cspanrs = spanrs< const T, I >;
template< class T, class I > using cetched_span = etched_span< const T, I >;

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

// utility defines, undefined below
/// @cond dev
#define _c4this  static_cast< Storage      * >(this)
#define _c4cthis static_cast< Storage const* >(this)
/// @endcond

/** CRTP base for non-resizeable contiguous storage */
template< class T, class I, class Storage >
struct _ctg_crtp
{

    _c4_DEFINE_ARRAY_TYPES(T, I)

    C4_ALWAYS_INLINE constexpr I type_size() const { return sizeof(T); }
    C4_ALWAYS_INLINE           I byte_size() const { return _c4this->size() * sizeof(T); }

public:

    // span interoperation

    using   span_type = span< T, I >;
    using  cspan_type = span< const T, I >;

    C4_ALWAYS_INLINE operator  span_type ()       { return  span_type( _c4this->data(),  _c4this->size()); }
    C4_ALWAYS_INLINE operator cspan_type () const { return cspan_type(_c4cthis->data(), _c4cthis->size()); }

     span_type get_span(I first = 0)       {  span_type s = *this; s = s.subspan(first); return s; }
    cspan_type get_span(I first = 0) const { cspan_type s = *this; s = s.subspan(first); return s; }

     span_type get_span(I first, I num)       {  span_type s = *this; s = s.subspan(first, num); return s; }
    cspan_type get_span(I first, I num) const { cspan_type s = *this; s = s.subspan(first, num); return s; }


    // spanrs interoperation

    using  spanrs_type = spanrs<       T, I >;
    using cspanrs_type = spanrs< const T, I >;

    C4_ALWAYS_INLINE operator  spanrs_type ()       { return  spanrs_type( _c4this->data(),  _c4this->size(),  _c4this->capacity()); }
    C4_ALWAYS_INLINE operator cspanrs_type () const { return cspanrs_type(_c4cthis->data(), _c4cthis->size(), _c4cthis->capacity()); }

     spanrs_type get_spanrs(I first = 0)       {  spanrs_type s = *this; s = s.subspan(first); return s; }
    cspanrs_type get_spanrs(I first = 0) const { cspanrs_type s = *this; s = s.subspan(first); return s; }

     spanrs_type get_spanrs(I first, I num)       {  spanrs_type s = *this; s = s.subspan(first, num); return s; }
    cspanrs_type get_spanrs(I first, I num) const { cspanrs_type s = *this; s = s.subspan(first, num); return s; }


    // etched_span interoperation

    using  etched_span_type = etched_span<       T, I >;
    using cetched_span_type = etched_span< const T, I >;

    C4_ALWAYS_INLINE operator  etched_span_type ()       { return  etched_span_type( _c4this->data(),  _c4this->size(),  _c4this->capacity(), 0); }
    C4_ALWAYS_INLINE operator cetched_span_type () const { return cetched_span_type(_c4cthis->data(), _c4cthis->size(), _c4cthis->capacity(), 0); }

     etched_span_type get_etched_span(I first = 0)       {  etched_span_type s = *this; s = s.subspan(first); return s; }
    cetched_span_type get_etched_span(I first = 0) const { cetched_span_type s = *this; s = s.subspan(first); return s; }

     etched_span_type get_etched_span(I first, I num)       {  etched_span_type s = *this; s = s.subspan(first, num); return s; }
    cetched_span_type get_etched_span(I first, I num) const { cetched_span_type s = *this; s = s.subspan(first, num); return s; }

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

    fixed_size(fixed_size const& that) { c4::copy_assign_n(m_arr, that.m_arr, N); }
    fixed_size(fixed_size     && that) { c4::move_assign_n(m_arr, that.m_arr, N); }

    fixed_size& operator= (fixed_size const& that) { c4::copy_assign_n(m_arr, that.m_arr, N); return *this; }
    fixed_size& operator= (fixed_size     && that) { c4::move_assign_n(m_arr, that.m_arr, N); return *this; }

    fixed_size (cspan< T, I > const& v) { C4_ASSERT(v.size() == N); c4::copy_assign_n(m_arr, v.data(), v.size()); }
    void assign(cspan< T, I > const& v) { C4_ASSERT(v.size() == N); c4::copy_assign_n(m_arr, v.data(), v.size()); }

    fixed_size (aggregate_t, std::initializer_list< T > il) { C4_ASSERT(il.size() == N); c4::copy_assign_n(m_arr, il.begin(), il.size()); }
    void assign(aggregate_t, std::initializer_list< T > il) { C4_ASSERT(il.size() == N); c4::copy_assign_n(m_arr, il.begin(), il.size()); }

    fixed_size (T const (&v)[N]) { c4::copy_assign_n(m_arr, v, N); }
    void assign(T const (&v)[N]) { c4::copy_assign_n(m_arr, v, N); }

    fixed_size (T const* v, I sz) { C4_ASSERT(sz == N); c4::copy_assign_n(m_arr, v, sz); }
    void assign(T const* v, I sz) { C4_ASSERT(sz == N); c4::copy_assign_n(m_arr, v, sz); }

    /** @warning do NOT pass in std::move() arguments. */
    template< class... Args > fixed_size(varargs_t, Args&&... args)
    {
         c4::copy_assign_n(m_arr, std::forward< Args >(args)...);
    }
    /** @warning do NOT pass in std::move() arguments. */
    template< class... Args > void assign(varargs_t, Args&&... args)
    {
        c4::copy_assign_n(m_arr, std::forward< Args >(args)...);
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

    C4_ALWAYS_INLINE                 reverse_iterator  rbegin()       noexcept { return reverse_iterator(m_arr + I(N)); }
    C4_ALWAYS_INLINE constexpr const_reverse_iterator  rbegin() const noexcept { return reverse_iterator(m_arr + I(N)); }
    C4_ALWAYS_INLINE constexpr const_reverse_iterator crbegin() const noexcept { return reverse_iterator(m_arr + I(N)); }

    C4_ALWAYS_INLINE                 reverse_iterator  rend()       noexcept { return const_reverse_iterator(m_arr); }
    C4_ALWAYS_INLINE constexpr const_reverse_iterator  rend() const noexcept { return const_reverse_iterator(m_arr); }
    C4_ALWAYS_INLINE constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator(m_arr); }

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
        c4::destroy_n(m_arr, N);
        c4::copy_assign_n(m_arr, v, N);
    }

};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

// utility defines, undefined below
/// @cond dev
#define _c4this  static_cast< Storage      * >(this)
#define _c4cthis static_cast< Storage const* >(this)
/// @endcond


/** CRTP base for resizeable contiguous storage */
template< class T, class I, class Storage >
struct _ctgrs_crtp : public _ctg_crtp< T, I, Storage >
{
    using _ctg_crtp< T, I, Storage >::is_valid_iterator;

public:

    _c4_DEFINE_ARRAY_TYPES(T, I)

public:


    // emplace

    template< class... Args >
    iterator emplace(const_iterator pos, Args&&... args)
    {
        C4_XASSERT(pos >= _c4this->data() && pos <= _c4this->data() + _c4this->size());
        I ipos = szconv<I>(pos - _c4this->data());
        _c4this->_growto(_c4this->size() + 1, pos);
        pos = _c4this->data() + ipos;
        construct(pos, std::forward< Args >(args)...);
        ++_c4this->size();
        return (iterator)pos;
    }
    template< class... Args >
    void emplace_back(Args&& ...a)
    {
        _c4this->_growto(_c4this->size() + 1);
        construct(_c4this->data() + _c4this->size(), std::forward< Args >(a)...);
        ++_c4this->size();
    }
    template< class... Args >
    void emplace_front(Args&& ...a)
    {
        _c4this->_growto(_c4this->size() + 1);
        construct(_c4this->data(), std::forward< Args >(a)...);
        ++_c4this->size();
    }


    // push

    void push_back(T const& val)
    {
        _c4this->_growto(_c4this->size() + 1);
        construct(_c4this->data() + _c4this->size(), val);
        ++_c4this->size();
    }
    void push_back(T && val)
    {
        _c4this->_growto(_c4this->size() + 1);
        construct(_c4this->data() + _c4this->size(), std::move(val));
        ++_c4this->size();
    }

    void push_front(T const& val)
    {
        _c4this->_growto(_c4this->size() + 1, _c4this->data());
        construct(_c4this->data(), val);
        ++_c4this->size();
    }
    void push_front(T && val)
    {
        _c4this->_growto(_c4this->size() + 1, _c4this->data());
        construct(_c4this->data(), std::move(val));
        ++_c4this->size();
    }


    // pop

    void pop_back()
    {
        C4_XASSERT(_c4this->size() > 0);
        _c4this->_growto(_c4this->size() - 1);
        --_c4this->size();
    }
    void pop_front()
    {
        C4_XASSERT(_c4this->size() > 0);
        _c4this->_growto(_c4this->size() - 1, _c4this->data());
        --_c4this->size();
    }


    // insert

    iterator insert(const_iterator pos, T const& value)
    {
        C4_XASSERT(is_valid_iterator(pos));
        I ipos = I(pos - _c4this->data());
        _c4this->_growto(_c4this->size() + 1, pos);
        pos = _c4this->data() + ipos;
        construct(pos, value);
        ++_c4this->size();
        return (iterator)pos;
    }
    iterator insert(const_iterator pos, T&& value)
    {
        C4_XASSERT(is_valid_iterator(pos));
        I ipos = szconv< I >(pos - _c4this->data());
        _c4this->_growto(_c4this->size() + 1, pos);
        pos = _c4this->data() + ipos;
        construct(pos, std::move(value));
        ++_c4this->size();
        return (iterator)pos;
    }
    iterator insert(const_iterator pos, I count, T const& value)
    {
        C4_XASSERT(is_valid_iterator(pos));
        I ipos = I(pos - _c4this->data());
        _c4this->_growto(_c4this->size() + count, pos);
        pos = _c4this->data() + ipos;
        construct_n(pos, value, count);
        _c4this->size() += count;
        return (iterator)pos;
    }
    template< class InputIt >
    iterator insert(const_iterator pos, InputIt first, InputIt last)
    {
        C4_XASSERT(is_valid_iterator(pos));
        I ipos = I(pos - _c4this->data());
        I count = (I)std::distance(first, last);
        _c4this->_growto(_c4this->size() + count, pos);
        pos = _c4this->data() + ipos;
        for(I i = 0; first != last; ++first, ++i)
        {
            construct(pos + i, first);
        }
        _c4this->size() += count;
        return (iterator)pos;
    }
    iterator insert(const_iterator pos, aggregate_t, std::initializer_list<T> ilist)
    {
        C4_XASSERT(is_valid_iterator(pos));
        I ipos = I(pos - _c4this->data());
        _c4this->_growto(_c4this->size() + ilist.size(), pos);
        pos = _c4this->data() + ipos;
        I i = 0;
        for(auto const& v : ilist)
        {
            construct((pos++) + i, v);
        }
        pos = _c4this->data() + ipos;
        _c4this->size() += ilist.size();
        return (iterator)pos;
    }


    // erase

    /** removes the element at pos */
    iterator erase(const_iterator pos)
    {
        C4_XASSERT(is_valid_iterator(pos) && _c4this->size() > 0);
        I ipos = I(pos - _c4this->data());
        _c4this->_growto(_c4this->size() - 1, pos);
        pos = _c4this->data() + ipos;
        --_c4this->size();
        return (iterator)pos;
    }
    /** removes the elements in the range [first; last). */
    iterator erase(const_iterator first, const_iterator last)
    {
        I dist = (I)std::distance(first, last);
        if(!dist) return (iterator)first;
        C4_XASSERT(is_valid_iterator(first) && _c4this->size() >= dist);
        I ipos = I(first - _c4this->data());
        _c4this->_growto(_c4this->size() - dist, first);
        first = _c4this->data() + ipos;
        _c4this->size() -= dist;
        return (iterator)first;
    }

};

#undef _c4this
#undef _c4cthis

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

C4_END_NAMESPACE(stg)
C4_END_NAMESPACE(c4)

#endif // _C4_STORAGE_CONTIGUOUS_HPP_
