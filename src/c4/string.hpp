#ifndef _C4_STRING_HPP_
#define _C4_STRING_HPP_

/** \file string.hpp
 * Provides string classes for owned (c4::string) and non-owned (c4::substring
 * and c4::substringrs) writeable strings. These classes are mostly stl
 * compatible, with some additions, like begins_with()/ends_with(),
 * trim()/trimws(), or notably split() and related methods such as
 * popr()/popl(), gpopr()/gpopl(), pushr()/pushl(), basename()/dirname()
 * and the / operator for path concatenation. Unlike STL's string_view,
 * the substrings are writeable.
 *
 * The classes are implementated with a CRTP base class (c4::string_impl)
 * which provides the actual string methods such
 * as find(), leaving only the resource management and sizing information
 * for the derived classes. Expression templates are used for string
 * concatenation either via + or /, which has the advantage of allowing
 * concatenation of substrings (and specifically, much less memory allocations).
 * Another significant source of allocation savings is in the preeminent use
 * of substring return types.
 *
 *
 *------------------------------------
 * Disabling expression templates (not advised):
 *
 * The default is to have string expression templates enabled.
 * Define C4_STR_DISABLE_EXPR_TPL to disable string expression templates.
 * This will cause switching to greedy evaluation of concatenation
 * operations. It will also disable some of the binary overloads.
 * DO NOT ADD VANILLA BINARY OPERATORS FOR NON-OWNED STRINGS
 * such as substring and substringrs. IT IS DANGEROUS. When expression
 * templates are not used, we must evaluate each operation immediately.
 * So summing two substrings must result in an allocated string. So eg
 * this would work:
 *
 * substring ss1("foo"), ss2("bar"); // ss1 and ss2 are pointing at static memory
 * string r;
 * r = ss1 + ss2; // OK, r is "foobar" and owns the memory
 *
 * BUT if we assign the sum to a substring instead, we would get a stale
 * pointer:
 *
 * substring ss3;
 * ss3 = ss1 + ss2; // BAD, ss3 points at deallocated memory
 *
 * The result of ss1+ss2 is an owned string. Because
 * ss3 is not an owned string, it is set to point at the contents of the
 * owned string (ss1+ss2), which gets destroyed after the assignment.
 * So after the statement ss3 is pointed at deallocated memory.
 *
 * For this reason, it is better to disable binary operators (+,/)
 * between non-owned strings. But these binary operators are fine
 * when any owned string is involved. To avoid the difficulties
 * inherent in greedily evaluating substring operations, the use of
 * expression templates is advised.
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
 
//-----------------------------------------------------------------------------
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

/** specialize std::hash for the given string_impl-derived type. Search
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
C4_ALWAYS_INLINE classname& operator+= (const char* str)                \
{                                                                       \
    this->append(str);                                                  \
    return *this;                                                       \
}                                                                       \
C4_ALWAYS_INLINE classname& operator/= (const char* str)                \
{                                                                       \
    this->pushr(str, '/');                                              \
    return *this;                                                       \
}                                                                       \
template< size_t N >                                                    \
C4_ALWAYS_INLINE classname& operator+= (const char (&str)[N])           \
{                                                                       \
    this->append(str, N-1);                                             \
    return *this;                                                       \
}                                                                       \
template< size_t N >                                                    \
C4_ALWAYS_INLINE classname& operator/= (const char (&str)[N])           \
{                                                                       \
    this->pushr(str, N-1, '/');                                         \
    return *this;                                                       \
}                                                                       \
template< typename OSize, class OStr, class OSub >                      \
C4_ALWAYS_INLINE classname& operator+= (string_impl< charname, OSize, OStr, OSub > const& that) \
{                                                                       \
    this->append(that);                                                 \
    return *this;                                                       \
}                                                                       \
template< typename OSize, class OStr, class OSub >                      \
C4_ALWAYS_INLINE classname& operator/= (string_impl< charname, OSize, OStr, OSub > const& that) \
{                                                                       \
    this->pushr(that, '/');                                             \
    return *this;                                                       \
}

//-----------------------------------------------------------------------------
/** assignment operations without expression templates */

#if defined(C4_STR_DISABLE_EXPR_TPL)

#define _C4STR_ASSIGNOPS(charname, classname)                           \
                                                                        \
/** allow other string_impl derived classes access to our members.      \
 * @warning the ::c4:: qualifier is needed as a workaround to           \
 * gcc bug https://gcc.gnu.org/bugzilla/show_bug.cgi?id=52625 */        \
template< typename sifC, typename sifSize, class sifStr, class sifSub > \
friend class ::c4::string_impl;                                         \
                                                                        \
_C4STR_ASSIGNOPS_ANY(charname, classname)


#define _C4STR_DEFINE_BINOPS1TY(ty, tyr) \
    _C4STR_DEFINE_BINOPS1TY_TPL( , ty, tyr)

#define _C4STR_DEFINE_BINOPS1TY_TPL(template_spec, ty, tyr) \
                                                            \
template_spec                                               \
C4_ALWAYS_INLINE tyr operator+                              \
(                                                           \
    ty lhs,                                                 \
    ty rhs                                                  \
)                                                           \
{                                                           \
    tyr result;                                             \
    result.reserve(strsz(lhs) + strsz(rhs));                \
    result.append(lhs);                                     \
    result.append(rhs);                                     \
    return result;                                          \
}                                                           \
template_spec                                               \
C4_ALWAYS_INLINE tyr operator/                              \
(                                                           \
    ty lhs,                                                 \
    ty rhs                                                  \
)                                                           \
{                                                           \
    tyr result;                                             \
    result.reserve(strsz(lhs) + 1 + strsz(rhs));            \
    result.append(lhs);                                     \
    result.append_dir(rhs);                                 \
    return result;                                          \
}


#define _C4STR_DEFINE_BINOPS2TY(ty1, ty2, tyr) \
_C4STR_DEFINE_BINOPS2TY_TPL( , ty1, ty2, tyr)

#define _C4STR_DEFINE_BINOPS2TY_TPL(template_spec, ty1, ty2, tyr)   \
                                                                    \
template_spec                                                       \
C4_ALWAYS_INLINE tyr operator+                                      \
(                                                                   \
    ty1 lhs,                                                        \
    ty2 rhs                                                         \
)                                                                   \
{                                                                   \
    tyr result;                                                     \
    result.reserve(strsz(lhs) + strsz(rhs));                        \
    result.append(lhs);                                             \
    result.append(rhs);                                             \
    return result;                                                  \
}                                                                   \
template_spec                                                       \
C4_ALWAYS_INLINE tyr operator+                                      \
(                                                                   \
    ty2 lhs,                                                        \
    ty1 rhs                                                         \
)                                                                   \
{                                                                   \
    tyr result;                                                     \
    result.reserve(strsz(lhs) + strsz(rhs));                        \
    result.append(lhs);                                             \
    result.append(rhs);                                             \
    return result;                                                  \
}                                                                   \
                                                                    \
template_spec                                                       \
C4_ALWAYS_INLINE tyr operator/                                      \
(                                                                   \
    ty1 lhs,                                                        \
    ty2 rhs                                                         \
)                                                                   \
{                                                                   \
    tyr result;                                                     \
    result.reserve(strsz(lhs) + 1 + strsz(rhs));                    \
    result.append(lhs);                                             \
    result.append_dir(rhs);                                         \
    return result;                                                  \
}                                                                   \
template_spec                                                       \
C4_ALWAYS_INLINE tyr operator/                                      \
(                                                                   \
    ty2 lhs,                                                        \
    ty1 rhs                                                         \
)                                                                   \
{                                                                   \
    tyr result;                                                     \
    result.reserve(strsz(lhs) + 1 + strsz(rhs));                    \
    result.append(lhs);                                             \
    result.append_dir(rhs);                                         \
    return result;                                                  \
}

#endif // defined(C4_STR_DISABLE_EXPR_TPL)

//-----------------------------------------------------------------------------
/** assignment operations with expression templates */

#ifndef C4_STR_DISABLE_EXPR_TPL

#define _C4STR_ASSIGNOPS(charname, classname)                           \
                                                                        \
_C4STR_ASSIGNOPS_ANY(charname, classname)                               \
                                                                        \
/** allow other string_impl derived classes access to our members.      \
 * @warning the ::c4:: qualifier is needed as a workaround to           \
 * gcc bug https://gcc.gnu.org/bugzilla/show_bug.cgi?id=52625 */        \
template< typename sifC, typename sifSize, class sifStr, class sifSub > \
friend class ::c4::string_impl;                                         \
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

/** expression template node wrapping a string_impl or std::string, std::array< char >, std::vector< char >, etc */
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
// expression template node wrapping a char array
template< typename C >
struct _strWrapCharPtr : public _strExpr< C, _strWrapCharPtr< C > >
{
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

// expression template for summing two string expressions
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

// sum two expressions
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
// dircat
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
// sum two string_impls with same type
template< typename C, typename Size, class Str, class Sub >
_strPlus< C, _strWrapImpl< C, Str >, _strWrapImpl< C, Str > >
operator+
(
    string_impl< C, Size, Str, Sub > const& l_,
    string_impl< C, Size, Str, Sub > const& r_
)
{
    typedef _strWrapImpl< C, Str > etype;
    Str const& l = static_cast< Str const& >(l_);
    Str const& r = static_cast< Str const& >(r_);
    _strPlus< C, etype, etype > expr(l, r);
    return expr;
}
// dircat
template< typename C, typename Size, class Str, class Sub >
_strSlash< C, _strWrapImpl< C, Str >, _strWrapImpl< C, Str > >
operator/
(
    string_impl< C, Size, Str, Sub > const& l_,
    string_impl< C, Size, Str, Sub > const& r_
)
{
    typedef _strWrapImpl< C, Str > etype;
    Str const& l = static_cast< Str const& >(l_);
    Str const& r = static_cast< Str const& >(r_);
    _strSlash< C, etype, etype > expr(l, r);
    return expr;
}

//--------------------------
// sum two string_impls of different type
template< typename C, typename SizeL, class StrL, class SubL, typename SizeR, class StrR, class SubR >
_strPlus< C, _strWrapImpl< C, StrL >, _strWrapImpl< C, StrR > >
operator+
(
    string_impl< C, SizeL, StrL, SubL > const& l_,
    string_impl< C, SizeR, StrR, SubR > const& r_
)
{
    typedef _strWrapImpl< C, StrL > ltype;
    typedef _strWrapImpl< C, StrR > rtype;
    StrL const& l = static_cast< StrL const& >(l_);
    StrR const& r = static_cast< StrR const& >(r_);
    _strPlus< C, ltype, rtype > expr(l, r);
    return expr;
}
// dircat
template< typename C, typename SizeL, class StrL, class SubL, typename SizeR, class StrR, class SubR >
_strSlash< C, _strWrapImpl< C, StrL >, _strWrapImpl< C, StrR > >
operator/
(
    string_impl< C, SizeL, StrL, SubL > const& l_,
    string_impl< C, SizeR, StrR, SubR > const& r_
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
// sum string_impl and expr
template< typename C, typename Size, class Str, class Sub, class ExprR >
_strPlus< C, _strWrapImpl< C, Str >, _strExpr< C, ExprR > >
operator+
(
    string_impl< C, Size, Str, Sub > const& l_,
    _strExpr< C, ExprR > const& r
)
{
    typedef _strWrapImpl< C, Str > ltype;
    typedef _strExpr< C, ExprR >   rtype;
    Str const& l = static_cast< Str const& >(l_);
    _strPlus< C, ltype, rtype > expr(l, r);
    return expr;
}
// dircat
template< typename C, typename Size, class Str, class Sub, class ExprR >
_strSlash< C, _strWrapImpl< C, Str >, _strExpr< C, ExprR > >
operator/
(
    string_impl< C, Size, Str, Sub > const& l_,
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
// sum expr and string_impl
template< class ExprL, typename C, typename Size, class Str, class Sub >
_strPlus< C, _strExpr< C, ExprL >, _strWrapImpl< C, Str > >
operator+
(
    _strExpr< C, ExprL > const& l,
    string_impl< C, Size, Str, Sub > const& r_
)
{
    typedef _strExpr< C, ExprL > ltype;
    typedef _strWrapImpl< C, Str > rtype;
    Str const& r = static_cast< Str const& >(r_);
    _strPlus< C, ltype, rtype > expr(l, r);
    return expr;
}
// dircat
template< class ExprL, typename C, typename Size, class Str, class Sub >
_strSlash< C, _strExpr< C, ExprL >, _strWrapImpl< C, Str > >
operator/
(
    _strExpr< C, ExprL > const& l,
    string_impl< C, Size, Str, Sub > const& r_
)
{
    typedef _strExpr< C, ExprL > ltype;
    typedef _strWrapImpl< C, Str > rtype;
    Str const& r = static_cast< Str const& >(r_);
    _strSlash< C, ltype, rtype > expr(l, r);
    return expr;
}

//--------------------------
// sum string_impl and const char*
template< typename C, typename Size, class Str, class Sub >
_strPlus< C, _strWrapImpl< C, Str >, _strWrapCharPtr< C > >
operator+
(
    string_impl< C, Size, Str, Sub > const& l_,
    const char* r
)
{
    typedef _strWrapImpl< C, Str > ltype;
    typedef _strWrapCharPtr< C > rtype;
    Str const& l = static_cast< Str const& >(l_);
    _strPlus< C, ltype, rtype > expr(l, rtype(r));
    return expr;
}
// dircat
template< typename C, typename Size, class Str, class Sub >
_strSlash< C, _strWrapImpl< C, Str >, _strWrapCharPtr< C > >
operator/
(
    string_impl< C, Size, Str, Sub > const& l_,
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
// sum const char* and string_impl
template< typename C, typename Size, class Str, class Sub >
_strPlus< C, _strWrapCharPtr< C >, _strWrapImpl< C, Str > >
operator+
(
    const char* l,
    string_impl< C, Size, Str, Sub > const& r_
)
{
    typedef _strWrapCharPtr< C > ltype;
    typedef _strWrapImpl< C, Str > rtype;
    Str const& r = static_cast< Str const& >(r_);
    _strPlus< C, ltype, rtype > expr(ltype(l), r);
    return expr;
}
// dircat
template< typename C, typename Size, class Str, class Sub >
_strSlash< C, _strWrapCharPtr< C >, _strWrapImpl< C, Str > >
operator/
(
    const char* l,
    string_impl< C, Size, Str, Sub > const& r_
)
{
    typedef _strWrapCharPtr< C > ltype;
    typedef _strWrapImpl< C, Str > rtype;
    Str const& r = static_cast< Str const& >(r_);
    _strSlash< C, ltype, rtype > expr(ltype(l), r);
    return expr;
}

//--------------------------
// sum expr and const char*
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
// dircat
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
// sum const char* and expr
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
// dircat
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

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/** a CRTP base implementation for string member methods.
 * Derived classes must have the following members:
 *   -m_str
 *   -m_size
 *   -npos.
 * Derived classes must have the following methods:
 *   -resize()
 *   -reserve()
 */
template< typename C, typename SizeType, class StrType, class SubStrType >
class string_impl
{
#define _c4this     static_cast< StrType      * >(this)
#define _c4thisSZ   static_cast< StrType      * >(this)->size()
#define _c4thisSTR  static_cast< StrType      * >(this)->data()
#define _c4thatSZ   static_cast< StrType      * >(&that)->size()
#define _c4thatSTR  static_cast< StrType      * >(&that)->data()
#define _c4cthisSZ  static_cast< StrType const* >(this)->size()
#define _c4cthisSTR static_cast< StrType const* >(this)->data()
#define _c4cthatSZ  static_cast< StrType const* >(&that)->size()
#define _c4cthatSTR static_cast< StrType const* >(&that)->data()
#define _c4CCAST(ty) (ty)const_cast< string_impl const* >(this)
#define _c4cSZ(ptr)  static_cast< StrType const* >(ptr)->size()
#define _c4cSTR(ptr) static_cast< StrType const* >(ptr)->data()

public:

    using value_type   = C;
    using char_type    = C;
    using traits_type  = c4::char_traits< C >;
    using size_type    = SizeType;
    using ssize_type   = typename std::make_signed< size_type >::type;
    using cont_type    = StrType;
    using subcont_type = SubStrType;

protected:

    // protect the dtor to prevent creation of bare string_impl
    string_impl(){}
    // protect the dtor to prevent destruction through a base pointer
    ~string_impl(){}

public:

    // automatically convert to span
    C4_ALWAYS_INLINE operator  span< C, SizeType > ()       { return span < C, SizeType >(_c4thisSTR, _c4thisSZ); }
    C4_ALWAYS_INLINE operator cspan< C, SizeType > () const { return cspan< C, SizeType >(_c4cthisSTR, _c4cthisSZ); }

    template< typename OtherSizeType >
    C4_ALWAYS_INLINE operator  span< C, OtherSizeType > ()       { return span < C, OtherSizeType >(_c4thisSTR, szconv< OtherSizeType >(_c4thisSZ)); }
    template< typename OtherSizeType >
    C4_ALWAYS_INLINE operator cspan< C, OtherSizeType > () const { return cspan< C, OtherSizeType >(_c4cthisSTR, szconv< OtherSizeType >(_c4cthisSZ)); }

public:

    C4_ALWAYS_INLINE C const*   data()  const noexcept { return _c4cthisSTR; }
    C4_ALWAYS_INLINE C      *   data()        noexcept { return _c4thisSTR; }

    C4_ALWAYS_INLINE size_type  size()  const noexcept { return _c4cthisSZ; }

    C4_ALWAYS_INLINE ssize_type ssize() const noexcept { return szconv< ssize_type >(_c4cthisSZ); }
    C4_ALWAYS_INLINE bool       empty() const noexcept { return _c4cthisSZ == 0; }

    C4_ALWAYS_INLINE C  front() const C4_NOEXCEPT_X { C4_XASSERT(!empty()); return *_c4cthisSTR; }
    C4_ALWAYS_INLINE C& front()       C4_NOEXCEPT_X { C4_XASSERT(!empty()); return *_c4thisSTR; }

    C4_ALWAYS_INLINE C  back() const C4_NOEXCEPT_X { C4_XASSERT(!empty()); return *(_c4cthisSTR + _c4thisSZ - 1); }
    C4_ALWAYS_INLINE C& back()       C4_NOEXCEPT_X { C4_XASSERT(!empty()); return *(_c4thisSTR + _c4thisSZ - 1); }

public:

    typedef C      *       iterator;
    typedef C const* const_iterator;

    C4_ALWAYS_INLINE iterator begin() noexcept { return _c4thisSTR; }
    C4_ALWAYS_INLINE iterator end()   noexcept { return _c4thisSTR + _c4thisSZ; }

    C4_ALWAYS_INLINE const_iterator begin() const noexcept { return _c4cthisSTR; }
    C4_ALWAYS_INLINE const_iterator end()   const noexcept { return _c4cthisSTR + _c4cthisSZ; }

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
    size_type count(C val, size_type first_char, size_type num_chars = npos) const C4_NOEXCEPT_X
    {
        size_type num = 0;
        C4_XASSERT(first_char >= 0 && first_char < sz);
        size_type last_char == npos ? _c4cthisSZ - first_char : first_char + num_chars;
        C4_XASSERT(last_char  >= 0 && last_char  < sz);
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
        for(size_type i = start_pos; i < sz; ++i)
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

    /** COMPlement Left: return the complement to the left of the beginning of the given substring.
     * If ss does not begin inside this, returns an empty substring. */
    SubStrType compl(SubStrType const& ss) const noexcept
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

    /** COMPlement Right: return the complement to the right of the end of the given substring.
     * If ss does not end inside this, returns an empty substring. */
    SubStrType compr(SubStrType const& ss) const noexcept
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
    size_type C4_ALWAYS_INLINE find(string_impl const& that, size_type pos = 0) const { return find(_c4cthatSTR, _c4cthatSZ, pos); }
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
    size_type C4_ALWAYS_INLINE find_first_of(string_impl const& that, size_type pos = 0) const { return find_first_of(_c4cthatSTR, _c4cthatSZ, pos); }
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
    size_type C4_ALWAYS_INLINE find_first_not_of(string_impl const& that, size_type pos = 0) const { return find_first_not_of(_c4cthatSTR, _c4cthatSZ, pos); }
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
    size_type C4_ALWAYS_INLINE find_last_of(string_impl const& that, size_type pos = StrType::npos) const { return find_last_of(_c4cthatSTR, _c4cthatSZ, pos); }
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
    size_type C4_ALWAYS_INLINE find_last_not_of(string_impl const& that, size_type pos = StrType::npos) const { return find_last_not_of(_c4cthatSTR, _c4cthatSZ, pos); }
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
    bool C4_ALWAYS_INLINE begins_with(string_impl const& that) const { return begins_with(_c4cthatSTR, _c4cthatSZ); }
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
    bool C4_ALWAYS_INLINE ends_with(string_impl const& that) const { return ends_with(_c4cthatSTR, _c4cthatSZ); }
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
        return const_cast<string_impl*>(this)->next_split(sep, start_pos, out); // FIXME
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
    SubStrType C4_ALWAYS_INLINE triml(string_impl const& that) const { return triml(_c4cthatSTR, _c4cthatSZ); }
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
    SubStrType C4_ALWAYS_INLINE trimr(string_impl const& that) const { return trimr(_c4cthatSTR, _c4cthatSZ); }
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
    SubStrType C4_ALWAYS_INLINE trim(string_impl const& that) const { return trim(_c4cthatSTR, _c4cthatSZ); }
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
                auto rem = compr(ss);
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
        ss = compr(ss);
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
        ss = compl(ss);
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

    int compare(string_impl const& that) const
    {
        size_type mx = _c4cthisSZ > _c4cthatSZ ? _c4cthisSZ : _c4cthatSZ;
        return traits_type::compare(_c4cthisSTR, _c4cthatSTR, mx);
    }
    template< typename OSize, class OStr, class OSub >
    int compare(string_impl< C, OSize, OStr, OSub > const& that) const
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

    bool operator== (string_impl const& that) const
    {
        if(_c4cthatSZ != _c4cthisSZ) return false;
        if(_c4cthisSTR == _c4cthatSTR) return true;
        return traits_type::compare(_c4cthisSTR, _c4cthatSTR, _c4cthisSZ) == 0;
    }
    C4_ALWAYS_INLINE bool operator!= (string_impl const& that) const { return !this->operator== (that); }
    C4_ALWAYS_INLINE bool operator>= (string_impl const& that) const { return compare(that) >= 0; }
    C4_ALWAYS_INLINE bool operator<= (string_impl const& that) const { return compare(that) <= 0; }
    C4_ALWAYS_INLINE bool operator>  (string_impl const& that) const { return compare(that) >  0; }
    C4_ALWAYS_INLINE bool operator<  (string_impl const& that) const { return compare(that) <  0; }

    template< typename OSize, class OStr, class OSub > C4_ALWAYS_INLINE bool operator== (string_impl< C, OSize, OStr, OSub > const& that) const { return compare(that) == 0; }
    template< typename OSize, class OStr, class OSub > C4_ALWAYS_INLINE bool operator!= (string_impl< C, OSize, OStr, OSub > const& that) const { return compare(that) != 0; }
    template< typename OSize, class OStr, class OSub > C4_ALWAYS_INLINE bool operator>= (string_impl< C, OSize, OStr, OSub > const& that) const { return compare(that) >= 0; }
    template< typename OSize, class OStr, class OSub > C4_ALWAYS_INLINE bool operator<= (string_impl< C, OSize, OStr, OSub > const& that) const { return compare(that) <= 0; }
    template< typename OSize, class OStr, class OSub > C4_ALWAYS_INLINE bool operator>  (string_impl< C, OSize, OStr, OSub > const& that) const { return compare(that) >  0; }
    template< typename OSize, class OStr, class OSub > C4_ALWAYS_INLINE bool operator<  (string_impl< C, OSize, OStr, OSub > const& that) const { return compare(that) <  0; }

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

    C4_ALWAYS_INLINE friend bool operator== (const C *s, string_impl const& n) { return (n == s); }
    C4_ALWAYS_INLINE friend bool operator!= (const C *s, string_impl const& n) { return (n != s); }
    C4_ALWAYS_INLINE friend bool operator>= (const C *s, string_impl const& n) { return (n <= s); }
    C4_ALWAYS_INLINE friend bool operator<= (const C *s, string_impl const& n) { return (n >= s); }
    C4_ALWAYS_INLINE friend bool operator>  (const C *s, string_impl const& n) { return (n <  s); }
    C4_ALWAYS_INLINE friend bool operator<  (const C *s, string_impl const& n) { return (n >  s); }

    template< size_t N >
    bool operator== (const C (&a)[N]) const
    {
        if(N-1 != _c4cthisSZ) return false;
        if(_c4cthisSTR == _c4cthatSTR) return true;
        return traits_type::compare(_c4cthisSTR, a, _c4cthisSZ) == 0;
    }
    template< size_t N > C4_ALWAYS_INLINE bool operator!= (const C (&a)[N]) const { return !this->operator== (a); }
    template< size_t N > C4_ALWAYS_INLINE bool operator>= (const C (&a)[N]) const { return compare(a) >= 0; }
    template< size_t N > C4_ALWAYS_INLINE bool operator<= (const C (&a)[N]) const { return compare(a) <= 0; }
    template< size_t N > C4_ALWAYS_INLINE bool operator>  (const C (&a)[N]) const { return compare(a) >  0; }
    template< size_t N > C4_ALWAYS_INLINE bool operator<  (const C (&a)[N]) const { return compare(a) <  0; }

    template< size_t N > C4_ALWAYS_INLINE friend bool operator== (const C (&s)[N], string_impl const& n) { return (n == s); }
    template< size_t N > C4_ALWAYS_INLINE friend bool operator!= (const C (&s)[N], string_impl const& n) { return (n != s); }
    template< size_t N > C4_ALWAYS_INLINE friend bool operator>= (const C (&s)[N], string_impl const& n) { return (n <= s); }
    template< size_t N > C4_ALWAYS_INLINE friend bool operator<= (const C (&s)[N], string_impl const& n) { return (n >= s); }
    template< size_t N > C4_ALWAYS_INLINE friend bool operator>  (const C (&s)[N], string_impl const& n) { return (n <  s); }
    template< size_t N > C4_ALWAYS_INLINE friend bool operator<  (const C (&s)[N], string_impl const& n) { return (n >  s); }

public:

    C4_ALWAYS_INLINE void append(C const* str) { append(str, szconv< size_type >(traits_type::length(str))); }
    template< size_t N >
    C4_ALWAYS_INLINE void append(C const (&str)[N]) { append(&str[0], N-1); }
    C4_ALWAYS_INLINE void append(string_impl const& that) { append(_c4cthatSTR, _c4cthatSZ); }
    template< typename OSize, class OStrType, class OSub >
    C4_ALWAYS_INLINE void append(string_impl< C, OSize, OStrType, OSub > const& that)
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
    C4_ALWAYS_INLINE void prepend(string_impl const& that) { prepend(_c4cthatSTR, _c4cthatSZ); }
    template< typename OSize, class OStrType, class OSub >
    C4_ALWAYS_INLINE void prepend(string_impl< C, OSize, OStrType, OSub > const& that)
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
    C4_ALWAYS_INLINE void pushr(string_impl const& that, C sep=C('/')) { pushr(_c4cthatSTR, _c4cthatSZ, sep); }
    template< typename OSize, class OStrType, class OSub >
    C4_ALWAYS_INLINE void pushr(string_impl< C, OSize, OStrType, OSub > const& that, C sep=C('/'))
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
    C4_ALWAYS_INLINE void pushl(string_impl const& that, C sep=C('/')) { pushl(_c4cthatSTR, _c4cthatSZ, sep); }
    template< typename OSize, class OStrType, class OSub >
    C4_ALWAYS_INLINE void pushl(string_impl< C, OSize, OStrType, OSub > const& that, C sep=C('/'))
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
    C4_ALWAYS_INLINE void erase(string_impl const& that) { erase(_c4cthatSTR, _c4cthatSZ); }
    template< typename OSize, class OStrType, class OSub >
    C4_ALWAYS_INLINE void erase(string_impl< C, OSize, OStrType, OSub > const& that)
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

}; // string_impl
_C4_IMPLEMENT_TPL_STRIMPL_HASH(
    c4::string_impl< C C4_COMMA SizeType C4_COMMA StrType C4_COMMA SubStrType >,
    typename C, typename SizeType, class StrType, class SubStrType)


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// comparison operators

template< typename C, typename Size, class Str, class Sub > C4_ALWAYS_INLINE bool operator== (const C *s, string_impl< C, Size, Str, Sub > const& n) { return (n == s); }
template< typename C, typename Size, class Str, class Sub > C4_ALWAYS_INLINE bool operator!= (const C *s, string_impl< C, Size, Str, Sub > const& n) { return (n != s); }
template< typename C, typename Size, class Str, class Sub > C4_ALWAYS_INLINE bool operator>= (const C *s, string_impl< C, Size, Str, Sub > const& n) { return (n <= s); }
template< typename C, typename Size, class Str, class Sub > C4_ALWAYS_INLINE bool operator<= (const C *s, string_impl< C, Size, Str, Sub > const& n) { return (n >= s); }
template< typename C, typename Size, class Str, class Sub > C4_ALWAYS_INLINE bool operator>  (const C *s, string_impl< C, Size, Str, Sub > const& n) { return (n <  s); }
template< typename C, typename Size, class Str, class Sub > C4_ALWAYS_INLINE bool operator<  (const C *s, string_impl< C, Size, Str, Sub > const& n) { return (n >  s); }

template< size_t N, typename C, typename Size, class Str, class Sub > C4_ALWAYS_INLINE bool operator== (const C (&s)[N], string_impl< C, Size, Str, Sub > const& n) { return (n == s); }
template< size_t N, typename C, typename Size, class Str, class Sub > C4_ALWAYS_INLINE bool operator!= (const C (&s)[N], string_impl< C, Size, Str, Sub > const& n) { return (n != s); }
template< size_t N, typename C, typename Size, class Str, class Sub > C4_ALWAYS_INLINE bool operator>= (const C (&s)[N], string_impl< C, Size, Str, Sub > const& n) { return (n <= s); }
template< size_t N, typename C, typename Size, class Str, class Sub > C4_ALWAYS_INLINE bool operator<= (const C (&s)[N], string_impl< C, Size, Str, Sub > const& n) { return (n >= s); }
template< size_t N, typename C, typename Size, class Str, class Sub > C4_ALWAYS_INLINE bool operator>  (const C (&s)[N], string_impl< C, Size, Str, Sub > const& n) { return (n <  s); }
template< size_t N, typename C, typename Size, class Str, class Sub > C4_ALWAYS_INLINE bool operator<  (const C (&s)[N], string_impl< C, Size, Str, Sub > const& n) { return (n >  s); }


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// stream io

template< class StreamStringType, typename C, typename Size, class Str, class Sub >
C4_ALWAYS_INLINE
c4::sstream< StreamStringType >& operator<< (c4::sstream< StreamStringType >& os, string_impl< C, Size, Str, Sub > const& n)
{
    os.write(n.data(), n.size());
    return is;
}

template< class StreamStringType, typename C, typename Size, class Str, class Sub >
c4::sstream< StreamStringType >& operator>> (c4::sstream< StreamStringType >& is, string_impl< C, Size, Str, Sub > & n)
{
    Str::size_type pos = 0;
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
    ncast.resize(pos);
    is.read(ncast.data(), ncast.size());
    return is;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** a proxy string which does not manage any memory.
 *  -It is in general NOT null terminated.
 *  -Can be resized, but only to smaller sizes.
 *  -Can be written to output streams.
 *  -CANNOT be read into with input streams unless the read value
 *    is guaranteed to be smaller.
 *  @see basic_substringrs if you need to augment the size up to a capacity. */
template< class C, class SizeType >
class basic_substring : public string_impl< C, SizeType, basic_substring< C, SizeType >, basic_substring< C, SizeType > >
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

    using nstring_type = basic_substring< char   , SizeType >; ///< narrow string type
    using wstring_type = basic_substring< wchar_t, SizeType >; ///< wide string type

public:

    C4_ALWAYS_INLINE operator bool () const { return m_size > 0; }

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

    C4_ALWAYS_INLINE void clear() { m_str = nullptr; m_size = 0; }

    C4_ALWAYS_INLINE const C* data () const noexcept { return m_str; }
    C4_ALWAYS_INLINE       C* data ()       noexcept { return m_str; }

    C4_ALWAYS_INLINE SizeType size () const noexcept { return m_size; }

    C4_ALWAYS_INLINE void resize (SizeType sz) { C4_CHECK(sz <= m_size); m_size = sz; }
    C4_ALWAYS_INLINE void reserve(SizeType sz) { C4_CHECK(sz <= m_size); m_size = sz; }

    C4_ALWAYS_INLINE SizeType capacity() const { return m_size; }
    constexpr C4_ALWAYS_INLINE SizeType max_size() const { return std::numeric_limit< SizeType >::max(); }

private:

    C        * m_str;
    SizeType   m_size;

    template< class C2, class S2 >
    friend class basic_substring;

    friend void test_string_member_alignment();

}; // basic_substring

_C4_IMPLEMENT_TPL_STRIMPL_HASH(c4::basic_substring< C C4_COMMA SizeType >, typename C, typename SizeType)

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** substringReSizeable: same as substring, but additionally has a fixed
 * max capacity, so it can be resizeable up to capacity. Therefore, it
 * can be modified with methods that change its size such as append(), prepend(),
 * or read with input streams (again, up to capacity). */
template< class C, class SizeType >
class basic_substringrs : public string_impl< C, SizeType, basic_substringrs< C, SizeType >, basic_substringrs< C, SizeType > >
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

    using nstring_type = basic_substringrs< char   , SizeType >; ///< narrow string type
    using wstring_type = basic_substringrs< wchar_t, SizeType >; ///< wide string type

public:

    C4_ALWAYS_INLINE operator bool () const { return m_size > 0; }

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

    C4_ALWAYS_INLINE void clear() { m_size = 0; }

    C4_ALWAYS_INLINE const C* data () const noexcept { return m_str; }
    C4_ALWAYS_INLINE       C* data ()       noexcept { return m_str; }

    C4_ALWAYS_INLINE SizeType size () const noexcept { return m_size; }

    C4_ALWAYS_INLINE void resize(SizeType sz) { C4_CHECK(sz <= m_capacity && (m_str != nullptr || sz == 0)); m_size = sz; }

    C4_ALWAYS_INLINE SizeType capacity() const { return m_capacity; }
    constexpr C4_ALWAYS_INLINE SizeType max_size() const { return std::numeric_limit< SizeType >::max(); }

private:

    C * m_str;
    SizeType m_size;
    SizeType m_capacity;

    template< class C2, class S2 >
    friend class basic_substringrs;
    friend void test_string_member_alignment();

}; // basic_substringrs

_C4_IMPLEMENT_TPL_STRIMPL_HASH(c4::basic_substringrs< C C4_COMMA SizeType >, typename C, typename SizeType)

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template< class C, class SizeType, class Alloc >
class basic_string : public string_impl< C, SizeType, basic_string< C, SizeType, Alloc >, basic_substring< C, SizeType > >
{
    using impl_type = string_impl< C, SizeType, basic_string< C, SizeType, Alloc >, basic_substring< C, SizeType > >;
public:

    using char_type = C;
    using value_type = C;
    using traits_type = typename impl_type::traits_type;
    using alloc_type = Alloc;
    using alloc_traits = std::allocator_traits< Alloc >;
    using size_type  = SizeType;
    using ssize_type = typename std::make_signed< SizeType >::type;

    using nalloc_type  = typename Alloc::template rebind< char    >::other; ///< narrow allocator
    using walloc_type  = typename Alloc::template rebind< wchar_t >::other; ///< wide allocator
    using nstring_type = basic_string< char,    SizeType, nalloc_type >; ///< narrow string type
    using wstring_type = basic_string< wchar_t, SizeType, walloc_type >; ///< wide string type

    static constexpr const SizeType npos = SizeType(-1);

private:

    C *      m_str;
    SizeType m_size;
    SizeType m_capacity;
    Alloc    m_alloc;

    friend void test_string_member_alignment();

public:

    C4_ALWAYS_INLINE operator bool     () const { return m_size > 0; }

    C4_ALWAYS_INLINE operator basic_substring< C, SizeType > const& () const
    {
        return *(basic_substring< C, SizeType > const*)this;
    }
    C4_ALWAYS_INLINE operator basic_substringrs< C, SizeType > const& () const
    {
        return *(basic_substringrs< C, SizeType > const*)this;
    }

public:

    _C4STR_ASSIGNOPS(C, basic_string)

    C4_ALWAYS_INLINE basic_string() : m_str(nullptr), m_size(0), m_capacity(0), m_alloc() {}
    C4_ALWAYS_INLINE basic_string(Alloc const& a) : m_str(nullptr), m_size(0), m_capacity(0), m_alloc(a) {}
    C4_ALWAYS_INLINE ~basic_string()
    {
        free();
    }

    C4_ALWAYS_INLINE basic_string(SizeType sz) : m_str(nullptr), m_size(0), m_capacity(0), m_alloc() { resize(sz); }
    C4_ALWAYS_INLINE basic_string(SizeType sz, Alloc const& a) : m_str(nullptr), m_size(0), m_capacity(0), m_alloc(a) { resize(sz); }

    C4_ALWAYS_INLINE basic_string(SizeType sz, SizeType cap) : m_str(nullptr), m_size(0), m_capacity(0), m_alloc() { reserve(cap); resize(sz); }
    C4_ALWAYS_INLINE basic_string(SizeType sz, SizeType cap, Alloc const& a) : m_str(nullptr), m_size(0), m_capacity(0), m_alloc(a) { reserve(cap); resize(sz); }

    C4_ALWAYS_INLINE basic_string(basic_string const& s) : basic_string() { assign(s); }
    C4_ALWAYS_INLINE basic_string(basic_string     && s) : basic_string() { assign(std::move(s)); }
    C4_ALWAYS_INLINE basic_string(basic_string const& s, Alloc const& a) : basic_string(a) { assign(s); }
    C4_ALWAYS_INLINE basic_string(basic_string     && s, Alloc const& a) : basic_string(a) { assign(std::move(s)); }
    C4_ALWAYS_INLINE basic_string& operator= (basic_string const& s) { assign(s); return *this; }
    C4_ALWAYS_INLINE basic_string& operator= (basic_string     && s) { assign(std::move(s)); return *this; }
    void assign(basic_string const& s)
    {
        resize(s.m_size);
        assign(s.m_str, s.m_size);
    }
    void assign(basic_string     && s)
    {
        free();
        m_str = s.m_str;
        m_size = s.m_size;
        m_capacity = s.m_capacity;
        s.m_str = nullptr;
        s.m_size = 0;
        s.m_capacity = 0;
    }

    C4_ALWAYS_INLINE basic_string(substring const& s) : basic_string() { assign(s.data(), s.size()); }
    C4_ALWAYS_INLINE basic_string(substring const& s, Alloc const& a) : basic_string(a) { assign(s.data(), s.size()); }
    C4_ALWAYS_INLINE basic_string& operator= (substring const& s) { assign(s.data(), s.size()); return *this; }
    C4_ALWAYS_INLINE void assign(substring const& s) { assign(s.data(), s.size()); }

    /** @warning assumes the string is null-terminated */
    C4_ALWAYS_INLINE basic_string(const C *s) : basic_string() { assign(s, szconv< size_type >(traits_type::length(s))); }
    /** @warning assumes the string is null-terminated */
    C4_ALWAYS_INLINE basic_string(const C *s, Alloc const& a) : basic_string(a) { assign(s, szconv< size_type >(traits_type::length(s))); }
    /** @warning assumes the string is null-terminated */
    C4_ALWAYS_INLINE basic_string& operator= (const C  *s) { assign(s, szconv< size_type >(traits_type::length(s))); return *this; }
    /** @warning assumes the string is null-terminated */
    void assign(const C *s) { assign(s, szconv< size_type >(traits_type::length(s))); }

    C4_ALWAYS_INLINE basic_string(const C *s, SizeType n) : basic_string() { assign(s, n); }
    C4_ALWAYS_INLINE basic_string(const C *s, SizeType n, Alloc const& a) : basic_string(a) { assign(s, n); }
    C4_ALWAYS_INLINE void assign(const C *s, SizeType n)
    {
        resize(n);
        traits_type::copy(m_str, s, n);
        m_str[m_size] = '\0';
    }

    template< int N > C4_ALWAYS_INLINE basic_string(const C (&s)[N]) : basic_string() { assign(&s[0], N-1); }
    template< int N > C4_ALWAYS_INLINE basic_string(const C (&s)[N], Alloc const& a) : basic_string(a) { assign(&s[0], N-1); }
    template< int N > C4_ALWAYS_INLINE basic_string& operator= (const C (&s)[N]) { assign(&s[0], N-1); return *this; }
    template< int N > C4_ALWAYS_INLINE void assign(const C (&s)[N]) { assign(&s[0], N-1); }
    template< int N > C4_ALWAYS_INLINE void assign(const C (&s)[N], SizeType n) { assign(&s[0], n); }

    C4_ALWAYS_INLINE basic_string(const C *s, SizeType n, SizeType cap)                 : basic_string()  { assign(s, n, cap); }
    C4_ALWAYS_INLINE basic_string(const C *s, SizeType n, SizeType cap, Alloc const& a) : basic_string(a) { assign(s, n, cap); }
    void assign(const C *s, SizeType n, SizeType cap) { C4_ASSERT(cap >= n); reserve(cap); assign(s, n); }

    C4_ALWAYS_INLINE basic_string(C c, SizeType n)                 : basic_string()  { assign(c, n, n); }
    C4_ALWAYS_INLINE basic_string(C c, SizeType n, Alloc const& a) : basic_string(a) { assign(c, n, n); }
    C4_ALWAYS_INLINE void assign(C c, SizeType n) { assign(c, n, n); }

    C4_ALWAYS_INLINE basic_string(C c, SizeType n, SizeType cap)                 : basic_string()  { assign(c, n, cap); }
    C4_ALWAYS_INLINE basic_string(C c, SizeType n, SizeType cap, Alloc const& a) : basic_string(a) { assign(c, n, cap); }
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
    C4_ALWAYS_INLINE basic_string(basic_string< C, OtherSize, Al_ > const& that) { assign(that); }
    template< class OtherSize, class Al_ >
    C4_ALWAYS_INLINE void assign(basic_string< C, OtherSize, Al_ > const& that) { assign(that.m_str, szconv< size_type >(that.m_size)); }
    template< class OtherSize, class Al_ >
    C4_ALWAYS_INLINE basic_string& operator= (basic_string< C, OtherSize, Al_ > const& that) { assign(that); return *this; }

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

_C4_IMPLEMENT_TPL_STRIMPL_HASH(c4::basic_string< C C4_COMMA SizeType C4_COMMA Allocator >,
    typename C, typename SizeType, class Allocator)

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template< class C, class SizeType, class Alloc >
class basic_small_string : public string_impl< C, SizeType, basic_small_string< C, SizeType, Alloc >, basic_substring< C, SizeType > >
{
    using impl_type = string_impl< C, SizeType, basic_small_string< C, SizeType, Alloc >, basic_substring< C, SizeType > >;

    struct _long
    {
        SizeType flag_n_sz; ///< flag (first bit) and size
        SizeType cap;
        C *      str;
    };
   
    template< class _Char > struct _short;
    template<> struct _short< char    > { char flag_n_sz;             char    arr[(sizeof(_long) - 1)/sizeof(char)   ]; };
    template<> struct _short< wchar_t > { char flag_n_sz; char __pad; wchar_t arr[(sizeof(_long) - 1)/sizeof(wchar_t)]; };

public:

    using char_type = C;
    using value_type = C;
    using traits_type = typename impl_type::traits_type;
    using alloc_type = Alloc;
    using alloc_traits = std::allocator_traits< Alloc >;
    using size_type  = SizeType;
    using ssize_type = typename std::make_signed< SizeType >::type;

    using nalloc_type  = typename Alloc::template rebind< char    >::other; ///< narrow allocator
    using walloc_type  = typename Alloc::template rebind< wchar_t >::other; ///< wide allocator
    using nstring_type = basic_small_string< char,    SizeType, nalloc_type >; ///< narrow string type
    using wstring_type = basic_small_string< wchar_t, SizeType, walloc_type >; ///< wide string type

    static constexpr const SizeType npos = SizeType(-1);
    static constexpr const SizeType arr_size = C4_COUNTOF(_short< C >::arr);

private:

    /**
     * @see http://stackoverflow.com/questions/21694302/what-are-the-mechanics-of-short-string-optimization-in-libc
     * @see https://github.com/elliotgoodrich/SSO-23
     */
    union {
        _short< C > m_short;
        _long       m_long;
        char        m_raw[sizeof(_short<C>) > sizeof(_long) ? sizeof(_short<C>) : sizeof(_long)];
    };
    Alloc m_alloc;

    void     _set_short_sz(SizeType sz)   { m_short.flag_n_sz = ((char)sz) << 1; }
    void     _set_long_sz(SizeType sz) { m_long.flag_n_sz = (sz << 1) | 1; }
    SizeType _get_short_sz() const { return m_short.flag_n_sz >> 1; }
    SizeType _get_long_sz() const { return m_long.flag_n_sz >> 1; }

    template< class, class >
    friend void test_small_string_flag_alignment();

public:

    C4_ALWAYS_INLINE operator bool () const { return ! empty(); }

    C4_ALWAYS_INLINE operator const basic_substring  < const C, SizeType > () const { return basic_substring  < const C, SizeType >(data(), size()); }
    C4_ALWAYS_INLINE operator const basic_substring  <       C, SizeType > ()       { return basic_substring  < const C, SizeType >(data(), size()); }
    C4_ALWAYS_INLINE operator const basic_substringrs< const C, SizeType > () const { return basic_substringrs< const C, SizeType >(data(), size()); }
    C4_ALWAYS_INLINE operator const basic_substringrs<       C, SizeType > ()       { return basic_substringrs< const C, SizeType >(data(), size()); }
    
public:

    C4_ALWAYS_INLINE char is_long() const { return m_short.flag_n_sz & 1; }

    C4_ALWAYS_INLINE       C* c_str()       C4_NOEXCEPT_A { C4_ASSERT(!empty()); return data(); }
    C4_ALWAYS_INLINE const C* c_str() const C4_NOEXCEPT_A { C4_ASSERT(!empty()); return data(); }

    C4_ALWAYS_INLINE       C* data()       noexcept { return is_long() ? m_long.str : m_short.arr; }
    C4_ALWAYS_INLINE const C* data() const noexcept { return is_long() ? m_long.str : m_short.arr; }

    C4_ALWAYS_INLINE SizeType size() const noexcept { return is_long() ? _get_long_sz() : _get_short_sz(); }

    C4_ALWAYS_INLINE SizeType   capacity() const noexcept { return is_long() ? m_long.cap : arr_size; }
    constexpr C4_ALWAYS_INLINE SizeType max_size() const noexcept { return (alloc_traits::max_size(m_alloc) << 1) - 1; } // there's one less bit for the short/long flag

public:

    _C4STR_ASSIGNOPS(C, basic_small_string)

    C4_ALWAYS_INLINE basic_small_string() : m_alloc() { memset(m_raw, 0, sizeof(m_raw)); }
    C4_ALWAYS_INLINE basic_small_string(Alloc const& a) : m_alloc(a) { memset(m_raw, 0, sizeof(m_raw)); }
    C4_ALWAYS_INLINE ~basic_small_string()
    {
        this->_free();
    }

    C4_ALWAYS_INLINE basic_small_string(SizeType sz)                 : basic_small_string()  { resize(sz); }
    C4_ALWAYS_INLINE basic_small_string(SizeType sz, Alloc const& a) : basic_small_string(a) { resize(sz); }

    C4_ALWAYS_INLINE basic_small_string(SizeType sz, SizeType cap)                 : basic_small_string()  { reserve(cap); resize(sz); }
    C4_ALWAYS_INLINE basic_small_string(SizeType sz, SizeType cap, Alloc const& a) : basic_small_string(a) { reserve(cap); resize(sz); }

    C4_ALWAYS_INLINE basic_small_string(basic_small_string const& s)                 : basic_small_string(s.m_alloc) { assign(s); }
    C4_ALWAYS_INLINE basic_small_string(basic_small_string     && s)                 : basic_small_string(s.m_alloc) { assign(std::move(s)); }
    C4_ALWAYS_INLINE basic_small_string(basic_small_string const& s, Alloc const& a) : basic_small_string(a) { assign(s); }
    C4_ALWAYS_INLINE basic_small_string(basic_small_string     && s, Alloc const& a) : basic_small_string(a) { assign(std::move(s)); }
    C4_ALWAYS_INLINE basic_small_string& operator= (basic_small_string const& s) { assign(s); return *this; }
    C4_ALWAYS_INLINE basic_small_string& operator= (basic_small_string     && s) { assign(std::move(s)); return *this; }
    void assign(basic_small_string const& that)
    {
        if(&that == this) return;
        resize(that.size());
        assign(that.data(), that.size());
    }
    // keep the current long status, even though the short array
    // would be enough to hold the string. This is because the
    // memory is already allocated, so there's little cost in
    // keeeping using it. If explicit resizing is desired, call shrink_to_fit()
    void assign(basic_small_string && that)
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

    C4_ALWAYS_INLINE basic_small_string(substring const& s)                 : basic_small_string()  { assign(s.data(), s.size()); }
    C4_ALWAYS_INLINE basic_small_string(substring const& s, Alloc const& a) : basic_small_string(a) { assign(s.data(), s.size()); }
    C4_ALWAYS_INLINE basic_small_string& operator= (substring const& s) { assign(s.data(), s.size()); return *this; }
    C4_ALWAYS_INLINE void assign(substring const& s) { assign(s.data(), s.size()); }

    /** construct from a null-terminated string */
    C4_ALWAYS_INLINE basic_small_string(const C *s) : basic_small_string() { assign(s, szconv< size_type >(traits_type::length(s))); }
    /** construct from a null-terminated string */
    C4_ALWAYS_INLINE basic_small_string(const C *s, Alloc const& a) : basic_small_string(a) { assign(s, szconv< size_type >(traits_type::length(s))); }
    /** construct from a null-terminated string */
    C4_ALWAYS_INLINE basic_small_string& operator= (const C  *s) { assign(s, szconv< size_type >(traits_type::length(s))); return *this; }
    /** assign from a null-terminated string */
    void assign(const C *s) { assign(s, szconv< size_type >(traits_type::length(s))); }

    C4_ALWAYS_INLINE basic_small_string(const C *s, SizeType n)                 : basic_small_string()  { assign(s, n); }
    C4_ALWAYS_INLINE basic_small_string(const C *s, SizeType n, Alloc const& a) : basic_small_string(a) { assign(s, n); }
    C4_ALWAYS_INLINE void assign(const C *s, SizeType n)
    {
        resize(n);
        C *d = data();
        traits_type::copy(d, s, n);
        d[n] = C(0);
    }

    template< int N > C4_ALWAYS_INLINE basic_small_string(const C (&s)[N])                 : basic_small_string()  { assign(&s[0], N-1); }
    template< int N > C4_ALWAYS_INLINE basic_small_string(const C (&s)[N], Alloc const& a) : basic_small_string(a) { assign(&s[0], N-1); }
    template< int N > C4_ALWAYS_INLINE basic_small_string& operator= (const C (&s)[N]) { assign(&s[0], N-1); return *this; }
    template< int N > C4_ALWAYS_INLINE void assign(const C (&s)[N])              { assign(&s[0], N-1); }

    C4_ALWAYS_INLINE basic_small_string(const C *s, SizeType n, SizeType cap)                 : basic_small_string()  { assign(s, n, cap); }
    C4_ALWAYS_INLINE basic_small_string(const C *s, SizeType n, SizeType cap, Alloc const& a) : basic_small_string(a) { assign(s, n, cap); }
    void assign(const C *s, SizeType n, SizeType cap) { C4_ASSERT(cap >= n); reserve(cap); assign(s, n); }

    C4_ALWAYS_INLINE basic_small_string(C c, SizeType n)                 : basic_small_string()  { assign(c, n, n); }
    C4_ALWAYS_INLINE basic_small_string(C c, SizeType n, Alloc const& a) : basic_small_string(a) { assign(c, n, n); }
    C4_ALWAYS_INLINE void assign(C c, SizeType n) { assign(c, n, n); }
    C4_ALWAYS_INLINE void assign(C c) { assign(c, size(), capacity()); }

    C4_ALWAYS_INLINE basic_small_string(C c, SizeType n, SizeType cap)                 : basic_small_string()  { assign(c, n, cap); }
    C4_ALWAYS_INLINE basic_small_string(C c, SizeType n, SizeType cap, Alloc const& a) : basic_small_string(a) { assign(c, n, cap); }
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

    template< class Sz_, class Al_ > C4_ALWAYS_INLINE basic_small_string(basic_small_string< C, Sz_, Al_ > const& that) { assign(that); }
    template< class Sz_, class Al_ > C4_ALWAYS_INLINE basic_small_string& operator= (basic_small_string< C, Sz_, Al_ > const& that) { assign(that); return *this; }
    template< class Sz_, class Al_ > C4_ALWAYS_INLINE void assign(basic_small_string< C, Sz_, Al_ > const& that) { assign(that.data(), szconv< size_type >(that.size())); }
   
public:

    void push_back(C c)
    {
        SizeType sz = m_size;
        grow(1);
        data()[sz] = c;
    }
 
public:

    void resize(SizeType sz)
    {
        reserve(sz + 1);
        _set_size(sz);
        data()[sz] = C(0);
    }
    void reserve(SizeType cap)
    {
        SizeType next = (SizeType)(1.618f * float(size()));
        cap = cap > next ? cap : next;
        if(cap > capacity())
        {
            _resizebuf(cap);
        }
    }
    void grow(SizeType more)
    {
        auto sz = m_size + more;
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


_C4_IMPLEMENT_TPL_STRIMPL_HASH(c4::basic_small_string< C C4_COMMA SizeType C4_COMMA Allocator >,
    typename C, typename SizeType, class Allocator)

//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------

#if defined(C4_STR_DISABLE_EXPR_TPL)

// binary operators: basic_string + basic_string
_C4STR_DEFINE_BINOPS1TY_TPL(
    template< typename C C4_COMMA typename SizeType C4_COMMA class Allocator >,
    basic_string< C C4_COMMA SizeType C4_COMMA Allocator > const&,
    basic_string< C C4_COMMA SizeType C4_COMMA Allocator >)

// binary operators: basic_string + const char*
_C4STR_DEFINE_BINOPS2TY_TPL(
    template< typename C C4_COMMA typename SizeType C4_COMMA class Allocator >,
    basic_string< C C4_COMMA SizeType C4_COMMA Allocator > const&,
    const char*,
    basic_string< C C4_COMMA SizeType C4_COMMA Allocator >)

// binary operators: basic_string + basic_substring
_C4STR_DEFINE_BINOPS2TY_TPL(
    template< typename C C4_COMMA typename SizeType C4_COMMA class Allocator >,
    basic_string< C C4_COMMA SizeType C4_COMMA Allocator > const&,
    basic_substring< C C4_COMMA SizeType > const&,
    basic_string< C C4_COMMA SizeType C4_COMMA Allocator >)

// binary operators: basic_string + basic_substringrs
_C4STR_DEFINE_BINOPS2TY_TPL(
    template< typename C C4_COMMA typename SizeType C4_COMMA class Allocator >,
    basic_string< C C4_COMMA SizeType C4_COMMA Allocator > const&,
    basic_substringrs< C C4_COMMA SizeType > const&,
    basic_string< C C4_COMMA SizeType C4_COMMA Allocator >)
#endif
   
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
    string_impl<char   ,  I,  Impl,  SubStrType > const& mbstr,
    string_impl<wchar_t, WI, WImpl, WSubStrType >      * output
)
{
    static_cast< WImpl* >(output)->resize(szconv< WI >(mbstr.size()));
    s2ws(mbstr.data(), mbstr.size(), output->data());
}
template< class I, class Impl, class SubStrType, class WI, class WImpl, class WSubStrType >
void ws2s
(
    string_impl<wchar_t, WI, WImpl, WSubStrType > const& wstr,
    string_impl<char   ,  I,  Impl,  SubStrType >      * output
)
{
    static_cast< Impl* >(output)->resize(wstr.size());
    ws2s(wstr.data(), wstr.size(), output->data());
}

template< class I, class Impl, class SubStrType >
basic_string< wchar_t, I > s2ws(string_impl<char, I, Impl, SubStrType > const& mbstr)
{
    basic_string< wchar_t, I > output;
    output.resize(mbstr.size());
    s2ws(mbstr.data(), mbstr.size(), output.data());
    return output;
}
template< class I, class Impl, class SubStrType >
basic_string< char, I > ws2s(string_impl<wchar_t, I, Impl, SubStrType > const& wstr)
{
    basic_string< char, I > output;
    output.resize(wstr.size());
    ws2s(wstr.data(), wstr.size(), output.data());
    return output;
}

C4_END_NAMESPACE(c4)

#endif // _C4_STRING_HPP_
