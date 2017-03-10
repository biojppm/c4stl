#ifndef _C4_TEST_ARCHETYPES_HPP_
#define _C4_TEST_ARCHETYPES_HPP_

#include "c4/test.hpp"
#include "c4/memory_resource.hpp"
#include "c4/allocator.hpp"
#include "c4/char_traits.hpp"

C4_BEGIN_NAMESPACE(c4)

template< class String > class sstream;

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
inline void check_archetype(char     a, char     ref) { EXPECT_EQ(a, ref); }
inline void check_archetype(wchar_t  a, wchar_t  ref) { EXPECT_EQ(a, ref); }
inline void check_archetype(int8_t   a, int8_t   ref) { EXPECT_EQ(a, ref); }
inline void check_archetype(uint8_t  a, uint8_t  ref) { EXPECT_EQ(a, ref); }
inline void check_archetype(int16_t  a, int16_t  ref) { EXPECT_EQ(a, ref); }
inline void check_archetype(uint16_t a, uint16_t ref) { EXPECT_EQ(a, ref); }
inline void check_archetype(int32_t  a, int32_t  ref) { EXPECT_EQ(a, ref); }
inline void check_archetype(uint32_t a, uint32_t ref) { EXPECT_EQ(a, ref); }
inline void check_archetype(int64_t  a, int64_t  ref) { EXPECT_EQ(a, ref); }
inline void check_archetype(uint64_t a, uint64_t ref) { EXPECT_EQ(a, ref); }
inline void check_archetype(float    a, float    ref) { EXPECT_EQ(a, ref); }
inline void check_archetype(double   a, double   ref) { EXPECT_EQ(a, ref); }


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template< class T >
struct archetype_proto
{
    static T get(int which)
    {
        C4_ASSERT(which < 8);
        T p = which;
        return p;
    }
};

template<>
struct archetype_proto<std::string>
{
    static std::string const& get(int which)
    {
        C4_ASSERT(which < 8);
        static const std::string arr[8] = {
            "str0", "str1", "str2", "str3",
            "str4", "str5", "str6", "str7",
        };
        return arr[which];
    }
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template <class T>
struct container_test : public ::testing::Test
{
    using atype = T;

    static std::initializer_list<atype> getil()
    {
        static const std::initializer_list< atype > il{
            archetype_proto<T>::get(0), archetype_proto<T>::get(1), archetype_proto<T>::get(2), archetype_proto<T>::get(3),
            archetype_proto<T>::get(4), archetype_proto<T>::get(5), archetype_proto<T>::get(6), archetype_proto<T>::get(7)
        };
        return il;
    }
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** Resource-owning archetype */

template< class T >
struct exvec3
{
    T x, y, z;
    bool operator== (exvec3 const& that) const
    {
        return x == that.x && y == that.y && z == that.z;
    }
};
template< class String, class T >
sstream< String >& operator<< (sstream< String >& ss, exvec3<T> const& v)
{
    using char_type = typename sstream< String >::char_type;
    ss.printp(C4_TXTTY("({},{},{})", char_type), v.x, v.y, v.z);
    return ss;
}
template< class String, class T >
sstream< String >& operator>> (sstream< String >& ss, exvec3<T> & v)
{
    using char_type = typename sstream< String >::char_type;
    ss.scanp(C4_TXTTY("({},{},{})", char_type), v.x, v.y, v.z);
    return ss;
}

template< class T >
struct archetype_proto<exvec3<T>>
{
    static exvec3<T> get(int which)
    {
        C4_ASSERT(which < 8);
        static const exvec3<T> arr[8] = {
            {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {6, 7, 8},
            {6, 7, 8}, {6, 7, 8}, {6, 7, 8}, {6, 7, 8}
        };
        return arr[which];
    }
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** Resource-owning archetype */
struct IdOwner
{
    static int s_current;
    int id;
    int val;

    void check() const
    {
        EXPECT_TRUE(id > 0);
    }
    void check(IdOwner const& that) const
    {
        check();
        EXPECT_NE(id, that.id);
    }

    IdOwner(int v = 0) { id = ++s_current; v = val; }
    ~IdOwner() { if(id > 0) --s_current; }
    IdOwner(IdOwner const& that) { id = ++s_current; val = that.val; }
    IdOwner(IdOwner     && that) { id = that.id; val = that.val; that.id = 0; }
    IdOwner& operator= (IdOwner const& that) { C4_CHECK(id > 0); --s_current; id = ++s_current; val = that.val; return *this; }
    IdOwner& operator= (IdOwner     && that) { C4_CHECK(id > 0); --s_current; id = that.id; val = that.val; that.id = 0; return *this; }
    bool operator== (IdOwner const& that) const
    {
        return val == that.val;
    }
};

template<>
struct archetype_proto<IdOwner>
{
    static IdOwner get(int which)
    {
        C4_ASSERT(which < 8);
        static const IdOwner arr[8] = {
            0, 1, 2, 3, 4, 5, 6, 7
        };
        return arr[which];
    }
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
        mem = (T*)c4::get_memory_resource()->allocate(sizeof(T), alignof(T));
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
        mem = (T*)c4::get_memory_resource()->allocate(sizeof(T), alignof(T));
        new (mem) T(*that.mem);
    }
    MemOwner(MemOwner && that)
    {
        mem = (T*)c4::get_memory_resource()->allocate(sizeof(T), alignof(T));
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
        mem = (T*)c4::get_memory_resource()->allocate(sizeof(T), alignof(T));
        new (mem) T(std::move(*that.mem));
        return *this;
    }
    bool operator== (MemOwner const& that) const
    {
        return *mem == *that.mem;
    }
};

template <class T>
struct archetype_proto<MemOwner<T>>
{
    static MemOwner<T> get(int which)
    {
        C4_ASSERT(which < 8);
        static const MemOwner<T> arr[8] = {
            archetype_proto<T>::get(0), archetype_proto<T>::get(1), archetype_proto<T>::get(2), archetype_proto<T>::get(3),
            archetype_proto<T>::get(4), archetype_proto<T>::get(5), archetype_proto<T>::get(6), archetype_proto<T>::get(7)
        };
        return arr[which];
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
    using alloc_traits = std::allocator_traits< Alloc >;

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
        mem = alloc_traits::allocate(m_alloc, 1);
        alloc_traits::construct(m_alloc, mem, std::forward< Args >(args)...);
    }
    void free()
    {
        alloc_traits::destroy(m_alloc, mem);
        alloc_traits::deallocate(m_alloc, mem, 1);
        mem = nullptr;
    }
    ~MemOwnerAlloc()
    {
        if(!mem) return;
        free();
    }
    MemOwnerAlloc(MemOwnerAlloc const& that)
    {
        mem = alloc_traits::allocate(m_alloc, 1);
        alloc_traits::construct(m_alloc, mem, *that.mem);
    }
    MemOwnerAlloc(MemOwnerAlloc && that)
    {
        mem = alloc_traits::allocate(m_alloc, 1);
        alloc_traits::construct(m_alloc, mem, std::move(*that.mem));
    }
    MemOwnerAlloc& operator= (MemOwnerAlloc const& that)
    {
        if(mem)
        {
            free();
        }
        mem = alloc_traits::allocate(m_alloc, 1);
        alloc_traits::construct(m_alloc, mem, *that.mem);
        return *this;
    }
    MemOwnerAlloc& operator= (MemOwnerAlloc && that)
    {
        if(mem)
        {
            free();
        }
        mem = alloc_traits::allocate(m_alloc, 1);
        alloc_traits::construct(m_alloc, mem, std::move(*that.mem));
        return *this;
    }
    bool operator== (MemOwnerAlloc const& that) const
    {
        return *mem == *that.mem;
    }
};

template <class T>
struct archetype_proto<MemOwnerAlloc<T>>
{
    static MemOwnerAlloc<T> get(int which)
    {
        C4_ASSERT(which < 8);
        static const MemOwnerAlloc<T> arr[8] = {
            archetype_proto<T>::get(0), archetype_proto<T>::get(1), archetype_proto<T>::get(2), archetype_proto<T>::get(3),
            archetype_proto<T>::get(4), archetype_proto<T>::get(5), archetype_proto<T>::get(6), archetype_proto<T>::get(7)
        };
        return arr[which];
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
template< class T >
struct InsidePtr
{
    T a;
    T b;
    T c;
    T *ptr;

    InsidePtr(int which = 0) : a(), b(), c(), ptr(&a + (which % 3)) {}
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
    bool operator== (InsidePtr const& that) const
    {
        return that.a == a && that.b == b && that.c == c && (ptr - &a) == (that.ptr - &that.a);
    }

};

template <class T>
struct archetype_proto<InsidePtr<T>>
{
    static InsidePtr<T> get(int which)
    {
        C4_ASSERT(which < 8);
        static const InsidePtr<T> arr[8] = {
            0, 1, 2, 3, 4, 5, 6, 7
        };
        return arr[which];
    }
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
C4_BEGIN_NAMESPACE(archetypes)

using scalars = ::testing::Types<
    char, wchar_t, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t, float, double
>;
using containees = ::testing::Types<
    char, wchar_t, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t, float, double,
    IdOwner,
    MemOwner<int>, MemOwner<std::string>,
    MemOwnerAlloc<int>, MemOwnerAlloc<std::string>,
    InsidePtr<int>, InsidePtr<std::string>
>;

C4_END_NAMESPACE(archetypes)

TYPED_TEST_CASE_P(container_test);

C4_END_NAMESPACE(c4)

#endif // _C4_TEST_ARCHETYPES_HPP_
