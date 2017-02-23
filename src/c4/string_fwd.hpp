#ifndef _C4_STRING_FWD_HPP_
#define _C4_STRING_FWD_HPP_

/** \file string_fwd.hpp forward declarations for string classes,
 * and basic string type definitions */
#include "c4/config.hpp"

C4_BEGIN_NAMESPACE(c4)

using str_size_type = C4_STR_SIZE_TYPE;
template< class T > class Allocator;

template< typename C, typename Size, class Str, class Sub > class string_impl;

template< typename C, typename Size=C4_STR_SIZE_TYPE > class basic_substring;
template< typename C, typename Size=C4_STR_SIZE_TYPE > class basic_substringrs;
template< typename C, typename Size=C4_STR_SIZE_TYPE, class Alloc=c4::Allocator<C> > class basic_string;
template< typename C, typename Size=C4_STR_SIZE_TYPE, class Alloc=c4::Allocator<C> > class basic_small_string;

using substring     = basic_substring  < char          >;
using substringrs   = basic_substringrs< char          >;

using wsubstring    = basic_substring  < wchar_t       >;
using wsubstringrs  = basic_substringrs< wchar_t       >;

using csubstring    = basic_substring  < const char    >;
using csubstringrs  = basic_substringrs< const char    >;

using cwsubstring   = basic_substring  < const wchar_t >;
using cwsubstringrs = basic_substringrs< const wchar_t >;

using string        = basic_string< char    >;
using wstring       = basic_string< wchar_t >;

using small_string  = basic_small_string< char    >;
using small_wstring = basic_small_string< wchar_t >;

C4_END_NAMESPACE(c4)

#endif // _C4_STRING_FWD_HPP_

