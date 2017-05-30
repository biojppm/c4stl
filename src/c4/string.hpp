#ifndef _C4_STRING_HPP_
#define _C4_STRING_HPP_

/** @file string.hpp Provides string classes. */

/** @defgroup string_classes String classes
 *
 * Provides strings: owned (c4::string, c4::text) and non-owning
 * (c4::substring and c4::substringrs). These classes are meant to be
 * a superset of the std::string design. Some additions are
 * begins_with()/ends_with(), trim()/trimws(), or notably split() and
 * related methods such as popr()/popl(), gpopr()/gpopl(),
 * pushr()/pushl(), basename()/dirname() and the / operator for path
 * concatenation (for example "foo"/"bar" ---> "foo/bar").
 *
 * Unlike STL's string_view, the substrings are writeable; they are
 * essentially a span (in the case of substring) or a spanrs (in the
 * case of substringrs) enriched with all the string methods.
 *
 * The string types are implemented with a CRTP base class without any
 * use of virtual methods. This CRTP base provides the actual string
 * methods, leaving only the resource management and sizing
 * information for the derived classes.
 *
 * A significant improvement over the STL design is that all the
 * sub-selection methods (eg substr()) return a non-owning substring
 * instead of an owning string; this provides substantial savings in
 * memory allocations (depending on usage patterns, of course).
 *
 * Expression templates are used for the string concatenation
 * operators + and / which has the advantage of saving many memory
 * allocations because 1) no temporary strings are used and 2) the
 * length of the result can be found out before any of the partial
 * operations is performed, thus allowing a reserve() before the
 * operations begin. The expression templates incur a compilation
 * cost, though we've generally felt that it is small. If this cost is
 * too much for your usage pattern, the expression templates can be
 * disabled by defining the macro C4_STR_DISABLE_EXPR_TPL (which is
 * disabled by default). This will cause switching to greedy
 * evaluation of the concatenation operations + and / (with an
 * allocation in every operation). So summing two substrings will
 * result in a temporary allocated string. Note that there is still
 * the ownership problem (discussed below) to address with substrings
 * so the savings in compilation time might not be worth the cost of
 * increased memory allocations. As always, test one and the other
 * with your actual usage pattern, and proceed as you find most
 * convenient.
 *
 * The ownership semantics are clear, too:
 *
 * @code
 * string s1("foo"), s2;
 * substring ss1, ss2;
 * csubstring css1, css2;
 *
 * s2 = s1;      // resize s2, and copy "foo" to its buffer.
 * s2 = "bar";   // copy "bar" to the buffer of s2
 *
 * ss1 = s1;     // ss1 is now pointing at the buffer of s1 (no copies are made, just a point-to)
 * ss2 = s2;     // ss2 is now pointing at the buffer of s2 (no copies are made, just a point-to)
 *
 * ss1[0] = 'F'; // ss1 can be used to write into s1: s1's buffer is now "Foo".
 *
 * s1  = "bar";  // copy "bar" to the buffer of s1
 * ss1 = "foo";  // ERROR: ss1 allows writes and cannot point to const memory
 * css1 = "foo"; // OK: csubstring is const-memory.
 * css1 = s1;    // OK. But no writes will be allowed into s1 through css1.
 *
 * ss1 = s2;     // ss1 is now pointing at the buffer of s2 (no copies are made, just a point-to)
 * s1  = ss1;    // copy "bar" to the buffer of s1
 * @endcode
 *
 *
 *------------------------------------
 * ## A word of caution
 *
 *
 * @warning Non-owning strings have a pitfall. Assignment from string
 * objects means "point there", whereas assignment from string
 * concatenation operations means "write the result of this operation
 * into the substring's current buffer". The reason for this is that
 * since they are non-owning, substrings cannot be made to point at
 * the temporary objects created by the concatenation operation (as an
 * aside that would make the substrings point to stale memory, as
 * those temporary objects are destroyed after the assignment). To
 * illustrate this consider the following code:
 *
 * @code
 * string s("supercallifragilisticexpialidocious");
 * string e("a non empty buffer");
 * csubstring css1("foo"), css2("bar"); // these are in const static memory
 * substring ss;
 * C4_ASSERT(ss1.empty());           // true. ss is empty.
 * C4_ASSERT(ss1.data() == nullptr); // true.
 *
 * // NOTE the difference in the meaning of substring assignment
 * // operations marked with (1) or (2):
 * //   (1) assign from existing object: reference semantics (make
 * //       the substring point to the existing object)
 * //   (2) assign from cat operation: copy semantics (sequentially
 * //       into the substring's current buffer).
 *
 * ss = s;                           // (1) Make ss point to the buffer of s
 * C4_ASSERT( ! ss.empty());         // true. ss is now pointing at s.
 * C4_ASSERT(ss.data() == s.data()); // true.
 *
 * s  = css1 + css2; // s's buffer is now "foobar\0llifragilisticexpialidocious\0"
 *
 * ss = css1 + css2; // (2) same result as above
 * ss = css1 / css2; // (2) s's buffer is now "foo/bar\0lifragilisticexpialidocious\0"
 *
 * ss = e;           // (1) make ss point to the buffer of e
 * ss = css1 + css2; // (2) e's buffer is now "foobar\0mpty buffer\0""
 * ss = css1 / css2; // (2) e's buffer is now "foo/bar\0pty buffer\0""
 * ss = css1 + css2; // (2) e's buffer is now "foobar\0\0pty buffer\0""
 *
 * s = css2; // copy from css2's buffer to s's buffer: "bar\0bar\0lifragilisticexpialidocious\0"
 * @endcode
 *
 * This difference in semantics does not apply to owning strings, which
 * always use the less-efficient copy semantics.
 *
 *
 *
 * ## Mechanics of concatenation operations
 *
 * When expression templates are enabled the sequence of cat
 * operations is lazily transformed (by the compiler) into a syntax
 * tree, its result size is found, then the receiving string is
 * reserve()d, and finally the result is written into it. No temporary
 * objects are created and so no copies are made to those temporary
 * objects. Also, no allocations occur except possibly when the
 * receiving string is reserved.
 *
 * When expression templates are disabled, each partial operation is
 * greedily evaluated and assigned to a temporary object which
 * contains an owning string as member (of type basic_text if a
 * basic_text object is one of the operands; otherwise
 * basic_string). This temporary object will then be use as operand to
 * the following cat operation, in turn producing another temporary
 * object. This will proceed until the assignment occurs. Special
 * overloads exist for assigning from this object such that a copy
 * occurs at this time (instead of a vanilla assignment, which as
 * explained above results in a reference when the receiving string is
 * a substring). Thus ensuring that the final result is the same as
 * that of expression templates. This is really (as far as I can tell)
 * the only possible behaviour here: if the return type of the cat
 * operators were directly a basic_string or basic_text, then when
 * assigning to a receiving substring we would get a reference to the
 * stale memory of the meanwhile destroyed temporary object.
 *
 *
 *
 * ## Buffer overflows
 *
 * As always, the efficiency offered by substrings comes with added
 * responsibility. The following is important:
 *
 * @warning Do not assign string concatenation operations (operators + and /)
 * to non-owning strings unless you are sure they have room to write into.
 *
 * c4stl has generous assertions spliced into to the code to prevent
 * buffer overflow accidents from happening.
 *
 * @see C4_ASSERT
 * @see C4_XASSERT
 */

#ifndef _C4_CONFIG_HPP_
#   include "c4/config.hpp"
#endif

#ifndef _C4_ERROR_HPP_
#   include "c4/error.hpp"
#endif

#include "c4/log.hpp"

#include "c4/allocator.hpp"
#include "c4/szconv.hpp"

#include "c4/span.hpp"
#include "c4/sstream.hpp"
#include "c4/string_fwd.hpp"
#include "c4/char_traits.hpp"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


C4_BEGIN_NAMESPACE(c4)

// forward declarations
template< typename C, typename Size, class Str, class Sub >
class _str_crtp;

//-----------------------------------------------------------------------------
/** @todo calls to strsz() should be replaced to calls to the char_traits */
C4_ALWAYS_INLINE size_t strsz(const char* s) { return ::strlen(s); }
C4_ALWAYS_INLINE size_t strsz(char* s) { return ::strlen(s); }
C4_ALWAYS_INLINE size_t strsz(const wchar_t* ws) { return ::wcslen(ws); }
C4_ALWAYS_INLINE size_t strsz(wchar_t* ws) { return ::wcslen(ws); }
template< class StrType >
C4_ALWAYS_INLINE size_t strsz(StrType const& s)
{
    return s.size();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template< class StringClass >
struct string_traits;

template< typename C, typename SizeType >
struct string_traits< basic_substring<C, SizeType> >
{
    enum {
        is_owning = false,
        is_increasable = false,
        has_upper_capacity = true,
        is_allocatable = false,
    };
};
template< typename C, typename Size >
struct string_traits< basic_substringrs<C, Size> >
{
    enum {
        is_owning = false,
        is_increasable = true,
        has_upper_capacity = true,
        is_allocatable = false,
    };
};
template< typename C, typename Size, class Alloc >
struct string_traits< basic_string<C, Size, Alloc> >
{
    enum {
        is_owning = true,
        is_increasable = true,
        has_upper_capacity = false,
        is_allocatable = true,
    };
};
template< typename C, typename Size, class Alloc >
struct string_traits< basic_text<C, Size, Alloc> >
{
    enum {
        is_owning = true,
        is_increasable = true,
        has_upper_capacity = false,
        is_allocatable = true,
    };
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/// @cond dev

/** specialize std::hash for the given _str_crtp-derived type. Search
 for uses of this to find out how to use it. */
#define _C4_IMPLEMENT_TPL_STRIMPL_HASH(str_type, ...)               \
                                                                    \
C4_CONSTEXPR14 size_t hash_bytes(void const* data, size_t sz) noexcept; \
                                                                    \
C4_END_NAMESPACE(c4)                                                \
C4_BEGIN_NAMESPACE(std)                                             \
                                                                    \
template< class Key > struct hash;                                  \
                                                                    \
template< __VA_ARGS__ >                                             \
struct hash< str_type >                                             \
{                                                                   \
    typedef size_t result_type;                                     \
                                                                    \
    C4_ALWAYS_INLINE size_t operator() (str_type const& n) const    \
    {                                                               \
        return c4::hash_bytes(n.data(), n.size());                  \
    }                                                               \
};                                                                  \
                                                                    \
C4_END_NAMESPACE(std)                                               \
C4_BEGIN_NAMESPACE(c4)


#define _C4_IMPLEMENT_STRIMPL_HASH(str_type)    \
    _C4_IMPLEMENT_TPL_STRIMPL_HASH(str_type, )

//-----------------------------------------------------------------------------
// string assignment operations valid irrespective of expression templates enabled

#define _C4STR_ASSIGNOPS_ANY(charname, classname)                       \
                                                                        \
C4_ALWAYS_INLINE classname& operator+= (const charname* str)            \
{                                                                       \
    this->append(str);                                                  \
    return *this;                                                       \
}                                                                       \
C4_ALWAYS_INLINE classname& operator/= (const charname* str)            \
{                                                                       \
    this->pushr(str, '/');                                              \
    return *this;                                                       \
}                                                                       \
template< size_t N >                                                    \
C4_ALWAYS_INLINE classname& operator+= (const charname (&str)[N])       \
{                                                                       \
    this->append(str, N-1);                                             \
    return *this;                                                       \
}                                                                       \
template< size_t N >                                                    \
C4_ALWAYS_INLINE classname& operator/= (const charname (&str)[N])       \
{                                                                       \
    this->pushr(str, N-1, '/');                                         \
    return *this;                                                       \
}                                                                       \
template< typename OSize, class OStr, class OSub >                      \
C4_ALWAYS_INLINE classname& operator+= (_str_crtp< charname, OSize, OStr, OSub > const& that) \
{                                                                       \
    this->append(that);                                                 \
    return *this;                                                       \
}                                                                       \
template< typename OSize, class OStr, class OSub >                      \
C4_ALWAYS_INLINE classname& operator/= (_str_crtp< charname, OSize, OStr, OSub > const& that) \
{                                                                       \
    this->pushr(that, '/');                                             \
    return *this;                                                       \
}

//-----------------------------------------------------------------------------
/** assignment operations without expression templates */

#if defined(C4_STR_DISABLE_EXPR_TPL)


/** wraps the result of a string concatenation + or / operation
 * Use of this class is needed so that the
 * semantics of assigning a concatenation operation to a non-owning
 * string are the same as with expression templates: with this class,
 * we can define special construction and assignment operators,
 * so we are sure that a copy will be performed when assignment from a
 * concatenation operation occurs. This prevents substrings from
 * pointing at stale memory from a temporary object which is destroyed
 * after asssignment. */
template< class Str >
struct _strCatRes
{
    using StrType = typename std::remove_reference< typename std::remove_const< Str >::type >::type;
    StrType tmp;
};


/** define string assignment operations. */
#define _C4STR_ASSIGNOPS(charname, classname)                           \
                                                                        \
_C4STR_ASSIGNOPS_ANY(charname, classname)                               \
                                                                        \
/** allow other _str_crtp derived classes access to our members.        \
 * @warning the ::c4:: qualifier is needed as a workaround to           \
 * gcc bug https://gcc.gnu.org/bugzilla/show_bug.cgi?id=52625 */        \
template< typename sifC, typename sifSize, class sifStr, class sifSub > \
friend class ::c4::_str_crtp;                                           \
                                                                        \
/** @todo add a move assignment; it will need to be SFINAEd based on    \
 * whether the current class is owning or non-owning. */                \
template< class Str >                                                   \
C4_ALWAYS_INLINE classname& operator= (_strCatRes< Str > const& that)   \
{                                                                       \
    C4_ASSERT_SAME_TYPE(typename Str::char_type, char_type);            \
    SizeType sz = szconv< SizeType >(that.tmp.size());                  \
    this->reserve(sz);                                                  \
    this->resize(0);                                                    \
    this->append(that.tmp.data(), sz);                                  \
    return *this;                                                       \
}                                                                       \
                                                                        \
template< class Str >                                                   \
C4_ALWAYS_INLINE void append_res(_strCatRes< Str > const& that)         \
{                                                                       \
    C4_ASSERT_SAME_TYPE(typename Str::char_type, char_type);            \
    this->append(that.tmp);                                             \
}                                                                       \
template< class Str >                                                   \
C4_ALWAYS_INLINE void append_res_dir(_strCatRes< Str > const& that)     \
{                                                                       \
    C4_ASSERT_SAME_TYPE(typename Str::char_type, char_type);            \
    this->pushr(that.tmp, '/');                                         \
}                                                                       \
                                                                        \
template< class Str >                                                   \
C4_ALWAYS_INLINE classname& operator+= (_strCatRes< Str > const& that)  \
{                                                                       \
    C4_ASSERT_SAME_TYPE(typename Str::char_type, char_type);            \
    this->append(that.tmp);                                             \
    return *this;                                                       \
}                                                                       \
template< class Str >                                                   \
C4_ALWAYS_INLINE classname& operator/= (_strCatRes< Str > const& that)  \
{                                                                       \
    C4_ASSERT_SAME_TYPE(typename Str::char_type, char_type);            \
    this->pushr(that.tmp, '/');                                         \
    return *this;                                                       \
}



/** define binary string concatenation operators for same-type operands */
#define _C4STR_DEFINE_BINOPS1TY(ty, tyr) \
    _C4STR_DEFINE_BINOPS1TY_TPL( , ty, tyr)



/** define binary string concatenation operators for same-type
 * operands - template version for when any of the classes are
 * templates */
#define _C4STR_DEFINE_BINOPS1TY_TPL(template_spec, ty, tyr)     \
                                                                \
template_spec                                                   \
C4_ALWAYS_INLINE _strCatRes<tyr> operator+                      \
(                                                               \
    ty lhs,                                                     \
    ty rhs                                                      \
)                                                               \
{                                                               \
    _strCatRes<tyr> result;                                     \
    result.tmp.reserve(strsz(lhs) + strsz(rhs));                \
    result.tmp.append(lhs);                                     \
    result.tmp.append(rhs);                                     \
    return result;                                              \
}                                                               \
template_spec                                                   \
C4_ALWAYS_INLINE _strCatRes<tyr> operator/                      \
(                                                               \
    ty lhs,                                                     \
    ty rhs                                                      \
)                                                               \
{                                                               \
    _strCatRes<tyr> result;                                     \
    result.tmp.reserve(strsz(lhs) + 1 + strsz(rhs));            \
    result.tmp.append(lhs);                                     \
    result.tmp.pushr(rhs);                                      \
    return result;                                              \
}                                                               \
                                                                \
template_spec                                                   \
C4_ALWAYS_INLINE _strCatRes<tyr> operator+                      \
(                                                               \
    _strCatRes<ty> lhs,                                         \
    ty rhs                                                      \
)                                                               \
{                                                               \
    _strCatRes<tyr> result;                                     \
    result.tmp.reserve(strsz(lhs.tmp) + strsz(rhs));            \
    result.tmp.append(lhs.tmp);                                 \
    result.tmp.append(rhs);                                     \
    return result;                                              \
}                                                               \
template_spec                                                   \
C4_ALWAYS_INLINE _strCatRes<tyr> operator/                      \
(                                                               \
    _strCatRes<ty> lhs,                                         \
    ty rhs                                                      \
)                                                               \
{                                                               \
    _strCatRes<tyr> result;                                     \
    result.tmp.reserve(strsz(lhs.tmp) + 1 + strsz(rhs));        \
    result.tmp.append(lhs.tmp);                                 \
    result.tmp.pushr(rhs);                                      \
    return result;                                              \
}                                                               \
                                                                \
template_spec                                                   \
C4_ALWAYS_INLINE _strCatRes<tyr> operator+                      \
(                                                               \
    ty lhs,                                                     \
    _strCatRes<ty> rhs                                          \
)                                                               \
{                                                               \
    _strCatRes<tyr> result;                                     \
    result.tmp.reserve(strsz(lhs) + strsz(rhs.tmp));            \
    result.tmp.append(lhs);                                     \
    result.tmp.append(rhs.tmp);                                 \
    return result;                                              \
}                                                               \
template_spec                                                   \
C4_ALWAYS_INLINE _strCatRes<tyr> operator/                      \
(                                                               \
    ty lhs,                                                     \
    _strCatRes<ty> rhs                                          \
)                                                               \
{                                                               \
    _strCatRes<tyr> result;                                     \
    result.tmp.reserve(strsz(lhs) + 1 + strsz(rhs.tmp));        \
    result.tmp.append(lhs);                                     \
    result.tmp.pushr(rhs.tmp);                                  \
    return result;                                              \
}                                                               \
                                                                \
template_spec                                                   \
C4_ALWAYS_INLINE _strCatRes<tyr> operator+                      \
(                                                               \
    _strCatRes<ty> lhs,                                         \
    _strCatRes<ty> rhs                                          \
)                                                               \
{                                                               \
    _strCatRes<tyr> result;                                     \
    result.tmp.reserve(strsz(lhs.tmp) + strsz(rhs.tmp));        \
    result.tmp.append(lhs.tmp);                                 \
    result.tmp.append(rhs.tmp);                                 \
    return result;                                              \
}                                                               \
template_spec                                                   \
C4_ALWAYS_INLINE _strCatRes<tyr> operator/                      \
(                                                               \
    _strCatRes<ty> lhs,                                         \
    _strCatRes<ty> rhs                                          \
)                                                               \
{                                                               \
    _strCatRes<tyr> result;                                     \
    result.tmp.reserve(strsz(lhs.tmp) + 1 + strsz(rhs.tmp));    \
    result.tmp.append(lhs.tmp);                                 \
    result.tmp.pushr(rhs.tmp);                                  \
    return result;                                              \
}




/** define binary string concatenation operators for different-type
 * operands. Commutativity is enabled via both in lhs+rhs and rhs+lhs
 * versions.*/
#define _C4STR_DEFINE_BINOPS2TY(ty1, ty2, tyr) \
_C4STR_DEFINE_BINOPS2TY_TPL( , ty1, ty2, tyr)


/** define binary string concatenation operators for different-type
 * operands - template version for when any of the classes are
 * templates. Commutativity is enabled via both in lhs+rhs and rhs+lhs
 * versions. */
#define _C4STR_DEFINE_BINOPS2TY_TPL(template_spec, ty1, ty2, tyr)   \
                                                                    \
                                                                    \
/* 4 WRAPPED ty1 + WRAPPED ty2 =================================*/  \
                                                                    \
/* 4.1 lhs=wty1 rhs=wty2 -------------------------------------*/    \
                                                                    \
/* 4.1.1 wty1+wty2 */                                               \
template_spec                                                       \
C4_ALWAYS_INLINE _strCatRes<tyr> operator+                          \
(                                                                   \
    _strCatRes<ty1> const& lhs,                                     \
    _strCatRes<ty2> const& rhs                                      \
)                                                                   \
{                                                                   \
    _strCatRes<tyr> result;                                         \
    result.tmp.reserve(strsz(lhs.tmp) + strsz(rhs.tmp));            \
    result.tmp.append(lhs.tmp);                                     \
    result.tmp.append(rhs.tmp);                                     \
    return result;                                                  \
}                                                                   \
/* 4.1.2 wty1/wty2 */                                               \
template_spec                                                       \
C4_ALWAYS_INLINE _strCatRes<tyr> operator/                          \
(                                                                   \
    _strCatRes<ty1> const& lhs,                                     \
    _strCatRes<ty2> const& rhs                                      \
)                                                                   \
{                                                                   \
    _strCatRes<tyr> result;                                         \
    result.tmp.reserve(strsz(lhs.tmp) + 1 + strsz(rhs.tmp));        \
    result.tmp.append(lhs.tmp);                                     \
    result.tmp.pushr(rhs.tmp);                                 \
    return result;                                                  \
}                                                                   \
                                                                    \
                                                                    \
/* 4.2 lhs=wty2 rhs=wty1 ----------------------------------------*/ \
                                                                    \
/* 4.2.1 wty2+wty1 */                                               \
template_spec                                                       \
C4_ALWAYS_INLINE _strCatRes<tyr> operator+                          \
(                                                                   \
    _strCatRes<ty2> const& lhs,                                     \
    _strCatRes<ty1> const& rhs                                      \
)                                                                   \
{                                                                   \
    _strCatRes<tyr> result;                                         \
    result.tmp.reserve(strsz(lhs.tmp) + strsz(rhs.tmp));            \
    result.tmp.append(lhs.tmp);                                     \
    result.tmp.append(rhs.tmp);                                     \
    return result;                                                  \
}                                                                   \
/* 4.2.2 wty2/wty1 */                                               \
template_spec                                                       \
C4_ALWAYS_INLINE _strCatRes<tyr> operator/                          \
(                                                                   \
    _strCatRes<ty2> const& lhs,                                     \
    _strCatRes<ty1> const& rhs                                      \
)                                                                   \
{                                                                   \
    _strCatRes<tyr> result;                                         \
    result.tmp.reserve(strsz(lhs.tmp) + 1 + strsz(rhs.tmp));        \
    result.tmp.append(lhs.tmp);                                     \
    result.tmp.pushr(rhs.tmp);                                      \
    return result;                                                  \
}                                                                   \
                                                                    \
                                                                    \
/* 3 BARE ty1 + WRAPPED ty2 =================================*/     \
                                                                    \
/* 3.1 lhs=ty1 rhs=wty2 -------------------------------------*/     \
                                                                    \
/* 3.1.1 ty1+wty2 */                                                \
template_spec                                                       \
C4_ALWAYS_INLINE _strCatRes<tyr> operator+                          \
(                                                                   \
    ty1 const& lhs,                                                 \
    _strCatRes<ty2> const& rhs                                      \
)                                                                   \
{                                                                   \
    _strCatRes<tyr> result;                                         \
    result.tmp.reserve(strsz(lhs) + strsz(rhs.tmp));                \
    result.tmp.append(lhs);                                         \
    result.tmp.append(rhs.tmp);                                     \
    return result;                                                  \
}                                                                   \
/* 3.1.2 ty1/wty2 */                                                \
template_spec                                                       \
C4_ALWAYS_INLINE _strCatRes<tyr> operator/                          \
(                                                                   \
    ty1 const& lhs,                                                 \
    _strCatRes<ty2> const& rhs                                      \
)                                                                   \
{                                                                   \
    _strCatRes<tyr> result;                                         \
    result.tmp.reserve(strsz(lhs) + 1 + strsz(rhs.tmp));            \
    result.tmp.append(lhs);                                         \
    result.tmp.pushr(rhs.tmp);                                      \
    return result;                                                  \
}                                                                   \
                                                                    \
                                                                    \
/* 3.2 lhs=wty2 rhs=ty1 ----------------------------------------*/  \
                                                                    \
/* 3.2.1 wty2+ty1 */                                                \
template_spec                                                       \
C4_ALWAYS_INLINE _strCatRes<tyr> operator+                          \
(                                                                   \
    _strCatRes<ty2> const& lhs,                                     \
    ty1 const& rhs                                                  \
)                                                                   \
{                                                                   \
    _strCatRes<tyr> result;                                         \
    result.tmp.reserve(strsz(lhs.tmp) + strsz(rhs));                \
    result.tmp.append(lhs.tmp);                                     \
    result.tmp.append(rhs);                                         \
    return result;                                                  \
}                                                                   \
/* 3.2.2 wty2/ty1 */                                                \
template_spec                                                       \
C4_ALWAYS_INLINE _strCatRes<tyr> operator/                          \
(                                                                   \
    _strCatRes<ty2> const& lhs,                                     \
    ty1 const& rhs                                                  \
)                                                                   \
{                                                                   \
    _strCatRes<tyr> result;                                         \
    result.tmp.reserve(strsz(lhs.tmp) + 1 + strsz(rhs));            \
    result.tmp.append(lhs.tmp);                                     \
    result.tmp.pushr(rhs);                                          \
    return result;                                                  \
}                                                                   \
                                                                    \
                                                                    \
/* 2 WRAPPED ty1 + BARE ty2 =================================*/     \
                                                                    \
/* 2.1 lhs=wty1 rhs=ty2 ----------------------------------------*/  \
                                                                    \
/* 2.1.1 wty1+ty2 */                                                \
template_spec                                                       \
C4_ALWAYS_INLINE _strCatRes<tyr> operator+                          \
(                                                                   \
    _strCatRes<ty1> const& lhs,                                     \
    ty2 const& rhs                                                  \
)                                                                   \
{                                                                   \
    _strCatRes<tyr> result;                                         \
    result.tmp.reserve(strsz(lhs.tmp) + strsz(rhs));                \
    result.tmp.append(lhs.tmp);                                     \
    result.tmp.append(rhs);                                         \
    return result;                                                  \
}                                                                   \
/* 2.1.2 wty1/ty2 */                                                \
template_spec                                                       \
C4_ALWAYS_INLINE _strCatRes<tyr> operator/                          \
(                                                                   \
    _strCatRes<ty1> const& lhs,                                     \
    ty2 const& rhs                                                  \
)                                                                   \
{                                                                   \
    _strCatRes<tyr> result;                                         \
    result.tmp.reserve(strsz(lhs.tmp) + 1 + strsz(rhs));            \
    result.tmp.append(lhs.tmp);                                     \
    result.tmp.pushr(rhs);                                          \
    return result;                                                  \
}                                                                   \
                                                                    \
                                                                    \
/* 2.2 lhs=ty2 rhs=wty1 ----------------------------------------*/  \
                                                                    \
/* 2.2.1 ty2+wty1 */                                                \
template_spec                                                       \
C4_ALWAYS_INLINE _strCatRes<tyr> operator+                          \
(                                                                   \
    ty2 const& lhs,                                                 \
    _strCatRes<ty1> const& rhs                                      \
)                                                                   \
{                                                                   \
    _strCatRes<tyr> result;                                         \
    result.tmp.reserve(strsz(lhs) + strsz(rhs.tmp));                \
    result.tmp.append(lhs);                                         \
    result.tmp.append(rhs.tmp);                                     \
    return result;                                                  \
}                                                                   \
/* 2.2.2 ty2/wty1 */                                                \
template_spec                                                       \
C4_ALWAYS_INLINE _strCatRes<tyr> operator/                          \
(                                                                   \
    ty2 const& lhs,                                                 \
    _strCatRes<ty1> const& rhs                                      \
)                                                                   \
{                                                                   \
    _strCatRes<tyr> result;                                         \
    result.tmp.reserve(strsz(lhs) + 1 + strsz(rhs.tmp));            \
    result.tmp.append(lhs);                                         \
    result.tmp.pushr(rhs.tmp);                                      \
    return result;                                                  \
}                                                                   \
                                                                    \
                                                                    \
/* 1 BARE TYPES ===============================================*/   \
                                                                    \
/* 1.1 lhs=ty1 rhs=ty2 ----------------------------------------*/   \
                                                                    \
/* 1.1.1 ty1+ty2 */                                                 \
template_spec                                                       \
C4_ALWAYS_INLINE _strCatRes<tyr> operator+                          \
(                                                                   \
    ty1 const& lhs,                                                 \
    ty2 const& rhs                                                  \
)                                                                   \
{                                                                   \
    _strCatRes<tyr> result;                                         \
    result.tmp.reserve(strsz(lhs) + strsz(rhs));                    \
    result.tmp.append(lhs);                                         \
    result.tmp.append(rhs);                                         \
    return result;                                                  \
}                                                                   \
/* 1.1.2 ty1/ty2 */                                                 \
template_spec                                                       \
C4_ALWAYS_INLINE _strCatRes<tyr> operator/                          \
(                                                                   \
    ty1 const& lhs,                                                 \
    ty2 const& rhs                                                  \
)                                                                   \
{                                                                   \
    _strCatRes<tyr> result;                                         \
    result.tmp.reserve(strsz(lhs) + 1 + strsz(rhs));                \
    result.tmp.append(lhs);                                         \
    result.tmp.pushr(rhs);                                          \
    return result;                                                  \
}                                                                   \
                                                                    \
                                                                    \
/* 1.2 lhs=ty2 rhs=ty1 ----------------------------------------*/   \
                                                                    \
/* 1.2.1 ty2+ty1 */                                                 \
template_spec                                                       \
C4_ALWAYS_INLINE _strCatRes<tyr> operator+                          \
(                                                                   \
    ty2 const& lhs,                                                 \
    ty1 const& rhs                                                  \
)                                                                   \
{                                                                   \
    _strCatRes<tyr> result;                                         \
    result.tmp.reserve(strsz(lhs) + strsz(rhs));                    \
    result.tmp.append(lhs);                                         \
    result.tmp.append(rhs);                                         \
    return result;                                                  \
}                                                                   \
/* 1.2.2 ty2/ty1 */                                                 \
template_spec                                                       \
C4_ALWAYS_INLINE _strCatRes<tyr> operator/                          \
(                                                                   \
    ty2 const& lhs,                                                 \
    ty1 const& rhs                                                  \
)                                                                   \
{                                                                   \
    _strCatRes<tyr> result;                                         \
    result.tmp.reserve(strsz(lhs) + 1 + strsz(rhs));                \
    result.tmp.append(lhs);                                         \
    result.tmp.pushr(rhs);                                          \
    return result;                                                  \
}


#endif // defined(C4_STR_DISABLE_EXPR_TPL)


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** assignment operations with expression templates */

#ifndef C4_STR_DISABLE_EXPR_TPL

#define _C4STR_ASSIGNOPS(charname, classname)                           \
                                                                        \
_C4STR_ASSIGNOPS_ANY(charname, classname)                               \
                                                                        \
/** allow other _str_crtp derived classes access to our members.        \
 * @warning the ::c4:: qualifier is needed as a workaround to           \
 * gcc bug https://gcc.gnu.org/bugzilla/show_bug.cgi?id=52625 */        \
template< typename sifC, typename sifSize, class sifStr, class sifSub > \
friend class ::c4::_str_crtp;                                           \
                                                                        \
template< class Expr >                                                  \
C4_ALWAYS_INLINE classname(_strExpr< charname, Expr > const& expr)      \
{                                                                       \
    this->assign_expr(expr);                                            \
}                                                                       \
template< class Expr >                                                  \
C4_ALWAYS_INLINE classname& operator= (_strExpr< charname, Expr > const& expr) \
{                                                                       \
    this->assign_expr(expr);                                            \
    return *this;                                                       \
}                                                                       \
template< class Expr >                                                  \
C4_ALWAYS_INLINE classname& operator+= (_strExpr< charname, Expr > const& expr) \
{                                                                       \
    this->append_expr(expr);                                            \
    return *this;                                                       \
}                                                                       \
template< class Expr >                                                  \
C4_ALWAYS_INLINE classname& operator/= (_strExpr< charname, Expr > const& expr) \
{                                                                       \
    this->append_dir_expr(expr);                                        \
    return *this;                                                       \
}

//-----------------------------------------------------------------------------
/** the main expression template class for string operations */
template< class C, class Expr >
struct _strExpr
{
    typedef Expr expr_type;
    typedef C    value_type;

    C4_ALWAYS_INLINE operator Expr      & ()       { return static_cast< Expr      & >(*this); }
    C4_ALWAYS_INLINE operator Expr const& () const { return static_cast< Expr const& >(*this); }

    C4_ALWAYS_INLINE size_t size() const
    {
        return static_cast< Expr const* >(this)->size();
    }

    C4_ALWAYS_INLINE size_t put(C *str, size_t pos) const
    {
        return static_cast< Expr const* >(this)->put(str, pos);
    }

};

//-----------------------------------------------------------------------------

/** expression template node wrapping a _str_crtp or std::string, std::array< char >, std::vector< char >, etc */
template< typename C, typename S >
struct _strWrapImpl : public _strExpr< C, _strWrapImpl< C, S > >
{
    using traits_type = char_traits< C >;
    using value_type = C;
    using expr_type  = S;

    S const& str;
    size_t len;

    _strWrapImpl(S const& s)
        : str(s), len(str.size())
    {
        //printf(" strwrap:  hi. %lu %.*s\n", len, (int)len, str.data());fflush(nullptr);
    }
    ~_strWrapImpl()
    {
        //printf("~strwrap: bye. %lu %.*s\n", len, (int)len, str.data());fflush(nullptr);
    }

    C4_ALWAYS_INLINE size_t size() const
    {
        return len;
    }
    C4_ALWAYS_INLINE size_t put(C *out, size_t pos) const
    {
        C4_ASSERT_MSG(out != str.data(), "cannot assign to rvalues in this expression. the lvalue is present in the rhs");
        C const* in = str.data();
        C4_ASSERT_MSG(in + len < out + pos || in > out + pos + len, "must not overlap");
        traits_type::copy(out + pos, in, len);
        return pos + len;
    }

};
/// expression template node wrapping a char array
template< typename C >
struct _strWrapCharPtr : public _strExpr< C, _strWrapCharPtr< C > >
{
    using traits_type = char_traits< C >;
    typedef C  value_type;
    typedef C const * expr_type;

    C const* str;
    size_t len;

    _strWrapCharPtr(const C* s, size_t len_)
        : str(s), len(len_)
    {
    }

    template< size_t N >
    _strWrapCharPtr(const C (&s)[N])
        : str(s), len(N-1)
    {
    }

    _strWrapCharPtr(const C* s)
        : str(s), len(strsz(s))
    {
    }

    C4_ALWAYS_INLINE size_t size() const
    {
        return len;
    }
    C4_ALWAYS_INLINE size_t put(C* out, size_t pos) const
    {
        C4_ASSERT_MSG(out != str, "cannot assign to rvalues in this expression. the lvalue is present in the rhs");
        C4_ASSERT_MSG(str + len < out + pos || str > out + pos + len, "must not overlap");
        traits_type::copy(out + pos, str, len);
        return pos + len;
    }

};

//-----------------------------------------------------------------------------

/** Expression template nodes have the lifetime of the expression root
 * and can be stored by reference.
 * However, wrapper expressions nodes for strings or char arrays
 * are created temporarily, and thus need to be stored by value,
 * NOT by reference */
template< class Expr >
struct _strExprRef
{
    typedef Expr const& expr_ref;
};
/** _strWrapImpl needs to be stored by value */
template< typename C, typename S >
struct _strExprRef< _strWrapImpl< C, S > >
{
    typedef _strWrapImpl< C, S > expr_ref;
};
/** _strWrapCharPtr needs to be stored by value */
template< typename C >
struct _strExprRef< _strWrapCharPtr< C > >
{
    typedef _strWrapCharPtr< C > expr_ref;
};

//-----------------------------------------------------------------------------

/// expression template for summing two string expressions
template< class C, class ExprL, class ExprR >
struct _strPlus : public _strExpr< C, _strPlus< C, ExprL, ExprR > >
{
    typedef C value_type;

    typedef ExprL expr_left;
    typedef ExprR expr_right;

    typename _strExprRef< ExprL >::expr_ref lexpr;
    typename _strExprRef< ExprR >::expr_ref rexpr;

    _strPlus(ExprL const& l, ExprR const& r)
     : lexpr(l), rexpr(r)
    {
    //printf(" plus: l=%lu, r=%lu\n", lexpr.size(), rexpr.size());fflush(nullptr);
    }
    ~_strPlus()
    {
    //printf("~plus: l=%lu, r=%lu\n", lexpr.size(), rexpr.size());fflush(nullptr);
    }

    C4_ALWAYS_INLINE size_t size() const
    {
        return lexpr.size() + rexpr.size();
    }
    C4_ALWAYS_INLINE size_t put(char *str, size_t pos) const
    {
        pos = lexpr.put(str, pos);
        pos = rexpr.put(str, pos);
        return pos;
    }

};

//-----------------------------------------------------------------------------

/** expression template for path-like concatenation of two string expression templates */
template< class C, class ExprL, class ExprR >
struct _strSlash : public _strExpr< C, _strSlash< C, ExprL, ExprR > >
{
    typedef C value_type;

    typedef ExprL expr_left;
    typedef ExprR expr_right;

    typename _strExprRef< ExprL >::expr_ref lexpr;
    typename _strExprRef< ExprR >::expr_ref rexpr;

    _strSlash(ExprL const& l, ExprR const& r)
     : lexpr(l), rexpr(r)
    {
    }
    ~_strSlash()
    {
    }

    C4_ALWAYS_INLINE size_t size() const
    {
        return lexpr.size() + 1 + rexpr.size();
    }
    C4_ALWAYS_INLINE size_t put(char *str, size_t pos) const
    {
        pos = lexpr.put(str, pos);
        *(str + pos) = '/';
        ++pos;
        pos = rexpr.put(str, pos);
        return pos;
    }

};


//-----------------------------------------------------------------------------
// string expression template operators

/// sum two expressions
template< typename C, class ExprL, class ExprR >
_strPlus< C, _strExpr< C, ExprL >, _strExpr< C, ExprR > >
operator+
(
    _strExpr< C, ExprL > const& l,
    _strExpr< C, ExprR > const& r
)
{
    typedef _strExpr< C, ExprL > ltype;
    typedef _strExpr< C, ExprR > rtype;
    _strPlus< C, ltype, rtype > expr(l, r);
    return expr;
}
/// dircat two expressions
template< typename C, class ExprL, class ExprR >
_strSlash< C, _strExpr< C, ExprL >, _strExpr< C, ExprR > >
operator/
(
    _strExpr< C, ExprL > const& l,
    _strExpr< C, ExprR > const& r
)
{
    typedef _strExpr< C, ExprL > ltype;
    typedef _strExpr< C, ExprR > rtype;
    _strSlash< C, ltype, rtype > expr(l, r);
    return expr;
}

//--------------------------
/// sum two _str_crtps with same type
template< typename C, typename Size, class Str, class Sub >
_strPlus< C, _strWrapImpl< C, Str >, _strWrapImpl< C, Str > >
operator+
(
    _str_crtp< C, Size, Str, Sub > const& l_,
    _str_crtp< C, Size, Str, Sub > const& r_
)
{
    typedef _strWrapImpl< C, Str > etype;
    Str const& l = static_cast< Str const& >(l_);
    Str const& r = static_cast< Str const& >(r_);
    _strPlus< C, etype, etype > expr(l, r);
    return expr;
}
/// dircat two _str_crtps with same type
template< typename C, typename Size, class Str, class Sub >
_strSlash< C, _strWrapImpl< C, Str >, _strWrapImpl< C, Str > >
operator/
(
    _str_crtp< C, Size, Str, Sub > const& l_,
    _str_crtp< C, Size, Str, Sub > const& r_
)
{
    typedef _strWrapImpl< C, Str > etype;
    Str const& l = static_cast< Str const& >(l_);
    Str const& r = static_cast< Str const& >(r_);
    _strSlash< C, etype, etype > expr(l, r);
    return expr;
}

//--------------------------
/// sum two _str_crtps of different type
template< typename C, typename SizeL, class StrL, class SubL, typename SizeR, class StrR, class SubR >
_strPlus< C, _strWrapImpl< C, StrL >, _strWrapImpl< C, StrR > >
operator+
(
    _str_crtp< C, SizeL, StrL, SubL > const& l_,
    _str_crtp< C, SizeR, StrR, SubR > const& r_
)
{
    typedef _strWrapImpl< C, StrL > ltype;
    typedef _strWrapImpl< C, StrR > rtype;
    StrL const& l = static_cast< StrL const& >(l_);
    StrR const& r = static_cast< StrR const& >(r_);
    _strPlus< C, ltype, rtype > expr(l, r);
    return expr;
}
/// dircat _str_crtp and expr
template< typename C, typename SizeL, class StrL, class SubL, typename SizeR, class StrR, class SubR >
_strSlash< C, _strWrapImpl< C, StrL >, _strWrapImpl< C, StrR > >
operator/
(
    _str_crtp< C, SizeL, StrL, SubL > const& l_,
    _str_crtp< C, SizeR, StrR, SubR > const& r_
)
{
    typedef _strWrapImpl< C, StrL > ltype;
    typedef _strWrapImpl< C, StrR > rtype;
    StrL const& l = static_cast< StrL const& >(l_);
    StrR const& r = static_cast< StrR const& >(r_);
    _strSlash< C, ltype, rtype > expr(l, r);
    return expr;
}

//--------------------------
/// sum _str_crtp and expr
template< typename C, typename Size, class Str, class Sub, class ExprR >
_strPlus< C, _strWrapImpl< C, Str >, _strExpr< C, ExprR > >
operator+
(
    _str_crtp< C, Size, Str, Sub > const& l_,
    _strExpr< C, ExprR > const& r
)
{
    typedef _strWrapImpl< C, Str > ltype;
    typedef _strExpr< C, ExprR >   rtype;
    Str const& l = static_cast< Str const& >(l_);
    _strPlus< C, ltype, rtype > expr(l, r);
    return expr;
}
/// dircat _str_crtp and expr
template< typename C, typename Size, class Str, class Sub, class ExprR >
_strSlash< C, _strWrapImpl< C, Str >, _strExpr< C, ExprR > >
operator/
(
    _str_crtp< C, Size, Str, Sub > const& l_,
    _strExpr< C, ExprR > const& r
)
{
    typedef _strWrapImpl< C, Str > ltype;
    typedef _strExpr< C, ExprR >   rtype;
    Str const& l = static_cast< Str const& >(l_);
    _strSlash< C, ltype, rtype > expr(l, r);
    return expr;
}

//--------------------------
/// sum expr and _str_crtp
template< class ExprL, typename C, typename Size, class Str, class Sub >
_strPlus< C, _strExpr< C, ExprL >, _strWrapImpl< C, Str > >
operator+
(
    _strExpr< C, ExprL > const& l,
    _str_crtp< C, Size, Str, Sub > const& r_
)
{
    typedef _strExpr< C, ExprL > ltype;
    typedef _strWrapImpl< C, Str > rtype;
    Str const& r = static_cast< Str const& >(r_);
    _strPlus< C, ltype, rtype > expr(l, r);
    return expr;
}
/// dircat expr and _str_crtp
template< class ExprL, typename C, typename Size, class Str, class Sub >
_strSlash< C, _strExpr< C, ExprL >, _strWrapImpl< C, Str > >
operator/
(
    _strExpr< C, ExprL > const& l,
    _str_crtp< C, Size, Str, Sub > const& r_
)
{
    typedef _strExpr< C, ExprL > ltype;
    typedef _strWrapImpl< C, Str > rtype;
    Str const& r = static_cast< Str const& >(r_);
    _strSlash< C, ltype, rtype > expr(l, r);
    return expr;
}

//--------------------------
/// sum _str_crtp and const char*
template< typename C, typename Size, class Str, class Sub >
_strPlus< C, _strWrapImpl< C, Str >, _strWrapCharPtr< C > >
operator+
(
    _str_crtp< C, Size, Str, Sub > const& l_,
    const char* r
)
{
    typedef _strWrapImpl< C, Str > ltype;
    typedef _strWrapCharPtr< C > rtype;
    Str const& l = static_cast< Str const& >(l_);
    _strPlus< C, ltype, rtype > expr(l, rtype(r));
    return expr;
}
/// dircat _str_crtp and const char*
template< typename C, typename Size, class Str, class Sub >
_strSlash< C, _strWrapImpl< C, Str >, _strWrapCharPtr< C > >
operator/
(
    _str_crtp< C, Size, Str, Sub > const& l_,
    const char* r
)
{
    typedef _strWrapImpl< C, Str > ltype;
    typedef _strWrapCharPtr< C > rtype;
    Str const& l = static_cast< Str const& >(l_);
    _strSlash< C, ltype, rtype > expr(l, rtype(r));
    return expr;
}

//--------------------------
/** sum const C* and _str_crtp */
template< typename C, typename Size, class Str, class Sub >
_strPlus< C, _strWrapCharPtr< C >, _strWrapImpl< C, Str > >
operator+
(
    const char* l,
    _str_crtp< C, Size, Str, Sub > const& r_
)
{
    typedef _strWrapCharPtr< C > ltype;
    typedef _strWrapImpl< C, Str > rtype;
    Str const& r = static_cast< Str const& >(r_);
    _strPlus< C, ltype, rtype > expr(ltype(l), r);
    return expr;
}
/** dircat const C* and _str_crtp */
template< typename C, typename Size, class Str, class Sub >
_strSlash< C, _strWrapCharPtr< C >, _strWrapImpl< C, Str > >
operator/
(
    const char* l,
    _str_crtp< C, Size, Str, Sub > const& r_
)
{
    typedef _strWrapCharPtr< C > ltype;
    typedef _strWrapImpl< C, Str > rtype;
    Str const& r = static_cast< Str const& >(r_);
    _strSlash< C, ltype, rtype > expr(ltype(l), r);
    return expr;
}

//--------------------------
/** sum expr and const C* */
template< typename C, class ExprL >
_strPlus< C, _strExpr< C, ExprL >, _strWrapCharPtr< C > >
operator+
(
    _strExpr< C, ExprL > const& l,
    const char* r
)
{
    typedef _strExpr< C, ExprL > ltype;
    typedef _strWrapCharPtr< C > rtype;
    _strPlus< C, ltype, rtype > expr(l, rtype(r));
    return expr;
}
/** dircat expr and const char */
template< typename C, class ExprL >
_strSlash< C, _strExpr< C, ExprL >, _strWrapCharPtr< C > >
operator/
(
    _strExpr< C, ExprL > const& l,
    const char* r
)
{
    typedef _strExpr< C, ExprL > ltype;
    typedef _strWrapCharPtr< C > rtype;
    _strSlash< C, ltype, rtype > expr(l, rtype(r));
    return expr;
}

//--------------------------
/** sum const C* and expr */
template< typename C, class ExprR >
_strPlus< C, _strExpr< C, ExprR >, _strWrapCharPtr< C > >
operator+
(
    const char* l,
    _strExpr< C, ExprR > const& r
)
{
    typedef _strWrapCharPtr< C > ltype;
    typedef _strExpr< C, ExprR > rtype;
    _strPlus< C, ltype, rtype > expr(ltype(l), r);
    return expr;
}
/** dircat const C* and expr */
template< typename C, class ExprR >
_strSlash< C, _strExpr< C, ExprR >, _strWrapCharPtr< C > >
operator/
(
    const char* l,
    _strExpr< C, ExprR > const& r
)
{
    typedef _strWrapCharPtr< C > ltype;
    typedef _strExpr< C, ExprR > rtype;
    _strSlash< C, ltype, rtype > expr(ltype(l), r);
    return expr;
}

#endif // C4_ENABLE_STRING_EXPRTPL

/// @endcond

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/** A CRTP base implementation for string member methods.
 *
 * @tparam C the character type
 * @tparam SizeType an integral size type
 * @tparam StrType the final string type - must inherit from this class.
 * @tparam SubStrType a substring type to be used in all selection methods.
 *         When StrType is an owning type, this provides savings in number
 *         of allocations over the alternative of returning the StrType
 *         directly.
 *
 * The final string type must have the following static member:
 *  - npos
 * and the following methods:
 *  - size()
 *  - data()
 *  - resize()
 *  - reserve()
 *
 * @todo this class should be separated into a non-modifying and a modifying class
 */
template< typename C, typename SizeType, class StrType, class SubStrType >
class _str_crtp
{
/// @cond dev
// utility defines; undefined below
#define _c4this     static_cast< StrType      * >(this)
#define _c4thisSZ   static_cast< StrType      * >(this)->size()
#define _c4thisSTR  static_cast< StrType      * >(this)->data()
#define _c4thatSZ   static_cast< StrType      * >(&that)->size()
#define _c4thatSTR  static_cast< StrType      * >(&that)->data()
#define _c4cthisSZ  static_cast< StrType const* >(this)->size()
#define _c4cthisSTR static_cast< StrType const* >(this)->data()
#define _c4cthatSZ  static_cast< StrType const* >(&that)->size()
#define _c4cthatSTR static_cast< StrType const* >(&that)->data()
#define _c4CCAST(ty) (ty)const_cast< _str_crtp const* >(this)
#define _c4cSZ(ptr)  static_cast< StrType const* >(ptr)->size()
#define _c4cSTR(ptr) static_cast< StrType const* >(ptr)->data()
/// @endcond

    C4_STATIC_ASSERT(std::is_integral< SizeType >::value);

public:

    using value_type   = C;
    using char_type    = C;
    using traits_type  = c4::char_traits< C >;
    using size_type    = SizeType;
    using ssize_type   = typename std::make_signed< size_type >::type;
    using cont_type    = StrType;
    using subcont_type = SubStrType;

protected:

    // protect the dtor to prevent creation of bare _str_crtp
    _str_crtp(){}
    // protect the dtor to prevent destruction through a base pointer
    ~_str_crtp(){}

public:

    // automatically convert to span
    C4_ALWAYS_INLINE operator  span< C, SizeType > ()       { return span < C, SizeType >(_c4thisSTR, _c4thisSZ); }
    C4_ALWAYS_INLINE operator cspan< C, SizeType > () const { return cspan< C, SizeType >(_c4cthisSTR, _c4cthisSZ); }

    template< typename OtherSizeType >
    C4_ALWAYS_INLINE operator  span< C, OtherSizeType > ()       { return span < C, OtherSizeType >(_c4thisSTR, szconv< OtherSizeType >(_c4thisSZ)); }
    template< typename OtherSizeType >
    C4_ALWAYS_INLINE operator cspan< C, OtherSizeType > () const { return cspan< C, OtherSizeType >(_c4cthisSTR, szconv< OtherSizeType >(_c4cthisSZ)); }

public:

    C4_ALWAYS_INLINE C      *   data()        noexcept { return _c4thisSTR; }
    C4_ALWAYS_INLINE C const*   data()  const noexcept { return _c4cthisSTR; }

    C4_ALWAYS_INLINE bool       empty() const noexcept { return _c4cthisSZ == 0; }
    C4_ALWAYS_INLINE size_type  size()  const noexcept { return _c4cthisSZ; }
    C4_ALWAYS_INLINE ssize_type ssize() const noexcept { return szconv< ssize_type >(_c4cthisSZ); }

    C4_ALWAYS_INLINE C& front()       C4_NOEXCEPT_X { C4_XASSERT(!empty()); return *_c4thisSTR; }
    C4_ALWAYS_INLINE C  front() const C4_NOEXCEPT_X { C4_XASSERT(!empty()); return *_c4cthisSTR; }

    C4_ALWAYS_INLINE C& back()       C4_NOEXCEPT_X { C4_XASSERT(!empty()); return *(_c4thisSTR + _c4thisSZ - 1); }
    C4_ALWAYS_INLINE C  back() const C4_NOEXCEPT_X { C4_XASSERT(!empty()); return *(_c4cthisSTR + _c4cthisSZ - 1); }

public:

    using iterator = C*;
    using const_iterator = C const*;
    using reverse_iterator = std::reverse_iterator<C*>;
    using const_reverse_iterator = std::reverse_iterator<C const*>;

    C4_ALWAYS_INLINE       iterator  begin()       noexcept { return _c4thisSTR; }
    C4_ALWAYS_INLINE const_iterator  begin() const noexcept { return _c4cthisSTR; }
    C4_ALWAYS_INLINE const_iterator cbegin() const noexcept { return _c4cthisSTR; }

    C4_ALWAYS_INLINE       iterator  end()       noexcept { return _c4thisSTR + _c4thisSZ; }
    C4_ALWAYS_INLINE const_iterator  end() const noexcept { return _c4cthisSTR + _c4cthisSZ; }
    C4_ALWAYS_INLINE const_iterator cend() const noexcept { return _c4cthisSTR + _c4cthisSZ; }

    C4_ALWAYS_INLINE       iterator  rbegin()       noexcept { return       reverse_iterator(_c4thisSTR + _c4thisSZ); }
    C4_ALWAYS_INLINE const_iterator  rbegin() const noexcept { return const_reverse_iterator(_c4cthisSTR + _c4cthisSZ); }
    C4_ALWAYS_INLINE const_iterator crbegin() const noexcept { return const_reverse_iterator(_c4cthisSTR + _c4cthisSZ); }

    C4_ALWAYS_INLINE       iterator  rend()       noexcept { return       reverse_iterator(_c4thisSTR); }
    C4_ALWAYS_INLINE const_iterator  rend() const noexcept { return const_reverse_iterator(_c4cthisSTR); }
    C4_ALWAYS_INLINE const_iterator crend() const noexcept { return const_reverse_iterator(_c4cthisSTR); }

public:

    C4_ALWAYS_INLINE bool nullterminated() const
    {
        C4_ASSERT(_c4thisSZ < _c4this->capacity());
        return (_c4cthisSTR[_c4cthisSZ] == '\0');
    }
    /** @todo make this private */
    C4_ALWAYS_INLINE void __nullterminate()
    {
        C4_ASSERT(_c4thisSZ < _c4this->capacity());
        _c4thisSTR[_c4thisSZ] = '\0';
    }

public:

    C4_ALWAYS_INLINE C operator[] ( int8_t  i) const C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && szconv< SizeType >(i) < _c4cthisSZ); return *(_c4cthisSTR + i); }
    C4_ALWAYS_INLINE C operator[] ( int16_t i) const C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && szconv< SizeType >(i) < _c4cthisSZ); return *(_c4cthisSTR + i); }
    C4_ALWAYS_INLINE C operator[] ( int32_t i) const C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && szconv< SizeType >(i) < _c4cthisSZ); return *(_c4cthisSTR + i); }
    C4_ALWAYS_INLINE C operator[] ( int64_t i) const C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && szconv< SizeType >(i) < _c4cthisSZ); return *(_c4cthisSTR + i); }
    C4_ALWAYS_INLINE C operator[] (uint8_t  i) const C4_NOEXCEPT_X { C4_XASSERT(          szconv< SizeType >(i) < _c4cthisSZ); return *(_c4cthisSTR + i); }
    C4_ALWAYS_INLINE C operator[] (uint16_t i) const C4_NOEXCEPT_X { C4_XASSERT(          szconv< SizeType >(i) < _c4cthisSZ); return *(_c4cthisSTR + i); }
    C4_ALWAYS_INLINE C operator[] (uint32_t i) const C4_NOEXCEPT_X { C4_XASSERT(          szconv< SizeType >(i) < _c4cthisSZ); return *(_c4cthisSTR + i); }
    C4_ALWAYS_INLINE C operator[] (uint64_t i) const C4_NOEXCEPT_X { C4_XASSERT(          szconv< SizeType >(i) < _c4cthisSZ); return *(_c4cthisSTR + i); }

    C4_ALWAYS_INLINE C& operator[] ( int8_t  i) C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && szconv< SizeType >(i) < _c4cthisSZ); return *(_c4thisSTR + i); }
    C4_ALWAYS_INLINE C& operator[] ( int16_t i) C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && szconv< SizeType >(i) < _c4cthisSZ); return *(_c4thisSTR + i); }
    C4_ALWAYS_INLINE C& operator[] ( int32_t i) C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && szconv< SizeType >(i) < _c4cthisSZ); return *(_c4thisSTR + i); }
    C4_ALWAYS_INLINE C& operator[] ( int64_t i) C4_NOEXCEPT_X { C4_XASSERT(i >= 0 && szconv< SizeType >(i) < _c4cthisSZ); return *(_c4thisSTR + i); }
    C4_ALWAYS_INLINE C& operator[] (uint8_t  i) C4_NOEXCEPT_X { C4_XASSERT(          szconv< SizeType >(i) < _c4cthisSZ); return *(_c4thisSTR + i); }
    C4_ALWAYS_INLINE C& operator[] (uint16_t i) C4_NOEXCEPT_X { C4_XASSERT(          szconv< SizeType >(i) < _c4cthisSZ); return *(_c4thisSTR + i); }
    C4_ALWAYS_INLINE C& operator[] (uint32_t i) C4_NOEXCEPT_X { C4_XASSERT(          szconv< SizeType >(i) < _c4cthisSZ); return *(_c4thisSTR + i); }
    C4_ALWAYS_INLINE C& operator[] (uint64_t i) C4_NOEXCEPT_X { C4_XASSERT(          szconv< SizeType >(i) < _c4cthisSZ); return *(_c4thisSTR + i); }

public:

    /** count the number of characters whose value is C */
    size_type count(C val) const noexcept
    {
        size_type num = 0;
        for(size_type i = 0, sz = _c4cthisSZ; i != sz; ++i)
        {
            if(_c4cthisSTR[i] == val)
            {
                ++num;
            }
        }
        return num;
    }
    /** count the number of characters whose value is C */
    size_type count(C val, size_type first_char, size_type num_chars = StrType::npos) const C4_NOEXCEPT_X
    {
        size_type num = 0;
        C4_XASSERT(first_char >= 0 && first_char < _c4cthisSZ);
        size_type last_char = StrType::npos ? _c4cthisSZ - first_char : first_char + num_chars;
        C4_XASSERT(last_char  >= 0 && last_char  < _c4cthisSZ);
        C const* d = _c4cthisSTR;
        for(size_type i = first_char; i != last_char; ++i)
        {
            if(d[i] == val)
            {
                ++num;
            }
        }
        return num;
    }

public:

    /** replace all occurrences of the character val with the character rep.
     * return the number of replacements. */
    size_type replace(C val, C rep, C start_pos = 0)
    {
        size_type num = 0;
        C * d = _c4thisSTR;
        for(size_type i = start_pos; i < _c4cthisSZ; ++i)
        {
            if(d[i] == val)
            {
                ++num;
                d[i] = rep;
            }
        }
        return num;
    }

public:

    /** get a substring from a first,length pair. use range() when you want a first,last position pair */
    SubStrType substr(size_type first = 0, size_type len = cont_type::npos) C4_NOEXCEPT_X
    {
        len = len == cont_type::npos ? _c4cthisSZ-first : len;
        C4_XASSERT(first < _c4thisSZ || empty() || (len == 0 && first == _c4thisSZ));
        C4_XASSERT(first + len <= _c4thisSZ);
        return SubStrType(_c4thisSTR + first, len);
    }
    /** get a substring from a first,length pair. use range() when you want a first,last position pair */
    const SubStrType substr(size_type first = 0, size_type len = cont_type::npos) const C4_NOEXCEPT_X
    {
        len = len == cont_type::npos ? _c4cthisSZ-first : len;
        C4_XASSERT(first < _c4cthisSZ || empty() || (len == 0 && first == _c4cthisSZ));
        C4_XASSERT(first + len <= _c4cthisSZ);
        return SubStrType((C*)(_c4cthisSTR + first), len); // FIXME
    }

    /** get a substring from a first,last position pair. use substr() when you want a first,length pair */
    C4_ALWAYS_INLINE SubStrType range(size_type first = 0, size_type last = cont_type::npos) C4_NOEXCEPT_X
    {
        last = last == cont_type::npos ? _c4cthisSZ : last;
        C4_XASSERT(first < _c4thisSZ || empty() || (last == first && first == _c4cthisSZ));
        C4_XASSERT(last <= _c4thisSZ);
        return SubStrType(_c4thisSTR + first, last - first);
    }
    /** get a substring from a first,last position pair. use substr() when you want a first,length pair */
    const SubStrType range(size_type first = 0, size_type last = cont_type::npos) const C4_NOEXCEPT_X
    {
        last = last == cont_type::npos ? _c4cthisSZ : last;
        C4_XASSERT(first < _c4cthisSZ || empty() || (last == first && first == _c4cthisSZ));
        C4_XASSERT(last <= _c4cthisSZ);
        return SubStrType((C*)(_c4cthisSTR + first), last - first); // FIXME
    }

    bool is_substr(SubStrType const& ss) const noexcept
    {
        if(_c4cthisSTR == nullptr) return false;
        auto *b = begin(), *e = end();
        auto *ssb = ss.begin(), *sse = ss.end();
        if(ssb >= b && sse <= e)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    /** COMPLement Left: return the complement to the left of the beginning of the given substring.
     * If ss does not begin inside this, returns an empty substring. */
    SubStrType compll(SubStrType const& ss) const C4_NOEXCEPT_X
    {
        auto ssb = ss.begin();
        auto b = begin();
        auto e = end();
        if(ssb >= b && ssb <= e)
        {
            return substr(0, ssb - b);
        }
        else
        {
            return substr(0, 0);
        }
    }

    /** COMPLement Right: return the complement to the right of the end of the given substring.
     * If ss does not end inside this, returns an empty substring. */
    SubStrType complr(SubStrType const& ss) const C4_NOEXCEPT_X
    {
        auto sse = ss.end();
        auto b = begin();
        auto e = end();
        if(sse >= b && sse <= e)
        {
            return substr(sse - b, e - sse);
        }
        else
        {
            return substr(0, 0);
        }
    }

public:

    template< size_t N >
    size_type C4_ALWAYS_INLINE find(const C (&pat)[N], size_type pos = 0) const { return find(pat, N-1, pos); }
    size_type C4_ALWAYS_INLINE find(C c, size_type pos = 0) const { return find(&c, 1, pos); }
    size_type C4_ALWAYS_INLINE find(_str_crtp const& that, size_type pos = 0) const { return find(_c4cthatSTR, _c4cthatSZ, pos); }
    size_type C4_ALWAYS_INLINE find(const C *pat, size_type pos = 0) const { return find(pat, szconv< size_type >(traits_type::length(pat)), pos); }
    size_type find(const C *pat, size_type len, size_type pos) const 
    {
        if(empty()) return StrType::npos;
        C4_XASSERT(pos >= 0 && pos < _c4cthisSZ);
        for(size_type i = pos; i < _c4cthisSZ; ++i)
        {
            if(_c4cthisSTR[i] == pat[0])
            {
                size_type j = 1;
                for(; j < len && i+j < _c4cthisSZ; ++j)
                {
                    if(_c4cthisSTR[i + j] != pat[j])
                    {
                        break;
                    }
                }
                if(j == len)
                {
                    return i;
                }
            }
        }
        return StrType::npos;
    }

public:

    template< size_t N >
    size_type C4_ALWAYS_INLINE find_first_of(const C (&pat)[N], size_type pos = 0) const { return find_first_of(pat, N-1, pos); }
    size_type C4_ALWAYS_INLINE find_first_of(C c, size_type pos = 0) const { return find_first_of(&c, 1, pos); }
    size_type C4_ALWAYS_INLINE find_first_of(_str_crtp const& that, size_type pos = 0) const { return find_first_of(_c4cthatSTR, _c4cthatSZ, pos); }
    size_type C4_ALWAYS_INLINE find_first_of(const C *pat, size_type pos = 0) const { return find_first_of(pat, szconv< size_type >(traits_type::length(pat)), pos); }
    size_type find_first_of(const C *pat, size_type len, size_type pos) const
    {
        if(empty()) return StrType::npos;
        C4_XASSERT(pos >= 0 && pos < _c4cthisSZ);
        for(size_type i = pos; i < _c4cthisSZ; ++i)
        {
            for(size_type j = 0; j < len; ++j)
            {
                if(_c4cthisSTR[i] == pat[j])
                {
                    return i;
                }
            }
        }
        return StrType::npos;
    }

    template< size_t N >
    size_type C4_ALWAYS_INLINE find_first_not_of(const C (&pat)[N], size_type pos = 0) const { return find_first_not_of(pat, N-1, pos); }
    size_type C4_ALWAYS_INLINE find_first_not_of(C c, size_type pos = 0) const { return find_first_not_of(&c, 1, pos); }
    size_type C4_ALWAYS_INLINE find_first_not_of(_str_crtp const& that, size_type pos = 0) const { return find_first_not_of(_c4cthatSTR, _c4cthatSZ, pos); }
    size_type C4_ALWAYS_INLINE find_first_not_of(const C *pat, size_type pos = 0) const { return find_first_not_of(pat, szconv< size_type >(traits_type::length(pat)), pos); }
    size_type find_first_not_of(const C *pat, size_type len, size_type pos) const
    {
        if(empty()) return StrType::npos;
        C4_XASSERT(pos >= 0 && pos < _c4cthisSZ);
        for(size_type i = pos; i < _c4cthisSZ; ++i)
        {
            size_type j;
            for(j = 0; j < len; ++j)
            {
                if(_c4cthisSTR[i] == pat[j])
                {
                    break;
                }
            }
            if(j == len)
            {
                return i;
            }
        }
        return StrType::npos;
    }

public:

    template< size_t N >
    size_type C4_ALWAYS_INLINE find_last_of(const C (&pat)[N], size_type pos = StrType::npos) const { return find_last_of(pat, N-1, pos); }
    size_type C4_ALWAYS_INLINE find_last_of(C c, size_type pos = StrType::npos) const { return find_last_of(&c, 1, pos); }
    size_type C4_ALWAYS_INLINE find_last_of(_str_crtp const& that, size_type pos = StrType::npos) const { return find_last_of(_c4cthatSTR, _c4cthatSZ, pos); }
    size_type C4_ALWAYS_INLINE find_last_of(const C *pat, size_type pos = StrType::npos) const { return find_last_of(pat, szconv< size_type >(traits_type::length(pat)), pos); }
    size_type find_last_of(const C *pat, size_type len, size_type pos) const
    {
        if(empty()) return StrType::npos;
        if(pos == StrType::npos)
        {
            pos = _c4cthisSZ-1;
        }
        C4_XASSERT(pos >= 0 && pos < _c4cthisSZ);
        for(size_type i = pos; i != size_type(-1); --i) // to defend against underflow
        {
            for(size_type j = 0; j < len; ++j)
            {
                if(_c4cthisSTR[i] == pat[j])
                {
                    return i;
                }
            }
        }
        return StrType::npos;
    }

    template< size_t N >
    size_type C4_ALWAYS_INLINE find_last_not_of(const C (&pat)[N], size_type pos = StrType::npos) const { return find_last_not_of(pat, N-1, pos); }
    size_type C4_ALWAYS_INLINE find_last_not_of(C c, size_type pos = StrType::npos) const { return find_last_not_of(&c, 1, pos); }
    size_type C4_ALWAYS_INLINE find_last_not_of(_str_crtp const& that, size_type pos = StrType::npos) const { return find_last_not_of(_c4cthatSTR, _c4cthatSZ, pos); }
    size_type C4_ALWAYS_INLINE find_last_not_of(const C *pat, size_type pos = StrType::npos) const { return find_last_not_of(pat, szconv< size_type >(traits_type::length(pat)), pos); }
    size_type find_last_not_of(const C *pat, size_type len, size_type pos) const
    {
        if(empty()) return StrType::npos;
        if(pos == StrType::npos)
        {
            pos = _c4cthisSZ-1;
        }
        C4_XASSERT(pos >= 0 && pos < _c4cthisSZ);
        for(size_type i = pos; i != size_type(-1); --i) // to defend against underflow
        {
            size_type j;
            for(j = 0; j < len; ++j)
            {
                if(_c4cthisSTR[i] == pat[j])
                {
                    break;
                }
            }
            if(j == len)
            {
                return i;
            }
        }
        return StrType::npos;
    }

public:

    template< size_t N >
    bool C4_ALWAYS_INLINE begins_with(const C (&pat)[N]) const { return begins_with(pat, N-1); }
    bool C4_ALWAYS_INLINE begins_with(C c) const { return begins_with(&c, 1); }
    bool C4_ALWAYS_INLINE begins_with(_str_crtp const& that) const { return begins_with(_c4cthatSTR, _c4cthatSZ); }
    bool C4_ALWAYS_INLINE begins_with(const C *pat) const { return begins_with(pat, szconv< size_type >(traits_type::length(pat))); }
    bool begins_with(const C *pat, size_type len) const
    {
        if(len == 0 || len > _c4cthisSZ)
        {
            return false;
        }
        for(size_type i = 0; i < len; ++i)
        {
            if(_c4cthisSTR[i] != pat[i])
            {
                return false;
            }
        }
        return true;
    }

    template< size_t N >
    bool C4_ALWAYS_INLINE ends_with(const C (&pat)[N]) const { return ends_with(pat, N-1); }
    bool C4_ALWAYS_INLINE ends_with(C c) const { return ends_with(&c, 1); }
    bool C4_ALWAYS_INLINE ends_with(_str_crtp const& that) const { return ends_with(_c4cthatSTR, _c4cthatSZ); }
    bool C4_ALWAYS_INLINE ends_with(const C *pat) const { return ends_with(pat, szconv< size_type >(traits_type::length(pat))); }
    bool ends_with(const C *pat, size_type len) const
    {
        if(len == 0 || len > _c4cthisSZ)
        {
            return false;
        }
        for(size_type i = 0, s = _c4cthisSZ - len; i < len; ++i)
        {
            if(_c4cthisSTR[s + i] != pat[i])
            {
                return false;
            }
        }
        return true;
    }

public:

    C4_ALWAYS_INLINE size_type num_splits(C sep) const
    {
        return empty() ? 0 : size_type(1) + this->count(sep);
    }
    C4_ALWAYS_INLINE size_type num_splits(C sep, size_type start_pos) const
    {
        return empty() ? 0 : size_type(1) + this->count(sep, start_pos);
    }

    SubStrType next_split(C sep, size_type *C4_RESTRICT start_pos)
    {
        SubStrType out;
        next_split(sep, start_pos, &out);
        return out;
    }
    const SubStrType next_split(C sep, size_type *C4_RESTRICT start_pos) const
    {
        SubStrType out;
        next_split(sep, start_pos, &out);
        return out;
    }

    bool next_split(C sep, size_type *C4_RESTRICT start_pos, SubStrType *C4_RESTRICT out) const
    {
        return const_cast<_str_crtp*>(this)->next_split(sep, start_pos, out); // FIXME
    }
    /** returns true if the string has not been exhausted yet, meaning it's
     * ok to call next_split() again. When no instance of sep exists in
     * the string, returns the full string. When the input is an empty
     * string the following happens:
     *   -the output string is the empty string also.
     *   -always return false. */
    bool next_split(C sep, size_type *C4_RESTRICT start_pos, SubStrType *C4_RESTRICT out)
    {
        size_type sz = _c4cthisSZ;
        C* d = _c4thisSTR;
        if(C4_LIKELY(*start_pos < sz))
        {
            for(size_type i = *start_pos, e = _c4cthisSZ; i < e; i++)
            {
                if(d[i] == sep)
                {
                    out->assign(d + *start_pos, i - *start_pos);
                    *start_pos = i+1;
                    return true;
                }
            }
            out->assign(d + *start_pos, sz - *start_pos);
            *start_pos = sz + 1;
            return true;
        }
        else
        {
            bool valid = sz > 0 && (*start_pos == sz);
            if(valid && !empty() && d[sz-1] == sep)
            {
                out->assign(d + sz, 0);
            }
            else
            {
                out->assign(d + sz + 1, 0);
            }
            *start_pos = sz + 1;
            return valid;
        }
    }

    struct split_proxy_impl
    {
        struct split_iterator_impl
        {
            split_proxy_impl const* m_proxy;
            SubStrType m_str;
            typename SubStrType::size_type m_pos;
            C m_sep;

            split_iterator_impl(split_proxy_impl const* proxy, typename SubStrType::size_type pos, C sep)
                : m_proxy(proxy), m_pos(pos), m_sep(sep)
            {
                _tick();
            }

            void _tick()
            {
                m_proxy->m_str.next_split(m_sep, &m_pos, &m_str);
            }

            split_iterator_impl& operator++ () { _tick(); return *this; }
            split_iterator_impl  operator++ (int) { split_iterator_impl it = *this; _tick(); return it; }

            SubStrType& operator* () { return m_str; }
            SubStrType* operator-> () { return &m_str; }

            bool operator!= (split_iterator_impl const& that) const
            {
                return !(this->operator==(that));
            }
            bool operator== (split_iterator_impl const& that) const
            {
                C4_XASSERT_MSG(m_sep == that.m_sep, "cannot compare split iterators with different separators");
                if(m_str.size() != that.m_str.size()) return false;
                if(m_str.data() != that.m_str.data()) return false;
                return m_pos == that.m_pos;
            }
        };

        SubStrType m_str;
        typename SubStrType::size_type m_start_pos;
        C m_sep;

        split_proxy_impl(SubStrType str, typename SubStrType::size_type start_pos, C sep)
            : m_str(str), m_start_pos(start_pos), m_sep(sep)
        {
        }

        split_iterator_impl begin() const
        {
            auto it = split_iterator_impl(this, m_start_pos, m_sep);
            return it;
        }
        split_iterator_impl end() const
        {
            using stype = typename SubStrType::size_type;
            stype pos = m_str.size() + 1;
            auto it = split_iterator_impl(this, pos, m_sep);
            return it;
        }
    };

    using       split_proxy = split_proxy_impl;
    using const_split_proxy = split_proxy_impl;

    split_proxy split(C sep, size_type start_pos = 0)
    {
        C4_XASSERT(start_pos >= 0 && start_pos < _c4cthisSZ || empty());
        auto sz = _c4thisSZ;
        auto ss = substr(0, sz);
        auto it = split_proxy(ss, start_pos, sep);
        return it;
    }
    const_split_proxy split(C sep, size_type start_pos = 0) const
    {
        C4_XASSERT(start_pos >= 0 && start_pos < _c4cthisSZ || empty());
        auto sz = _c4cthisSZ;
        auto ss = substr(0, sz);
        auto it = split_proxy(ss, start_pos, sep);
        return it;
    }

public:

    /** return the next position where the character is different from the one at start_pos.
     * If all characters are equal, returns npos */
    size_type nextdiff(size_type start_pos) const
    {
        C4_XASSERT(start_pos >= 0 && start_pos < _c4cthisSZ);
        C c = _c4cthisSTR[start_pos];
        size_type pos = start_pos;
        while(++pos < _c4cthisSZ)
        {
            if(_c4cthisSTR[pos] != c)
            {
                return pos;
            }
        }
        return StrType::npos;
    }

    /** return the previous position where the character is different from the one at start_pos.
     * If all characters are equal, returns npos. */
    size_type prevdiff(size_type start_pos) const
    {
        C4_XASSERT(start_pos >= 0 && start_pos < _c4cthisSZ);
        C c = _c4cthisSTR[start_pos];
        size_type del = 0; // count from 0 up to avoid an underflow
        while(++del < start_pos)
        {
            if(_c4cthisSTR[start_pos - del] != c)
            {
                return start_pos - del;
            }
        }
        return StrType::npos;
    }

public:

    template< size_t N >
    SubStrType C4_ALWAYS_INLINE triml(const C (&pat)[N]) const { return triml(pat, N-1); }
    SubStrType C4_ALWAYS_INLINE triml(C c) const { return triml(&c, 1); }
    SubStrType C4_ALWAYS_INLINE triml(_str_crtp const& that) const { return triml(_c4cthatSTR, _c4cthatSZ); }
    SubStrType C4_ALWAYS_INLINE triml(const C *pat) const { return triml(pat, szconv< size_type >(traits_type::length(pat))); }
    /** trim the given characters from the left of the string, return resulting trimmed substring. */
    SubStrType triml(const C* str, size_type len) const
    {
        if(empty()) return substr();
        size_type pos = find_first_not_of(str, len, 0);
        if(pos != StrType::npos)
        {
            return substr(pos);
        }
        return substr(0, 0);
    }

    template< size_t N >
    SubStrType C4_ALWAYS_INLINE trimr(const C (&pat)[N]) const { return trimr(pat, N-1); }
    SubStrType C4_ALWAYS_INLINE trimr(C c) const { return trimr(&c, 1); }
    SubStrType C4_ALWAYS_INLINE trimr(_str_crtp const& that) const { return trimr(_c4cthatSTR, _c4cthatSZ); }
    SubStrType C4_ALWAYS_INLINE trimr(const C *pat) const { return trimr(pat, szconv< size_type >(traits_type::length(pat))); }
    /** trim the given characters from the right of the string, return resulting trimmed substring. */
    SubStrType trimr(const C* str, size_type len) const
    {
        if(empty()) return substr();
        size_type pos = find_last_not_of(str, len, StrType::npos);
        if(pos != StrType::npos)
        {
            return substr(0, pos+1);
        }
        return substr(0, 0);
    }

    template< size_t N >
    SubStrType C4_ALWAYS_INLINE trim(const C (&pat)[N]) const { return trim(pat, N-1); }
    SubStrType C4_ALWAYS_INLINE trim(C c) const { return trim(&c, 1); }
    SubStrType C4_ALWAYS_INLINE trim(_str_crtp const& that) const { return trim(_c4cthatSTR, _c4cthatSZ); }
    SubStrType C4_ALWAYS_INLINE trim(const C *pat) const { return trim(pat, szconv< size_type >(traits_type::length(pat))); }
    /** trim the given characters from the right and left of the string, return resulting trimmed substring. */
    SubStrType trim(const C* str, size_type len) const
    {
        if(empty()) return substr();
        size_type b = find_first_not_of(str, len, 0);
        if(b == StrType::npos)
        {
            return substr(0, 0);
        }
        size_type e = find_last_not_of(str, len, StrType::npos);
        return substr(b, e - b + 1);
    }

public:

    /** trim whitespace from the left of the string, return resulting trimmed substring. */
    SubStrType trimwsl() const
    {
        return triml(traits_type::whitespace_chars, traits_type::num_whitespace_chars);
    }

    /** trim whitespace from the right of the string, return resulting trimmed substring. */
    SubStrType trimwsr() const
    {
        return trimr(traits_type::whitespace_chars, traits_type::num_whitespace_chars);
    }

    /** trim whitespace from the right and left of the string, return resulting trimmed substring. */
    SubStrType trimws() const
    {
        return trim(traits_type::whitespace_chars, traits_type::num_whitespace_chars);
    }

public:

    SubStrType basename(C sep = C('/')) const
    {
        auto ss = popr(sep, /*skip_empty*/true);
        ss = ss.trimr(sep);
        return ss;
    }
    SubStrType dirname(C sep = C('/')) const
    {
        auto ss = gpopl(sep, /*skip_empty*/true);
        // try to finish with the separator
        if( ! ss.ends_with(sep))
        {
            if(_c4cthisSZ > ss.size())
            {
                auto rem = complr(ss);
                if(rem.count(sep) == rem.size())
                {
                    return substr();
                }
                else if((*this)[ss.size()] == sep)
                {
                    ss = substr(0, ss.size()+1);
                }
            }
        }
        return ss;
    }

    /** pop right: return the first split from the right. Use gpopl() to get
     * the reciprocal part. */
    SubStrType popr(C sep = C('/'), bool skip_empty=false) const
    {
        if(C4_LIKELY(_c4cthisSZ > 1))
        {
            auto pos = find_last_of(sep);
            if(pos != StrType::npos)
            {
                if(pos < _c4cthisSZ - 1) // does not end with sep
                {
                    return substr(pos + 1); // return from sep to end
                }
                else // the string ends with sep
                {
                    if( ! skip_empty)
                    {
                        return substr(pos + 1, 0);
                    }
                    auto ppos = find_last_not_of(sep); // skip repeated seps
                    if(ppos == StrType::npos) // the string is all made of seps
                    {
                        return substr(0, 0);
                    }
                    // find the previous sep
                    auto pos0 = find_last_of(sep, ppos);
                    if(pos0 == StrType::npos) // only the last sep exists
                    {
                        return substr(); // return the full string (because skip_empty is true)
                    }
                    ++pos0;
                    return substr(pos0);
                }
            }
            else // no sep was found, return the full string
            {
                return substr();
            }
        }
        else if(_c4cthisSZ == 1)
        {
            if(begins_with(sep))
            {
                return substr(0, 0);
            }
            return substr();
        }
        else // an empty string
        {
            return substr();
        }
    }

    /** pop left: return the first split from the left. Use gpopr() to get
     * the reciprocal part. */
    SubStrType popl(C sep = C('/'), bool skip_empty=false) const
    {
        if(C4_LIKELY(_c4cthisSZ > 1))
        {
            auto pos = find_first_of(sep);
            if(pos != StrType::npos)
            {
                if(pos > 0)  // does not start with sep
                {
                    return substr(0, pos); //  return everything up to it
                }
                else  // the string starts with sep
                {
                    if( ! skip_empty)
                    {
                        return substr(0, 0);
                    }
                    auto ppos = find_first_not_of(sep); // skip repeated seps
                    if(ppos == StrType::npos) // the string is all made of seps
                    {
                        return substr(0, 0);
                    }
                    // find the next sep
                    auto pos0 = find_first_of(sep, ppos);
                    if(pos0 == StrType::npos) // only the first sep exists
                    {
                        return substr(); // return the full string (because skip_empty is true)
                    }
                    C4_XASSERT(pos0 > 0);
                    // return everything up to the second sep
                    return substr(0, pos0);
                }
            }
            else // no sep was found, return the full string
            {
                return substr();
            }
        }
        else if(_c4cthisSZ == 1)
        {
            if(begins_with(sep))
            {
                return substr(0, 0);
            }
            return substr();
        }
        else // an empty string
        {
            return substr();
        }
    }

    /** Greedy POP Right: starting from the right, get all the splits except
     * the last on the left. */
    SubStrType gpopr(C sep = C('/'), bool skip_empty=false) const
    {
        auto ss = popl(sep, skip_empty);
        ss = complr(ss);
        auto pos = ss.find(sep);
        if(pos != StrType::npos)
        {
            if(ss.begins_with(sep))
            {
                if(skip_empty)
                {
                    ss = ss.triml(sep);
                }
                else
                {
                    ss = ss.substr(1);
                }
            }
        }
        return ss;
    }

    /** Greedy POP Left: starting from the left, get all the splits except
     * the last on the right. */
    SubStrType gpopl(C sep = C('/'), bool skip_empty=false) const
    {
        auto ss = popr(sep, skip_empty);
        ss = compll(ss);
        auto pos = ss.find(sep);
        if(pos != StrType::npos)
        {
            if(ss.ends_with(sep))
            {
                if(skip_empty)
                {
                    ss = ss.trimr(sep);
                }
                else
                {
                    ss = ss.substr(0, ss.size() - 1); // safe to subtract because ends_with(sep)
                }
            }
        }
        return ss;
    }

public:

    int compare(_str_crtp const& that) const
    {
        size_type mx = _c4cthisSZ > _c4cthatSZ ? _c4cthisSZ : _c4cthatSZ;
        return traits_type::compare(_c4cthisSTR, _c4cthatSTR, mx);
    }
    template< typename OSize, class OStr, class OSub >
    int compare(_str_crtp< C, OSize, OStr, OSub > const& that) const
    {
        auto cthat = static_cast< OStr const& >(that);
        C const* str = cthat.data();
        size_type sz = szconv< size_type >(cthat.size());
        size_type mx = _c4cthisSZ > sz ? _c4cthisSZ : sz;
        return traits_type::compare(_c4cthisSTR, str, mx);
    }
    template< size_t N >
    int compare(const C (&a)[N]) const
    {
        size_type mx = _c4cthisSZ > N-1 ? _c4cthisSZ : N-1;
        if(a == _c4cthisSTR) return true;
        return traits_type::compare(_c4cthisSTR, a, mx);
    }
    int compare(const C *a, SizeType sz) const
    {
        size_type mx = _c4cthisSZ > sz ? _c4cthisSZ : sz;
        if(a == _c4cthisSTR) return true;
        return traits_type::compare(_c4cthisSTR, a, mx);
    }
    int compare(const C *a) const
    {
        return traits_type::compare(_c4cthisSTR, a, _c4cthisSZ);
    }

    bool operator== (_str_crtp const& that) const
    {
        if(_c4cthatSZ != _c4cthisSZ) return false;
        if(_c4cthisSTR == _c4cthatSTR) return true;
        return traits_type::compare(_c4cthisSTR, _c4cthatSTR, _c4cthisSZ) == 0;
    }
    C4_ALWAYS_INLINE bool operator!= (_str_crtp const& that) const { return !this->operator== (that); }
    C4_ALWAYS_INLINE bool operator>= (_str_crtp const& that) const { return compare(that) >= 0; }
    C4_ALWAYS_INLINE bool operator<= (_str_crtp const& that) const { return compare(that) <= 0; }
    C4_ALWAYS_INLINE bool operator>  (_str_crtp const& that) const { return compare(that) >  0; }
    C4_ALWAYS_INLINE bool operator<  (_str_crtp const& that) const { return compare(that) <  0; }

    template< typename OSize, class OStr, class OSub > C4_ALWAYS_INLINE bool operator== (_str_crtp< C, OSize, OStr, OSub > const& that) const { return compare(that) == 0; }
    template< typename OSize, class OStr, class OSub > C4_ALWAYS_INLINE bool operator!= (_str_crtp< C, OSize, OStr, OSub > const& that) const { return compare(that) != 0; }
    template< typename OSize, class OStr, class OSub > C4_ALWAYS_INLINE bool operator>= (_str_crtp< C, OSize, OStr, OSub > const& that) const { return compare(that) >= 0; }
    template< typename OSize, class OStr, class OSub > C4_ALWAYS_INLINE bool operator<= (_str_crtp< C, OSize, OStr, OSub > const& that) const { return compare(that) <= 0; }
    template< typename OSize, class OStr, class OSub > C4_ALWAYS_INLINE bool operator>  (_str_crtp< C, OSize, OStr, OSub > const& that) const { return compare(that) >  0; }
    template< typename OSize, class OStr, class OSub > C4_ALWAYS_INLINE bool operator<  (_str_crtp< C, OSize, OStr, OSub > const& that) const { return compare(that) <  0; }

    bool operator== (const C *a) const
    {
        size_type m = szconv< size_type >(traits_type::length(a));
        if(m != _c4cthisSZ) return false;
        return traits_type::compare(_c4cthisSTR, a, _c4cthisSZ) == 0;
    }
    C4_ALWAYS_INLINE bool operator!= (const C *a) const { return !this->operator== (a); }
    C4_ALWAYS_INLINE bool operator>= (const C *a) const { return compare(a) >= 0; }
    C4_ALWAYS_INLINE bool operator<= (const C *a) const { return compare(a) <= 0; }
    C4_ALWAYS_INLINE bool operator>  (const C *a) const { return compare(a) >  0; }
    C4_ALWAYS_INLINE bool operator<  (const C *a) const { return compare(a) <  0; }

    C4_ALWAYS_INLINE friend bool operator== (const C *s, _str_crtp const& n) { return (n == s); }
    C4_ALWAYS_INLINE friend bool operator!= (const C *s, _str_crtp const& n) { return (n != s); }
    C4_ALWAYS_INLINE friend bool operator>= (const C *s, _str_crtp const& n) { return (n <= s); }
    C4_ALWAYS_INLINE friend bool operator<= (const C *s, _str_crtp const& n) { return (n >= s); }
    C4_ALWAYS_INLINE friend bool operator>  (const C *s, _str_crtp const& n) { return (n <  s); }
    C4_ALWAYS_INLINE friend bool operator<  (const C *s, _str_crtp const& n) { return (n >  s); }

    template< size_t N >
    bool operator== (const C (&a)[N]) const
    {
        if(N-1 != _c4cthisSZ) return false;
        if(_c4cthisSTR == &a[0]) return true;
        return traits_type::compare(_c4cthisSTR, a, _c4cthisSZ) == 0;
    }
    template< size_t N > C4_ALWAYS_INLINE bool operator!= (const C (&a)[N]) const { return !this->operator== (a); }
    template< size_t N > C4_ALWAYS_INLINE bool operator>= (const C (&a)[N]) const { return compare(a) >= 0; }
    template< size_t N > C4_ALWAYS_INLINE bool operator<= (const C (&a)[N]) const { return compare(a) <= 0; }
    template< size_t N > C4_ALWAYS_INLINE bool operator>  (const C (&a)[N]) const { return compare(a) >  0; }
    template< size_t N > C4_ALWAYS_INLINE bool operator<  (const C (&a)[N]) const { return compare(a) <  0; }

    template< size_t N > C4_ALWAYS_INLINE friend bool operator== (const C (&s)[N], _str_crtp const& n) { return (n == s); }
    template< size_t N > C4_ALWAYS_INLINE friend bool operator!= (const C (&s)[N], _str_crtp const& n) { return (n != s); }
    template< size_t N > C4_ALWAYS_INLINE friend bool operator>= (const C (&s)[N], _str_crtp const& n) { return (n <= s); }
    template< size_t N > C4_ALWAYS_INLINE friend bool operator<= (const C (&s)[N], _str_crtp const& n) { return (n >= s); }
    template< size_t N > C4_ALWAYS_INLINE friend bool operator>  (const C (&s)[N], _str_crtp const& n) { return (n <  s); }
    template< size_t N > C4_ALWAYS_INLINE friend bool operator<  (const C (&s)[N], _str_crtp const& n) { return (n >  s); }

public:

    C4_ALWAYS_INLINE void append(C const* str) { append(str, szconv< size_type >(traits_type::length(str))); }
    template< size_t N >
    C4_ALWAYS_INLINE void append(C const (&str)[N]) { append(&str[0], N-1); }
    C4_ALWAYS_INLINE void append(_str_crtp const& that) { append(_c4cthatSTR, _c4cthatSZ); }
    template< typename OSize, class OStrType, class OSub >
    C4_ALWAYS_INLINE void append(_str_crtp< C, OSize, OStrType, OSub > const& that)
    {
        auto cthat = static_cast< OStrType const& >(that);
        C const* str = cthat.data();
        size_type sz = szconv< size_type >(cthat.size());
        append(str, sz);
    }
    void append(C const* str, SizeType sz)
    {
        SizeType start = _c4thisSZ;
        _c4this->resize(start + sz);
        C * dest = _c4thisSTR + start;
        if(str + sz < dest || str > dest + sz) // if there's overlapping use memmove
        {
            traits_type::move(dest, str, sz);
        }
        else
        {
            traits_type::copy(dest, str, sz);
        }
        __nullterminate();
    }

    C4_ALWAYS_INLINE void prepend(C const* str) { prepend(str, szconv< size_type >(traits_type::length(str))); }
    template< size_t N >
    C4_ALWAYS_INLINE void prepend(C const (&str)[N]) { prepend(&str[0], N-1); }
    C4_ALWAYS_INLINE void prepend(_str_crtp const& that) { prepend(_c4cthatSTR, _c4cthatSZ); }
    template< typename OSize, class OStrType, class OSub >
    C4_ALWAYS_INLINE void prepend(_str_crtp< C, OSize, OStrType, OSub > const& that)
    {
        auto cthat = static_cast< OStrType const& >(that);
        C const* str = cthat.data();
        size_type sz = szconv< size_type >(cthat.size());
        prepend(str, sz);
    }
    void prepend(C const* str, SizeType sz)
    {
        SizeType mysz = _c4cthisSZ;
        _c4this->resize(mysz + sz);
        __nullterminate();
        C *mystr = _c4thisSTR;
        // shift current string right
        if(mysz > 0)
        {
            // since the memory will overlap, memmove() must be used
            traits_type::move(mystr + sz, mystr, mysz);
        }
        // now put the incoming string starting at the beginning
        traits_type::copy(mystr, str, sz);
    }

public:

    C4_ALWAYS_INLINE void pushr(C const* str, C sep=C('/')) { pushr(str, szconv< size_type >(traits_type::length(str)), sep); }
    template< size_t N >
    C4_ALWAYS_INLINE void pushr(C const (&str)[N], C sep=C('/')) { pushr(&str[0], N-1, sep); }
    C4_ALWAYS_INLINE void pushr(_str_crtp const& that, C sep=C('/')) { pushr(_c4cthatSTR, _c4cthatSZ, sep); }
    template< typename OSize, class OStrType, class OSub >
    C4_ALWAYS_INLINE void pushr(_str_crtp< C, OSize, OStrType, OSub > const& that, C sep=C('/'))
    {
        auto cthat = static_cast< OStrType const& >(that);
        C const* str = cthat.data();
        size_type sz = szconv< size_type >(cthat.size());
        pushr(str, sz, sep);
    }
    void pushr(C const* str, SizeType sz, C sep=C('/'))
    {
        // skip excess separators on the src string
        while(sz > 0 && *str == sep)
        {
            ++str;
            --sz;
        }
        //if(sz == 0) return;
        int start = ((int)_c4thisSZ);
        C * dest = _c4thisSTR;
        // skip excess separators on the dest string
        while(start > 0 && dest[start-1] == sep)
        {
            --start;
        }
        _c4this->resize(start + 1 + sz);
        dest = _c4thisSTR + start;
        *dest = '/';
        ++dest;
        traits_type::copy(dest, str, sz);
        __nullterminate();
    }

    C4_ALWAYS_INLINE void pushl(C const* str, C sep=C('/')) { pushl(str, szconv< size_type >(traits_type::length(str)), sep); }
    template< size_t N >
    C4_ALWAYS_INLINE void pushl(C const (&str)[N], C sep=C('/')) { pushl(&str[0], N-1, sep); }
    C4_ALWAYS_INLINE void pushl(_str_crtp const& that, C sep=C('/')) { pushl(_c4cthatSTR, _c4cthatSZ, sep); }
    template< typename OSize, class OStrType, class OSub >
    C4_ALWAYS_INLINE void pushl(_str_crtp< C, OSize, OStrType, OSub > const& that, C sep=C('/'))
    {
        auto cthat = static_cast< OStrType const& >(that);
        C const* str = cthat.data();
        size_type sz = szconv< size_type >(cthat.size());
        pushl(str, sz, sep);
    }
    C4_ALWAYS_INLINE void pushl(C const* str, size_type sz, C sep=C('/'))
    {
        // skip excess separators on the src string
        while(sz > 0 && *(str + sz - 1) == sep)
        {
            --sz;
        }
        //if(sz == 0) return;
        C * mystr = _c4thisSTR;
        size_type mysz = _c4thisSZ, myskip = 0;
        // skip excess separators on the dest string
        if(mysz > 0)
        {
            while(*mystr == sep)
            {
                ++mystr;
                --mysz;
                ++myskip;
            }
        }
        // since we may have skipped any number of separators,
        // we may need to do the copy now, or the subsequent resize
        // could delete characters from the end
        size_type nextdim = sz + 1 + mysz;
        if(_c4thisSZ >= nextdim)
        {
            traits_type::move(_c4thisSTR, mystr, mysz);
            myskip = 0;
        }
        // now resize
        _c4this->resize(sz + 1 + mysz);
        __nullterminate();
        mystr = _c4thisSTR;
        // move the existing contents to their final position
        traits_type::move(mystr + sz + 1, mystr + myskip, mysz);
        traits_type::copy(mystr, str, sz);
        mystr[sz] = sep;
    }

public:

    template< size_t N >
    C4_ALWAYS_INLINE void erase(C const (&str)[N]) { erase(&str[0], N-1); }
    C4_ALWAYS_INLINE void erase(C const *chars) { erase(chars, szconv< size_type >(traits_type::length(chars))); }
    C4_ALWAYS_INLINE void erase(_str_crtp const& that) { erase(_c4cthatSTR, _c4cthatSZ); }
    template< typename OSize, class OStrType, class OSub >
    C4_ALWAYS_INLINE void erase(_str_crtp< C, OSize, OStrType, OSub > const& that)
    {
        auto cthat = static_cast< OStrType const& >(that);
        C const* str = cthat.data();
        size_type sz = szconv< size_type >(cthat.size());
        erase(str, sz);
    }
    void erase(const char *chars, SizeType num_chars_to_remove)
    {
        for(size_type i = _c4thisSZ-1; i != size_type(-1); --i)
        {
            char c = _c4thisSTR[i];
            for(size_type j = 0; j < num_chars_to_remove; ++j)
            {
                if(c == chars[j])
                {
                    traits_type::move(_c4thisSTR + i, _c4thisSTR + i + 1, _c4thisSZ/* - 1*/ - i);
                    _c4this->resize(_c4thisSZ - 1);
                    break;
                }
            }
            if(_c4thisSZ == 0) return;
        }
    }

public:

#ifndef C4_STR_DISABLE_EXPR_TPL

    // expression templates
    template< class Expr >
    void assign_expr(_strExpr< C, Expr > const& expr)
    {
        _c4this->resize(szconv< SizeType >(expr.size()));
        C *curr = _c4thisSTR;
        size_t out = expr.put(curr, 0);
        C4_UNUSED(out); C4_ASSERT(out == expr.size());
        __nullterminate();
    }
    template< class Expr >
    void append_expr(_strExpr< C, Expr > const& expr)
    {
        _c4this->resize(szconv< SizeType >(size_t(_c4thisSZ) + expr.size()));
        C *curr = _c4thisSTR + _c4thisSZ;
        size_t out = expr.put(curr, 0);
        C4_UNUSED(out); C4_ASSERT(out == expr.size());
        __nullterminate();
    }
    template< class Expr >
    void push_expr(_strExpr< C, Expr > const& expr)
    {
        _c4this->resize(szconv< SizeType >(size_t(_c4thisSZ) + 1 + expr.size()));
        C *curr = _c4thisSTR + _c4thisSZ;
        *curr = '/';
        ++curr;
        size_t out = expr.put(curr, 0);
        C4_UNUSED(out); C4_ASSERT(out == expr.size());
        __nullterminate();
    }

#endif // ! C4_STR_DISABLE_EXPR_TPL

#ifdef C4_STR_DISABLE_EXPR_TPL
#endif // C4_STR_DISABLE_EXPR_TPL

#undef _c4this
#undef _c4thisSZ
#undef _c4thisSTR
#undef _c4thatSZ
#undef _c4thatSTR
#undef _c4cthisSZ
#undef _c4cthisSTR
#undef _c4cthatSZ
#undef _c4cthatSTR
#undef _c4CCAST

}; // _str_crtp
_C4_IMPLEMENT_TPL_STRIMPL_HASH(
    c4::_str_crtp< C C4_COMMA SizeType C4_COMMA StrType C4_COMMA SubStrType >,
    typename C, typename SizeType, class StrType, class SubStrType)


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// comparison operators

template< typename C, typename Size, class Str, class Sub > C4_ALWAYS_INLINE bool operator== (const C *s, _str_crtp< C, Size, Str, Sub > const& n) { return (n == s); }
template< typename C, typename Size, class Str, class Sub > C4_ALWAYS_INLINE bool operator!= (const C *s, _str_crtp< C, Size, Str, Sub > const& n) { return (n != s); }
template< typename C, typename Size, class Str, class Sub > C4_ALWAYS_INLINE bool operator>= (const C *s, _str_crtp< C, Size, Str, Sub > const& n) { return (n <= s); }
template< typename C, typename Size, class Str, class Sub > C4_ALWAYS_INLINE bool operator<= (const C *s, _str_crtp< C, Size, Str, Sub > const& n) { return (n >= s); }
template< typename C, typename Size, class Str, class Sub > C4_ALWAYS_INLINE bool operator>  (const C *s, _str_crtp< C, Size, Str, Sub > const& n) { return (n <  s); }
template< typename C, typename Size, class Str, class Sub > C4_ALWAYS_INLINE bool operator<  (const C *s, _str_crtp< C, Size, Str, Sub > const& n) { return (n >  s); }

template< size_t N, typename C, typename Size, class Str, class Sub > C4_ALWAYS_INLINE bool operator== (const C (&s)[N], _str_crtp< C, Size, Str, Sub > const& n) { return (n == s); }
template< size_t N, typename C, typename Size, class Str, class Sub > C4_ALWAYS_INLINE bool operator!= (const C (&s)[N], _str_crtp< C, Size, Str, Sub > const& n) { return (n != s); }
template< size_t N, typename C, typename Size, class Str, class Sub > C4_ALWAYS_INLINE bool operator>= (const C (&s)[N], _str_crtp< C, Size, Str, Sub > const& n) { return (n <= s); }
template< size_t N, typename C, typename Size, class Str, class Sub > C4_ALWAYS_INLINE bool operator<= (const C (&s)[N], _str_crtp< C, Size, Str, Sub > const& n) { return (n >= s); }
template< size_t N, typename C, typename Size, class Str, class Sub > C4_ALWAYS_INLINE bool operator>  (const C (&s)[N], _str_crtp< C, Size, Str, Sub > const& n) { return (n <  s); }
template< size_t N, typename C, typename Size, class Str, class Sub > C4_ALWAYS_INLINE bool operator<  (const C (&s)[N], _str_crtp< C, Size, Str, Sub > const& n) { return (n >  s); }


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// stream io

template< class StreamStringType, typename C, typename Size, class Str, class Sub >
C4_ALWAYS_INLINE
c4::sstream< StreamStringType >& operator<< (c4::sstream< StreamStringType >& os, _str_crtp< C, Size, Str, Sub > const& n)
{
    os.write(n.data(), n.size());
    return os;
}

template< class StreamStringType, typename C, typename Size, class Str, class Sub >
c4::sstream< StreamStringType >& operator>> (c4::sstream< StreamStringType >& is, _str_crtp< C, Size, Str, Sub > & n)
{
    decltype(is.remg()) pos = 0;
    // skip leading whitespace
    while(is.remg() >= pos && c4::isspace(is.peek(pos)))
    {
        ++pos;
    }
    // bump the stream to the end of the whitespace
    is.advg(pos);
    // now count until the next whitespace or until the stream ends
    pos = 0;
    while(is.remg() > pos)
    {
        C c = is.peek(pos);
        if(c4::isspace(c) || c == '\0') break;
        ++pos;
    }
    Str& ncast = static_cast< Str& >(n);
    ncast.resize(szconv< Size >(pos));
    is.read(ncast.data(), pos);
    return is;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** a proxy string which does not manage any memory. Basically a char/wchar_t
 * span enriched with string methods.
 *  - It is in general NOT null terminated.
 *  - Can be resized, but only to smaller sizes.
 *  - Can be written to output streams.
 *  - CANNOT be read into with input streams unless the read value
 *    is guaranteed to be smaller.
 * @see substring
 * @see wsubstring
 * @see basic_substringrs if you need to augment the size up to a capacity.
 * @ingroup string_classes
 * @ingroup nonowning_containers */
template< class C, class SizeType >
class basic_substring : public _str_crtp< C, SizeType, basic_substring< C, SizeType >, basic_substring< C, SizeType > >
{
public:

    using char_type = C;
    using traits_type = std::char_traits< C >;
    using size_type = SizeType;

    using value_type = typename traits_type::char_type;
    using difference_type = SizeType;
    using reference = C&;
    using const_reference = C const&;
    using pointer = C*;
    using const_pointer = C const*;

    static constexpr const SizeType npos = SizeType(-1);

    template< class C_ >
    using parameterized_string_type = basic_substring< C_, SizeType >;

public:

    //C4_ALWAYS_INLINE operator bool () const { return m_size > 0; }

public:

    _C4STR_ASSIGNOPS(C, basic_substring)

    C4_ALWAYS_INLINE basic_substring() : m_str(nullptr), m_size(0) {}

    C4_ALWAYS_INLINE basic_substring(C *p, SizeType sz) : m_str(p), m_size(sz) {}
    C4_ALWAYS_INLINE void assign(C *p, SizeType sz) { m_str = p; m_size = sz; }

    C4_ALWAYS_INLINE basic_substring(C *p) : m_str(p), m_size(szconv< size_type >(traits_type::length(p))) {}
    C4_ALWAYS_INLINE void assign(C *p) { m_str = p; m_size = szconv< size_type >(traits_type::length(p)); }
    C4_ALWAYS_INLINE basic_substring& operator= (C *p) { m_str = p; m_size = szconv< size_type >(traits_type::length(p)); return *this; }

    template< SizeType N > C4_ALWAYS_INLINE basic_substring(C (&a)[N]) : m_str(a), m_size(N-1) {} //< the size of C arrays includes the \0, so take off one
    template< SizeType N > C4_ALWAYS_INLINE void assign(C (&a)[N]) { m_str = &a[0]; m_size = N-1; } //< the size of C arrays includes the \0, so take off one
    template< SizeType N > C4_ALWAYS_INLINE basic_substring& operator= (C (&a)[N]) { m_str = &a[0]; m_size = N-1; return *this; } //< the size of C arrays includes the \0, so take off one

    C4_ALWAYS_INLINE basic_substring(std::nullptr_t) : m_str(nullptr), m_size(0) {}
    C4_ALWAYS_INLINE void assign(std::nullptr_t) { m_str = nullptr; m_size = 0; }
    C4_ALWAYS_INLINE basic_substring& operator= (std::nullptr_t) { m_str = nullptr; m_size = 0; return *this; }

    C4_ALWAYS_INLINE basic_substring(basic_substring const& that) : m_str(that.m_str), m_size(that.m_size) {}
    C4_ALWAYS_INLINE basic_substring(basic_substring     && that) : m_str(that.m_str), m_size(that.m_size) { that.m_str = nullptr; that.m_size = 0; }
    C4_ALWAYS_INLINE basic_substring& operator= (basic_substring const& that) { m_str = that.m_str; m_size = that.m_size; return *this; }
    C4_ALWAYS_INLINE basic_substring& operator= (basic_substring     && that) { m_str = that.m_str; m_size = that.m_size; that.m_str = nullptr; that.m_size = 0; return *this; }

    template< class OtherSize >
    C4_ALWAYS_INLINE basic_substring(basic_substring< C, OtherSize > const& that) : m_str(that.m_str), m_size(szconv< size_type >(that.m_size)) {}
    template< class OtherSize >
    C4_ALWAYS_INLINE void assign(basic_substring< C, OtherSize > const& that) { m_str = (that.m_str); m_size = (szconv< size_type >(that.m_size)); }
    template< class OtherSize >
    C4_ALWAYS_INLINE basic_substring& operator= (basic_substring< C, OtherSize > const& that) { m_str = (that.m_str); m_size = (szconv< size_type >(that.m_size)); return *this; }

    template< class OtherSize >
    C4_ALWAYS_INLINE basic_substring(span< C, OtherSize > const& that) : m_str(that.data()), m_size(szconv< size_type >(that.size())) {}
    template< class OtherSize >
    C4_ALWAYS_INLINE basic_substring(spanrs< C, OtherSize > const& that) : m_str(that.data()), m_size(szconv< size_type >(that.size())) {}

    C4_ALWAYS_INLINE void clear() { m_str = nullptr; m_size = 0; }

    C4_ALWAYS_INLINE const C* data () const noexcept { return m_str; }
    C4_ALWAYS_INLINE       C* data ()       noexcept { return m_str; }

    C4_ALWAYS_INLINE SizeType size () const noexcept { return m_size; }

    C4_ALWAYS_INLINE void resize (SizeType sz) { C4_CHECK(sz <= m_size); m_size = sz; }
    C4_ALWAYS_INLINE void reserve(SizeType sz) { C4_CHECK(sz <= m_size); m_size = sz; }

    C4_ALWAYS_INLINE SizeType capacity() const { return m_size; }
    constexpr C4_ALWAYS_INLINE SizeType max_size() const { return std::numeric_limits< SizeType >::max(); }

private:

    C        * m_str;
    SizeType   m_size;

    template< class C2, class S2 >
    friend class basic_substring;

    friend void test_string_member_alignment();

}; // basic_substring

template< class C, class SizeType >
const SizeType basic_substring< C, SizeType >::npos;

_C4_IMPLEMENT_TPL_STRIMPL_HASH(c4::basic_substring< C C4_COMMA SizeType >, typename C, typename SizeType)

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** substringReSizeable: same as substring, but additionally has a fixed
 * max capacity, so it can be resizeable up to capacity. Therefore, it
 * can be modified with methods that change its size such as append(), prepend(),
 * or read with input streams (again, up to capacity).
 * @see substringrs
 * @see wsubstringrs
 * @see basic_substring
 * @ingroup string_classes
 * @ingroup nonowning_containers */
template< class C, class SizeType >
class basic_substringrs : public _str_crtp< C, SizeType, basic_substringrs< C, SizeType >, basic_substringrs< C, SizeType > >
{
public:

    using char_type = C;
    using traits_type = std::char_traits< C >;
    using size_type = SizeType;

    using value_type = typename traits_type::char_type;
    using difference_type = SizeType;
    using reference = C&;
    using const_reference = C const&;
    using pointer = C*;
    using const_pointer = C const*;

    static constexpr const size_type npos = SizeType(-1);

    template< class C_ >
    using parameterized_string_type = basic_substringrs< C_, SizeType >;

public:

    //C4_ALWAYS_INLINE operator bool () const { return m_size > 0; }

    C4_ALWAYS_INLINE operator substring const& () const { return *(substring const*)this; }
    C4_ALWAYS_INLINE operator substring      & ()       { return *(substring      *)this; }

public:

    _C4STR_ASSIGNOPS(C, basic_substringrs)

    C4_ALWAYS_INLINE basic_substringrs() : m_str(nullptr), m_size(0), m_capacity(0) {}

    C4_ALWAYS_INLINE basic_substringrs(C *p, SizeType sz) : m_str(p), m_size(sz), m_capacity(sz) {}
    C4_ALWAYS_INLINE void assign(C *p, SizeType sz) { m_str = p; m_size = sz; m_capacity = sz; }

    C4_ALWAYS_INLINE basic_substringrs(C *p, SizeType sz, SizeType cap) : m_str(p), m_size(sz), m_capacity(cap) {}
    C4_ALWAYS_INLINE void assign(C *p, SizeType sz, SizeType cap) { m_str = p; m_size = sz; m_capacity = cap; }

    C4_ALWAYS_INLINE basic_substringrs(C* p) : m_str(p), m_size(szconv< size_type >(traits_type::length(p))), m_capacity(m_size+1) {}
    C4_ALWAYS_INLINE void assign(C *p) { m_str = p; m_size = szconv< size_type >(traits_type::length(p)); m_capacity = m_size+1; }
    C4_ALWAYS_INLINE basic_substringrs& operator= (C *p) { m_str = p; m_size = szconv< size_type >(traits_type::length(p)); m_capacity = m_size+1; return *this; }

    template< SizeType N > C4_ALWAYS_INLINE basic_substringrs(C (&a)[N]) : m_str(a), m_size(N-1), m_capacity(N) {} ///< the size of C arrays includes the \0, so take off one
    template< SizeType N > C4_ALWAYS_INLINE void assign(C (&a)[N]) { m_str = &a[0]; m_size = N-1; m_capacity = N; } ///< the size of C arrays includes the \0, so take off one
    template< SizeType N > C4_ALWAYS_INLINE basic_substringrs& operator= (C (&a)[N]) { m_str = &a[0]; m_size = N-1; m_capacity = N; return *this; } ///< the size of C arrays includes the \0, so take off one

    C4_ALWAYS_INLINE basic_substringrs(std::nullptr_t) : m_str(nullptr), m_size(0), m_capacity(0) {}
    C4_ALWAYS_INLINE void assign(std::nullptr_t) { m_str = nullptr; m_size = 0; m_capacity = 0;  }
    C4_ALWAYS_INLINE basic_substringrs& operator= (std::nullptr_t) { m_str = nullptr; m_size = 0; m_capacity = 0; return *this; }

    C4_ALWAYS_INLINE basic_substringrs(basic_substringrs const& that) : m_str(that.m_str), m_size(that.m_size), m_capacity(that.m_capacity) {}
    C4_ALWAYS_INLINE basic_substringrs(basic_substringrs     && that) : m_str(that.m_str), m_size(that.m_size), m_capacity(that.m_capacity) { that.m_str = nullptr; that.m_size = 0; that.m_capacity = 0; }
    C4_ALWAYS_INLINE basic_substringrs& operator= (basic_substringrs const& that) { m_str = (that.m_str); m_size = (that.m_size); m_capacity = that.m_capacity; return *this; }
    C4_ALWAYS_INLINE basic_substringrs& operator= (basic_substringrs     && that) { m_str = (that.m_str); m_size = (that.m_size); that.m_str = nullptr; that.m_size = 0; that.m_capacity = 0; return *this; }

    template< class OtherSize >
    C4_ALWAYS_INLINE basic_substringrs(basic_substringrs< C, OtherSize > const& that) : m_str(that.m_str), m_size(szconv< size_type >(that.m_size)), m_capacity(szconv< size_type >(that.m_capacity)) {}
    template< class OtherSize >
    C4_ALWAYS_INLINE void assign(basic_substringrs< C, OtherSize > const& that) { m_str = (that.m_str); m_size = (szconv< size_type >(that.m_size)); m_capacity = (szconv< size_type >(that.m_capacity)); }
    template< class OtherSize >
    C4_ALWAYS_INLINE basic_substringrs& operator= (basic_substringrs< C, OtherSize > const& that) { m_str = (that.m_str); m_size = (szconv< size_type >(that.m_size)); m_capacity = (szconv< size_type >(that.m_capacity)); return *this; }

    template< class OtherSize >
    C4_ALWAYS_INLINE basic_substringrs(span< C, OtherSize > const& that) : m_str(that.data()), m_size(szconv< size_type >(that.size())) {}
    template< class OtherSize >
    C4_ALWAYS_INLINE basic_substringrs(spanrs< C, OtherSize > const& that) : m_str(that.data()), m_size(szconv< size_type >(that.size())) {}

    C4_ALWAYS_INLINE void clear() { m_size = 0; }

    C4_ALWAYS_INLINE const C* data () const noexcept { return m_str; }
    C4_ALWAYS_INLINE       C* data ()       noexcept { return m_str; }

    C4_ALWAYS_INLINE SizeType size () const noexcept { return m_size; }

    C4_ALWAYS_INLINE void resize(SizeType sz) { C4_CHECK(sz <= m_capacity && (m_str != nullptr || sz == 0)); m_size = sz; }
    C4_ALWAYS_INLINE void reserve(SizeType sz) { C4_CHECK(sz <= m_capacity && (m_str != nullptr || sz == 0)); m_size = sz; }

    C4_ALWAYS_INLINE SizeType capacity() const { return m_capacity; }
    constexpr C4_ALWAYS_INLINE SizeType max_size() const { return std::numeric_limits< SizeType >::max(); }

private:

    C * m_str;
    SizeType m_size;
    SizeType m_capacity;

    template< class C2, class S2 >
    friend class basic_substringrs;
    friend void test_string_member_alignment();

}; // basic_substringrs

template< class C, class SizeType >
constexpr const SizeType basic_substringrs< C, SizeType >::npos;

_C4_IMPLEMENT_TPL_STRIMPL_HASH(c4::basic_substringrs< C C4_COMMA SizeType >, typename C, typename SizeType)

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** a string which always allocates memory, ie, without the small
 * string optimization
 * @ingroup string_classes */
template< class C, class SizeType, class Alloc >
class basic_text : public _str_crtp< C, SizeType, basic_text< C, SizeType, Alloc >, basic_substring< C, SizeType > >
{
    using impl_type = _str_crtp< C, SizeType, basic_text< C, SizeType, Alloc >, basic_substring< C, SizeType > >;
public:

    using char_type = C;
    using value_type = C;
    using traits_type = typename impl_type::traits_type;
    using allocator_type = Alloc;
    using alloc_traits = std::allocator_traits< Alloc >;
    using size_type  = SizeType;
    using ssize_type = typename std::make_signed< SizeType >::type;

    template< class C_ >
    using parameterized_string_type = basic_text< C_, SizeType, typename alloc_traits::template rebind_alloc< C_ > >;

    static constexpr const SizeType npos = SizeType(-1);

private:

    C *      m_str;
    SizeType m_size;
    SizeType m_capacity;
    Alloc    m_alloc;

    friend void test_string_member_alignment();

public:

    //C4_ALWAYS_INLINE operator bool     () const { return m_size > 0; }

    C4_ALWAYS_INLINE operator basic_substring< C, SizeType > const& () const
    {
        return *(basic_substring< C, SizeType > const*)this;
    }
    C4_ALWAYS_INLINE operator basic_substringrs< C, SizeType > const& () const
    {
        return *(basic_substringrs< C, SizeType > const*)this;
    }

public:

    _C4STR_ASSIGNOPS(C, basic_text)

    C4_ALWAYS_INLINE basic_text() : m_str(nullptr), m_size(0), m_capacity(0), m_alloc() {}
    C4_ALWAYS_INLINE basic_text(Alloc const& a) : m_str(nullptr), m_size(0), m_capacity(0), m_alloc(a) {}
    C4_ALWAYS_INLINE ~basic_text()
    {
        free();
    }

    C4_ALWAYS_INLINE basic_text(SizeType sz) : m_str(nullptr), m_size(0), m_capacity(0), m_alloc() { resize(sz); }
    C4_ALWAYS_INLINE basic_text(SizeType sz, Alloc const& a) : m_str(nullptr), m_size(0), m_capacity(0), m_alloc(a) { resize(sz); }

    C4_ALWAYS_INLINE basic_text(SizeType sz, SizeType cap) : m_str(nullptr), m_size(0), m_capacity(0), m_alloc() { reserve(cap); resize(sz); }
    C4_ALWAYS_INLINE basic_text(SizeType sz, SizeType cap, Alloc const& a) : m_str(nullptr), m_size(0), m_capacity(0), m_alloc(a) { reserve(cap); resize(sz); }

    C4_ALWAYS_INLINE basic_text(basic_text const& s) : basic_text() { assign(s); }
    C4_ALWAYS_INLINE basic_text(basic_text     && s) : basic_text() { assign(std::move(s)); }
    C4_ALWAYS_INLINE basic_text(basic_text const& s, Alloc const& a) : basic_text(a) { assign(s); }
    C4_ALWAYS_INLINE basic_text(basic_text     && s, Alloc const& a) : basic_text(a) { assign(std::move(s)); }
    C4_ALWAYS_INLINE basic_text& operator= (basic_text const& s) { assign(s); return *this; }
    C4_ALWAYS_INLINE basic_text& operator= (basic_text     && s) { assign(std::move(s)); return *this; }
    void assign(basic_text const& s)
    {
        resize(s.m_size);
        assign(s.m_str, s.m_size);
    }
    void assign(basic_text     && s)
    {
        free();
        m_str = s.m_str;
        m_size = s.m_size;
        m_capacity = s.m_capacity;
        s.m_str = nullptr;
        s.m_size = 0;
        s.m_capacity = 0;
    }

    C4_ALWAYS_INLINE basic_text(substring const& s) : basic_text() { assign(s.data(), s.size()); }
    C4_ALWAYS_INLINE basic_text(substring const& s, Alloc const& a) : basic_text(a) { assign(s.data(), s.size()); }
    C4_ALWAYS_INLINE basic_text& operator= (substring const& s) { assign(s.data(), s.size()); return *this; }
    C4_ALWAYS_INLINE void assign(substring const& s) { assign(s.data(), s.size()); }

    /** @warning assumes the string is null-terminated */
    C4_ALWAYS_INLINE basic_text(const C *s) : basic_text() { assign(s, szconv< size_type >(traits_type::length(s))); }
    /** @warning assumes the string is null-terminated */
    C4_ALWAYS_INLINE basic_text(const C *s, Alloc const& a) : basic_text(a) { assign(s, szconv< size_type >(traits_type::length(s))); }
    /** @warning assumes the string is null-terminated */
    C4_ALWAYS_INLINE basic_text& operator= (const C  *s) { assign(s, szconv< size_type >(traits_type::length(s))); return *this; }
    /** @warning assumes the string is null-terminated */
    void assign(const C *s) { assign(s, szconv< size_type >(traits_type::length(s))); }

    C4_ALWAYS_INLINE basic_text(const C *s, SizeType n) : basic_text() { assign(s, n); }
    C4_ALWAYS_INLINE basic_text(const C *s, SizeType n, Alloc const& a) : basic_text(a) { assign(s, n); }
    C4_ALWAYS_INLINE void assign(const C *s, SizeType n)
    {
        resize(n);
        traits_type::copy(m_str, s, n);
        m_str[m_size] = '\0';
    }

    template< int N > C4_ALWAYS_INLINE basic_text(const C (&s)[N]) : basic_text() { assign(&s[0], N-1); }
    template< int N > C4_ALWAYS_INLINE basic_text(const C (&s)[N], Alloc const& a) : basic_text(a) { assign(&s[0], N-1); }
    template< int N > C4_ALWAYS_INLINE basic_text& operator= (const C (&s)[N]) { assign(&s[0], N-1); return *this; }
    template< int N > C4_ALWAYS_INLINE void assign(const C (&s)[N]) { assign(&s[0], N-1); }
    template< int N > C4_ALWAYS_INLINE void assign(const C (&s)[N], SizeType n) { assign(&s[0], n); }

    template< class OtherSize >
    C4_ALWAYS_INLINE basic_text(span< C, OtherSize > const& that) : basic_text() { assign(that.data(), that.size()); }
    template< class OtherSize >
    C4_ALWAYS_INLINE basic_text(spanrs< C, OtherSize > const& that) : basic_text() { assign(that.data(), that.size()); }

    C4_ALWAYS_INLINE basic_text(const C *s, SizeType n, SizeType cap)                 : basic_text()  { assign(s, n, cap); }
    C4_ALWAYS_INLINE basic_text(const C *s, SizeType n, SizeType cap, Alloc const& a) : basic_text(a) { assign(s, n, cap); }
    void assign(const C *s, SizeType n, SizeType cap) { C4_ASSERT(cap >= n); reserve(cap); assign(s, n); }

    C4_ALWAYS_INLINE basic_text(C c, SizeType n)                 : basic_text()  { assign(c, n, n); }
    C4_ALWAYS_INLINE basic_text(C c, SizeType n, Alloc const& a) : basic_text(a) { assign(c, n, n); }
    C4_ALWAYS_INLINE void assign(C c, SizeType n) { assign(c, n, n); }

    C4_ALWAYS_INLINE basic_text(C c, SizeType n, SizeType cap)                 : basic_text()  { assign(c, n, cap); }
    C4_ALWAYS_INLINE basic_text(C c, SizeType n, SizeType cap, Alloc const& a) : basic_text(a) { assign(c, n, cap); }
    void assign(C c, SizeType n, SizeType cap)
    {
        C4_ASSERT(cap >= n);
        reserve(cap);
        resize(n);
        for(size_type i = 0; i < n; ++i)
        {
            m_str[i] = c;
        }
        m_str[m_size] = '\0';
    }

    template< class OtherSize, class Al_ >
    C4_ALWAYS_INLINE basic_text(basic_text< C, OtherSize, Al_ > const& that) { assign(that); }
    template< class OtherSize, class Al_ >
    C4_ALWAYS_INLINE void assign(basic_text< C, OtherSize, Al_ > const& that) { assign(that.m_str, szconv< size_type >(that.m_size)); }
    template< class OtherSize, class Al_ >
    C4_ALWAYS_INLINE basic_text& operator= (basic_text< C, OtherSize, Al_ > const& that) { assign(that); return *this; }

public:

    C4_ALWAYS_INLINE       C * c_str()       C4_NOEXCEPT_A { C4_ASSERT(m_str != nullptr); return m_str; }
    C4_ALWAYS_INLINE const C * c_str() const C4_NOEXCEPT_A { C4_ASSERT(m_str != nullptr); return m_str; }

    C4_ALWAYS_INLINE const C* data() const noexcept { return m_str; }
    C4_ALWAYS_INLINE       C* data()       noexcept { return m_str; }

    C4_ALWAYS_INLINE SizeType size() const noexcept { return m_size; }

    C4_ALWAYS_INLINE SizeType capacity() const noexcept { return m_capacity; }
    constexpr C4_ALWAYS_INLINE SizeType max_size() const noexcept { return alloc_traits::max_size(m_alloc) - 1; }

public:

    void resize(SizeType sz)
    {
        reserve(sz);
        m_size = sz;
        m_str[m_size] = '\0';
    }
    void reserve(SizeType sz)
    {
        //if(C4_UNLIKELY(sz == 0)) return;
        if((sz + 1) > m_capacity)
        {
            _resizebuf(sz + 1);
        }
    }
    void grow(SizeType more)
    {
        SizeType next = (SizeType)(1.618f * float(m_size));
        SizeType sz = m_size + more;
        sz = sz > next ? sz : next;
        reserve(sz);
        m_size = sz;
        m_str[m_size] = '\0';
    }
    void clear()
    {
        m_size = 0;
        if(m_str)
        {
            m_str[0] = '\0';
        }
    }
    void free()
    {
        m_alloc.deallocate(m_str, m_capacity);
        m_str = nullptr;
        m_size = 0;
        m_capacity = 0;
    }

    void shrink()
    {
        if(m_size == 0)
        {
            free();
        }
        else if(capacity() > m_size + 1)
        {
            _resizebuf(m_size + 1);
        }
    }

private:

    void _resizebuf(SizeType sz)
    {
        C *tmp = (C*)m_alloc.allocate(sizeof(C) * sz);
        if(m_str)
        {
            traits_type::copy(tmp, m_str, m_size+1);
            m_alloc.deallocate(m_str, m_capacity);
        }
        m_str = tmp;
        m_capacity = sz;
    }

public:

    void push_back(C c)
    {
        SizeType sz = m_size;
        grow(1);
        m_str[sz] = c;
    }

};

template< class C, class SizeType, class Alloc >
constexpr const SizeType basic_text< C, SizeType, Alloc >::npos;

_C4_IMPLEMENT_TPL_STRIMPL_HASH(c4::basic_text< C C4_COMMA SizeType C4_COMMA Alloc >,
    typename C, typename SizeType, class Alloc);

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/// @cond dev

template< class _Char, size_t NumBytes >
struct _shortstr;

template< size_t NumBytes >
struct _shortstr< char, NumBytes >
{
    C4_STATIC_ASSERT(sizeof(char) == 1); // maybe this isn't true in some galaxy?
    char flag_n_sz;
    char arr[NumBytes - 1];
};

template< size_t NumBytes >
struct _shortstr< wchar_t, NumBytes >
{
    C4_STATIC_ASSERT(sizeof(char) == 1); // maybe this isn't true in some galaxy?
    C4_STATIC_ASSERT(NumBytes % sizeof(wchar_t) == 0);
    char flag_n_sz;
    char __pad[sizeof(wchar_t) - 1]; // this will only work if sizeof(wchar_t) > 1
    wchar_t arr[NumBytes/sizeof(wchar_t) - 1];
};

/// @endcond

//-----------------------------------------------------------------------------
/** a string class with the small string optimization
 * @ingroup string_classes */
template< class C, class SizeType, class Alloc >
class basic_string : public _str_crtp< C, SizeType, basic_string< C, SizeType, Alloc >, basic_substring< C, SizeType > >
{
    using impl_type = _str_crtp< C, SizeType, basic_string< C, SizeType, Alloc >, basic_substring< C, SizeType > >;

    /// @cond dev

    struct _long
    {
        SizeType flag_n_sz; ///< flag (first bit) and size
        SizeType cap;
        C *      str;
    };
    using _short = _shortstr< C, sizeof(_long) >;

    /// @endcond

public:

    using char_type = C;
    using value_type = C;
    using traits_type = typename impl_type::traits_type;
    using allocator_type = Alloc;
    using alloc_traits = std::allocator_traits< Alloc >;
    using size_type  = SizeType;
    using ssize_type = typename std::make_signed< SizeType >::type;

    template< class C_ >
    using parameterized_string_type = basic_string< C_, SizeType, typename alloc_traits::template rebind_alloc< C_ > >;

    static constexpr const SizeType npos = SizeType(-1);
    static constexpr const SizeType arr_size = C4_COUNTOF(_short::arr);

private:

    /**
     * @see http://stackoverflow.com/questions/21694302/what-are-the-mechanics-of-short-string-optimization-in-libc
     * @see https://github.com/elliotgoodrich/SSO-23
     */
    union {
        _short m_short;
        _long  m_long;
        //char   m_raw[sizeof(_short) > sizeof(_long) ? sizeof(_short) : sizeof(_long)];
    };
    Alloc m_alloc;

    void     _set_short_sz(SizeType sz) { C4_XASSERT_MSG((((char)sz << 1) >> 1) == sz, "size overflow"); m_short.flag_n_sz = (char)sz << 1; }
    void     _set_long_sz(SizeType sz) { C4_XASSERT_MSG(((sz << 1) >> 1) == sz, "size overflow"); m_long.flag_n_sz = (sz << 1) | 1; }
    SizeType _get_short_sz() const { return m_short.flag_n_sz >> 1; }
    SizeType _get_long_sz() const { return m_long.flag_n_sz >> 1; }

    template< class, class >
    friend void test_small_string_flag_alignment();

public:

    //C4_ALWAYS_INLINE operator bool () const { return size() > 0; }

    C4_ALWAYS_INLINE operator const basic_substring  < const C, SizeType > () const { return basic_substring  < const C, SizeType >(data(), size()); }
    C4_ALWAYS_INLINE operator const basic_substring  <       C, SizeType > ()       { return basic_substring  <       C, SizeType >(data(), size()); }
    C4_ALWAYS_INLINE operator const basic_substringrs< const C, SizeType > () const { return basic_substringrs< const C, SizeType >(data(), size()); }
    C4_ALWAYS_INLINE operator const basic_substringrs<       C, SizeType > ()       { return basic_substringrs<       C, SizeType >(data(), size()); }

public:

    C4_ALWAYS_INLINE char is_long() const { return m_short.flag_n_sz & 1; }

    C4_ALWAYS_INLINE       C* c_str()       C4_NOEXCEPT_A { C4_ASSERT(size() > 0); return data(); }
    C4_ALWAYS_INLINE const C* c_str() const C4_NOEXCEPT_A { C4_ASSERT(size() > 0); return data(); }

    C4_ALWAYS_INLINE       C* data()       noexcept { return is_long() ? m_long.str : m_short.arr; }
    C4_ALWAYS_INLINE const C* data() const noexcept { return is_long() ? m_long.str : m_short.arr; }

    C4_ALWAYS_INLINE SizeType size() const noexcept { return is_long() ? _get_long_sz() : _get_short_sz(); }

    C4_ALWAYS_INLINE SizeType capacity() const noexcept { return is_long() ? m_long.cap : arr_size; }

    constexpr C4_ALWAYS_INLINE SizeType max_size() const noexcept { return (alloc_traits::max_size(m_alloc) >> 1) - 1; } // there's one less bit for the short/long flag

public:

    _C4STR_ASSIGNOPS(C, basic_string)

    C4_ALWAYS_INLINE basic_string() : m_alloc() { memset(&m_short, 0, sizeof(m_short)); }
    C4_ALWAYS_INLINE basic_string(Alloc const& a) : m_alloc(a) { memset(&m_short, 0, sizeof(m_short)); }
    C4_ALWAYS_INLINE ~basic_string()
    {
        this->_free();
    }

    C4_ALWAYS_INLINE basic_string(SizeType sz)                 : basic_string()  { resize(sz); }
    C4_ALWAYS_INLINE basic_string(SizeType sz, Alloc const& a) : basic_string(a) { resize(sz); }

    C4_ALWAYS_INLINE basic_string(SizeType sz, SizeType cap)                 : basic_string()  { reserve(cap); resize(sz); }
    C4_ALWAYS_INLINE basic_string(SizeType sz, SizeType cap, Alloc const& a) : basic_string(a) { reserve(cap); resize(sz); }

    C4_ALWAYS_INLINE basic_string(basic_string const& s)                 : basic_string(s.m_alloc) { assign(s); }
    C4_ALWAYS_INLINE basic_string(basic_string     && s)                 : basic_string(s.m_alloc) { assign(std::move(s)); }
    C4_ALWAYS_INLINE basic_string(basic_string const& s, Alloc const& a) : basic_string(a) { assign(s); }
    C4_ALWAYS_INLINE basic_string(basic_string     && s, Alloc const& a) : basic_string(a) { assign(std::move(s)); }
    C4_ALWAYS_INLINE basic_string& operator= (basic_string const& s) { assign(s); return *this; }
    C4_ALWAYS_INLINE basic_string& operator= (basic_string     && s) { assign(std::move(s)); return *this; }
    void assign(basic_string const& that)
    {
        if(&that == this) return;
        resize(that.size());
        traits_type::copy(data(), that.data(), that.size());
    }
    /** keep the current long status, even though the short array
     * would be enough to hold the string. This is because the
     * memory is already allocated, so there's little cost in
     * keeping using it. If explicit resizing is desired, call shrink_to_fit() */
    void assign(basic_string && that)
    {
        if(&that == this) return;
        if(that.is_long())
        {
            if(is_long())
            {
                this->_free();
            }
            m_long = that.m_long;
            memset(&that.m_long, 0, sizeof(that.m_long));
        }
        else
        {
            auto sz = that._get_short_sz();
            if(is_long())
            {
                _set_long_sz(sz);
                traits_type::copy(m_long.str, that.m_short.arr, sz);
                m_long.str[sz] = C(0);
            }
            else
            {
                _set_short_sz(sz);
                traits_type::copy(m_short.arr, that.m_short.arr, sz);
                m_short.arr[sz] = C(0);
            }
            that._set_short_sz(0);
            that.m_short.arr[0] = C(0);
        }
    }

    C4_ALWAYS_INLINE basic_string(substring const& s)                 : basic_string()  { assign(s.data(), s.size()); }
    C4_ALWAYS_INLINE basic_string(substring const& s, Alloc const& a) : basic_string(a) { assign(s.data(), s.size()); }
    C4_ALWAYS_INLINE basic_string& operator= (substring const& s) { assign(s.data(), s.size()); return *this; }
    C4_ALWAYS_INLINE void assign(substring const& s) { assign(s.data(), s.size()); }

    /** construct from a null-terminated string */
    C4_ALWAYS_INLINE basic_string(const C *s) : basic_string() { assign(s, szconv< size_type >(traits_type::length(s))); }
    /** construct from a null-terminated string */
    C4_ALWAYS_INLINE basic_string(const C *s, Alloc const& a) : basic_string(a) { assign(s, szconv< size_type >(traits_type::length(s))); }
    /** construct from a null-terminated string */
    C4_ALWAYS_INLINE basic_string& operator= (const C  *s) { assign(s, szconv< size_type >(traits_type::length(s))); return *this; }
    /** assign from a null-terminated string */
    void assign(const C *s) { assign(s, szconv< size_type >(traits_type::length(s))); }

    C4_ALWAYS_INLINE basic_string(const C *s, SizeType n)                 : basic_string()  { assign(s, n); }
    C4_ALWAYS_INLINE basic_string(const C *s, SizeType n, Alloc const& a) : basic_string(a) { assign(s, n); }
    C4_ALWAYS_INLINE void assign(const C *s, SizeType n)
    {
        resize(n);
        C *d = data();
        traits_type::copy(d, s, n);
        d[n] = C(0);
    }

    template< int N > C4_ALWAYS_INLINE basic_string(const C (&s)[N])                 : basic_string()  { assign(&s[0], N-1); }
    template< int N > C4_ALWAYS_INLINE basic_string(const C (&s)[N], Alloc const& a) : basic_string(a) { assign(&s[0], N-1); }
    template< int N > C4_ALWAYS_INLINE basic_string& operator= (const C (&s)[N]) { assign(&s[0], N-1); return *this; }
    template< int N > C4_ALWAYS_INLINE void assign(const C (&s)[N])              { assign(&s[0], N-1); }

    C4_ALWAYS_INLINE basic_string(const C *s, SizeType n, SizeType cap)                 : basic_string()  { assign(s, n, cap); }
    C4_ALWAYS_INLINE basic_string(const C *s, SizeType n, SizeType cap, Alloc const& a) : basic_string(a) { assign(s, n, cap); }
    void assign(const C *s, SizeType n, SizeType cap) { C4_ASSERT(cap >= n); reserve(cap); assign(s, n); }

    C4_ALWAYS_INLINE basic_string(C c, SizeType n)                 : basic_string()  { assign(c, n, n); }
    C4_ALWAYS_INLINE basic_string(C c, SizeType n, Alloc const& a) : basic_string(a) { assign(c, n, n); }
    C4_ALWAYS_INLINE void assign(C c, SizeType n) { assign(c, n, n); }
    C4_ALWAYS_INLINE void assign(C c) { assign(c, size(), capacity()); }

    C4_ALWAYS_INLINE basic_string(C c, SizeType n, SizeType cap)                 : basic_string()  { assign(c, n, cap); }
    C4_ALWAYS_INLINE basic_string(C c, SizeType n, SizeType cap, Alloc const& a) : basic_string(a) { assign(c, n, cap); }
    void assign(C c, SizeType n, SizeType cap)
    {
        C4_ASSERT(cap >= n);
        reserve(cap);
        resize(n);
        C *d = data();
        for(size_type i = 0; i < n; ++i)
        {
            d[i] = c;
        }
        d[n] = C(0);
    }

    template< class Sz_, class Al_ > C4_ALWAYS_INLINE basic_string(basic_string< C, Sz_, Al_ > const& that) { assign(that); }
    template< class Sz_, class Al_ > C4_ALWAYS_INLINE basic_string& operator= (basic_string< C, Sz_, Al_ > const& that) { assign(that); return *this; }
    template< class Sz_, class Al_ > C4_ALWAYS_INLINE void assign(basic_string< C, Sz_, Al_ > const& that) { assign(that.data(), szconv< size_type >(that.size())); }

    template< class OtherSize >
    C4_ALWAYS_INLINE basic_string(span< C, OtherSize > const& that) : basic_string() { assign(that.data(), that.size()); }
    template< class OtherSize >
    C4_ALWAYS_INLINE basic_string(spanrs< C, OtherSize > const& that) : basic_string() { assign(that.data(), that.size()); }

public:

    void push_back(C c)
    {
        SizeType sz = size();
        grow(1);
        data()[sz] = c;
    }

public:

    void resize(SizeType sz)
    {
        SizeType prev = size();
        _resize(sz);
        if(sz > prev)
        {
            traits_type::assign(data() + prev, sz - prev, C(0));
        }
        data()[sz] = C(0);
    }
    void reserve(SizeType cap)
    {
        SizeType next = szconv< SizeType >((size_t)(1.618f * float(size())));
        cap = cap > next ? cap : next;
        _reserve(cap);
    }
    void grow(SizeType more)
    {
        auto sz = size() + more;
        reserve(sz + 1);
        _set_size(sz);
        data()[sz] = C(0);
    }
    void clear()
    {
        _set_size(0);
        C *d = data();
        if(d)
        {
            d[0] = C(0);
        }
    }

    void shrink_to_fit()
    {
        if( ! is_long()) return;

        auto sz = _get_long_sz();
        if(sz < arr_size)
        {
            _long saved = m_long;
            traits_type::copy(m_short.arr, saved.str, sz);
            m_short.arr[sz] = C(0);
            _set_short_sz(sz);
            m_alloc.deallocate(saved.str, saved.cap);
        }
        else if(m_long.cap > sz + 1)
        {
            _resizebuf(sz + 1);
        }
    }

private:

    void _free()
    {
        if(is_long())
        {
            m_alloc.deallocate(m_long.str, m_long.cap);
            m_long.flag_n_sz = 0; // also sets the flag to short mode
            m_long.cap = 0; 
            m_long.str = nullptr;
        }
        else
        {
            m_short.flag_n_sz = 0;
        }
    }
    void _set_size(SizeType sz)
    {
        if(is_long())
        {
            _set_long_sz(sz);
        }
        else
        {
            _set_short_sz(sz);
        }
    }
    /** resizes without writing anything into the buffer. Does not grow the buffer. */
    void _resize(SizeType sz)
    {
        reserve(sz + 1);
        _set_size(sz);
    }
    void _reserve(SizeType cap)
    {
        if(cap > capacity())
        {
            _resizebuf(cap);
        }
    }
    /** resizes the buffer to the given capacity */
    void _resizebuf(SizeType cap)
    {
        if(cap > arr_size) // create a long version
        {
            C *tmp = m_alloc.allocate(cap);
            if(is_long())
            {
                auto sz = _get_long_sz();
                if(sz)
                {
                    traits_type::copy(tmp, m_long.str, sz);
                    tmp[sz] = C(0);
                    m_alloc.deallocate(m_long.str, m_long.cap);
                }
            }
            else
            {
                auto sz = _get_short_sz();
                traits_type::copy(tmp, m_short.arr, sz);
                tmp[sz] = C(0);
            }
            m_long.str = tmp;
            m_long.cap = cap;
            m_long.flag_n_sz |= 1;
        }
        else // create a short version
        {
            if(is_long())
            {
                auto sz = _get_long_sz();
                if(sz > 0)
                {
                    C4_ASSERT(sz < (arr_size - 1));
                    _long saved = m_long; // save the current m_long as it will be overwriten in the memcpy
                    traits_type::copy(m_short.arr, m_long.str, sz);
                    m_alloc.deallocate(saved.str, saved.cap);
                    _set_short_sz(sz);
                }
            }
            else
            {
                // nothing to see here.
            }
        }
    }

};

template< class C, class SizeType, class Alloc >
constexpr const SizeType basic_string< C, SizeType, Alloc >::npos;

_C4_IMPLEMENT_TPL_STRIMPL_HASH(c4::basic_string< C C4_COMMA SizeType C4_COMMA Alloc >,
    typename C, typename SizeType, class Alloc)

//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------

/// @cond dev

#ifdef C4_STR_DISABLE_EXPR_TPL


// It's hard enough as it is. Increase readability by temporarily defining
// a short form for C4_COMMA.
#ifdef __
#   error "__ is already defined - this won't go well"
#endif
#ifdef WC
#   error "WC is already defined - this won't go well"
#endif
// don't worry - they're undefined below
#define __ C4_COMMA
#define WC typename std::remove_const<C>::type // non-const (writeable) version of char type C



/// binary operators: basic_substring + basic_substring. return a basic_string result wrapper!!!
_C4STR_DEFINE_BINOPS1TY_TPL(
    template< typename C __ typename SizeType >,
    basic_substring< C __ SizeType >,
    basic_string< WC __ SizeType __ typename basic_string<WC>::allocator_type >)
/// binary operators: basic_substringrs + basic_substringrs. return a basic_string result wrapper!!!
_C4STR_DEFINE_BINOPS1TY_TPL(
    template< typename C __ typename SizeType >,
    basic_substringrs< C __ SizeType >,
    basic_string< WC __ SizeType __ typename basic_string<WC>::allocator_type >)
/// binary operators: basic_string + basic_string
_C4STR_DEFINE_BINOPS1TY_TPL(
    template< typename C __ typename SizeType __ class Alloc >,
    basic_string< C __ SizeType __ Alloc >,
    basic_string< C __ SizeType __ Alloc >)
/// binary operators: basic_text + basic_text
_C4STR_DEFINE_BINOPS1TY_TPL(
    template< typename C __ typename SizeType __ class Alloc >,
    basic_text< C __ SizeType __ Alloc >,
    basic_text< C __ SizeType __ Alloc >)



/// binary operators: basic_substring + const char* - will use default allocator
_C4STR_DEFINE_BINOPS2TY_TPL(
    template< typename C __ typename SizeType >,
    basic_substring< C __ SizeType >,
    const C*,
    basic_string< C __ SizeType __ typename basic_string< C >::allocator_type >)
_C4STR_DEFINE_BINOPS2TY_TPL(
    template< typename C __ typename SizeType >,
    basic_substring< const C __ SizeType >,
    const C*,
    basic_string< C __ SizeType __ typename basic_string< C >::allocator_type >)
/// binary operators: basic_substringrs + const char* - will use default allocator
_C4STR_DEFINE_BINOPS2TY_TPL(
    template< typename C __ typename SizeType >,
    basic_substringrs< C __ SizeType >,
    const C*,
    basic_string< C __ SizeType __ typename basic_string< C >::allocator_type >)
_C4STR_DEFINE_BINOPS2TY_TPL(
    template< typename C __ typename SizeType >,
    basic_substringrs< const C __ SizeType >,
    const C*,
    basic_string< C __ SizeType __ typename basic_string< C >::allocator_type >)
/// binary operators: basic_string + const char*
_C4STR_DEFINE_BINOPS2TY_TPL(
    template< typename C __ typename SizeType __ class Alloc >,
    basic_string< C __ SizeType __ Alloc >,
    const C*,
    basic_string< C __ SizeType __ Alloc >)
/// binary operators: basic_text + const char*
_C4STR_DEFINE_BINOPS2TY_TPL(
    template< typename C __ typename SizeType __ class Alloc >,
    basic_text< C __ SizeType __ Alloc >,
    const C*,
    basic_text< C __ SizeType __ Alloc >)


#ifdef C4NOTTHIS
/// binary operators: basic_substring + _strCatRes< basic_string >
_C4STR_DEFINE_BINOPS2TY_TPL(
    template< typename C __ typename SizeType __ class Alloc >,
    basic_substring< C __ SizeType >,
    _strCatRes< basic_string< C __ SizeType __ Alloc > >,
    basic_string< C __ SizeType __ Alloc >)
/// binary operators: basic_substringrs + _strCatRes< basic_string >
_C4STR_DEFINE_BINOPS2TY_TPL(
    template< typename C __ typename SizeType __ class Alloc >,
    basic_substringrs< C __ SizeType >,
    _strCatRes< basic_string< C __ SizeType __ Alloc > >,
    basic_string< C __ SizeType __ Alloc >)
/// binary operators: basic_string + _strCatRes< basic_string >
_C4STR_DEFINE_BINOPS2TY_TPL(
    template< typename C __ typename SizeType __ class Alloc >,
    basic_string< C __ SizeType __ Alloc >,
    _strCatRes< basic_string< C __ SizeType __ Alloc > >,
    basic_string< C __ SizeType __ Alloc >)
/// binary operators: basic_text + _strCatRes< basic_string >
_C4STR_DEFINE_BINOPS2TY_TPL(
    template< typename C __ typename SizeType __ class Alloc >,
    basic_text< C __ SizeType __ Alloc >,
    _strCatRes< basic_string< C __ SizeType __ Alloc > >,
    basic_string< C __ SizeType __ Alloc >)



/// binary operators: basic_substring + _strCatRes< basic_text >
_C4STR_DEFINE_BINOPS2TY_TPL(
    template< typename C __ typename SizeType __ class Alloc >,
    basic_substring< C __ SizeType >,
    _strCatRes< basic_text< C __ SizeType __ Alloc > >,
    basic_text< C __ SizeType __ Alloc >)
/// binary operators: basic_substringrs + _strCatRes< basic_string >
_C4STR_DEFINE_BINOPS2TY_TPL(
    template< typename C __ typename SizeType __ class Alloc >,
    basic_substringrs< C __ SizeType >,
    _strCatRes< basic_text< C __ SizeType __ Alloc > >,
    basic_text< C __ SizeType __ Alloc >)
/// binary operators: basic_string + _strCatRes< basic_string >
_C4STR_DEFINE_BINOPS2TY_TPL(
    template< typename C __ typename SizeType __ class Alloc >,
    basic_string< C __ SizeType __ Alloc >,
    _strCatRes< basic_text< C __ SizeType __ Alloc > >,
    basic_text< C __ SizeType __ Alloc >)
/// binary operators: basic_text + _strCatRes< basic_string >
_C4STR_DEFINE_BINOPS2TY_TPL(
    template< typename C __ typename SizeType __ class Alloc >,
    basic_text< C __ SizeType __ Alloc >,
    _strCatRes< basic_text< C __ SizeType __ Alloc > >,
    basic_text< C __ SizeType __ Alloc >)
#endif


/// binary operators: basic_substring + basic_string
_C4STR_DEFINE_BINOPS2TY_TPL(
    template< typename C __ typename SizeType __ class Alloc >,
    basic_string< C __ SizeType __ Alloc >,
    basic_substring< C __ SizeType >,
    basic_string< C __ SizeType __ Alloc >)
/// binary operators: basic_substringrs + basic_string
_C4STR_DEFINE_BINOPS2TY_TPL(
    template< typename C __ typename SizeType __ class Alloc >,
    basic_string< C __ SizeType __ Alloc >,
    basic_substringrs< C __ SizeType >,
    basic_string< C __ SizeType __ Alloc >)
/// binary operators: basic_text + basic_string
_C4STR_DEFINE_BINOPS2TY_TPL(
    template< typename C __ typename SizeType __ class Alloc >,
    basic_string< C __ SizeType __ Alloc >,
    basic_text< C __ SizeType __ Alloc >,
    basic_string< C __ SizeType __ Alloc >)



/// binary operators: basic_substring + basic_text
_C4STR_DEFINE_BINOPS2TY_TPL(
    template< typename C __ typename SizeType __ class Alloc >,
    basic_text< C __ SizeType __ Alloc >,
    basic_substring< C __ SizeType >,
    basic_text< C __ SizeType __ Alloc >)
/// binary operators: basic_substringrs + basic_text
_C4STR_DEFINE_BINOPS2TY_TPL(
    template< typename C __ typename SizeType __ class Alloc >,
    basic_text< C __ SizeType __ Alloc >,
    basic_substringrs< C __ SizeType >,
    basic_text< C __ SizeType __ Alloc >)
/// binary operators: basic_string + basic_text
_C4STR_DEFINE_BINOPS2TY_TPL(
    template< typename C __ typename SizeType __ class Alloc >,
    basic_text< C __ SizeType __ Alloc >,
    basic_string< C __ SizeType __ Alloc >,
    basic_text< C __ SizeType __ Alloc >)

#undef __
#undef WC

#endif // C4_STR_DISABLE_EXPR_TPL

/// @endcond

//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------

void s2ws(char    const *mbstr, size_t len, wchar_t *output);
void ws2s(wchar_t const *mbstr, size_t len, char    *output);


template< class I, class WI >
void s2ws(span<char const, I > const& mbstr, span< wchar_t, WI > *output)
{
    C4_ASSERT(output->size() == szconv< I >(mbstr.size()));
    s2ws(mbstr.data(), mbstr.size(), output->data());
}
template< class I, class WI >
void ws2s(span<wchar_t const, WI > const& wstr, span< char, I > *output)
{
    C4_ASSERT(output->size() == szconv< WI >(wstr.size()));
    ws2s(wstr.data(), wstr.size(), output->data());
}


template< class I, class WI >
void s2ws(spanrs<char const, I > const& mbstr, spanrs< wchar_t, WI > *output)
{
    C4_ASSERT(output->size() == szconv< I >(mbstr.size()));
    s2ws(mbstr.data(), mbstr.size(), output->data());
}
template< class I, class WI >
void ws2s(spanrs<wchar_t const, WI > const& wstr, spanrs< char, I > *output)
{
    C4_ASSERT(output->size() == szconv< WI >(wstr.size()));
    ws2s(wstr.data(), wstr.size(), output->data());
}


template< class I, class Impl, class SubStrType, class WI, class WImpl, class WSubStrType >
void s2ws
(
    _str_crtp<char   ,  I,  Impl,  SubStrType > const& mbstr,
    _str_crtp<wchar_t, WI, WImpl, WSubStrType >      * output
)
{
    static_cast< WImpl* >(output)->resize(szconv< WI >(mbstr.size()));
    s2ws(mbstr.data(), mbstr.size(), output->data());
}
template< class I, class Impl, class SubStrType, class WI, class WImpl, class WSubStrType >
void ws2s
(
    _str_crtp<wchar_t, WI, WImpl, WSubStrType > const& wstr,
    _str_crtp<char   ,  I,  Impl,  SubStrType >      * output
)
{
    static_cast< Impl* >(output)->resize(wstr.size());
    ws2s(wstr.data(), wstr.size(), output->data());
}

template< class I, class Impl, class SubStrType >
basic_string< wchar_t, I > s2ws(_str_crtp<char, I, Impl, SubStrType > const& mbstr)
{
    basic_string< wchar_t, I > output;
    output.resize(mbstr.size());
    s2ws(mbstr.data(), mbstr.size(), output.data());
    return output;
}
template< class I, class Impl, class SubStrType >
basic_string< char, I > ws2s(_str_crtp<wchar_t, I, Impl, SubStrType > const& wstr)
{
    basic_string< char, I > output;
    output.resize(wstr.size());
    ws2s(wstr.data(), wstr.size(), output.data());
    return output;
}

C4_END_NAMESPACE(c4)

#endif // _C4_STRING_HPP_
