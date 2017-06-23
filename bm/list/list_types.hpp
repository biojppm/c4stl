#ifndef _C4_BM_LIST_TYPES_HPP_
#define _C4_BM_LIST_TYPES_HPP_

#include "c4/list.hpp"

#include <list>
#include <forward_list>

namespace bm = benchmark;

namespace c4 {
template< size_t N > struct NumBytes { char buf[N]; };

template< class L >
struct do_reserve
{
    static void reserve(L &li, size_t cap)
    {
        li.reserve(szconv< I >(cap));
    }
};

template< class T, class... Args >
struct do_reserve< std::list< T, Args... > >
{
    using L = std::list< T, Args... >;
    static void reserve(L &/*li*/, size_t /*cap*/)
    {
    }
};

template< class T, class... Args >
struct do_reserve< std::forward_list< T, Args... > >
{
    using L = std::forward_list< T, Args... >;
    static void reserve(L &/*li*/, size_t /*cap*/)
    {
    }
};

} // end namespace c4

#endif // _C4_BM_LIST_TYPES_HPP_
