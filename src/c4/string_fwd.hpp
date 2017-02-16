#ifndef _C4_STRING_FWD_HPP_
#define _C4_STRING_FWD_HPP_

/** \file string_fwd.hpp forward declarations for string classes,
 * and basic string type definitions */

C4_BEGIN_NAMESPACE(c4)

using str_size_type = C4_STR_SIZE_TYPE;
template< class T > class Allocator;

template< typename C, typename Size=str_size_type > class basic_static_string;
using static_string = basic_static_string< char, str_size_type >;
using static_wstring = basic_static_string< wchar_t, str_size_type >;

template< typename C, typename Size, class Str, class Sub > class string_impl;

template< class C, class Size=str_size_type > class basic_substring;
using substring = basic_substring< char, str_size_type >;
using csubstring = basic_substring< const char, str_size_type >;
using wsubstring = basic_substring< wchar_t, str_size_type >;
using cwsubstring = basic_substring< const wchar_t, str_size_type >;

template< class C, class SizeType=str_size_type > class basic_substringrs;
using substringrs = basic_substringrs< char, str_size_type >;
using csubstringrs = basic_substringrs< const char, str_size_type >;
using wsubstringrs = basic_substringrs< wchar_t, str_size_type >;
using cwsubstringrs = basic_substringrs< const wchar_t, str_size_type >;

template< class C, class SizeType=str_size_type, class= Allocator<C> > class basic_string;
using string = basic_string< char, str_size_type >;
using wstring = basic_string< wchar_t, str_size_type >;

C4_END_NAMESPACE(c4)

#endif // _C4_STRING_FWD_HPP_

