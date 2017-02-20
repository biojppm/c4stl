#ifndef _C4_CHAR_TRAITS_HPP_
#define _C4_CHAR_TRAITS_HPP_

#include "c4/config.hpp"

#include <string> // needed because of std::char_traits
#include <cctype>
#include <cwctype>

C4_BEGIN_NAMESPACE(c4)

C4_ALWAYS_INLINE bool isspace(char c) { return std::isspace(c); }
C4_ALWAYS_INLINE bool isspace(wchar_t c) { return std::iswspace(c); }

//-----------------------------------------------------------------------------
template< typename C >
struct char_traits;

template<>
struct char_traits< char > : public std::char_traits< char >
{
    constexpr static const char whitespace_chars[] = " \f\n\r\t\v";
    constexpr static const size_t num_whitespace_chars = sizeof(whitespace_chars) - 1;
};

template<>
struct char_traits< wchar_t > : public std::char_traits< wchar_t >
{
    constexpr static const wchar_t whitespace_chars[] = L" \f\n\r\t\v";
    constexpr static const size_t num_whitespace_chars = sizeof(whitespace_chars) - 1;
};

C4_END_NAMESPACE(c4)

#endif /* _C4_CHAR_TRAITS_HPP_ */
