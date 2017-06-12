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

template< class List >
void BM_ListPushBack(bm::State& st)
{
    List li;
    using T = typename List::value_type;
    do_reserve<List>::reserve(li, st.range(0));
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
    st.SetComplexityN(st.range(0));
    st.SetItemsProcessed(count);
    st.SetBytesProcessed(count * sizeof(T));
}


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

#define CALL_BM_FOR_LIST_TYPES(mcr, ...)                                \
    mcr(std::list<int>);                                                \
                                                                        \
    mcr(flat_list__raw<int C4_COMMA uint64_t >);                        \
    mcr(flat_list__raw<int C4_COMMA  int64_t >);                        \
    mcr(flat_list__raw<int C4_COMMA uint32_t >);                        \
    mcr(flat_list__raw<int C4_COMMA  int32_t >);                        \
                                                                        \
    mcr(split_list__raw<int C4_COMMA uint64_t >);                       \
    mcr(split_list__raw<int C4_COMMA  int64_t >);                       \
    mcr(split_list__raw<int C4_COMMA uint32_t >);                       \
    mcr(split_list__raw<int C4_COMMA  int32_t >);                       \
                                                                        \
    mcr(flat_fwd_list__raw<int C4_COMMA uint64_t >);                    \
    mcr(flat_fwd_list__raw<int C4_COMMA  int64_t >);                    \
    mcr(flat_fwd_list__raw<int C4_COMMA uint32_t >);                    \
    mcr(flat_fwd_list__raw<int C4_COMMA  int32_t >);                    \
                                                                        \
    mcr(split_fwd_list__raw<int C4_COMMA uint64_t >);                   \
    mcr(split_fwd_list__raw<int C4_COMMA  int64_t >);                   \
    mcr(split_fwd_list__raw<int C4_COMMA uint32_t >);                   \
    mcr(split_fwd_list__raw<int C4_COMMA  int32_t >);                   \
                                                                        \
    mcr(flat_list__raw_paged<int C4_COMMA uint64_t C4_COMMA 256 >);     \
    mcr(flat_list__raw_paged<int C4_COMMA  int64_t C4_COMMA 256 >);     \
    mcr(flat_list__raw_paged<int C4_COMMA uint32_t C4_COMMA 256 >);     \
    mcr(flat_list__raw_paged<int C4_COMMA  int32_t C4_COMMA 256 >);     \
                                                                        \
    mcr(split_list__raw_paged<int C4_COMMA uint64_t C4_COMMA 256 >);    \
    mcr(split_list__raw_paged<int C4_COMMA  int64_t C4_COMMA 256 >);    \
    mcr(split_list__raw_paged<int C4_COMMA uint32_t C4_COMMA 256 >);    \
    mcr(split_list__raw_paged<int C4_COMMA  int32_t C4_COMMA 256 >);    \
                                                                        \
    mcr(flat_fwd_list__raw_paged<int C4_COMMA uint64_t C4_COMMA 256 >); \
    mcr(flat_fwd_list__raw_paged<int C4_COMMA  int64_t C4_COMMA 256 >); \
    mcr(flat_fwd_list__raw_paged<int C4_COMMA uint32_t C4_COMMA 256 >); \
    mcr(flat_fwd_list__raw_paged<int C4_COMMA  int32_t C4_COMMA 256 >); \
                                                                        \
    mcr(split_fwd_list__raw_paged<int C4_COMMA uint64_t C4_COMMA 256 >); \
    mcr(split_fwd_list__raw_paged<int C4_COMMA  int64_t C4_COMMA 256 >); \
    mcr(split_fwd_list__raw_paged<int C4_COMMA uint32_t C4_COMMA 256 >); \
    mcr(split_fwd_list__raw_paged<int C4_COMMA  int32_t C4_COMMA 256 >);

#define BM(ty) BENCHMARK_TEMPLATE(BM_ListPushBack, ty)->Range(1<<0, 1<<18)->Complexity()
CALL_BM_FOR_LIST_TYPES(BM)
#undef BM


BENCHMARK_MAIN();
