#ifndef _C4_SSTREAM_DEF_HPP_
#define _C4_SSTREAM_DEF_HPP_

#ifndef _C4_SSTREAM_HPP_
#include "c4/sstream.hpp"
#endif

#include "c4/sstream.cpp" // FIXME

C4_BEGIN_NAMESPACE(c4)

//-----------------------------------------------------------------------------
template <class StringType>
template <class T, class... MoreArgs>
void sstream<StringType>::printp_(const char* fmt, T const& arg, MoreArgs&& ...more)
{
    size_type next = nextarg_(fmt); // where does the next argument token start?
    if(C4_UNLIKELY(next == npos)) // no more tokens were found
    {
        printp_(fmt); // write everything
        return;
    }
    write(fmt, next); // write the string up to the {} token
    fmt += next + 2; // prepare next call: advance the string (2 for the token)
    *this << arg; // print the argument
    printp_(fmt, std::forward< MoreArgs >(more)...); // print more arguments
}

//-----------------------------------------------------------------------------
template <class StringType>
template <class T, class... MoreArgs>
void sstream<StringType>::scanp_(const char* fmt, T & arg, MoreArgs&& ...more)
{
    size_type next = nextarg_(fmt); // where does the next argument token start?
    if(C4_UNLIKELY(next == npos)) // no more tokens were found
    {
        scanp_(fmt);
        return;
    }
    fmt += next + 2; // prepare next call: advance the string (2 for the token)
    m_getpos += next; // advance the needle
    *this >> arg; // scan the argument
    scanp_(fmt, std::forward< MoreArgs >(more)...); // scan more arguments
}

C4_END_NAMESPACE(c4)

#endif /* _C4_SSTREAM_DEF_HPP_ */
