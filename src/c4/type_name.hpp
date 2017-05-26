#ifndef _C4_TYPENAME_HPP_
#define _C4_TYPENAME_HPP_

#include "c4/span.hpp"

#include <stdio.h>

/// @cond dev
struct _c4t
{
    template< size_t N > _c4t(const char (&s)[N]) : str(s), sz(N-1) {} // take off the \0
    const char *str; size_t sz;
};
// this is a more abbreviated way of getting the type name
// (if we used span in the return type the name would involve
// templates and would create longer type name strings,
// as well as more differences between compilers)
template< class T >
C4_CONSTEXPR14 C4_ALWAYS_INLINE
_c4t _c4tn()
{
    auto p = _c4t(C4_PRETTY_FUNC);
    return p;
}
/// @endcond


C4_BEGIN_NAMESPACE(c4)
/** adapted from this answer: http://stackoverflow.com/a/20170989/5875572 */
template< class T > C4_CONSTEXPR14 cspan<char> type_name()
{
    const _c4t p = _c4tn< T >();

#if 0 && defined(_C4_THIS_IS_A_DEBUG_SCAFFOLD)
    for(int index = 0; index < p.sz; ++index)
    {
        printf(" %2c", p.str[index]);
    }
    printf("\n");
    for(int index = 0; index < p.sz; ++index)
    {
        printf(" %2d", index);
    }
    printf("\n");
#endif

#if defined(_MSC_VER)
#   if defined(__clang__) // Visual Studio has the clang toolset
    // example:
    // ..........................xxx.
    // _c4t __cdecl _c4tn() [T = int]
    enum { tstart = 26, tend = 1};

#   elif defined(C4_MSVC_2015) || defined(C4_MSVC_2017)
    // Note: subtract 7 at the end because the function terminates with ">(void)" in VS2015+
    cspan<char>::size_type tstart = 26, tend = 7;

    const char *s = p.str + tstart; // look at the start

    // we're not using strcmp to spare the #include

    // does it start with 'class '?
    if(p.sz > 6 && s[0] == 'c' && s[1] == 'l' && s[2] == 'a' && s[3] == 's' && s[4] == 's' && s[5] == ' ')
    {
        tstart += 6;
    }
    // does it start with 'struct '?
    else if(p.sz > 7 && s[0] == 's' && s[1] == 't' && s[2] == 'r' && s[3] == 'u' && s[4] == 'c' && s[5] == 't' && s[6] == ' ')
    {
        tstart += 7;
    }

#   else
        C4_NOT_IMPLEMENTED();
#   endif

#elif defined(__ICC)
    // example:
    // ........................xxx.
    // "_c4t _c4tn() [with T = int]"
    enum { tstart = 23, tend = 1};

#elif defined(__clang__)
    // example:
    // ...................xxx.
    // "_c4t _c4tn() [T = int]"
    enum { tstart = 18, tend = 1};

#elif defined(__GNUC__)
    // example:
    // ........................xxx.
    // "_c4t _c4tn() [with T = int]"
    enum { tstart = 23, tend = 1 };
#else
    C4_NOT IMPLEMENTED();
#endif

    cspan<char> o(p.str + tstart, p.sz - tstart - tend);

    return o;
}

C4_END_NAMESPACE(c4)

#endif //_C4_TYPENAME_HPP_
