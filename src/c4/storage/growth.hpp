#ifndef _C4_STORAGE_GROWTH_HPP_
#define _C4_STORAGE_GROWTH_HPP_

#include "c4/config.hpp"
#include "c4/error.hpp"

C4_BEGIN_NAMESPACE(c4)
C4_BEGIN_NAMESPACE(stg)

/** @defgroup storage_growth_policies */


/** Grow by the least possible amount.
 * @ingroup storage_growth_policies */
struct growth_least
{
    C4_ALWAYS_INLINE static size_t next_size(size_t /*elm_size*/, size_t curr, size_t at_least) noexcept
    {
        if(at_least <= curr) return curr;
        return at_least;
    }
};


/** Grow to the double of the current size if it is bigger than at_least;
 * if not, then just to at_least.
 * @ingroup storage_growth_policies  */
struct growth_pot
{
    C4_ALWAYS_INLINE static size_t next_size(size_t /*elm_size*/, size_t curr, size_t at_least) noexcept
    {
        if(at_least <= curr) return curr;
        size_t nxt = (curr << 1);
        return nxt > at_least ? nxt : at_least;
    }
};


/** Grow by the Fibonacci ratio if the result is bigger than at_least;
 * if not, then just to at_least.
 * @ingroup storage_growth_policies  */
struct growth_phi
{
    C4_ALWAYS_INLINE static size_t next_size(size_t /*elm_size*/, size_t curr, size_t at_least) noexcept
    {
        if(at_least <= curr) return curr;
        size_t nxt = size_t(float(curr) * 1.618f);
        nxt = nxt > 0 ? nxt : 1;
        return nxt > at_least ? nxt : at_least;
    }
};


/** grow another growth policy in fixed chunk sizes. Useful for SIMD buffers.
 * @ingroup storage_growth_policies  */
template< class GrowthPolicy, size_t PowerOfTwoChunkSize >
struct growth_chunks
{
    C4_STATIC_ASSERT(PowerOfTwoChunkSize > 1);
    C4_STATIC_ASSERT_MSG((PowerOfTwoChunkSize & (PowerOfTwoChunkSize - 1)) == 0, "chunk size must be a power of two");

    using growth_policy = GrowthPolicy;
    constexpr static const size_t chunk_size = PowerOfTwoChunkSize;

    C4_ALWAYS_INLINE static size_t next_size(size_t elm_size, size_t curr, size_t at_least) noexcept
    {
        size_t next = GrowthPolicy::next_size(curr, at_least);
        size_t rem = (next & (PowerOfTwoChunkSize - 1));
        next += rem ? PowerOfTwoChunkSize - rem : 0;
        C4_ASSERT((next % PowerOfTwoChunkSize) == 0);
        return next;
    }
};
template< class Growth, size_t PowerOfTwoChunkSize >
constexpr const size_t growth_chunks< Growth, PowerOfTwoChunkSize >::chunk_size;


/** grow by powers of 2, then Fibonacci ratio
 * @ingroup storage_growth_policies */
struct growth_default
{
    constexpr static const size_t threshold = 1024;
    C4_ALWAYS_INLINE static size_t next_size(size_t elm_size, size_t curr, size_t at_least) noexcept
    {
        if(at_least*elm_size <= 1024)
            return growth_pot::next_size(elm_size, curr, at_least);
        else
            return growth_phi::next_size(elm_size, curr, at_least);
    }
};


/** @ingroup storage_growth_policies */
template< class SmallGrowthPolicy, size_t Threshold, class LargeGrowthPolicy >
struct growth_custom
{
    using small_policy = SmallGrowthPolicy;
    using large_policy = LargeGrowthPolicy;
    constexpr static const size_t threshold = Threshold;
    C4_ALWAYS_INLINE static size_t next_size(size_t elm_size, size_t curr, size_t at_least) noexcept
    {
        if(at_least*elm_size <= Threshold)
            return SmallGrowthPolicy::next_size(elm_size, curr, at_least);
        else
            return LargeGrowthPolicy::next_size(elm_size, curr, at_least);
    }
};
template< class SmallGrowthPolicy, size_t Threshold, class LargeGrowthPolicy >
constexpr const size_t growth_custom< SmallGrowthPolicy, Threshold, LargeGrowthPolicy >::threshold;

C4_END_NAMESPACE(stg)
C4_END_NAMESPACE(c4)

#endif // _C4_STORAGE_GROWTH_HPP_
