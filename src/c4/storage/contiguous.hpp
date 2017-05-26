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

template< class T > class Allocator;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

C4_BEGIN_NAMESPACE(stg)

// forward declarations

template< class T, class I, class RawStorage >
class contiguous;

template< class T, size_t N, class I=C4_SIZE_TYPE, I Alignment=alignof(T) >
class fixed_size;

template< class T, size_t N, class I=C4_SIZE_TYPE, I Alignment=alignof(T) >
using fixed_capacity=contiguous< T, I, raw_fixed<T, N, I, Alignment> >;

template< class T, size_t N, class I=C4_SIZE_TYPE, I Alignment=alignof(T) >
using small_vector=contiguous< T, I, raw_small<T, N, I, Alignment> >;

template< class T, class I=C4_SIZE_TYPE, I Alignment=alignof(T) >
using vector=contiguous< T, I, raw< T, I, Alignment > >;


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

// utility defines, undefined below
/// @cond dev
#define _c4this  static_cast< Storage      * >(this)
#define _c4cthis static_cast< Storage const* >(this)
/// @endcond


/** (WIP) CRTP base for resizeable contiguous storage */
template< class T, class I, class Storage >
struct _ctgrs_crtp : public _ctg_crtp< T, I, Storage >
{
    using _ctg_crtp< T, I, Storage >::is_valid_iterator;
    using _base_crtp = _ctg_crtp< T, I, Storage >;
    _c4_DEFINE_ARRAY_TYPES(T, I)

public:


    // emplace ----------------------------------

    template< class... Args >
    C4_ALWAYS_INLINE T& emplace(const_iterator pos, Args &&... args)
    {
        iterator wpos = _growat(pos);
        construct(wpos, std::forward< Args >(args)...);
        return *wpos;
    }
    template< class... Args >
    C4_ALWAYS_INLINE T& emplace_back(Args && ...args)
    {
        iterator wpos = _growat(_c4this->end());
        construct(wpos, std::forward< Args >(args)...);
        return *wpos;
    }
    template< class... Args >
    C4_ALWAYS_INLINE T& emplace_front(Args && ...args)
    {
        iterator wpos = _growat(_c4this->begin());
        construct(wpos, std::forward< Args >(args)...);
        return *wpos;
    }


    // push -------------------------------------

    C4_ALWAYS_INLINE void push_back(T const& val)
    {
        iterator wpos = _growat(_c4this->end());
        construct(wpos, val);
        return *wpos;
    }
    C4_ALWAYS_INLINE void push_back(T && val)
    {
        iterator wpos = _growat(_c4this->end());
        construct(wpos, std::move(val));
        return *wpos;
    }

    C4_ALWAYS_INLINE void push_front(T const& val)
    {
        iterator wpos = _growat(_c4this->begin());
        construct(wpos, val);
        return *wpos;
    }
    C4_ALWAYS_INLINE void push_front(T && val)
    {
        iterator wpos = _growat(_c4this->begin());
        construct(wpos, std::move(val));
        return *wpos;
    }


    // pop --------------------------------------

    C4_ALWAYS_INLINE void pop_back()
    {
        erase(_c4this->end());
    }
    C4_ALWAYS_INLINE void pop_front()
    {
        erase(_c4this->begin());
    }


    // insert -----------------------------------

    iterator insert(const_iterator pos, T const* first, T const* last)
    {
        C4_XASSERT(last > first);
        I count = szconv<I>(std::distance(first, last));
        iterator wpos = _growat(pos, count);
        copy_construct_n(wpos, first, count);
        return wpos;
    }
    template< class InputIt >
    iterator insert(const_iterator pos, InputIt first, InputIt last)
    {
        C4_ASSERT(last >= first);
        I count = szconv<I>(std::distance(first, last));
        iterator wpos = _growat(pos, count);
        for(I i = 0; first != last; ++first, ++i)
        {
            copy_construct(wpos + i, *first);
        }
        return wpos;
    }

    template< class ...Args >
    iterator insert(const_iterator pos, Args && ...args)
    {
        iterator wpos = _growat(pos);
        construct(wpos, std::forward< Args >(args)...);
        return wpos;
    }
    template< class ...Args >
    iterator insert_n(const_iterator pos, I count, Args && ...args)
    {
        iterator wpos = _growat(pos, count);
        construct_n(wpos, count, std::forward< Args >(args)...);
        return wpos;
    }

    iterator insert(const_iterator pos, aggregate_t, std::initializer_list<T> ilist)
    {
        return insert(pos, ilist.begin(), ilist.end());
    }


    // replace ----------------------------------


    /** unconditionally replace element at pos */
    void replace(iterator pos, T const& val)
    {
        C4_XASSERT(is_valid_iterator(pos));
        copy_assign(pos, val);
    }
    /** unconditionally replace n elements starting at pos */
    void replace_n(iterator pos, I n, T const& val)
    {
        C4_XASSERT(is_valid_iterator(pos));
        C4_XASSERT(is_valid_iterator(pos + n));
        copy_assign_n(pos, val, n);
    }

    /** unconditionally replace element at pos */
    void replace(iterator pos, T && val)
    {
        C4_XASSERT(is_valid_iterator(pos));
        move_assign(pos, val);
    }
    /** unconditionally replace n elements starting at pos */
    void replace_n(iterator pos, I n, T && val)
    {
        C4_XASSERT(is_valid_iterator(pos));
        C4_XASSERT(is_valid_iterator(pos + n));
        move_assign_n(pos, val, n);
    }

    /** unconditionally replace element at pos. Construct the new elements with the given args. */
    template< class ...Args >
    void replace(iterator pos, Args && ...args)
    {
        C4_XASSERT(is_valid_iterator(pos));
        destroy(pos);
        construct(pos, std::forward< Args >(args)...);
    }
    /** unconditionally replace n elements starting at pos. Construct the new elements with the given args. */
    template< class ...Args >
    void replace_n(iterator pos, I n, Args && ...args)
    {
        C4_XASSERT(is_valid_iterator(pos));
        C4_XASSERT(is_valid_iterator(pos + n));
        destroy_n(pos, n);
        construct_n(pos, n, std::forward< Args >(args)...);
    }


    // erase ------------------------------------

    /** removes the element at pos */
    iterator erase(const_iterator pos)
    {
        return _shrinkat(pos);
    }
    /** removes the elements in the range [first; last). */
    iterator erase(const_iterator first, const_iterator last)
    {
        C4_ASSERT(last >= first);
        I less = szconv<I>(std::distance(first, last));
        destroy_n(first, less);
        return _shrinkat(first, less);
    }


    // resize -----------------------------------

    template< class... CtorArgs >
    void resize(I sz, CtorArgs && ...args)
    {
        I csz = _c4this->size();
        if(sz > csz)
        {
            I more = sz - csz;
            auto wpos = _growat(_c4this->end(), more);
            construct_n(wpos, more, std::forward< CtorArgs >(args)...);
        }
        else if(sz < csz)
        {
            I less = csz - sz;
            destroy_n(_c4this->data() + sz, less);
            _shrinkat(_c4this->end(), less);
        }
    }

protected:

    C4_ALWAYS_INLINE iterator _growat(const_iterator pos)
    {
        C4_XASSERT(is_valid_iterator(pos));
        I ipos = szconv<I>(pos - _c4this->data());
        _c4this->_resizeto(_c4this->size() + 1, pos);
        return _c4this->data() + ipos;
    }
    C4_ALWAYS_INLINE iterator _growat(const_iterator pos, I count)
    {
        C4_XASSERT(is_valid_iterator(pos));
        I ipos = szconv<I>(pos - _c4this->data());
        _c4this->_resizeto(_c4this->size() + count, pos);
        return _c4this->data() + ipos;
    }

    C4_ALWAYS_INLINE iterator _shrinkat(const_iterator pos)
    {
        C4_XASSERT(is_valid_iterator(pos));
        I ipos = szconv<I>(pos - _c4this->data());
        _c4this->_resizeto(_c4this->size() - 1, pos);
        return _c4this->data() + ipos;
    }
    C4_ALWAYS_INLINE iterator _shrinkat(const_iterator pos, I count)
    {
        C4_XASSERT(is_valid_iterator(pos));
        C4_XASSERT(is_valid_iterator(pos + count));
        I ipos = szconv<I>(pos - _c4this->data());
        _c4this->_resizeto(_c4this->size() - count, pos);
        return _c4this->data() + ipos;
    }

};

#undef _c4this
#undef _c4cthis

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/** contiguous storage, fixed size+capacity (cannot resize down). */
template< class T, size_t N_, class I, I Alignment >
class fixed_size : public _ctg_crtp< T, I, fixed_size< T, N_, I, Alignment > >
{
    C4_STATIC_ASSERT(N_ <= std::numeric_limits< I >::max());

public:

    alignas(Alignment) T m_arr[N_];

public:

    _c4_DEFINE_ARRAY_TYPES(T, I)

    enum : I { alignment = Alignment, arr_size = (I)N_ };

    static constexpr const I N = arr_size;

    using base_type = _ctg_crtp< T, I, fixed_size< T, N, I, Alignment > >;
    friend struct _ctg_crtp< T, I, fixed_size< T, N, I, Alignment > >;

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

    C4_ALWAYS_INLINE                 iterator  end()       noexcept { return m_arr + N; }
    C4_ALWAYS_INLINE constexpr const_iterator  end() const noexcept { return m_arr + N; }
    C4_ALWAYS_INLINE constexpr const_iterator cend() const noexcept { return m_arr + N; }

    C4_ALWAYS_INLINE                 reverse_iterator  rbegin()       noexcept { return reverse_iterator(m_arr + N); }
    C4_ALWAYS_INLINE constexpr const_reverse_iterator  rbegin() const noexcept { return reverse_iterator(m_arr + N); }
    C4_ALWAYS_INLINE constexpr const_reverse_iterator crbegin() const noexcept { return reverse_iterator(m_arr + N); }

    C4_ALWAYS_INLINE                 reverse_iterator  rend()       noexcept { return const_reverse_iterator(m_arr); }
    C4_ALWAYS_INLINE constexpr const_reverse_iterator  rend() const noexcept { return const_reverse_iterator(m_arr); }
    C4_ALWAYS_INLINE constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator(m_arr); }

    C4_ALWAYS_INLINE           T      & front()       noexcept { return m_arr[0]; }
    C4_ALWAYS_INLINE constexpr T const& front() const noexcept { return m_arr[0]; }

    C4_ALWAYS_INLINE           T      & back()       noexcept { return m_arr[N - 1]; }
    C4_ALWAYS_INLINE constexpr T const& back() const noexcept { return m_arr[N - 1]; }

    C4_ALWAYS_INLINE           T      * data()       noexcept { return m_arr; }
    C4_ALWAYS_INLINE constexpr T const* data() const noexcept { return m_arr; }

    C4_ALWAYS_INLINE                T      & operator[] (I i)       C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < N); return m_arr[i]; }
    C4_ALWAYS_INLINE C4_CONSTEXPR14 T const& operator[] (I i) const C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < N); return m_arr[i]; }

    C4_ALWAYS_INLINE void fill(T const& v)
    {
        c4::copy_assign_n(m_arr, v, N);
    }

};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/**
 *
 * @todo maybe using multiple inheritance wouldn't be bad here: the layout is safe
 * in C++11, as it results in a standard class layout because only one of
 * the bases is non-empty: see http://stackoverflow.com/questions/11048045
 */
template< class T, class I, class RawStorage >
class contiguous : public _ctgrs_crtp< T, I, contiguous< T, I, RawStorage > >
{
public:

    RawStorage m_storage;
    I m_size;

public:

    using base_crtp = _ctgrs_crtp< T, I, contiguous< T, I, RawStorage > >;
    using raw_storage_type = RawStorage;

    _c4_DEFINE_ARRAY_TYPES(T, I)

public:

    contiguous() : m_storage(), m_size(0) {}
    ~contiguous()
    {
        C4_NOT_IMPLEMENTED();
    }

    contiguous(contiguous const& that)
    {
        C4_NOT_IMPLEMENTED();
    }
    contiguous(contiguous && that)
    {
        C4_NOT_IMPLEMENTED();
    }
    contiguous& operator=(contiguous const& that)
    {
        C4_NOT_IMPLEMENTED();
    }
    contiguous& operator=(contiguous && that)
    {
        C4_NOT_IMPLEMENTED();
    }

    contiguous (cspan< T, I > const& v) { C4_NOT_IMPLEMENTED(); }
    void assign(cspan< T, I > const& v) { C4_NOT_IMPLEMENTED(); }

    contiguous (aggregate_t, std::initializer_list< T > il) { C4_NOT_IMPLEMENTED(); }
    void assign(aggregate_t, std::initializer_list< T > il) { C4_NOT_IMPLEMENTED(); }

    template< I N > contiguous (T const (&v)[N]) { C4_NOT_IMPLEMENTED(); }
    template< I N > void assign(T const (&v)[N]) { C4_NOT_IMPLEMENTED(); }

    contiguous (T const* v, I sz) { C4_NOT_IMPLEMENTED(); }
    void assign(T const* v, I sz) { C4_NOT_IMPLEMENTED(); }

public:

    C4_ALWAYS_INLINE bool empty()    const noexcept { return m_size == 0; }
    C4_ALWAYS_INLINE I    size()     const noexcept { return m_size; }
    C4_ALWAYS_INLINE I    max_size() const noexcept { return m_size; }
    C4_ALWAYS_INLINE I    capacity() const noexcept { return m_size; }

    C4_ALWAYS_INLINE       iterator  begin()       noexcept { return m_storage.data(); }
    C4_ALWAYS_INLINE const_iterator  begin() const noexcept { return m_storage.data(); }
    C4_ALWAYS_INLINE const_iterator cbegin() const noexcept { return m_storage.data(); }

    C4_ALWAYS_INLINE       iterator  end()       noexcept { return m_storage.data() + m_size; }
    C4_ALWAYS_INLINE const_iterator  end() const noexcept { return m_storage.data() + m_size; }
    C4_ALWAYS_INLINE const_iterator cend() const noexcept { return m_storage.data() + m_size; }

    C4_ALWAYS_INLINE       reverse_iterator  rbegin()       noexcept { return reverse_iterator(m_storage.data() + m_size); }
    C4_ALWAYS_INLINE const_reverse_iterator  rbegin() const noexcept { return reverse_iterator(m_storage.data() + m_size); }
    C4_ALWAYS_INLINE const_reverse_iterator crbegin() const noexcept { return reverse_iterator(m_storage.data() + m_size); }

    C4_ALWAYS_INLINE       reverse_iterator  rend()       noexcept { return const_reverse_iterator(m_storage.data()); }
    C4_ALWAYS_INLINE const_reverse_iterator  rend() const noexcept { return const_reverse_iterator(m_storage.data()); }
    C4_ALWAYS_INLINE const_reverse_iterator crend() const noexcept { return const_reverse_iterator(m_storage.data()); }

    C4_ALWAYS_INLINE T      & front()       C4_NOEXCEPT_X { C4_XASSERT(!empty()); return m_storage.data()[0]; }
    C4_ALWAYS_INLINE T const& front() const C4_NOEXCEPT_X { C4_XASSERT(!empty()); return m_storage.data()[0]; }

    C4_ALWAYS_INLINE T      & back()       C4_NOEXCEPT_X { C4_XASSERT(!empty()); return m_storage.data()[m_size - 1]; }
    C4_ALWAYS_INLINE T const& back() const C4_NOEXCEPT_X { C4_XASSERT(!empty()); return m_storage.data()[m_size - 1]; }

    C4_ALWAYS_INLINE T      * data()       noexcept { return m_storage.data(); }
    C4_ALWAYS_INLINE T const* data() const noexcept { return m_storage.data(); }

    C4_ALWAYS_INLINE T      & operator[] (I i)       C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < m_size); return m_storage.data()[i]; }
    C4_ALWAYS_INLINE T const& operator[] (I i) const C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && i < m_size); return m_storage.data()[i]; }

public:

    void reserve(I cap)
    {
        if(cap <= m_storage.capacity()) return;
        m_storage._raw_grow(m_size, cap);
    }

    C4_ALWAYS_INLINE void _resizeto(I sz)
    {
        _growto(sz);
        m_size = sz;
    }

    C4_ALWAYS_INLINE void _growto(I sz)
    {
        C4_NOT_IMPLEMENTED();
    }
};

C4_END_NAMESPACE(stg)
C4_END_NAMESPACE(c4)

#endif // _C4_STORAGE_CONTIGUOUS_HPP_
