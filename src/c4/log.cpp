#include "c4/log.hpp"
#include "c4/memory_resource.hpp"
#include "c4/compiler.hpp"
#include "c4/time.hpp"

#include <assert.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#   include "c4/windows.hpp"
#endif

C4_BEGIN_NAMESPACE(c4)

// force early construction of c4::log to prevent other translation
// units from logging before this object is initialized.
Log& _fn_log()
{
    static Log log;
    return log;
}
Log& log = _fn_log();
thread_local LogBuffer s_log_buffer;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Log::Proxy Log::operator() (Channel* ch, Level_e lev)
{
    assert(channel_exists(ch));
    return Proxy(*ch, lev);
}
Log::Proxy Log::operator() (Level_e lev)
{
    return Proxy(*main_channel(), INFO);
}

LogBuffer& Log::buf()
{
    return s_log_buffer;
}

/** set the level of all channels */
void Log::level(Level_e l)
{
    for(uint32_t h = 0; h < m_num_channels; ++h)
    {
        m_channels[h].level = l;
    }
}

Log::Channel* Log::channel(const char *name)
{
    for(uint32_t h = 0; h < m_num_channels; ++h)
    {
        if(m_channels[h].cmp(name))
        {
            return const_cast< Channel* >(&(m_channels[0]) + h);
        }
    }
    C4_LOGF_ERR("could not find log channel named %s\n", name);
    return nullptr;
}

bool Log::channel_exists(const char *name) const
{
    for(uint32_t h = 0; h < m_num_channels; ++h)
    {
        if(m_channels[h].cmp(name))
        {
            return true;
        }
    }
    return false;
}

Log::Channel* Log::add_channel(const char *name, Level_e lev)
{
    C4_CHECK(m_num_channels < C4_LOG_MAX_CHANNELS);
    m_channels[m_num_channels].init(name, lev);
    Channel* ret = &(m_channels[0]) + m_num_channels;
    ++m_num_channels;
    return ret;
}

void Log::_print_prefix(Channel const& ch, LogBuffer &buf)
{
    uint8_t md = mode();
    if((md & SHOW_TIMESTAMP) && (ch.name_len > 0))
    {
        buf.printf("%lfms[%s]: ", Time::exetime().ms(), ch.name);
    }
    else if((md & SHOW_TIMESTAMP))
    {
        buf.printf("%lfms: ", Time::exetime().ms(), ch.name);
    }
    else if((ch.name_len > 0))
    {
        buf.printf("[%s]: ", ch.name);
    }
}

/** print formatted output to the main channel, at INFO level */
void Log::printf(const char *fmt, ...)
{
    Channel &ch = m_channels[0];
    if(ch.skip(INFO)) return;
    va_list args;
    va_start(args, fmt);
    auto& b = buf();
    _print_prefix(ch, b);
    b.vprintf(fmt, args);
    pump(b.rd(), b.pos);
    b.clear();
}
/** print formatted output to the main channel, at the given level */
void Log::printfl(Level_e level, const char *fmt, ...)
{
    Channel &ch = m_channels[0];
    if(ch.skip(level)) return;
    va_list args;
    va_start(args, fmt);
    auto& b = buf();
    _print_prefix(ch, b);
    b.vprintf(fmt, args);
    pump(b.rd(), b.pos);
    b.clear();
}
/** print formatted output to the given channel at the given level */
void Log::printfcl(Channel const* ch, Level_e level, const char *fmt, ...)
{
    if(ch->skip(level)) return;
    va_list args;
    va_start(args, fmt);
    auto& b = buf();
    _print_prefix(*ch, b);
    b.vprintf(fmt, args);
    pump(b.rd(), b.pos);
    b.clear();
}

/** directly print a string to the main channel at INFO level */
void Log::write(const char *s) { write(s, strlen(s)); }
/** directly print a string with specified size to the main channel at INFO level */
void Log::write(const char *s, size_t sz)
{
    Channel &ch = m_channels[0];
    if(ch.skip(INFO)) return;
    auto& b = buf();
    _print_prefix(ch, b);
    b.write(s, sz);
    pump(b.rd(), sz);
    b.clear();
}
/** directly print a string to the main channel at the given level */
void Log::writel(Level_e level, const char *s) { writel(level, s, strlen(s)); }
/** directly print a string with specified size to the main channel at the given level */
void Log::writel(Level_e level, const char *s, size_t sz)
{
    Channel &ch = m_channels[0];
    if(ch.skip(level)) return;
    auto& b = buf();
    _print_prefix(ch, b);
    b.write(s, sz);
    pump(b.rd(), sz);
    b.clear();
}
/** directly print a string to the given channel at the given level */
void Log::writecl(Channel const* ch, Level_e level, const char *s) { writel(level, s, strlen(s)); }
/** directly print a string with specified size to the given channel at the given level */
void Log::writecl(Channel const* ch, Level_e level, const char *s, size_t sz)
{
    if(ch->skip(level)) return;
    auto& b = buf();
    _print_prefix(*ch, b);
    b.write(s, sz);
    pump(b.rd(), sz);
    b.clear();
}

Log::StrReader&& Log::str()
{
#ifdef C4_LOG_THREAD_SAFE
    m_strbuf_mtx.lock();
    StrReader r(m_strbuf_mtx, m_strbuf);
#else
    StrReader r(m_strbuf);
#endif
    return std::move(r);
}

void Log::pump(const char *str, size_t sz)
{
    uint8_t md = mode();
    if(md & TO_TERM)
    {
        ::printf("%.*s", (int)sz, str);
#ifdef _MSC_VER
        if(IsDebuggerPresent())
        {
            OutputDebugStringA(str);
        }
#endif
    }
    if(md & TO_FILE)
    {
        if(file() == nullptr) abort();
        fprintf(file(), "%.*s", (int)sz, str);
    }
    if(md & TO_STR)
    {
#ifdef C4_LOG_THREAD_SAFE
        std::lock_guard< std::mutex > lock(m_strbuf_mtx);
#endif
        m_strbuf.write(str, sz);
    }
}

void Log::flush()
{
    uint8_t md = mode();
    if(md & TO_TERM)
    {
        fflush(stdout);
    }
    if(md & TO_FILE)
    {
        fflush(m_file);
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Log::Channel::init(const char *str, Level_e lev)
{
    level = lev;
    name_len = (uint8_t)strnlen(str, sizeof(name));
    if(name_len > (sizeof(name) - 1)) abort();
    if(name_len > 0)
    {
        snprintf(name, sizeof(name), "%s", str);
    }
    else
    {
        name[name_len] = '\0';
    }
}
bool Log::Channel::cmp(const char *str) const
{
    if(name_len == 0) return str[0] == '\0';
    return strncmp(name, str, name_len) == 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

LogBuffer::LogBuffer()
{
    buf = (char*)c4::alloc(C4_LOG_BUFFER_INITIAL_SIZE);
    size = 0;
    pos = 0;
}

LogBuffer::~LogBuffer()
{
    if(buf)
    {
        c4::free(buf);
        buf = nullptr;
    }
    size = 0;
    pos = 0;
}

void LogBuffer::resize(size_t sz)
{
    if(sz <= size) return;
    buf = (char*)c4::realloc(buf, size, sz);
    size = sz;
}

size_t LogBuffer::rem()
{
    assert(pos <= size);
    return size - pos;
}

void LogBuffer::clear()
{
    pos = 0;
    buf[0] = '\0';
}

void LogBuffer::growto(size_t sz) // grow by the max of sz and the golden ratio
{
    float n = 1.618f * float(size);
    assert(size_t(n) < max_idx);
    auto next = size_t(n);
    next = next > sz ? next : sz;
    resize(next);
}

void LogBuffer::write(const char *cstr)
{
    write(cstr, strlen(cstr));
}

void LogBuffer::write(const char *str, size_t sz)
{
    assert(sz <= max_idx);
    assert(sz + size_t(pos + 1) < max_idx);
    if(sz+1 > rem()) growto(pos + sz + 1);
    ::strncpy(wt(), str, sz);
    pos += sz;
    buf[pos] = '\0';
}

void LogBuffer::printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int inum = ::vsnprintf(wt(), rem(), fmt, args);
    size_t num = abs(inum); // silently skip output errors
    assert(num < max_idx);
    if(num >= rem()) // not enough space?
    {
        va_start(args, fmt);
        growto(pos + num + 1);
        assert(size_t(pos) + size_t(num) + 1 < max_idx);
        inum = ::vsnprintf(wt(), rem(), fmt, args);
        num = abs(inum);
        assert(num < max_idx);
    }
    assert(size_t(pos) + size_t(num) < max_idx);
    pos += num;
    buf[pos] = '\0';
    va_end(args);
}

void LogBuffer::vprintf(const char *fmt, va_list args)
{
    va_list args2;
    va_copy(args2, args);
    int inum = ::vsnprintf(wt(), rem(), fmt, args);
    size_t num = abs(inum); // silently skip output errors
    assert(num < max_idx);
    if(num >= rem()) // not enough space?
    {
        assert(num < max_idx);
        assert(size_t(pos) + size_t(num) + 1 < max_idx);
        growto(size + num + 1);
        inum = ::vsnprintf(wt(), rem(), fmt, args2);
        num = abs(inum);
    }
    assert(size_t(pos) + size_t(num) + 1 < max_idx);
    pos += num;
    buf[pos] = '\0';
    va_end(args2);
}

/// get the position where the next argument starts
size_t LogBuffer::nextarg_(const char *fmt)
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
    return size_t(-1); // no more tokens were found
}

C4_END_NAMESPACE(c4)
