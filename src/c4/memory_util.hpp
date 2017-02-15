#ifndef _C4_MEMORY_UTIL_HPP_
#define _C4_MEMORY_UTIL_HPP_

#include "c4/config.hpp"

#include <string.h>

// Some memory utilities.

C4_BEGIN_NAMESPACE(c4)

/** set the given memory to zero */
C4_ALWAYS_INLINE void mem_zero(void* mem, size_t num_bytes)
{
    memset(mem, 0, num_bytes);
}
/** set the given memory to zero */
template< class T >
C4_ALWAYS_INLINE void mem_zero(T* mem, size_t num_elms)
{
    memset(mem, 0, sizeof(T) * num_elms);
}
/** set the given memory to zero */
template< class T >
C4_ALWAYS_INLINE void mem_zero(T* mem)
{
    memset(mem, 0, sizeof(T));
}

bool mem_overlaps(void const* a, void const* b, size_t sza, size_t szb);

void mem_repeat(void* dest, void const* pattern, size_t pattern_size, size_t num_times);

C4_END_NAMESPACE(c4)

#endif /* _C4_MEMORY_UTIL_HPP_ */
