#ifndef _C4_COMPILER_HPP_
#define _C4_COMPILER_HPP_

/** @file compiler.hpp Provides compiler information macros
 * @ingroup basic_headers */

#include "c4/platform.hpp"

// Compilers:
//      C4_MSVC
//             Visual Studio 2017: MSVC++ 15
//             Visual Studio 2015: MSVC++ 14
//             Visual Studio 2013: MSVC++ 13
//             Visual Studio 2013: MSVC++ 12
//             Visual Studio 2012: MSVC++ 11
//             Visual Studio 2010: MSVC++ 10
//             Visual Studio 2008: MSVC++ 09
//             Visual Studio 2005: MSVC++ 08
//      C4_CLANG
//      C4_GCC
//      C4_ICC (intel compiler)
/** @see http://sourceforge.net/p/predef/wiki/Compilers/ for a list of compiler identifier macros */
/** @see https://msdn.microsoft.com/en-us/library/b0084kay.aspx for VS2013 predefined macros */

#if defined(_MSC_VER) && (defined(C4_WIN) || defined(C4_XBOX) || defined(C4_UE4))
#   define C4_MSVC
#   define C4_MSVC_VERSION_2017 15
#   define C4_MSVC_VERSION_2015 14
#   define C4_MSVC_VERSION_2013 12
#   define C4_MSVC_VERSION_2012 11
#   if _MSC_VER > 1900
#       define C4_MSVC_VERSION C4_MSVC_VERSION_2017  // visual studio 2017
#       define C4_MSVC_2017
#   elif _MSC_VER == 1900
#       define C4_MSVC_VERSION C4_MSVC_VERSION_2015  // visual studio 2015
#       define C4_MSVC_2015
#   elif _MSC_VER == 1800
#       error "MSVC version not supported"
#       define C4_MSVC_VERSION C4_MSVC_VERSION_2013  // visual studio 2013
#       define C4_MSVC_2013
#   elif _MSC_VER == 1700
#       error "MSVC version not supported"
#       define C4_MSVC_VERSION C4_MSVC_VERSION_2012  // visual studio 2012
#       define C4_MSVC_2012
#   elif _MSC_VER == 1600
#       error "MSVC version not supported"
#       define C4_MSVC_VERSION 10  // visual studio 2010
#       define C4_MSVC_2010
#   elif _MSC_VER == 1500
#       error "MSVC version not supported"
#       define C4_MSVC_VERSION 09  // visual studio 2008
#       define C4_MSVC_2008
#   elif _MSC_VER == 1400
#       error "MSVC version not supported"
#       define C4_MSVC_VERSION 08  // visual studio 2005
#       define C4_MSVC_2005
#   else
#       error "MSVC version not supported"
#   endif // _MSC_VER
#else
#   define C4_MSVC_VERSION 0   // visual studio not present
#   define C4_GCC_LIKE
#   ifdef __INTEL_COMPILER // check ICC before checking GCC, as ICC defines __GNUC__ too
#       define C4_ICC
#       define C4_ICC_VERSION __INTEL_COMPILER
#   elif defined(__clang__)
#       define C4_CLANG
#       ifndef __apple_build_version__
#           define C4_CLANG_VERSION C4_VERSION_ENCODED(__clang_major__, __clang_minor__, __clang_patchlevel__)
#       else
#           define C4_CLANG_VERSION __apple_build_version__
#       endif
#   elif defined(__GNUC__)
#       define C4_GCC
#       if defined(__GNUC_PATCHLEVEL__)
#           define C4_GCC_VERSION C4_VERSION_ENCODED(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__)
#       else
#           define C4_GCC_VERSION C4_VERSION_ENCODED(__GNUC__, __GNUC_MINOR__, 0)
#       endif
#   endif
#endif // defined(C4_WIN) && defined(_MSC_VER)

#ifdef _MSC_VER
#   pragma warning(disable : 4068) // unknown pragma
#   pragma warning(disable : 4100) // unreferenced formal parameter
#   pragma warning(disable : 4127) // conditional expression is constant -- eg  do {} while(1);
#   pragma warning(disable : 4201) // nonstandard extension used : nameless struct/union
#   pragma warning(disable : 4238) // nonstandard extension used: class rvalue used as lvalue, eg set_str(&(get_str() + "-suffix))
#   pragma warning(disable : 4244)
#   pragma warning(disable : 4503) // decorated name length exceeded, name was truncated
#   pragma warning(disable : 4702) // unreachable code
#   pragma warning(disable : 4714) // function marked as __forceinline not inlined
#   pragma warning(disable : 4800) // forcing value to bool 'true' or 'false' (performance warning)
#   pragma warning(disable : 4996) // 'strncpy': This function or variable may be unsafe
#endif

#endif /* _C4_COMPILER_HPP_ */
