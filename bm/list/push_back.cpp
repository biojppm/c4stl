#include "benchmark/benchmark.h"

#include "c4/log.hpp"
#include "c4/allocator.hpp"
#include "c4/list.hpp"

#include <list>
#include <forward_list>

namespace bm = benchmark;

template< size_t N > struct NumBytes { char buf[N]; };

template< class L >
struct do_reserve
{
    static void reserve(L &li, size_t cap)
    {
        li.reserve(cap);
    }
};

template< class T, class... Args >
struct do_reserve< std::list< T, Args... > >
{
    using L = std::list< T, Args... >;
    static void reserve(L &li, size_t cap)
    {
    }
};

template< class T, class... Args >
struct do_reserve< std::forward_list< T, Args... > >
{
    using L = std::forward_list< T, Args... >;
    static void reserve(L &li, size_t cap)
    {
    }
};


template< class T, class I > using flat_list__raw      = c4::flat_list     < T, I, c4::stg::raw< c4::flat_list_elm<T, I>, I > >;
template< class T, class I > using split_list__raw     = c4::split_list    < T, I, c4::stg::raw< T, I > >;
template< class T, class I > using flat_fwd_list__raw  = c4::flat_fwd_list < T, I, c4::stg::raw< c4::flat_fwd_list_elm<T, I>, I > >;
template< class T, class I > using split_fwd_list__raw = c4::split_fwd_list< T, I, c4::stg::raw< T, I > >;

template< class T, class I, size_t N > using flat_list__raw_fixed      = c4::flat_list     < T, I, c4::stg::raw_fixed< c4::flat_list_elm<T, I>, N, I > >;
template< class T, class I, size_t N > using split_list__raw_fixed     = c4::split_list    < T, I, c4::stg::raw_fixed< T, N, I > >;
template< class T, class I, size_t N > using flat_fwd_list__raw_fixed  = c4::flat_fwd_list < T, I, c4::stg::raw_fixed< c4::flat_fwd_list_elm<T, I>, N, I > >;
template< class T, class I, size_t N > using split_fwd_list__raw_fixed = c4::split_fwd_list< T, I, c4::stg::raw_fixed< T, N, I > >;

template< class T, class I, size_t N > using flat_list__raw_paged      = c4::flat_list     < T, I, c4::stg::raw_paged< c4::flat_list_elm<T, I>, I, N > >;
template< class T, class I, size_t N > using split_list__raw_paged     = c4::split_list    < T, I, c4::stg::raw_paged< T, I, N > >;
template< class T, class I, size_t N > using flat_fwd_list__raw_paged  = c4::flat_fwd_list < T, I, c4::stg::raw_paged< c4::flat_fwd_list_elm<T, I>, I, N > >;
template< class T, class I, size_t N > using split_fwd_list__raw_paged = c4::split_fwd_list< T, I, c4::stg::raw_paged< T, I, N > >;

template< class T, class I > using flat_list__raw_paged_rt      = c4::flat_list     < T, I, c4::stg::raw_paged_rt< c4::flat_list_elm<T, I>, I > >;
template< class T, class I > using split_list__raw_paged_rt     = c4::split_list    < T, I, c4::stg::raw_paged_rt< T, I > >;
template< class T, class I > using flat_fwd_list__raw_paged_rt  = c4::flat_fwd_list < T, I, c4::stg::raw_paged_rt< c4::flat_fwd_list_elm<T, I>, I > >;
template< class T, class I > using split_fwd_list__raw_paged_rt = c4::split_fwd_list< T, I, c4::stg::raw_paged_rt< T, I > >;

#define BM(name, li, ...)                                               \
    BENCHMARK_TEMPLATE(BM_List##name, li< __VA_ARGS__ >)                \
    ->RangeMultiplier(2)                                                \
    ->Range(4, 1<<19)                                                   \
    ->Complexity()

#define CALL_BM_FOR_C4LIST(name, litype, ty, ...) \
    BM(name, litype, ty, size_t, ## __VA_ARGS__);

#define CALL_BM(name, ty)                                           \
    BM(name, std::list, ty);                                        \
    CALL_BM_FOR_C4LIST(name, flat_list__raw, ty);                   \
    CALL_BM_FOR_C4LIST(name, flat_list__raw_paged, ty, 256);        \
    CALL_BM_FOR_C4LIST(name, flat_list__raw_paged_rt, ty);          \
    CALL_BM_FOR_C4LIST(name, split_list__raw, ty);                  \
    CALL_BM_FOR_C4LIST(name, split_list__raw_paged, ty, 256);       \
    CALL_BM_FOR_C4LIST(name, split_list__raw_paged_rt, ty);         \
    CALL_BM_FOR_C4LIST(name, flat_fwd_list__raw, ty);               \
    CALL_BM_FOR_C4LIST(name, flat_fwd_list__raw_paged, ty, 256);    \
    CALL_BM_FOR_C4LIST(name, flat_fwd_list__raw_paged_rt, ty);      \
    CALL_BM_FOR_C4LIST(name, split_fwd_list__raw, ty);              \
    CALL_BM_FOR_C4LIST(name, split_fwd_list__raw_paged, ty, 256);   \
    CALL_BM_FOR_C4LIST(name, split_fwd_list__raw_paged_rt, ty)

template< class List >
void do_push_back(List& li, bm::State &st)
{
    using T = typename List::value_type;
    T v{};
    size_t count = 0;
    while(st.KeepRunning())
    {
        for(int i = 0, e = st.range(0); i < e; ++i)
        {
            if(li.size() == li.max_size()) li.clear();
            li.push_back(v);
            ++count;
        }
        li.clear();
    }
    st.SetComplexityN(st.range(0) * sizeof(T));
    st.SetItemsProcessed(count);
    st.SetBytesProcessed(count * sizeof(T));
}

template< class List >
void BM_ListPushBack(bm::State& st)
{
    List li;
    do_push_back(li, st);
}

template< class List >
void BM_ListPushBackWithReserve(bm::State& st)
{
    List li;
    do_reserve<List>::reserve(li, st.range(0));
    do_push_back(li, st);
}

/* This is N2 as expected.
CALL_BM(PushBack, int);
CALL_BM(PushBack, NumBytes<64>);
CALL_BM(PushBack, NumBytes<512>);
*/

CALL_BM(PushBackWithReserve, int);
CALL_BM(PushBackWithReserve, NumBytes<64>);
CALL_BM(PushBackWithReserve, NumBytes<512>);


BENCHMARK_MAIN();
