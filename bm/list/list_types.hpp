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


template< class T, class I=C4_SIZE_TYPE > using flat_list__raw      = flat_list     < T, I, stg::raw< flat_list_elm<T, I>, I > >;
template< class T, class I=C4_SIZE_TYPE > using split_list__raw     = split_list    < T, I, stg::raw< T, I > >;
template< class T, class I=C4_SIZE_TYPE > using flat_fwd_list__raw  = flat_fwd_list < T, I, stg::raw< flat_fwd_list_elm<T, I>, I > >;
template< class T, class I=C4_SIZE_TYPE > using split_fwd_list__raw = split_fwd_list< T, I, stg::raw< T, I > >;

template< class T, class I=C4_SIZE_TYPE, size_t N=16 > using flat_list__small      = flat_list     < T, I, stg::raw_small< flat_list_elm<T, I>, I, N > >;
template< class T, class I=C4_SIZE_TYPE, size_t N=16 > using split_list__small     = split_list    < T, I, stg::raw_small< T, I, N > >;
template< class T, class I=C4_SIZE_TYPE, size_t N=16 > using flat_fwd_list__small  = flat_fwd_list < T, I, stg::raw_small< flat_fwd_list_elm<T, I>, I, N > >;
template< class T, class I=C4_SIZE_TYPE, size_t N=16 > using split_fwd_list__small = split_fwd_list< T, I, stg::raw_small< T, I, N > >;

template< class T, class I=C4_SIZE_TYPE, size_t N=256 > using flat_list__fixed      = flat_list     < T, I, stg::raw_fixed< flat_list_elm<T, I>, N, I > >;
template< class T, class I=C4_SIZE_TYPE, size_t N=256 > using split_list__fixed     = split_list    < T, I, stg::raw_fixed< T, N, I > >;
template< class T, class I=C4_SIZE_TYPE, size_t N=256 > using flat_fwd_list__fixed  = flat_fwd_list < T, I, stg::raw_fixed< flat_fwd_list_elm<T, I>, N, I > >;
template< class T, class I=C4_SIZE_TYPE, size_t N=256 > using split_fwd_list__fixed = split_fwd_list< T, I, stg::raw_fixed< T, N, I > >;

template< class T, class I=C4_SIZE_TYPE, size_t N=256 > using flat_list__paged      = flat_list     < T, I, stg::raw_paged< flat_list_elm<T, I>, I, N > >;
template< class T, class I=C4_SIZE_TYPE, size_t N=256 > using split_list__paged     = split_list    < T, I, stg::raw_paged< T, I, N > >;
template< class T, class I=C4_SIZE_TYPE, size_t N=256 > using flat_fwd_list__paged  = flat_fwd_list < T, I, stg::raw_paged< flat_fwd_list_elm<T, I>, I, N > >;
template< class T, class I=C4_SIZE_TYPE, size_t N=256 > using split_fwd_list__paged = split_fwd_list< T, I, stg::raw_paged< T, I, N > >;

template< class T, class I=C4_SIZE_TYPE > using flat_list__paged_rt      = flat_list     < T, I, stg::raw_paged_rt< flat_list_elm<T, I>, I > >;
template< class T, class I=C4_SIZE_TYPE > using split_list__paged_rt     = split_list    < T, I, stg::raw_paged_rt< T, I > >;
template< class T, class I=C4_SIZE_TYPE > using flat_fwd_list__paged_rt  = flat_fwd_list < T, I, stg::raw_paged_rt< flat_fwd_list_elm<T, I>, I > >;
template< class T, class I=C4_SIZE_TYPE > using split_fwd_list__paged_rt = split_fwd_list< T, I, stg::raw_paged_rt< T, I > >;

} // end namespace c4

#endif // _C4_BM_LIST_TYPES_HPP_
