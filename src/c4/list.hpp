#ifndef _C4_LIST_HPP_
#define _C4_LIST_HPP_

#include "c4/config.hpp"
#include "c4/error.hpp"
#include "c4/storage/raw.hpp"

#include <iterator>

C4_BEGIN_NAMESPACE(c4)

template< class T, class I=C4_SIZE_TYPE >
using default_list_storage = stg::raw_paged_rt< T, I >;

template< class T, class I=C4_SIZE_TYPE, template< class T_, class I_ > class LinearStorage=default_list_storage >
class split_list;

template< class T, class I=C4_SIZE_TYPE, template< class T_, class I_ > class LinearStorage=default_list_storage >
class split_fwd_list;

template< class T, class I=C4_SIZE_TYPE, template< class T_, class I_ > class LinearStorage=default_list_storage >
class flat_list;

template< class T, class I=C4_SIZE_TYPE, template< class T_, class I_ > class LinearStorage=default_list_storage >
class flat_fwd_list;

template< class T, class I=C4_SIZE_TYPE > struct flat_list_elm;
template< class T, class I=C4_SIZE_TYPE > struct flat_fwd_list_elm;

//-----------------------------------------------------------------------------
template< class T, class I >
struct flat_list_elm
{
    T elm;
    I prev;
    I next;
};

template< class T, class I >
struct flat_fwd_list_elm
{
    T elm;
    I next;
};

//-----------------------------------------------------------------------------

template< class T, class List >
struct list_iterator : public std::iterator< std::bidirectional_iterator_tag, typename List::value_type >
{
    using I = typename List::size_type;

    List *list;
    I i;

    list_iterator(List *l, I i_) : list(l), i(i_) {}

    C4_ALWAYS_INLINE T& operator*  () { return  list->elm(i); }
    C4_ALWAYS_INLINE T* operator-> () { return &list->elm(i); }

    C4_ALWAYS_INLINE list_iterator& operator++ (   ) noexcept {                           i = list->next(i); return *this; }
    C4_ALWAYS_INLINE list_iterator& operator++ (int) noexcept { list_iterator it = *this; i = list->next(i); return    it; }

    C4_ALWAYS_INLINE list_iterator& operator-- (   ) noexcept {                           i = list->prev(i); return *this; }
    C4_ALWAYS_INLINE list_iterator& operator-- (int) noexcept { list_iterator it = *this; i = list->prev(i); return    it; }

    C4_ALWAYS_INLINE bool operator== (list_iterator const& that) const noexcept { return i == that.i && list == that.list; }
    C4_ALWAYS_INLINE bool operator!= (list_iterator const& that) const noexcept { return i != that.i || list != that.list; }
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
    C4_ALWAYS_INLINE fwd_list_iterator& operator++ (int) noexcept { fwd_list_iterator it = *this; i = list->next(i); return    it; }

    C4_ALWAYS_INLINE bool operator== (fwd_list_iterator const& that) const noexcept { return i == that.i && list == that.list; }
    C4_ALWAYS_INLINE bool operator!= (fwd_list_iterator const& that) const noexcept { return i != that.i || list != that.list; }
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template< class T, class I, template< class T, class I > class RawStorage >
class flat_list
{
public:

    RawStorage< flat_list_elm<T, I>, I > m_elms;

    I m_head;
    I m_tail;
    I m_size;
    I m_fhead;

public:

    using value_type = T;
    using size_type = I;

    using iterator = list_iterator< T, flat_list >;
    using const_iterator = list_iterator< const T, const flat_list >;

public:

    flat_list() : m_elms(), m_head(0), m_tail(0), m_size(0), m_fhead(0) {}

public:

    C4_ALWAYS_INLINE I size    () const noexcept { return m_size; }
    C4_ALWAYS_INLINE I capacity() const noexcept { return m_elms.capacity(); }

    C4_ALWAYS_INLINE T      & elm(I i)       C4_NOEXCEPT_X { C4_XASSERT(i < m_size); return m_elms[i].elm; }
    C4_ALWAYS_INLINE T const& elm(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < m_size); return m_elms[i].elm; }

    C4_ALWAYS_INLINE I prev(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < m_size); return m_elms[i].prev; }
    C4_ALWAYS_INLINE I next(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < m_size); return m_elms[i].next; }

    C4_ALWAYS_INLINE iterator begin() noexcept { return iterator(this, m_head); }
    C4_ALWAYS_INLINE iterator end  () noexcept { return iterator(this, m_tail); }

    C4_ALWAYS_INLINE const_iterator begin() const noexcept { return const_iterator(this, m_head); }
    C4_ALWAYS_INLINE const_iterator end  () const noexcept { return const_iterator(this, m_tail); }

};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template< class T, class I, template< class T, class I > class RawStorage >
class split_list
{
public:

    RawStorage< T, I > m_elm;
    RawStorage< I, I > m_prev;
    RawStorage< I, I > m_next;

    I m_head;
    I m_tail;
    I m_size;
    I m_fhead; //< the head of the free list (the list containing free elements)

    enum : I { npos = static_cast< I >(-1) };

public:

    using value_type = T;
    using size_type = I;

    using iterator = list_iterator< T, split_list >;
    using const_iterator = list_iterator< const T, const split_list >;

public:

    split_list() : m_elm(), m_prev(), m_next(), m_head(0), m_tail(0), m_size(0), m_fhead(0)
    {
        _init_seq(0, capacity());
    }

    split_list(c4::with_capacity_t, I cap) : m_elm(cap), m_prev(cap), m_next(cap), m_head(0), m_tail(0), m_size(0), m_fhead(0)
    {
        _init_seq(0, capacity());
    }

    split_list(c4::aggregate_t, std::initializer_list< T > il) : m_elm(szconv< I >(il.size())), m_prev(szconv< I >(il.size())), m_next(szconv< I >(il.size())), m_head(0), m_tail(0), m_size(0), m_fhead(0)
    {
        _init_seq(0, capacity());
    }

    void _growto(I next_cap)
    {
        I curr = capacity();
        m_elm._raw_reserve(next_cap);
        m_prev._raw_reserve(next_cap);
        m_next._raw_reserve(next_cap);
        _init_seq(curr, next_cap);
    }

    void _init_seq(I first, I last)
    {
        C4_ASSERT(m_elm.capacity() == m_prev.capacity() && m_elm.capacity() == m_next.capacity());
        I cap = m_elm.capacity();
        if(cap == 0) return;
        m_prev[0] = npos;
        for(I i = 1; i < cap; ++i)
        {
            m_prev[i] = i - 1;
        }
        for(I i = 0; i < cap-1; ++i) // cap must be > 0 at this point
        {
            m_next[i] = i + 1;
        }
        m_next[cap - 1] = npos;
    }

public:

    C4_ALWAYS_INLINE I size() const noexcept { return m_size; }
    C4_ALWAYS_INLINE I capacity() const noexcept { return m_elm.capacity(); }

    C4_ALWAYS_INLINE T      & elm(I i)       C4_NOEXCEPT_X { C4_XASSERT(i < m_size); return m_elms[i]; }
    C4_ALWAYS_INLINE T const& elm(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < m_size); return m_elms[i]; }

    C4_ALWAYS_INLINE I prev(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < m_size); return m_prev[i]; }
    C4_ALWAYS_INLINE I next(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < m_size); return m_next[i]; }

    C4_ALWAYS_INLINE iterator begin() noexcept { return iterator(this, m_head); }
    C4_ALWAYS_INLINE iterator end  () noexcept { return iterator(this, m_tail); }

    C4_ALWAYS_INLINE const_iterator begin() const noexcept { return const_iterator(this, m_head); }
    C4_ALWAYS_INLINE const_iterator end  () const noexcept { return const_iterator(this, m_tail); }

public:

     void push_back(T const& var)
     {
         I cap = capacity();
         if(size() == cap)
         {
             _growto(cap, cap+1);
         }
         I pos = m_fhead;
         C4_XASSERT(pos != npos && pos < m_size);
         c4::construct(&m_elm[pos], var);
         m_prev[pos] = m_tail;
         m_next[pos] = npos;
         m_tail = pos;
         C4_XASSERT(m_fhead != npos);
         m_fhead = m_next[m_fhead];
         ++m_size;
     }
     void push_front(T const& var)
     {
     }

};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template< class T, class I, template< class T, class I > class RawStorage >
class flat_fwd_list
{
public:

    RawStorage< flat_fwd_list_elm<T, I>, I > m_elms;

    I m_head;
    I m_tail;
    I m_size;
    I m_fhead;

public:

    using value_type = T;
    using size_type = I;

    using iterator = list_iterator< T, flat_fwd_list >;
    using const_iterator = list_iterator< const T, const flat_fwd_list >;

public:

    flat_fwd_list() : m_elms(), m_head(0), m_tail(0), m_size(0), m_fhead(0) {}

public:

    C4_ALWAYS_INLINE I size() const noexcept { return m_size; }
    C4_ALWAYS_INLINE I capacity() const noexcept { return m_elm.capacity(); }

    C4_ALWAYS_INLINE T      & elm(I i)       C4_NOEXCEPT_X { C4_XASSERT(i < m_elm.size()); return m_elms[i].elm; }
    C4_ALWAYS_INLINE T const& elm(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < m_elm.size()); return m_elms[i].elm; }

    C4_ALWAYS_INLINE I next(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < m_next.size()); return m_next[i].next; }

    C4_ALWAYS_INLINE iterator begin() noexcept { return iterator(this, m_head); }
    C4_ALWAYS_INLINE iterator end  () noexcept { return iterator(this, m_tail); }

    C4_ALWAYS_INLINE const_iterator begin() const noexcept { return const_iterator(this, m_head); }
    C4_ALWAYS_INLINE const_iterator end  () const noexcept { return const_iterator(this, m_tail); }

};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template< class T, class I, template< class T, class I > class RawStorage >
class split_fwd_list
{
public:

    RawStorage< T, I > m_elm;
    RawStorage< I, I > m_next;

    I m_head;
    I m_tail;
    I m_size;
    I m_fhead;

public:

    using value_type = T;
    using size_type = I;

    using iterator = list_iterator< T, split_fwd_list >;
    using const_iterator = list_iterator< const T, const split_fwd_list >;

public:

    split_fwd_list() : m_elm(), m_next(), m_head(0), m_tail(0), m_size(0), m_fhead(0) {}

public:

    C4_ALWAYS_INLINE I size() const noexcept { return m_size; }
    C4_ALWAYS_INLINE I capacity() const noexcept { return m_elm.capacity(); }

    C4_ALWAYS_INLINE T      & elm(I i)       C4_NOEXCEPT_X { C4_XASSERT(i < m_size); return m_elms[i]; }
    C4_ALWAYS_INLINE T const& elm(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < m_size); return m_elms[i]; }

    C4_ALWAYS_INLINE I next(I i) const C4_NOEXCEPT_X { C4_XASSERT(i < m_size); return m_next[i]; }

    C4_ALWAYS_INLINE iterator begin() noexcept { return iterator(this, m_head); }
    C4_ALWAYS_INLINE iterator end  () noexcept { return iterator(this, m_tail); }

    C4_ALWAYS_INLINE const_iterator begin() const noexcept { return const_iterator(this, m_head); }
    C4_ALWAYS_INLINE const_iterator end  () const noexcept { return const_iterator(this, m_tail); }

};
C4_END_NAMESPACE(c4)

#endif /* _C4_LIST_HPP_ */
