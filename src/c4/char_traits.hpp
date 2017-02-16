#ifndef _C4_CHAR_TRAITS_HPP_
#define _C4_CHAR_TRAITS_HPP_

#include "c4/config.hpp"

#include <string> // needed because of std::char_traits
#include <cctype>
//#include <cwctype>

C4_BEGIN_NAMESPACE(c4)

//-----------------------------------------------------------------------------
template< typename C >
struct char_traits;

template<>
struct char_traits< char > : public std::char_traits< char >
{
    constexpr static const char whitespace_chars[] = " \f\n\r\t\v";
    constexpr static const size_t num_whitespace_chars = sizeof(whitespace_chars) - 1;

    // add more functions from cctype as needed
    //C4_FORCE_INLINE static bool isspace(int c) { return std::isspace(c); }
};

template<>
struct char_traits< wchar_t > : public std::char_traits< wchar_t >
{
    constexpr static const wchar_t whitespace_chars[] = L" \f\n\r\t\v";
    constexpr static const size_t num_whitespace_chars = sizeof(whitespace_chars) - 1;

    // add more functions from cwctype as needed
    //C4_FORCE_INLINE static bool isspace(wint_t c) { return std::iswspace(c); }
};

C4_END_NAMESPACE(c4)

#endif /* _C4_CHAR_TRAITS_HPP_ */
