#ifndef _C4_TEST_ARCHETYPES_HPP_
#define _C4_TEST_ARCHETYPES_HPP_

#include "c4/test.hpp"
#include "c4/memory_resource.hpp"
#include "c4/allocator.hpp"

C4_BEGIN_NAMESPACE(c4)
C4_BEGIN_NAMESPACE(archetypes)

template< class T > void check_archetype(T const& a) { a.check(); }
template< class T > void check_archetype(T const& a, T const& ref) { a.check(ref); }
inline void check_archetype(char    ) {}
inline void check_archetype(wchar_t ) {}
inline void check_archetype(int8_t  ) {}
inline void check_archetype(uint8_t ) {}
inline void check_archetype(int16_t ) {}
inline void check_archetype(uint16_t) {}
inline void check_archetype(int32_t ) {}
inline void check_archetype(uint32_t) {}
inline void check_archetype(int64_t ) {}
inline void check_archetype(uint64_t) {}
inline void check_archetype(float   ) {}
inline void check_archetype(double  ) {}
inline void check_archetype(char    , char    ) {}
inline void check_archetype(wchar_t , wchar_t ) {}
inline void check_archetype(int8_t  , int8_t  ) {}
inline void check_archetype(uint8_t , uint8_t ) {}
inline void check_archetype(int16_t , int16_t ) {}
inline void check_archetype(uint16_t, uint16_t) {}
inline void check_archetype(int32_t , int32_t ) {}
inline void check_archetype(uint32_t, uint32_t) {}
inline void check_archetype(int64_t , int64_t ) {}
inline void check_archetype(uint64_t, uint64_t) {}
inline void check_archetype(float   , float   ) {}
inline void check_archetype(double  , double  ) {}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** Resource-owning archetype */
struct IdOwner
{
    static int s_current;
    int id;

    void check() const
    {
        EXPECT_TRUE(id > 0);
    }
    void check(IdOwner const& that) const
    {
        check();
        EXPECT_NE(id, that.id);
    }

    IdOwner() { id = ++s_current; }
    ~IdOwner() { if(id > 0) --s_current; }
    IdOwner(IdOwner const& that) { id = ++s_current; }
    IdOwner(IdOwner     && that) { id = that.id; that.id = 0; }
    IdOwner& operator= (IdOwner const& that) { C4_CHECK(id > 0); --s_current; id = ++s_current; return *this; }
    IdOwner& operator= (IdOwner     && that) { C4_CHECK(id > 0); --s_current; id = that.id; that.id = 0; return *this; }
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** Memory-owning archetype, raw mem resource calls */
template< class T >
struct MemOwner
{
    T *mem;

    void check() const
    {
        EXPECT_NE(mem, nullptr);
        check_archetype(*mem);
    }
    void check(MemOwner const& that) const
    {
        EXPECT_NE(mem, that.mem);
    }

    template< class ...Args >
    MemOwner(Args && ...args)
    {
        mem = c4::get_memory_resource()->allocate(sizeof(T), alignof(T));
        new (mem) T(std::forward< Args >(args)...);
    }
    ~MemOwner()
    {
        if(!mem) return;
        mem->~T();
        c4::get_memory_resource()->deallocate(mem, sizeof(T), alignof(T));
        mem = nullptr;
    }
    MemOwner(MemOwner const& that)
    {
        mem = c4::get_memory_resource()->allocate(sizeof(T), alignof(T));
        new (mem) T(*that.mem);
    }
    MemOwner(MemOwner && that)
    {
        mem = c4::get_memory_resource()->allocate(sizeof(T), alignof(T));
        new (mem) T(std::move(*that.mem));
    }
    MemOwner& operator= (MemOwner const& that)
    {
        if(mem)
        {
            mem->~T();
            c4::get_memory_resource()->deallocate(mem, sizeof(T), alignof(T));
        }
        mem = c4::get_memory_resource()->allocate(sizeof(T), alignof(T));
        new (mem) T(*that.mem);
        return *this;
    }
    MemOwner& operator= (MemOwner && that)
    {
        if(mem)
        {
            mem->~T();
            c4::get_memory_resource()->deallocate(mem, sizeof(T), alignof(T));
        }
        mem = c4::get_memory_resource()->allocate(sizeof(T), alignof(T));
        new (mem) T(std::move(*that.mem));
        return *this;
    }
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** Memory-owning archetype, with allocator */
template< class T, class Alloc = c4::Allocator< T > >
struct MemOwnerAlloc
{
    T *mem;
    Alloc m_alloc;
    using allocator_traits = std::allocator_traits< Alloc >;

    void check() const
    {
        EXPECT_NE(mem, nullptr);
        check_archetype(*mem);
    }
    void check(MemOwnerAlloc const& that) const
    {
        check();
        EXPECT_NE(mem, that.mem);
    }

    template< class ...Args >
    MemOwnerAlloc(Args && ...args)
    {
        mem = allocator_traits::allocate(m_alloc, 1);
        allocator_traits::construct(m_alloc, mem, std::forward< Args >(args)...);
    }
    void free()
    {
        allocator_traits::destroy(m_alloc, mem);
        allocator_traits::deallocate(m_alloc, mem, 1);
        mem = nullptr;
    }
    ~MemOwnerAlloc()
    {
        if(!mem) return;
        free();
    }
    MemOwnerAlloc(MemOwnerAlloc const& that)
    {
        mem = allocator_traits::allocate(m_alloc, 1);
        allocator_traits::construct(m_alloc, mem, *that.mem);
    }
    MemOwnerAlloc(MemOwnerAlloc && that)
    {
        mem = allocator_traits::allocate(m_alloc, 1);
        allocator_traits::construct(m_alloc, mem, std::move(*that.mem));
    }
    MemOwnerAlloc& operator= (MemOwnerAlloc const& that)
    {
        if(mem)
        {
            free();
        }
        mem = allocator_traits::allocate(m_alloc, 1);
        allocator_traits::construct(m_alloc, mem, *that.mem);
        return *this;
    }
    MemOwnerAlloc& operator= (MemOwnerAlloc && that)
    {
        if(mem)
        {
            free();
        }
        mem = allocator_traits::allocate(m_alloc, 1);
        allocator_traits::construct(m_alloc, mem, std::move(*that.mem));
        return *this;
    }
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** base class archetype */
struct Base
{
    virtual ~Base() = default;
protected:
    Base() = default;
};
/** derived class archetype */
struct Derived : public Base
{

};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** base class archetype */
template< class T >
struct InsidePtr
{
    T a;
    T b;
    T c;
    T *ptr;

    InsidePtr() { ptr = &b; }
    InsidePtr(InsidePtr const& that) : a(that.a), b(that.b), c(that.c), ptr(&a + (that.ptr - &that.a)) {}
    InsidePtr(InsidePtr     && that) : a(that.a), b(that.b), c(that.c), ptr(&a + (that.ptr - &that.a)) { that.ptr = nullptr; }
    InsidePtr& operator= (InsidePtr const& that) { a = (that.a); b = (that.b); c = (that.c); ptr = (&a + (that.ptr - &that.a)); return *this; }
    InsidePtr& operator= (InsidePtr     && that) { a = (that.a); b = (that.b); c = (that.c); ptr = (&a + (that.ptr - &that.a)); that.ptr = nullptr; return *this; }
    ~InsidePtr() { EXPECT_TRUE(ptr == &a || ptr == &b || ptr == &c); }

    void check() const
    {
        EXPECT_TRUE(ptr == &a || ptr == &b || ptr == &c);
    }
    void check(InsidePtr const& that) const
    {
        check();
        EXPECT_EQ(ptr - &a, that.ptr - &that.a);
    }
};

using scalars = ::testing::Types< char, wchar_t, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t, float, double >;
using containees = ::testing::Types< char, wchar_t, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t, float, double,
    InsidePtr<int>, InsidePtr<std::string>
>;

C4_END_NAMESPACE(archetypes)
C4_END_NAMESPACE(c4)

#endif // _C4_TEST_ARCHETYPES_HPP_
