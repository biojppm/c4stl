#ifndef _C4_SSTREAM_DEF_HPP_
#define _C4_SSTREAM_DEF_HPP_

#ifndef _C4_SSTREAM_HPP_
#include "c4/sstream.hpp"
#endif


C4_BEGIN_NAMESPACE(c4)

//-----------------------------------------------------------------------------
template< class StringType >
template< class... Args >
sstream< StringType >::sstream(Args&& ...string_ctor_args)
:
    m_string(std::forward< Args >(string_ctor_args)...),
    m_putpos(0),
    m_getpos(0),
    m_status(0)
{
    reserve(m_string.capacity()); // HACK for std::string
}

//-----------------------------------------------------------------------------
template< class StringType >
StringType&& sstream< StringType >::move_out()
{
    m_string.resize(m_putpos);
    StringType&& mv = std::move(m_string);
    reset();
    //m_string.~StringType(); // not sure about this
    return mv;
}

//-----------------------------------------------------------------------------
template< class StringType >
void sstream< StringType >::errp_()
{
    m_status |= EOFP;
    if(m_status & IGNORE_ERR) return;
    C4_ERROR("could not write to stream");
}

//-----------------------------------------------------------------------------
template< class StringType >
void sstream< StringType >::errg_()
{
    m_status |= EOFG;
    if(m_status & IGNORE_ERR) return;
    C4_ERROR("could not read from stream");
}

//-----------------------------------------------------------------------------
template< class StringType >
void sstream< StringType >::errf_()
{
    m_status |= FAIL;
    if(m_status & IGNORE_ERR) return;
    C4_ERROR("could not interpret arguments");
}

//-----------------------------------------------------------------------------
template< class StringType >
void sstream< StringType >::reserve(size_type cap)
{
    if(cap >= m_string.size())
    {
        m_string.reserve(cap);
        m_string.resize(m_string.capacity());
    }
}

//-----------------------------------------------------------------------------
template< class StringType >
void sstream< StringType >::growto_(size_type sz)
{
    if(sz >= m_string.size())
    {
        m_string.reserve(sz);
        m_string.resize(m_string.capacity());
    }
}

//-----------------------------------------------------------------------------
template< class StringType >
typename sstream< StringType >::char_type sstream< StringType >::peek(size_type ahead)
{
    if(C4_UNLIKELY(ahead > remg()))
    {
        m_status |= EOFG;
    }

    if(C4_UNLIKELY(m_status & (FAIL|EOFG)))
    {
        errg_();
        return '\0';
    }
    return buf_()[m_getpos + ahead];
}

//-----------------------------------------------------------------------------
template< class StringType >
void sstream< StringType >::write(const char_type *cstr)
{
    write(cstr, traits_type::length(cstr));
}

//-----------------------------------------------------------------------------
template< class StringType >
void sstream< StringType >::write(const char_type *str, size_type sz)
{
    if(C4_UNLIKELY( ! okp(sz + 1)))
    {
        growto_(m_putpos + sz + 1); // this reserves and will allocate when needed
    }

    if(C4_UNLIKELY(m_status & EOFP))
    {
        return;
    }

    auto *b = buf_();
    traits_type::copy(b + m_putpos, str, sz);
    m_putpos += sz;
    b[m_putpos] = '\0';
}

//-----------------------------------------------------------------------------
template< class StringType >
void sstream< StringType >::read(char_type *str, size_type sz)
{
    if(C4_UNLIKELY( ! okg(sz))) // defend against overflow
    {
        errg_();
    }

    if(C4_UNLIKELY(m_status & EOFG))
    {
        return;
    }
    traits_type::copy(str, buf_() + m_getpos, sz);
    m_getpos += sz;
}

//-----------------------------------------------------------------------------
template< class StringType >
void sstream< StringType >::scanf____(const char_type *fmt, void *arg)
{
    C4_ASSERT(traits_type::length(fmt)>3 && traits_type::compare(fmt+strlen(fmt)-2, "%n", 2) == 0);
    int num_chars = 0, num_conv = 0;
    num_conv = c4::sscanf(buf_() + m_getpos, fmt, arg, &num_chars);
    size_type snum = size_type(num_chars);
    if(C4_UNLIKELY(num_conv != 1))
    {
        errf_();
    }
    if(C4_UNLIKELY(snum > remg()))
    {
        errg_();
        return;
    }
    m_getpos += snum;
}

//-----------------------------------------------------------------------------
template< class StringType >
void sstream< StringType >::printf(const char_type *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    this->vprintf(fmt, args);
}

//-----------------------------------------------------------------------------
template< class StringType >
void sstream< StringType >::vprintf(const char_type *fmt, va_list args)
{
#ifdef C4_VA_LIST_REUSE_MUST_COPY
    va_list args_dup;
    va_copy(args_dup, args);
#endif
    auto *b = buf_();
    /* vsnprintf() returns number of characters written if successful or negative
     * value if an error occurred. If the resulting string gets truncated due to
     * buf_size limit, function returns the total number of characters
     * (not including the terminating null-byte) which would have been written,
     * if the limit was not imposed.
     * @see http://en.cppreference.com/w/cpp/io/c/vfprintf */
    int inum = c4::vsnprintf(b + m_putpos, remp(), fmt, args);
    size_type snum = size_type(inum > 0 ? inum : 0);
    if(C4_UNLIKELY(inum < 0)) // bad formatting
    {
        errf_();
    }
    else if(C4_UNLIKELY(snum + 1 > remp())) // not enough space?
    {
        growto_(m_putpos + snum + 1); // don't forget the terminating character
        if(C4_UNLIKELY(m_status & EOFP))
        {
            goto clear_va_args; // please dear sir - kindly allow me this indiscretion
        }
        b = buf_();
#ifdef C4_VA_LIST_REUSE_MUST_COPY
        inum = c4::vsnprintf(b + m_putpos, remp(), fmt, args_dup);
#else
        inum = c4::vsnprintf(b + m_putpos, remp(), fmt, args);
#endif
        C4_ASSERT(inum >= 0 && size_type(inum) < remp());
        snum = size_type(inum > 0 ? inum : 0);
    }
    m_putpos += snum;
    b[m_putpos] = '\0';

clear_va_args:
    va_end(args);
#ifdef C4_VA_LIST_REUSE_MUST_COPY
    va_end(args_dup);
#endif
}

//-----------------------------------------------------------------------------
/// get the position where the next argument token starts
template< class StringType >
typename sstream< StringType >::size_type sstream< StringType >::nextarg_(const char_type *fmt)
{
    size_type next = 0;
    value_type prev = '\0';
    while(fmt[next] != '\0')
    {
        if(fmt[next] == '{' && fmt[next + 1] == '}')
        {
            if(next == 0 || fmt[next-1] != '\\')
            {
                return next;
            }
        }
        ++next;
    }
    return npos; // no more tokens were found
}

//-----------------------------------------------------------------------------
template <class StringType>
template <class T, class... MoreArgs>
void sstream<StringType>::printp_(const char_type* fmt, T const& arg, MoreArgs&& ...more)
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
void sstream<StringType>::scanp_(const char_type* fmt, T & arg, MoreArgs&& ...more)
{
    size_type next = nextarg_(fmt); // where does the next argument token start?
    if(C4_UNLIKELY(next == npos)) // no more tokens were found
    {
        scanp_(fmt);
        return;
    }
    m_getpos += next; // advance the needle
    fmt += next + 2; // prepare next call: advance the string (2 for the token)
    *this >> arg; // scan the argument
    scanp_(fmt, std::forward< MoreArgs >(more)...); // scan more arguments
}

C4_END_NAMESPACE(c4)

#endif /* _C4_SSTREAM_DEF_HPP_ */
