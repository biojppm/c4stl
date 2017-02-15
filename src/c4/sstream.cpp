#include "c4/sstream.hpp"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "c4/memory_resource.hpp"

C4_BEGIN_NAMESPACE(c4)

//-----------------------------------------------------------------------------
sstream::sstream()
{
    m_buf = nullptr;
    m_size = 0;
    m_putpos = 0;
    m_getpos = 0;
    m_status = 0;
}

//-----------------------------------------------------------------------------
sstream::~sstream()
{
    if(m_buf)
    {
        c4::free(m_buf);
        m_buf = nullptr;
    }
}

//-----------------------------------------------------------------------------
void sstream::errp_()
{
    if(m_status & IGNORE_ERR) return;
    C4_ERROR("could not write to stream");
}

//-----------------------------------------------------------------------------
void sstream::errg_()
{
    if(m_status & IGNORE_ERR) return;
    C4_ERROR("could not read from stream");
}

//-----------------------------------------------------------------------------
void sstream::resize_(size_t sz)
{
    if(sz <= m_size) return;
    m_buf = (char*)c4::realloc(m_buf, m_size, sz);
    m_size = sz;
    m_putpos = m_putpos < m_size ? m_putpos : m_size;
    m_getpos = m_getpos < m_size ? m_getpos : m_size;
}

//-----------------------------------------------------------------------------
bool sstream::growto_(size_t sz)
{
    C4_ASSERT(okp(sz));
    if(C4_UNLIKELY(m_size == 0))
    {
        m_buf = (char*)c4::alloc(sz);
    }
    // grow by the max of sz and the golden ratio of the current size
    float n = 1.618f * float(m_size);
    if(size_t(n) > size_t(max_size()))
    {
        m_status |= EOFP;
        return false;
    }
    size_t next = (size_t)n;
    next = next > sz ? next : sz;
    resize_(next);
    return true;
}

//-----------------------------------------------------------------------------
char sstream::peek(size_t ahead)
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
    return m_buf[m_getpos + ahead];
}

//-----------------------------------------------------------------------------
void sstream::write(const char *cstr)
{
    write(cstr, strlen(cstr));
}

//-----------------------------------------------------------------------------
void sstream::write(const char *str, size_t sz)
{
    if(C4_UNLIKELY( ! okp(sz)))
    {
        m_status |= EOFP;
    }
    else if(C4_UNLIKELY(sz+1 > remp()))
    {
        growto_(m_putpos + sz + 1); // sets EOFP if the grow is not successful
    }
    if(C4_UNLIKELY(m_status & EOFP))
    {
        errp_();
        return;
    }
    ::strncpy(m_buf + m_putpos, str, sz);
    m_putpos += sz;
    m_buf[m_putpos] = '\0';
}

//-----------------------------------------------------------------------------
void sstream::read(char *str, size_t sz)
{
    if(C4_UNLIKELY( ! okg(sz))) // defend against overflow
    {
        m_status |= EOFG;
    }
    if(C4_UNLIKELY(m_status & EOFG))
    {
        errg_();
        return;
    }
    ::strncpy(str, m_buf + m_getpos, sz);
    m_getpos += sz;
}

//-----------------------------------------------------------------------------
void sstream::scanf____(const char *fmt, void *arg)
{
    C4_ASSERT(strlen(fmt)>3 && strncmp(fmt+strlen(fmt)-2, "%n", 2) == 0);
    int num_chars = 0, num_conv = 0;
    num_conv = ::sscanf(m_buf + m_getpos, fmt, arg, &num_chars);
    size_t snum = size_t(num_chars);
    if(C4_UNLIKELY(num_conv != 1))
    {
        m_status |= FAIL;
    }
    if(C4_UNLIKELY(snum > remg()))
    {
        m_status |= EOFG;
    }
    m_getpos += snum;
}

//-----------------------------------------------------------------------------
void sstream::printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
}

//-----------------------------------------------------------------------------
void sstream::vprintf(const char *fmt, va_list args)
{
#ifdef C4_VA_LIST_REUSE_MUST_COPY
    va_list args_dup;
    va_copy(args_dup, args);
#endif
    /* vsnprintf() returns number of characters written if successful or negative
     * value if an error occurred. If the resulting string gets truncated due to
     * buf_size limit, function returns the total number of characters
     * (not including the terminating null-byte) which would have been written,
     * if the limit was not imposed.
     * @see http://en.cppreference.com/w/cpp/io/c/vfprintf */
    int inum = ::vsnprintf(m_buf + m_putpos, remp(), fmt, args);
    size_t snum = size_t(inum > 0 ? inum : 0);
    if(C4_UNLIKELY(inum < 0))
    {
        m_status |= FAIL;
    }
    else if(C4_UNLIKELY(snum + 1 > remp())) // not enough space?
    {
        growto_(m_putpos + snum + 1); // don't forget the terminating character
#ifdef C4_VA_LIST_REUSE_MUST_COPY
        inum = ::vsnprintf(m_buf + m_putpos, remp(), fmt, args_dup);
#else
        inum = ::vsnprintf(m_buf + m_putpos, remp(), fmt, args);
#endif
        C4_ASSERT(inum >= 0 && size_t(inum) < remp());
        snum = size_t(inum > 0 ? inum : 0);
    }
    m_putpos += snum;
    m_buf[m_putpos] = '\0';
    va_end(args);
#ifdef C4_VA_LIST_REUSE_MUST_COPY
    va_end(args_dup);
#endif
}

//-----------------------------------------------------------------------------
/// get the position where the next argument starts
size_t sstream::nextarg_(const char *fmt)
{
    size_t next = 0;
    while(fmt[next] != '\0')
    {
        if(fmt[next] == '{' && fmt[next + 1] == '}')
        {
            return next;
        }
        ++next;
    }
    return npos; // no more tokens were found
}

C4_END_NAMESPACE(c4)
