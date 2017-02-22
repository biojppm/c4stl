#ifndef _C4_ALLOCATOR_HPP_
#define _C4_ALLOCATOR_HPP_

#include "c4/memory_resource.hpp"
#include "c4/ctor_dtor.hpp"
#include <memory>

C4_BEGIN_NAMESPACE(c4)

struct AllocatorBase
{
protected:

    MemoryResource *m_resource;

    AllocatorBase() : m_resource(get_memory_resource()) {}
    AllocatorBase(MemoryResource* mem) noexcept : m_resource(mem) {}

public:

    MemoryResource* resource() const { return m_resource; }

    /** for construct:
     * @see http://en.cppreference.com/w/cpp/experimental/polymorphic_allocator/construct */

/** SFINAE: enable the function with a void return type when a condition is verified */
#define _c4_void_if(cond) C4_ALWAYS_INLINE typename std::enable_if< cond, void >::type
/** @see http://en.cppreference.com/w/cpp/memory/uses_allocator */
#define _c4_uses_allocator(U) std::uses_allocator< U, MemoryResource* >::value
/** @see http://en.cppreference.com/w/cpp/types/is_constructible */
#define _c4_is_constructible(...) std::is_constructible< __VA_ARGS__ >::value

    // 1. types with no allocators
    template< class U, class... Args >
    _c4_void_if( ! _c4_uses_allocator(U) && _c4_is_constructible(U, Args...) )
    construct(U* ptr, Args&&... args)
    {
        c4::construct(ptr, std::forward< Args >(args)...);
    }
    template< class U, class I, class... Args >
    _c4_void_if( ! _c4_uses_allocator(U) && _c4_is_constructible(U, Args...) )
    construct_n(U* ptr, I n, Args&&... args)
    {
        c4::construct_n(ptr, n, std::forward< Args >(args)...);
    }

    // 2. types using allocators (ie, containers)

    // 2.1. can construct(std::allocator_arg_t, MemoryResource*, Args...)
    template< class U, class... Args >
    _c4_void_if(_c4_uses_allocator(U) && _c4_is_constructible(U, std::allocator_arg_t, MemoryResource*, Args...))
    construct(U* ptr, Args&&... args)
    {
        c4::construct(ptr, std::allocator_arg, m_resource, std::forward< Args >(args)...);
    }
    template< class U, class I, class... Args >
    _c4_void_if(_c4_uses_allocator(U) && _c4_is_constructible(U, std::allocator_arg_t, MemoryResource*, Args...))
    construct_n(U* ptr, I n, Args&&... args)
    {
        c4::construct_n(ptr, n, std::allocator_arg, m_resource, std::forward< Args >(args)...);
    }

    // 2.2. can construct(Args..., MemoryResource*)
    template< class U, class... Args >
    _c4_void_if(_c4_uses_allocator(U) && _c4_is_constructible(U, Args..., MemoryResource*))
    construct(U* ptr, Args&&... args)
    {
        c4::construct(ptr, std::forward< Args >(args)..., m_resource);
    }
    template< class U, class I, class... Args >
    _c4_void_if(_c4_uses_allocator(U) && _c4_is_constructible(U, Args..., MemoryResource*))
    construct_n(U* ptr, I n, Args&&... args)
    {
        c4::construct_n(ptr, n, std::forward< Args >(args)..., m_resource);
    }

    template< class U >
    static C4_ALWAYS_INLINE void destroy(U* ptr)
    {
        c4::destroy(ptr);
    }
    template< class U, class I >
    static C4_ALWAYS_INLINE void destroy_n(U* ptr, I n)
    {
        c4::destroy_n(ptr, n);
    }

#undef _c4_void_if
#undef _c4_is_constructible
#undef _c4_uses_allocator

};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/** A polymorphic allocator, acting as a proxy to a memory resource */
template< class T >
class Allocator : public AllocatorBase
{
public:

    using value_type = T;

public:

    template< class U > struct rebind
    {
        using other = Allocator< U >;
    };
    template< class U > Allocator< U > rebound() { return Allocator< U >(*this); }

    Allocator() : AllocatorBase() {}
    Allocator(MemoryResource *r) : AllocatorBase(r) {}
    template< class U > Allocator(Allocator<U> const& that) : AllocatorBase(that.m_resource) {}

    Allocator(Allocator const&) = default;
    Allocator(Allocator     &&) = default;

    Allocator& operator= (Allocator const&) = delete; // WTF? why? @see http://en.cppreference.com/w/cpp/memory/polymorphic_allocator
    Allocator& operator= (Allocator     &&) = default;

    /** returns a default-constructed polymorphic allocator object
     * @see http://en.cppreference.com/w/cpp/memory/polymorphic_allocator/select_on_container_copy_construction      */
    Allocator select_on_container_copy_construct() const { return Allocator(*this); }

    T* allocate(size_t num_objs, size_t alignment = alignof(T))
    {
        C4_ASSERT(m_resource != nullptr);
        void* vmem = m_resource->allocate(num_objs * sizeof(T), alignment);
        T* mem = static_cast< T* >(vmem);
        return mem;
    }

    void deallocate(T * ptr, size_t num_objs, size_t alignment = alignof(T))
    {
        C4_ASSERT(m_resource != nullptr);
        m_resource->deallocate(ptr, num_objs * sizeof(T), alignment);
    }

    T* reallocate(T* ptr, size_t oldnum, size_t newnum, size_t alignment = alignof(T))
    {
        C4_ASSERT(m_resource != nullptr);
        void* vmem = m_resource->reallocate(ptr, oldnum * sizeof(T), newnum * sizeof(T), alignment);
        T* mem = static_cast< T* >(vmem);
        return mem;
    }

};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template< class T, size_t N = 16, size_t Alignment = alignof(T) >
class SmallAllocator : public AllocatorBase
{

    union {
        alignas(Alignment) char _m_arr[N * sizeof(T)];
        alignas(Alignment) T m_arr[N];
    };

public:

    template< class U > struct rebind
    {
        using other = SmallAllocator< U >;
    };
    template< class U > SmallAllocator< U > rebound() { return SmallAllocator< U >(*this); }

    SmallAllocator() : AllocatorBase() {}
    SmallAllocator(MemoryResource *r) : AllocatorBase(r) {}
    template< class U > SmallAllocator(SmallAllocator<U> const& that) : AllocatorBase(that.m_resource) {}

    SmallAllocator(SmallAllocator const&) = default;
    SmallAllocator(SmallAllocator     &&) = default;

    SmallAllocator& operator= (SmallAllocator const&) = delete; // WTF? why? @see http://en.cppreference.com/w/cpp/memory/polymorphic_allocator
    SmallAllocator& operator= (SmallAllocator     &&) = default;

    /** returns a default-constructed polymorphic allocator object
     * @see http://en.cppreference.com/w/cpp/memory/polymorphic_allocator/select_on_container_copy_construction      */
    SmallAllocator select_on_container_copy_construct() const { return SmallAllocator(*this); }

    T* allocate(size_t num_objs, size_t alignment = alignof(T))
    {
        C4_ASSERT(m_resource != nullptr);
        if(num_objs <= N)
        {
            return m_arr;
        }
        void* vmem = m_resource->allocate(num_objs * sizeof(T), alignment);
        T* mem = static_cast< T* >(vmem);
        return mem;
    }

    void deallocate(T * ptr, size_t num_objs, size_t alignment = alignof(T))
    {
        if(ptr == &m_arr[0])
        {
            return;
        }
        C4_ASSERT(m_resource != nullptr);
        m_resource->deallocate(ptr, num_objs * sizeof(T), alignment);
    }

    T* reallocate(T* ptr, size_t oldnum, size_t newnum, size_t alignment = alignof(T))
    {
        C4_ASSERT(m_resource != nullptr);
        if(oldnum <= N && newnum <= N)
        {
            return m_arr;
        }
        else if(oldnum <= N && newnum > N)
        {
            return allocate(newnum, alignment);
        }
        else if(oldnum > N && newnum <= N)
        {
            deallocate(ptr, oldnum, alignment);
            return m_arr;
        }
        void* vmem = m_resource->reallocate(ptr, oldnum * sizeof(T), newnum * sizeof(T), alignment);
        T* mem = static_cast< T* >(vmem);
        return mem;
    }

private:

    MemoryResource* m_resource;

};

C4_END_NAMESPACE(c4)

#endif /* _C4_ALLOCATOR_HPP_ */
