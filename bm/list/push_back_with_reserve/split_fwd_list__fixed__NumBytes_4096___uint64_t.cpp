#include "benchmark/benchmark.h"

#include "c4/log.hpp"
#include "c4/allocator.hpp"
#include "../list_types.hpp"

namespace bm = benchmark;

namespace c4 {

template< class List >
void BM_ListPushBackWithReserve(bm::State& st)
{
    List li;
    do_reserve<List>::reserve(li, st.range(0));
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

BENCHMARK_TEMPLATE(BM_ListPushBackWithReserve, split_fwd_list__fixed< NumBytes<4096>, uint64_t >)
    ->RangeMultiplier(2)
    ->Range(4, 1<<19)
    ->Complexity();

} // end namespace c4

BENCHMARK_MAIN()