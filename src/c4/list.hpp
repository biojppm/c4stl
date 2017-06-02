#ifndef _C4_LIST_HPP_
#define _C4_LIST_HPP_

#include "c4/config.hpp"
#include "c4/error.hpp"
#include "c4/storage/raw.hpp"

#include <iterator>

C4_BEGIN_NAMESPACE(c4)

//-----------------------------------------------------------------------------
template< class T, class I=C4_SIZE_TYPE >
struct flat_list_elm
{
    T elm;
    I prev;
    I next;

    template< class U >
    using rebind_type = flat_list_elm< U, I >;
    using value_type = T;
    using size_type = I;
};

template< class T, class I=C4_SIZE_TYPE >
struct flat_fwd_list_elm
{
    T elm;
    I next;

    template< class U >
    using rebind_type = flat_fwd_list_elm< U, I >;
    using value_type = T;
    using size_type = I;
};

//-----------------------------------------------------------------------------
template< class T, class I=C4_SIZE_TYPE >
using default_list_storage = stg::raw_paged_rt< T, I >;

template< class T, class I=C4_SIZE_TYPE, class LinearStorage=default_list_storage<flat_list_elm<T,I>,I> >
class flat_list;

template< class T, class I=C4_SIZE_TYPE, class LinearStorage=default_list_storage<T,I> >
class split_list;

template< class T, class I=C4_SIZE_TYPE, class LinearStorage=default_list_storage<flat_fwd_list_elm<T,I>,I> >
class flat_fwd_list;

template< class T, class I=C4_SIZE_TYPE, class LinearStorage=default_list_storage<T,I> >
class split_fwd_list;

//-----------------------------------------------------------------------------

template< class T, class List >
struct list_iterator : public std::iterator< std::bidirectional_iterator_tag, typename List::value_type >
{
    using I = typename List::size_type;

    List *list;
    I i;

    list_iterator(List *l, I i_) : list(l), i(i_) {}

    C4_ALWAYS_INLINE T* operator-> () { return &list->elm(i); }
    C4_ALWAYS_INLINE T& operator*  () { return  list->elm(i); }

    C4_ALWAYS_INLINE list_iterator& operator++ (   ) noexcept {                           i = list->next(i); return *this; }
    C4_ALWAYS_INLINE list_iterator  operator++ (int) noexcept { list_iterator it = *this; i = list->next(i); return    it; }

    C4_ALWAYS_INLINE list_iterator& operator-- (   ) noexcept {                           i = list->prev(i); return *this; }
    C4_ALWAYS_INLINE list_iterator  operator-- (int) noexcept { list_iterator it = *this; i = list->prev(i); return    it; }

    C4_ALWAYS_INLINE bool operator== (list_iterator const& that) const C4_NOEXCEPT_X { C4_XASSERT(that.list == list); return i == that.i; }
    C4_ALWAYS_INLINE bool operator!= (list_iterator const& that) const C4_NOEXCEPT_X { C4_XASSERT(that.list == list); return i != that.i; }
};

template< class T, class List >
struct fwd_list_iterator : public std::iterator< std::forward_iterator_tag, typename List::value_type >
{
    using I = typename List::size_type;

    List *list;
    I i;

    fwd_list_iterator(List *l, I i_) : list(l), i(i_) {}

    C4_ALWAYS_INLINE T* operator-> () { return &list->elm(i); }
    C4_ALWAYS_INLINE T& operator*  () { return  list->elm(i); }

    C4_ALWAYS_INLINE fwd_list_iterator& operator++ (   ) noexcept {                               i = list->next(i); return *this; }
    C4_ALWAYS_INLINE fwd_list_iterator  operator++ (int) noexcept { fwd_list_iterator it = *this; i = list->next(i); return    it; }

    C4_ALWAYS_INLINE bool operator== (fwd_list_iterator const& that) const C4_NOEXCEPT_X { C4_XASSERT(that.list == list); return i == that.i; }
    C4_ALWAYS_INLINE bool operator!= (fwd_list_iterator const& that) const C4_NOEXCEPT_X { C4_XASSERT(that.list == list); return i != that.i; }
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

// convenience defines; undefined below
/// @cond dev
#define _c4this  (static_cast<ListType      *>(this))
#define _c4cthis (static_cast<ListType const*>(this))
/// @endcond

/** common code to implement doubly or singly linked lists */
template< class T, class I, class ListType >
class _list_crtp
{
public:

    void _init_initlist(std::initializer_list< T > il)
    {
        I sz = szconv< I >(il.size());
        if(sz)
        {
            _c4this->_set_head(0);
            _c4this->_set_tail(sz - 1);
            _c4this->_set_fhead(sz);
            _c4this->m_size = sz;
        }
        sz = 0;
        for(auto const& e : il)
        {
            c4::copy_construct(&(_c4this->elm(sz++)), e);
        }
    }

    /** resize the storage so that more elements fit in it */
    void _make_room_for(I how_many)
    {
        I cap = _c4cthis->capacity();
        I cap_next = _c4cthis->m_size + how_many;
        if(cap_next > cap)
        {
            _c4this->_growto(cap, cap_next);
        }
    }

    /** claim one element from the free list, growing the storage if necessary */
    I _claim() C4_NOEXCEPT_X
    {
        this->_make_room_for(I(1));
        C4_XASSERT(_c4cthis->m_fhead != ListType::npos);
        C4_XASSERT(_c4cthis->m_size + 1 <= _c4cthis->capacity());
        I pos = _c4cthis->m_fhead;
        I last = _c4cthis->next(pos);
        _c4this->_set_fhead(last);
        return pos;
    }

    /** claim n elements from the free list, growing the storage if necessary */
    I _claim(I n) C4_NOEXCEPT_X
    {
        this->_make_foom_for(n);
        C4_XASSERT(_c4cthis->m_fhead != ListType::npos);
        C4_XASSERT(_c4cthis->m_size + n <= _c4cthis->capacity());
        I pos = _c4cthis->m_fhead;
        I count = 0, last = pos;
        for(count = 0; count < n; ++count)
        {
            C4_XASSERT(last != ListType::npos);
            last = _c4cthis->next(last);
        }
        C4_XASSERT(count == n);
        _c4this->_set_fhead(last);
        C4_XASSERT(pos != ListType::npos);
        return pos;
    }

    void _adjust_fhead(I curr_cap, I next_cap)
    {
        if(_c4this->m_fhead != ListType::npos)
        {
            return;
        }
        if(next_cap >= curr_cap)
        {
            _c4this->_set_fhead(curr_cap);
        }
        else
        {
            _c4this->_set_fhead(next_cap);
        }
    }

public:

    C4_ALWAYS_INLINE static constexpr I max_size() noexcept
    {
        return static_cast< I >(stg::raw_max_size_with_npos< typename ListType::storage_type >());
    }

    void reserve(I cap)
    {
        I curr = _c4this->capacity();
        if(cap > curr)
        {
            _c4this->_growto(curr, cap);
        }
    }

public:

    T      & front()       C4_NOEXCEPT_X { C4_XASSERT(!_c4this ->empty()); return _c4this ->elm(_c4this ->m_head); }
    T const& front() const C4_NOEXCEPT_X { C4_XASSERT(!_c4cthis->empty()); return _c4cthis->elm(_c4cthis->m_head); }
    T      & back ()       C4_NOEXCEPT_X { C4_XASSERT(!_c4this ->empty()); return _c4this ->elm(_c4this ->m_tail); }
    T const& back () const C4_NOEXCEPT_X { C4_XASSERT(!_c4cthis->empty()); return _c4cthis->elm(_c4cthis->m_tail); }

    void push_front(T const& var)
    {
        C4_UNUSED(var);
        C4_NOT_IMPLEMENTED();
    }

    void push_back(T const& var)
    {
        I pos = _c4this->_append();
        T *elm = &_c4this->elm(pos);
        c4::copy_construct(elm, var);
    }
    void push_back(T && var)
    {
        I pos = _c4this->_append();
        T *elm = &_c4this->elm(pos);
        c4::move_construct(elm, &var);
    }

    template< class... Args >
    void emplace_back(Args&&... args)
    {
        I pos = _c4this->_append();
        T *elm = &_c4this->elm(pos);
        c4::construct(elm, std::forward< Args >(args)...);
    }

};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/** common code to implement doubly linked lists */
template< class T, class I, class ListType >
class _dbl_list_crtp : public _list_crtp< T, I, ListType >
{
public:

    void _init_seq(I first, I last, I prev_ = ListType::npos, I next_ = ListType::npos)
    {
        C4_XASSERT(last >= first);
        if(last == first) return;
        _c4this->_set_prev(first, prev_);
        for(I i = first+1; i < last; ++i)
        {
            _c4this->_set_prev(i, i-1);
        }
        C4_XASSERT(last > 0);
        for(I i = first; i < last-1; ++i)
        {
            _c4this->_set_next(i, i+1);
        }
        _c4this->_set_next(last-1, next_);
    }

    I _append()
    {
        I pos = this->_claim();
        if(_c4this->m_size > 0)
        {
            _c4this->_set_prev(pos, _c4cthis->m_tail);
            _c4this->_set_next(_c4cthis->m_tail, pos);
        }
        else
        {
            _c4this->_set_head(0);
        }
        ++_c4this->m_size;
        _c4this->_set_tail(pos);
        return pos;
    }

};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/** common code to implement forward linked lists */
template< class T, class I, class ListType >
class _fwd_list_crtp : public _list_crtp< T, I, ListType >
{
public:

    void _init_seq(I first, I last, I prev_ = ListType::npos, I next_ = ListType::npos)
    {
        C4_UNUSED(prev_);
        C4_XASSERT(last >= first);
        if(last == first) return;
        C4_XASSERT(last > 0);
        for(I i = first; i < last-1; ++i)
        {
            _c4this->_set_next(i, i+1);
        }
        _c4this->_set_next(last-1, next_);
    }

    I _append()
    {
        I pos = this->_claim();
        if(_c4this->m_size > 0)
        {
            _c4this->_set_next(_c4cthis->m_tail, pos);
        }
        else
        {
            _c4this->_set_head(0);
        }
        ++_c4this->m_size;
        _c4this->_set_tail(pos);
        return pos;
    }

};

#undef _c4this
#undef _c4cthis

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** an array-based doubly-linked list where the indices are interleaved
 * with the contained elements. Using paged raw storage, insertions are O(1). */
template< class T, class I, class RawStorage >
class flat_list : public _dbl_list_crtp< T, I, flat_list<T, I, RawStorage> >
{
public:

    C4_STATIC_ASSERT(std::is_same< typename RawStorage::value_type C4_COMMA flat_list_elm<T C4_COMMA I>>::value);
    C4_STATIC_ASSERT(std::is_same< typename RawStorage::size_type  C4_COMMA I>::value);

    RawStorage m_elms;

    I m_head;
    I m_tail;
    I m_size;
    I m_fhead;

public:

    enum : I { npos = static_cast< I >(-1) };

    using value_type = T;
    using size_type = I;
    using difference_type = typename std::make_signed< I >::type;
    using reference = value_type&;
    using const_reference = value_type const&;
    using pointer = T*;
    using const_pointer = T const*;
    using allocator_type = typename RawStorage::allocator_type;
    using storage_type = RawStorage;
    using storage_traits = typename RawStorage::storage_traits;

    using iterator = list_iterator< T, flat_list >;
    using const_iterator = list_iterator< const T, const flat_list >;
    using reverse_iterator = std::reverse_iterator< iterator >;
    using const_reverse_iterator = std::reverse_iterator< const_iterator >;

    template< class U >
    using storage_rebind_type = typename RawStorage::template rebind_type< flat_list_elm<U, I> >;
    template< class U >
    using rebind_type = flat_list< U, I, storage_rebind_type<U> >;

public:

    ~flat_list()
    {
        for(I i = m_head; i != npos; i = m_elms[i].next)
        {
            c4::destroy(&m_elms[i].elm);
        }
    }

    flat_list() : m_elms(), m_head(npos), m_tail(npos), m_size(0), m_fhead(0)
    {
        this->_init_seq(0, capacity());
    }

    flat_list(c4::with_capacity_t, I cap) : m_elms(cap), m_head(npos), m_tail(npos), m_size(0), m_fhead(0)
    {
        this->_init_seq(0, capacity());
    }

    flat_list(c4::aggregate_t, std::initializer_list< T > il) : m_elms(szconv< I >(il.size())), m_head(npos), m_tail(npos), m_size(0), m_fhead(0)
    {
        this->_init_seq(0, capacity());
        this->_init_initlist(il);
    }

    void _growto(I cap, I next_cap)
    {
        C4_XASSERT(cap != next_cap);
        m_elms._raw_reserve(next_cap);
        C4_XASSERT(capacity() >= next_cap);
        next_cap = capacity(); // they may be different
        this->_init_seq(cap, next_cap);
        this->_adjust_fhead(cap, next_cap);
    }

public:

    C4_ALWAYS_INLINE T      & elm(I i)       C4_NOEXCEPT_X { C4_XASSERT(i < capacity()); return m_elms[i].elm; }
    C4_ALWAYS_INLINE T const& elm(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < capacity()); return m_elms[i].elm; }

    C4_ALWAYS_INLINE I  prev(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < capacity()); return m_elms[i].prev; }
    C4_ALWAYS_INLINE I  next(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < capacity()); return m_elms[i].next; }

    C4_ALWAYS_INLINE void _set_prev(I i, I val) C4_NOEXCEPT_X { C4_XASSERT(i < capacity()); m_elms[i].prev = val; }
    C4_ALWAYS_INLINE void _set_next(I i, I val) C4_NOEXCEPT_X { C4_XASSERT(i < capacity()); m_elms[i].next = val; }

    C4_ALWAYS_INLINE void _set_head(I i) C4_NOEXCEPT_X { C4_XASSERT(i < capacity()); m_head = i; m_elms[i].prev = npos; }
    C4_ALWAYS_INLINE void _set_tail(I i) C4_NOEXCEPT_X { C4_XASSERT(i < capacity()); m_tail = i; m_elms[i].next = npos; }

    C4_ALWAYS_INLINE void _set_fhead(I i) C4_NOEXCEPT_X { m_fhead = i; if(i < capacity() && i != npos) { m_elms[i].prev = npos; } }

public:

    C4_ALWAYS_INLINE bool empty() const noexcept { return m_size == 0; }

    C4_ALWAYS_INLINE I size    () const noexcept { return m_size; }
    C4_ALWAYS_INLINE I capacity() const noexcept { return m_elms.capacity(); }

    C4_ALWAYS_INLINE iterator begin() noexcept { return iterator(this, m_head); }
    C4_ALWAYS_INLINE iterator end  () noexcept { return iterator(this, npos); }

    C4_ALWAYS_INLINE const_iterator begin() const noexcept { return const_iterator(this, m_head); }
    C4_ALWAYS_INLINE const_iterator end  () const noexcept { return const_iterator(this, npos); }

};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** an array-based doubly-linked list where the indices are stored in separate
 * arrays from the contained elements. Using paged raw storage, insertions
 * are O(1). */
template< class T, class I, class RawStorageT >
class split_list : public _dbl_list_crtp< T, I, split_list<T, I, RawStorageT> >
{
public:

    C4_ASSERT_SAME_TYPE(typename RawStorageT::value_type, T);
    C4_ASSERT_SAME_TYPE(typename RawStorageT::size_type , I);
    using RawStorageI = typename RawStorageT::template rebind_type< I >;

    RawStorageT m_elm;
    RawStorageI m_prev;
    RawStorageI m_next;

    I m_head;
    I m_tail;
    I m_size;
    I m_fhead; //< the head of the free list (the list containing free elements)

public:

    enum : I { npos = static_cast< I >(-1) };

    using value_type = T;
    using size_type = I;
    using difference_type = typename std::make_signed< I >::type;
    using reference = value_type&;
    using const_reference = value_type const&;
    using pointer = T*;
    using const_pointer = T const*;
    using allocator_type = typename RawStorageT::allocator_type;
    using index_allocator_type = typename RawStorageI::allocator_type;
    using storage_type = RawStorageT;
    using storage_traits = typename RawStorageT::storage_traits;
    using index_storage_type = RawStorageI;
    using index_storage_traits = typename RawStorageI::storage_traits;

    using iterator = list_iterator< T, split_list >;
    using const_iterator = list_iterator< const T, const split_list >;
    using reverse_iterator = std::reverse_iterator< iterator >;
    using const_reverse_iterator = std::reverse_iterator< const_iterator >;

    template< class U >
    using storage_rebind_type = typename RawStorageT::template rebind_type< U >;
    template< class U >
    using rebind_type = split_list< U, I, storage_rebind_type< U > >;

public:

    ~split_list()
    {
        for(I i = m_head; i != npos; i = m_next[i])
        {
            c4::destroy(&m_elm[i]);
        }
    }

    split_list() : m_elm(), m_prev(), m_next(), m_head(npos), m_tail(npos), m_size(0), m_fhead(0)
    {
        this->_init_seq(0, capacity());
    }

    split_list(c4::with_capacity_t, I cap) : m_elm(cap), m_prev(cap), m_next(cap), m_head(npos), m_tail(npos), m_size(0), m_fhead(0)
    {
        this->_init_seq(0, capacity());
    }

    split_list(c4::aggregate_t, std::initializer_list< T > il) : m_elm(szconv< I >(il.size())), m_prev(szconv< I >(il.size())), m_next(szconv< I >(il.size())), m_head(npos), m_tail(npos), m_size(0), m_fhead(0)
    {
        this->_init_seq(0, capacity());
        this->_init_initlist(il);
    }

    void _growto(I curr_cap, I next_cap)
    {
        m_elm ._raw_reserve(next_cap);
        m_prev._raw_reserve(next_cap);
        m_next._raw_reserve(next_cap);
        C4_XASSERT(capacity() >= next_cap);
        next_cap = capacity(); // they may be different
        this->_init_seq(curr_cap, next_cap);
        this->_adjust_fhead(curr_cap, next_cap);
    }

public:

    C4_ALWAYS_INLINE T      & elm(I i)       C4_NOEXCEPT_X { C4_XASSERT(i < capacity()); return m_elm[i]; }
    C4_ALWAYS_INLINE T const& elm(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < capacity()); return m_elm[i]; }

    C4_ALWAYS_INLINE I prev(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < capacity()); return m_prev[i]; }
    C4_ALWAYS_INLINE I next(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < capacity()); return m_next[i]; }

    C4_ALWAYS_INLINE void _set_prev(I i, I val) C4_NOEXCEPT_X { C4_XASSERT(i < capacity()); m_prev[i] = val; }
    C4_ALWAYS_INLINE void _set_next(I i, I val) C4_NOEXCEPT_X { C4_XASSERT(i < capacity()); m_next[i] = val; }

    C4_ALWAYS_INLINE void _set_head(I i) C4_NOEXCEPT_X { C4_XASSERT(i < capacity()); m_head = i; m_prev[i] = npos; }
    C4_ALWAYS_INLINE void _set_tail(I i) C4_NOEXCEPT_X { C4_XASSERT(i < capacity()); m_tail = i; m_next[i] = npos; }

    C4_ALWAYS_INLINE void _set_fhead(I i) C4_NOEXCEPT_X { m_fhead = i; if(i < capacity() && i != npos) { m_prev[i] = npos; } }

public:

    C4_ALWAYS_INLINE bool empty() const noexcept { return m_size == 0; }

    C4_ALWAYS_INLINE I size() const noexcept { return m_size; }
    C4_ALWAYS_INLINE I capacity() const noexcept { return m_elm.capacity(); }

    C4_ALWAYS_INLINE iterator begin() noexcept { return iterator(this, m_head); }
    C4_ALWAYS_INLINE iterator end  () noexcept { return iterator(this, npos); }

    C4_ALWAYS_INLINE const_iterator begin() const noexcept { return const_iterator(this, m_head); }
    C4_ALWAYS_INLINE const_iterator end  () const noexcept { return const_iterator(this, npos); }

};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** an array-based forward-linked list where the indices are interleaved
 * with the contained elements. Using paged raw storage, insertions are O(1). */
template< class T, class I, class RawStorage >
class flat_fwd_list : public _fwd_list_crtp< T, I, flat_fwd_list<T, I, RawStorage> >
{
public:

    C4_STATIC_ASSERT(std::is_same< typename RawStorage::value_type C4_COMMA flat_fwd_list_elm<T C4_COMMA I>>::value);
    C4_STATIC_ASSERT(std::is_same< typename RawStorage::size_type  C4_COMMA I>::value);

    RawStorage m_elms;

    I m_head;
    I m_tail;
    I m_size;
    I m_fhead;

public:

    enum : I { npos = static_cast< I >(-1) };

    using value_type = T;
    using size_type = I;
    using difference_type = typename std::make_signed< I >::type;
    using reference = value_type&;
    using const_reference = value_type const&;
    using pointer = T*;
    using const_pointer = T const*;
    using allocator_type = typename RawStorage::allocator_type;
    using storage_type = RawStorage;
    using storage_traits = typename RawStorage::storage_traits;

    using iterator = list_iterator< T, flat_fwd_list >;
    using const_iterator = list_iterator< const T, const flat_fwd_list >;
    using reverse_iterator = std::reverse_iterator< iterator >;
    using const_reverse_iterator = std::reverse_iterator< const_iterator >;

    template< class U >
    using storage_rebind_type = typename RawStorage::template rebind_type< flat_fwd_list_elm<U, I> >;
    template< class U >
    using rebind_type = flat_fwd_list< U, I, storage_rebind_type<U> >;

public:

    ~flat_fwd_list()
    {
        for(I i = m_head; i != npos; i = m_elms[i].next)
        {
            c4::destroy(&m_elms[i].elm);
        }
    }

    flat_fwd_list() : m_elms(), m_head(npos), m_tail(npos), m_size(0), m_fhead(0)
    {
        this->_init_seq(0, capacity());
    }

    flat_fwd_list(c4::with_capacity_t, I cap) : m_elms(cap), m_head(npos), m_tail(npos), m_size(0), m_fhead(0)
    {
        this->_init_seq(0, capacity());
    }

    flat_fwd_list(c4::aggregate_t, std::initializer_list< T > il) : m_elms(szconv< I >(il.size())), m_head(npos), m_tail(npos), m_size(0), m_fhead(0)
    {
        this->_init_seq(0, capacity());
        this->_init_initlist(il);
    }

    void _growto(I cap, I next_cap)
    {
        m_elms._raw_reserve(next_cap);
        C4_XASSERT(capacity() >= next_cap);
        next_cap = capacity(); // they may be different
        this->_init_seq(cap, next_cap);
        this->_adjust_fhead(cap, next_cap);
    }

public:

    C4_ALWAYS_INLINE T      & elm(I i)       C4_NOEXCEPT_X { C4_XASSERT(i < capacity()); return m_elms[i].elm; }
    C4_ALWAYS_INLINE T const& elm(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < capacity()); return m_elms[i].elm; }

    C4_ALWAYS_INLINE I  next(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < capacity()); return m_elms[i].next; }

    C4_ALWAYS_INLINE void _set_next(I i, I val) C4_NOEXCEPT_X { C4_XASSERT(i < capacity()); m_elms[i].next = val; }

    C4_ALWAYS_INLINE void _set_head(I i) C4_NOEXCEPT_X { C4_XASSERT(i < capacity()); m_head = i; }
    C4_ALWAYS_INLINE void _set_tail(I i) C4_NOEXCEPT_X { C4_XASSERT(i < capacity()); m_tail = i; m_elms[i].next = npos; }

    C4_ALWAYS_INLINE void _set_fhead(I i) C4_NOEXCEPT_X { m_fhead = i; /* there's no prev to set. */}

public:

    C4_ALWAYS_INLINE bool empty() const noexcept { return m_size == 0; }

    C4_ALWAYS_INLINE I size() const noexcept { return m_size; }
    C4_ALWAYS_INLINE I capacity() const noexcept { return m_elms.capacity(); }

    C4_ALWAYS_INLINE iterator begin() noexcept { return iterator(this, m_head); }
    C4_ALWAYS_INLINE iterator end  () noexcept { return iterator(this, npos); }

    C4_ALWAYS_INLINE const_iterator begin() const noexcept { return const_iterator(this, m_head); }
    C4_ALWAYS_INLINE const_iterator end  () const noexcept { return const_iterator(this, npos); }

};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** an array-based forward-linked list where the indices are stored in a
 * separate array from the contained elements. Using paged raw storage,
 * insertions are O(1). */
template< class T, class I, class RawStorageT >
class split_fwd_list : public _fwd_list_crtp< T, I, split_fwd_list<T, I, RawStorageT> >
{
public:

    C4_ASSERT_SAME_TYPE(typename RawStorageT::value_type, T);
    C4_ASSERT_SAME_TYPE(typename RawStorageT::size_type , I);
    using RawStorageI = typename RawStorageT::template rebind_type< I >;

    RawStorageT m_elm;
    RawStorageI m_next;

    I m_head;
    I m_tail;
    I m_size;
    I m_fhead;

public:

    enum : I { npos = static_cast< I >(-1) };

    using value_type = T;
    using size_type = I;
    using difference_type = typename std::make_signed< I >::type;
    using reference = value_type&;
    using const_reference = value_type const&;
    using pointer = T*;
    using const_pointer = T const*;
    using allocator_type = typename RawStorageT::allocator_type;
    using index_allocator_type = typename RawStorageI::allocator_type;
    using storage_type = RawStorageT;
    using storage_traits = typename RawStorageT::storage_traits;
    using index_storage_type = RawStorageI;
    using index_storage_traits = typename RawStorageI::storage_traits;

    using iterator = list_iterator< T, split_fwd_list >;
    using const_iterator = list_iterator< const T, const split_fwd_list >;
    using reverse_iterator = std::reverse_iterator< iterator >;
    using const_reverse_iterator = std::reverse_iterator< const_iterator >;

    template< class U >
    using storage_rebind_type = typename RawStorageT::template rebind_type< U >;
    template< class U >
    using rebind_type = split_fwd_list< U, I, storage_rebind_type<U> >;

public:

    ~split_fwd_list()
    {
        for(I i = m_head; i != npos; i = m_next[i])
        {
            c4::destroy(&m_elm[i]);
        }
    }

    split_fwd_list() : m_elm(), m_next(), m_head(npos), m_tail(npos), m_size(0), m_fhead(0)
    {
        this->_init_seq(0, capacity());
    }

    split_fwd_list(c4::with_capacity_t, I cap) : m_elm(cap), m_next(cap), m_head(npos), m_tail(npos), m_size(0), m_fhead(0)
    {
        this->_init_seq(0, capacity());
    }

    split_fwd_list(c4::aggregate_t, std::initializer_list< T > il) : m_elm(szconv< I >(il.size())), m_next(szconv< I >(il.size())), m_head(npos), m_tail(npos), m_size(0), m_fhead(0)
    {
        this->_init_seq(0, capacity());
        this->_init_initlist(il);
    }

    void _growto(I curr_cap, I next_cap)
    {
        m_elm ._raw_reserve(next_cap);
        m_next._raw_reserve(next_cap);
        C4_XASSERT(capacity() >= next_cap);
        next_cap = capacity(); // they may be different
        this->_init_seq(curr_cap, next_cap);
        this->_adjust_fhead(curr_cap, next_cap);
    }

public:

    C4_ALWAYS_INLINE T      & elm(I i)       C4_NOEXCEPT_X { C4_XASSERT(i < capacity()); return m_elm[i]; }
    C4_ALWAYS_INLINE T const& elm(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < capacity()); return m_elm[i]; }

    C4_ALWAYS_INLINE I next(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < capacity()); return m_next[i]; }

    C4_ALWAYS_INLINE void _set_next(I i, I val) C4_NOEXCEPT_X { C4_XASSERT(i < capacity()); m_next[i] = val; }

    C4_ALWAYS_INLINE void _set_head(I i) C4_NOEXCEPT_X { C4_XASSERT(i < capacity()); m_head = i; }
    C4_ALWAYS_INLINE void _set_tail(I i) C4_NOEXCEPT_X { C4_XASSERT(i < capacity()); m_tail = i; m_next[i] = npos; }

    C4_ALWAYS_INLINE void _set_fhead(I i) C4_NOEXCEPT_X { m_fhead = i; /* there's no prev to set. */}

public:

    C4_ALWAYS_INLINE bool empty() const noexcept { return m_size == 0; }

    C4_ALWAYS_INLINE I size() const noexcept { return m_size; }
    C4_ALWAYS_INLINE I capacity() const noexcept { return m_elm.capacity(); }

    C4_ALWAYS_INLINE iterator begin() noexcept { return iterator(this, m_head); }
    C4_ALWAYS_INLINE iterator end  () noexcept { return iterator(this, npos); }

    C4_ALWAYS_INLINE const_iterator begin() const noexcept { return const_iterator(this, m_head); }
    C4_ALWAYS_INLINE const_iterator end  () const noexcept { return const_iterator(this, npos); }

};

C4_END_NAMESPACE(c4)

#endif /* _C4_LIST_HPP_ */
