#include "c4/sstream.hpp"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "c4/memory_resource.hpp"

C4_BEGIN_NAMESPACE(c4)

//-----------------------------------------------------------------------------
template< class StringType >
sstream< StringType >::~sstream()
{
    if(owner())
    {
        m_string.~StringType();
    }
}

//-----------------------------------------------------------------------------
/** will own a string object */
template< class StringType >
sstream< StringType >::sstream()
:
    m_string(),
    m_putpos(0),
    m_getpos(0),
    m_status(OWNER)
{
}

//-----------------------------------------------------------------------------
/** will map the given memory as the stream buffer. The string object
 * will be inactive. The caller needs to ensure that the lifetime of the
 * given memory will exceed that of this sstream object. */
template< class StringType >
sstream< StringType >::sstream(value_type *s, size_type sz)
:
    m_buf(s),
    m_capacity(sz),
    m_putpos(0),
    m_getpos(0),
    m_status(0)
{
}

//-----------------------------------------------------------------------------
template< class StringType >
sstream< StringType >::sstream(sstream const& that)
:
    m_putpos(that.m_putpos),
    m_getpos(that.m_getpos),
    m_status(that.m_status)
{
    if(that.owner())
    {
        new (m_string) StringType(that.m_string);
    }
    else
    {
        m_buf = that.m_buf;
        m_capacity = that.m_capacity;
    }
}

//-----------------------------------------------------------------------------
template< class StringType >
sstream< StringType >::sstream(sstream && that)
:
    m_putpos(that.m_putpos),
    m_getpos(that.m_getpos),
    m_status(that.m_status)
{
    if(that.owner())
    {
        new (m_string) StringType(std::move(that.m_string));
        that &= ~OWNER;
    }
    else
    {
        m_buf = that.m_buf;
        m_capacity = that.m_capacity;
    }
}

//-----------------------------------------------------------------------------
template< class StringType >
sstream< StringType >& sstream< StringType >::operator= (sstream const& that)
{
    if(that.owner())
    {
        if(owner())
        {
            m_string.operator=(that.m_string);
        }
        else
        {
            new (m_string) StringType(that.m_string);
            m_status |= OWNER;
        }
    }
    else
    {
        if(owner())
        {
            m_string.~StringType();
        }
        m_status &= ~OWNER;
        m_buf = that.m_buf;
        m_capacity = that.m_capacity;
    }
    m_putpos = that.m_putpos;
    m_getpos = that.m_getpos;
    m_status &= that.m_status & (~OWNER);
    m_status |= that.m_status & (~OWNER);
}

//-----------------------------------------------------------------------------
template< class StringType >
sstream< StringType >& sstream< StringType >::operator= (sstream && that)
{
    if(that.owner())
    {
        if(owner())
        {
             m_string.operator=(std::move(that.m_string));
        }
        else
        {
            new (m_string) StringType(std::move(that.m_string));
            m_status |= OWNER;
        }
        that.m_status &= ~OWNER;
    }
    else
    {
        if(owner())
        {
            m_string.~StringType();
        }
        m_status &= ~OWNER;
        m_buf = that.m_buf;
        m_capacity = that.m_capacity;
    }
    m_putpos = that.m_putpos;
    m_getpos = that.m_getpos;
    m_status &= that.m_status & (~OWNER);
    m_status |= that.m_status & (~OWNER);
}

//-----------------------------------------------------------------------------
template< class StringType >
StringType&& sstream< StringType >::move_out()
{
    C4_CHECK(owned());
    m_string.resize(m_putpos);
    m_status &= ~OWNED;
    m_buf = nullptr;
    m_capacity = 0;
    reset();
    return std::move(m_string);
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
    if(owner())
    {
        m_string.reserve(cap);
    }
    else
    {
        C4_CHECK(cap <= m_capacity);
    }
}

//-----------------------------------------------------------------------------
template< class StringType >
void sstream< StringType >::growto_(size_type sz)
{
    if(owner())
    {
        m_string.reserve(sz);
    }
    else
    {
        if(sz >= m_capacity)
        {
            errp_();
        }
    }
}

//-----------------------------------------------------------------------------
template< class StringType >
char sstream< StringType >::peek(size_type ahead)
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
void sstream< StringType >::write(const char *cstr)
{
    write(cstr, strlen(cstr));
}

//-----------------------------------------------------------------------------
template< class StringType >
void sstream< StringType >::write(const char *str, size_type sz)
{
    if(C4_UNLIKELY(sz+1 > remp()))
    {
        growto_(m_putpos + sz + 1);
    }

    if(C4_UNLIKELY(m_status & EOFP))
    {
        return;
    }
    auto *b = buf_();
    ::memcpy(b + m_putpos, str, sz * sizeof(value_type));
    m_putpos += sz;
    b[m_putpos] = '\0';
}

//-----------------------------------------------------------------------------
template< class StringType >
void sstream< StringType >::read(char *str, size_type sz)
{
    if(C4_UNLIKELY( ! okg(sz))) // defend against overflow
    {
        errg_();
    }

    if(C4_UNLIKELY(m_status & EOFG))
    {
        return;
    }
    ::memcpy(str, buf_() + m_getpos, sz * sizeof(value_type));
    m_getpos += sz;
}

//-----------------------------------------------------------------------------
template< class StringType >
void sstream< StringType >::scanf____(const char *fmt, void *arg)
{
    C4_ASSERT(strlen(fmt)>3 && strncmp(fmt+strlen(fmt)-2, "%n", 2) == 0);
    int num_chars = 0, num_conv = 0;
    num_conv = ::sscanf(buf_() + m_getpos, fmt, arg, &num_chars);
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
void sstream< StringType >::printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
}

//-----------------------------------------------------------------------------
template< class StringType >
void sstream< StringType >::vprintf(const char *fmt, va_list args)
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
    int inum = ::vsnprintf(b + m_putpos, remp(), fmt, args);
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
        inum = ::vsnprintf(b + m_putpos, remp(), fmt, args_dup);
#else
        inum = ::vsnprintf(b + m_putpos, remp(), fmt, args);
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
typename sstream< StringType >::size_type sstream< StringType >::nextarg_(const char *fmt)
{
    size_type next = 0;
    value_type prev = '\0';
    while(fmt[next] != '\0')
    {
        if(fmt[next] == '{' && fmt[next + 1] == '}')
        {
            if(next)
            return next;
        }
        ++next;
    }
    return npos; // no more tokens were found
}

C4_END_NAMESPACE(c4)
