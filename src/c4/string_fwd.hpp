#ifndef _C4_STRING_FWD_HPP_
#define _C4_STRING_FWD_HPP_

/** \file string_fwd.hpp forward declarations for string classes,
 * and basic string type definitions */

C4_BEGIN_NAMESPACE(c4)

using str_size_type = C4_STR_SIZE_TYPE;
template< class T > class Allocator;

template< typename C, typename Size, class Str, class Sub > class string_impl;

template< class C, class Size=str_size_type > class basic_substring;
template< class C, class Size=str_size_type > class basic_substringrs;
template< class C, class Size=str_size_type, class = Allocator<C> > class basic_string;

using substring     = basic_substring< char          >;
using wsubstring    = basic_substring< wchar_t       >;
using csubstring    = basic_substring< const char    >;
using cwsubstring   = basic_substring< const wchar_t >;
using substringrs   = basic_substringrs< char          >;
using wsubstringrs  = basic_substringrs< wchar_t       >;
using csubstringrs  = basic_substringrs< const char    >;
using cwsubstringrs = basic_substringrs< const wchar_t >;
using string        = basic_string< char    >;
using wstring       = basic_string< wchar_t >;

C4_END_NAMESPACE(c4)

#endif // _C4_STRING_FWD_HPP_

