#ifndef _C4_MEMORY_RESOURCE_HPP_
#define _C4_MEMORY_RESOURCE_HPP_

#include <stdint.h>
#include <cstddef>

#include "c4/preprocessor.hpp"
#include "c4/error.hpp"

C4_BEGIN_NAMESPACE(c4)

// c-style allocation ---------------------------------------------------------

// this API provides unaligned as well as aligned allocation functions.
// These functions forward the call to a modifiable function.

// aligned allocation. Thread-safe.
void* aalloc(size_t sz, size_t alignment);
void afree(void* ptr);
void* arealloc(void* ptr, size_t oldsz, size_t newsz, size_t alignment);

// classic, unaligned allocation. Thread-safe.
void* alloc(size_t sz);
void free(void* ptr);
void* realloc(void* ptr, size_t oldsz, size_t newsz);

// allocation setup facilities
using aalloc_type   = void* (*)(size_t size, size_t alignment);
using afree_type    = void  (*)(void *ptr);
using arealloc_type = void* (*)(void *ptr, size_t oldsz, size_t newsz, size_t alignment);

using alloc_type   = void* (*)(size_t size);
using free_type    = void  (*)(void *ptr);
using realloc_type = void* (*)(void *ptr, size_t oldsz, size_t newsz);

// set the function to be called
void set_aalloc(aalloc_type fn);
void set_afree(afree_type fn);
void set_arealloc(arealloc_type fn);

void set_alloc(alloc_type fn);
void set_free(free_type fn);
void set_realloc(realloc_type fn);

// get the function which will be called
alloc_type get_alloc();
free_type get_free();
realloc_type get_realloc();

aalloc_type get_aalloc();
free_type get_afree();
arealloc_type get_arealloc();

// c++-style allocation -------------------------------------------------------

/** C++17-style memory_resource. See http://en.cppreference.com/w/cpp/experimental/memory_resource */
struct MemoryResource
{
    const char *name = nullptr;
    virtual ~MemoryResource() {}

    void* allocate(size_t sz, size_t alignment = alignof(max_align_t))
    {
        void *mem = this->do_allocate(sz, alignment);
        C4_CHECK_MSG(mem != nullptr, "could not allocate %lu bytes", sz);
        return mem;
    }
    void* reallocate(void* ptr, size_t oldsz, size_t newsz, size_t alignment = alignof(max_align_t))
    {
        void *mem = this->do_reallocate(ptr, oldsz, newsz, alignment);
        C4_CHECK_MSG(mem != nullptr, "could not reallocate from %lu to %lu bytes", oldsz, newsz);
        return mem;
    }
    void deallocate(void* ptr, size_t sz, size_t alignment = alignof(max_align_t))
    {
        this->do_deallocate(ptr, sz, alignment);
    }

protected:

    virtual void* do_allocate(size_t sz, size_t alignment) = 0;
    virtual void* do_reallocate(void* ptr, size_t oldsz, size_t newsz, size_t alignment) = 0;
    virtual void  do_deallocate(void* ptr, size_t sz, size_t alignment) = 0;

};


/** A c4::aalloc-based memory resource. Thread-safe if the implementation called by
 * c4::aalloc() is safe. */
struct MemoryResourceMalloc : public MemoryResource
{

    MemoryResourceMalloc() { name = "malloc"; }
    virtual ~MemoryResourceMalloc() {}

protected:

    virtual void* do_allocate(size_t sz, size_t alignment) override
    {
        return c4::aalloc(sz, alignment);
    }

    virtual void  do_deallocate(void* ptr, size_t sz, size_t alignment) override
    {
        C4_UNUSED(sz);
        C4_UNUSED(alignment);
        c4::afree(ptr);
    }

    virtual void* do_reallocate(void* ptr, size_t oldsz, size_t newsz, size_t alignment) override
    {
        return c4::arealloc(ptr, oldsz, newsz, alignment);
    }

};

C4_ALWAYS_INLINE MemoryResource* get_memory_resource_malloc()
{
    static MemoryResourceMalloc mr;
    return &mr;
}

C4_BEGIN_NAMESPACE(detail)
C4_ALWAYS_INLINE MemoryResource* & get_memory_resource()
{
    static MemoryResource* mr = get_memory_resource_malloc();
    return mr;
}
C4_END_NAMESPACE(detail)

C4_ALWAYS_INLINE MemoryResource* get_memory_resource()
{
    return detail::get_memory_resource();
}
C4_ALWAYS_INLINE void set_memory_resource(MemoryResource* mr)
{
    C4_ASSERT(mr != nullptr);
    detail::get_memory_resource() = mr;
}

struct AllocationCounts
{
    size_t curr_allocs = 0;
    size_t curr_size = 0;
    size_t max_allocs = 0;
    size_t max_size = 0;
    size_t total_allocs = 0;
    size_t sum_size = 0;

    void clear_counts()
    {
        curr_allocs = 0;
        curr_size = 0;
        max_allocs = 0;
        max_size = 0;
        total_allocs = 0;
        sum_size = 0;
    }

    void add_counts(void* ptr, size_t sz)
    {
        if(ptr == nullptr) return;
        ++curr_allocs;
        curr_size += sz;
        max_allocs = curr_allocs > max_allocs ? curr_allocs : max_allocs;
        max_size = curr_size > max_size ? curr_size : max_size;
        ++total_allocs;
        sum_size += sz;
    }
    void rem_counts(void *ptr, size_t sz)
    {
        if(ptr == nullptr) return;
        --curr_allocs;
        curr_size -= sz;
    }

    AllocationCounts operator- (AllocationCounts const& that)
    {
        AllocationCounts r(*this);
        r.curr_allocs -= that.curr_allocs;
        r.curr_size -= that.curr_size;
        r.max_allocs = max_allocs > that.max_allocs ? max_allocs : that.max_allocs;
        r.max_size = max_size > that.max_size ? max_size : that.max_size;
        r.total_allocs -= that.total_allocs;
        r.sum_size -= that.sum_size;
        return r;
    }
    AllocationCounts operator+ (AllocationCounts const& that)
    {
        AllocationCounts r(*this);
        r.curr_allocs += that.curr_allocs;
        r.curr_size += that.curr_size;
        r.max_allocs += max_allocs > that.max_allocs ? max_allocs : that.max_allocs;
        r.max_size += max_size > that.max_size ? max_size : that.max_size;
        r.total_allocs += that.total_allocs;
        r.sum_size += that.sum_size;
        return r;
    }

};


/** a MemoryResource which latches onto another MemoryResource
 * and counts allocations and sizes. */
class MemoryResourceCounts : public MemoryResource
{
public:

    MemoryResourceCounts() : m_resource(get_memory_resource()) { name = m_resource->name; }
    MemoryResourceCounts(MemoryResource *res) : m_resource(res) { name = m_resource->name; }

protected:

    MemoryResource *m_resource;

public:

    AllocationCounts counts;

protected:

    virtual void* do_allocate(size_t sz, size_t alignment) override
    {
        void *ptr = m_resource->allocate(sz, alignment);
        counts.add_counts(ptr, sz);
        return ptr;
    }

    virtual void  do_deallocate(void* ptr, size_t sz, size_t alignment) override
    {
        counts.rem_counts(ptr, sz);
        m_resource->deallocate(ptr, sz, alignment);
    }

    virtual void* do_reallocate(void* ptr, size_t oldsz, size_t newsz, size_t alignment) override
    {
        counts.rem_counts(ptr, oldsz);
        void* nptr = m_resource->reallocate(ptr, oldsz, newsz, alignment);
        counts.add_counts(nptr, newsz);
        return nptr;
    }

};

C4_END_NAMESPACE(c4)

#endif /* _C4_MEMORY_RESOURCE_HPP_ */
