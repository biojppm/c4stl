#ifndef _C4_LOG_HPP_
#define _C4_LOG_HPP_

#include "c4/config.hpp"
#include "c4/language.hpp"

#include <stdint.h>
#include <stdio.h>

#ifdef C4_LOG_THREAD_SAFE
#include <mutex>
#endif


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

C4_BEGIN_NAMESPACE(c4)

/** a stringstream output buffer used by the logger
 * @code
 * LogBuffer b;
 * const char *name = "Foo";
 * int years = 123;
 * const char *years_str = "123";
 *
 * // raw writing - only accepts const char*
 * b.write("hello world I am ");
 * b.write(name)
 * b.write(" and I am ");
 * b.write(years_str)
 *
 * // chevron-style AKA iostream-style
 * b << "hello world I am " << name << " and I am " << years;
 *
 * // c-style, not type safe
 * b.printf("hello world I am %s and I am %d", name, years);
 *
 * // python-style, type safe
 * b.print("hello world I am {} and I am {}", name, years);
 *
 * // r-style, type safe
 * b.cat("hello world I am ", name, " and I am ", years);
 *
 * // using separators: this sucks...
 * const char *s[] = {"now", "we", "have", "10", "strings", "to", "cat", "one", "after", "the other"};
 * b.cat(s[0], ' ', s[1], ' ', s[2], ' ', s[3], ' ', s[4], ' ',
 *       s[5], ' ', s[6], ' ', s[7], ' ', s[8], ' ', s[9]);
 * * // ... and this resolves it
 * b.catsep(s[0], s[1], s[2],  s[3], s[4], s[5], s[6], s[7], s[8], s[9], ' ');
 * @endcode
 */
struct LogBuffer
{
    typedef size_t index_type;
    constexpr static const size_t max_idx = SIZE_MAX;

    char* buf;
    size_t size;
    size_t pos;

    LogBuffer();
    ~LogBuffer();

    LogBuffer(LogBuffer const&) = delete;
    LogBuffer(LogBuffer     &&) = default;
    LogBuffer& operator=(LogBuffer const&) = delete;
    LogBuffer& operator=(LogBuffer     &&) = default;

    /// string to read from
    C4_ALWAYS_INLINE const char* rd() const { return buf; }
    /// string to write into
    C4_ALWAYS_INLINE char* wt() { return buf + pos; }
    /// remaining size
    size_t rem();

    void resize(size_t sz);
    void clear();
    void growto(size_t sz);

    /// Raw writing of the string
    void write(const char *cstr);
    void write(const char *str, size_t sz);

    /// C-style
    void printf(const char *fmt, ...) C4_ATTR_FORMAT(printf, 1, 2);
    void vprintf(const char *fmt, va_list args);

    /// python-style type-safe printing with arguments indicated as {}
    /// ---> no positional arguments (eg {0} or {1})
    /// ---> no named arguments (eg {foo} or {bar})
    /// ---> no formatting of the arguments themselves
    /// If the above are needed, look into fmtlib https://github.com/fmtlib/fmt
    template <class... MoreArgs>
    void print(const char* fmt, MoreArgs&& ...more)
    {
        print_(fmt, std::forward<MoreArgs>(more)...);
    }

    /// R-style: append several arguments, formatting them each as a string
    /// and writing them together WITHOUT separation
    template <class T, class... MoreArgs>
    void cat(T const& arg, MoreArgs&& ...more)
    {
        *this << arg;
        cat(std::forward< MoreArgs >(more)...);
    }

    /// R-style: append several arguments, formatting them each as a string
    /// and writing them together separated by the given character
    template <class T, class... MoreArgs>
    void catsep(char sep, T const& arg, MoreArgs&& ...more)
    {
        *this << arg << sep;
        catsep(sep, std::forward<MoreArgs>(more)...);
    }
    template <class T>
    void catsep(char sep, T const& arg)
    {
        *this << arg;
    }

private:

    void cat() {} // terminate cat recursion

    template <class T, class... MoreArgs>
    void print_(const char* fmt, T const& arg, MoreArgs&& ...more)
    {
        size_t next = nextarg_(fmt); // where does the next argument token start?
        if(C4_UNLIKELY(next == size_t(-1))) // no more tokens were found
        {
            write(fmt); // write everything
            return;
        }
        write(fmt, next); // write the string up to the {} token
        fmt += next + 2; // prepare next call: advance the string (2 for the token)
        *this << arg; // write the argument
        print_(fmt, std::forward< MoreArgs >(more)...); // write more arguments
    }
    void print_(const char* fmt) // terminate the print recursion
    {
        write(fmt);
    }

    size_t nextarg_(const char *fmt);

};
template< class T > struct fmt_tag_ {};
template<> struct fmt_tag_< void *       > { static constexpr const char fmt[] = "%p"  ; };
template<> struct fmt_tag_< double       > { static constexpr const char fmt[] = "%lg" ; };
template<> struct fmt_tag_< float        > { static constexpr const char fmt[] = "%g"  ; };
template<> struct fmt_tag_< char         > { static constexpr const char fmt[] = "%c"  ; };
template<> struct fmt_tag_<  int64_t     > { static constexpr const char fmt[] = "%lld"; };
template<> struct fmt_tag_< uint64_t     > { static constexpr const char fmt[] = "%llu"; };
template<> struct fmt_tag_<  int32_t     > { static constexpr const char fmt[] = "%d"  ; };
template<> struct fmt_tag_< uint32_t     > { static constexpr const char fmt[] = "%u"  ; };
template<> struct fmt_tag_<  int16_t     > { static constexpr const char fmt[] = "%hd" ; };
template<> struct fmt_tag_< uint16_t     > { static constexpr const char fmt[] = "%hu" ; };
template<> struct fmt_tag_<  int8_t      > { static constexpr const char fmt[] = "%hhd"; };
template<> struct fmt_tag_< uint8_t      > { static constexpr const char fmt[] = "%hhu"; };
template<> struct fmt_tag_<       char * > { static constexpr const char fmt[] = "%s"  ; };
template<> struct fmt_tag_< const char * > { static constexpr const char fmt[] = "%s"  ; };

template< class T >
C4_ALWAYS_INLINE LogBuffer& operator<< (LogBuffer& ss, T const& var)
{
    using nctype = typename std::remove_const<T>::type;
    ss.printf(fmt_tag_<nctype>::fmt, var);
    return ss;
}
template< size_t N >
C4_ALWAYS_INLINE LogBuffer& operator<< (LogBuffer& ss, const char (&var)[N])
{
    ss.printf("%.*s", (int)(N-1), &var[0]);
    return ss;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class Log;

extern Log& log;

/**
 *
 */
class Log
{
public:

    typedef enum {
        ALWAYS = -10,
        ERR = -2,
        WARN = -1,
        INFO = 0,
        DEBUG = 1,
        TRACE1 = 2,
        TRACE2 = 3,
        TRACE3 = 4,
    } Level_e;

    typedef enum {
        SHOW_TIMESTAMP = 1 << 0,
        TO_TERM = 1 << 1,
        TO_FILE = 1 << 2,
        TO_STR  = 1 << 3,
        DEFAULT_MODE = SHOW_TIMESTAMP|TO_TERM,
    } Mode_e;

    /** a log channel, with a name and a log level */
    struct Channel
    {
        int8_t level;
        uint8_t name_len;
        char    name[30];

        Channel() : level{INFO}, name_len{0}, name{0} {}
        void init(const char *str, Level_e lev);
        bool cmp(const char *str) const;
        C4_ALWAYS_INLINE bool skip(Level_e lev) const
        {
            return lev > level && lev != ALWAYS;
        }
    };

    /** a RAII class to enable locking the log string while reading it */
    struct StrReader
    {
#ifdef C4_LOG_THREAD_SAFE
        StrReader(std::mutex &m, LogBuffer &s_) : lk(m, std::adopt_lock_t{}), s(s_) {}
        std::lock_guard< std::mutex > lk;
#else
        StrReader(LogBuffer &s_) : s(s_) {}
#endif
        LogBuffer &s;
        // do NOT read from c_str() after letting this object go out of scope
        const char* c_str() { return s.rd(); }
        void clear() { s.clear(); }
    };

    /** A proxy object which buffers prints to a log buffer.
     * It accumulates << calls and finishes outputting in the dtor. */
    struct Proxy
    {
        Channel const& channel;
        Level_e level;
        LogBuffer* buf; /* The buffer is set to NULL when the channel's log level
                         * is more verbose than the given log level. */
        Proxy(Channel const& ch, Level_e lev) : channel(ch), level(lev), buf(nullptr)
        {
            if(C4_LIKELY(channel.skip(level))) return; // optimize for verbose logs when quiet
            buf = &c4::log.buf();
            c4::log._print_prefix(channel, *buf);
        }
        ~Proxy()
        {
            if(C4_LIKELY(!buf)) return;
            c4::log.pump(buf->rd(), buf->pos);
            buf->clear();
        }
        template <class T>
        Proxy& operator<< (fastcref<T> v)
        {
            if(C4_LIKELY(!buf)) return;
            *buf << var;
            if(buf->pos > C4_LOG_BUFFER_REF_SIZE) // flush the buffer if it gets big
            {
                Log::pump(buf->rd(), buf->pos);
                buf->clear();
            }
        }
    };

public:

    /** select a proxy to log to the main channel at the given level */
    Proxy operator() (Level_e lev);
    /** select a proxy to log to the given channel at the given level */
    Proxy operator() (Channel* ch, Level_e lev = INFO);

    /** create a temporary proxy object to handle all the calls to <<.
     * It will accumulate the calls and output once after the last call. */
    template <class T>
    Proxy operator<< (fastcref<T> v)
    {
        Proxy s(main_channel(), INFO);
        s << v;
        return s;
    }

    // write() interface: raw string output (no formatting)

    void write  (                                  const char *s);
    void write  (                                  const char *s, size_t sz);
    void writel (                   Level_e level, const char *s);
    void writel (                   Level_e level, const char *s, size_t sz);
    void writecl(Channel const* ch, Level_e level, const char *s);
    void writecl(Channel const* ch, Level_e level, const char *s, size_t sz);

    // printf() interface

    void printf  (                                  const char *fmt, ...);
    void printfl (                   Level_e level, const char *fmt, ...);
    void printfcl(Channel const *ch, Level_e level, const char *fmt, ...);

    // python-like print() interface

    template <class... Args> void print  (                                  const char *fmt, Args&&... a) { printcl(main_channel(), INFO, fmt, std::forward< Args >(a)...); }
    template <class... Args> void printl (                   Level_e level, const char *fmt, Args&&... a) { printcl(main_channel(), level, fmt, std::forward< Args >(a)...); }
    template <class... Args> void printcl(Channel const* ch, Level_e level, const char *fmt, Args&&... a)
    {
        if(ch->skip(level)) return;
        auto &b = buf();
        _print_prefix(*ch, b);
        b.print(fmt, std::forward< Args >(a)...);
        pump(b.rd(), b.pos);
        b.clear();
    }

    // R-like cat() interface

    template <class... Args> void cat  (                                  Args&&... a) { catcl(main_channel(), INFO, std::forward< Args >(a)...); }
    template <class... Args> void catl (                   Level_e level, Args&&... a) { catcl(main_channel(), level, std::forward< Args >(a)...); }
    template <class... Args> void catcl(Channel const* ch, Level_e level, Args&&... a)
    {
        if(ch->skip(level)) return;
        auto &b = buf();
        _print_prefix(*ch, b);
        b.cat(std::forward< Args >(a)...);
        pump(b.rd(), b.pos);
        b.clear();
    }

    // R-like cat() interface, with separator

    template <class... Args> void catsep  (                                  Args&&... a, char sep) { catsepcl(main_channel(), INFO, std::forward< Args >(a)...); }
    template <class... Args> void catsepl (                   Level_e level, Args&&... a, char sep) { catsepcl(main_channel(), level, std::forward< Args >(a)...); }
    template <class... Args> void catsepcl(Channel const* ch, Level_e level, Args&&... a, char sep)
    {
        if(ch->skip(level)) return;
        auto &b = buf();
        _print_prefix(*ch, b);
        b.catsep(std::forward< Args >(a)..., sep);
        pump(b.rd(), b.pos);
        b.clear();
    }

public:

    Channel* add_channel(const char *name, Level_e lev = INFO);
    Channel* main_channel() const { return 0; }
    Channel* channel(const char *name) const;
    bool channel_exists(const char *name) const;
    bool channel_exists(Channel* h) const { return h >= &m_channels[0] && h < (&m_channels[0] + m_num_channels); }

    void flush();

    void level(Level_e l);

    void mode(uint8_t mode_flags) { m_mode = mode_flags; }
    uint8_t mode() { return m_mode; }

    /// specify a file to log to. The caller must ensure the file is available
    /// at any time that logging might occur.
    void file(FILE* f) { m_file = f; }
    FILE* file() { return m_file; }

    LogBuffer& buf();

    StrReader&& str();

    void pump(const char *str, size_t sz);

    void _print_prefix(Channel const& ch, LogBuffer &buf);

private:

    Channel m_channels[C4_LOG_MAX_CHANNELS];
    size_t m_num_channels = 0;
    FILE *m_file = nullptr;
#ifdef C4_LOG_THREAD_SAFE
    std::mutex m_strbuf_mtx;
#endif
    LogBuffer m_strbuf;
    uint8_t m_mode = DEFAULT_MODE;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#ifdef __clang__
    /* NOTE: using , ## __VA_ARGS__ to deal with zero-args calls to
     * variadic macros is not portable, but works in clang, gcc, msvc, icc.
     * clang requires switching off compiler warnings for pedantic mode.
     * @see http://stackoverflow.com/questions/32047685/variadic-macro-without-arguments */
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments" // warning: token pasting of ',' and __VA_ARGS__ is a GNU extension
#elif defined(__GNUC__)
    /* GCC also issues a warning for zero-args calls to variadic macros.
     * This warning is switched on with -pedantic and apparently there is no
     * easy way to turn it off as with clang. But marking this as a system
     * header works.
     * @see https://gcc.gnu.org/onlinedocs/cpp/System-Headers.html
     * @see http://stackoverflow.com/questions/35587137/ */
#   pragma GCC system_header
#endif

// Naming system for log macros:
// LOGP  = python-style log, type-safe.
// LOGF  = printf-style log
// LOGPC = python-style log, type-safe, to a specific channel
// LOGFC = printf-style log, to a specific channel

#define C4_LOGP(fmt, ...)        C4_LOGP_INFO(fmt, ## __VA_ARGS__)
#define C4_LOGF(fmt, ...)        C4_LOGF_INFO(fmt, ## __VA_ARGS__)
#define C4_LOGP_ERR(fmt, ...)    c4::log.printl (c4::Log::ERR,    fmt, ## __VA_ARGS__)
#define C4_LOGF_ERR(fmt, ...)    c4::log.printfl(c4::Log::ERR,    fmt, ## __VA_ARGS__)
#define C4_LOGP_WARN(fmt, ...)   c4::log.printl (c4::Log::WARN,   fmt, ## __VA_ARGS__)
#define C4_LOGF_WARN(fmt, ...)   c4::log.printfl(c4::Log::WARN,   fmt, ## __VA_ARGS__)
#define C4_LOGP_INFO(fmt, ...)   c4::log.printl (c4::Log::INFO,   fmt, ## __VA_ARGS__)
#define C4_LOGF_INFO(fmt, ...)   c4::log.printfl(c4::Log::INFO,   fmt, ## __VA_ARGS__)
#define C4_LOGP_DEBUG(fmt, ...)  c4::log.printl (c4::Log::DEBUG,  fmt, ## __VA_ARGS__)
#define C4_LOGF_DEBUG(fmt, ...)  c4::log.printfl(c4::Log::DEBUG,  fmt, ## __VA_ARGS__)
#define C4_LOGP_TRACE1(fmt, ...) c4::log.printl (c4::Log::TRACE1, fmt, ## __VA_ARGS__)
#define C4_LOGF_TRACE1(fmt, ...) c4::log.printfl(c4::Log::TRACE1, fmt, ## __VA_ARGS__)
#define C4_LOGP_TRACE2(fmt, ...) c4::log.printl (c4::Log::TRACE2, fmt, ## __VA_ARGS__)
#define C4_LOGF_TRACE2(fmt, ...) c4::log.printfl(c4::Log::TRACE2, fmt, ## __VA_ARGS__)
#define C4_LOGP_TRACE3(fmt, ...) c4::log.printl (c4::Log::TRACE3, fmt, ## __VA_ARGS__)
#define C4_LOGF_TRACE3(fmt, ...) c4::log.printfl(c4::Log::TRACE3, fmt, ## __VA_ARGS__)

#define C4_LOGPC(channel, fmt, ...)        C4_LOGPC_INFO(channel, fmt, ## __VA_ARGS__)
#define C4_LOGFC(channel, fmt, ...)        C4_LOGFC_INFO(channel, fmt, ## __VA_ARGS__)
#define C4_LOGPC_ERR(channel, fmt, ...)    c4::log.printcl (channel, c4::Log::ERR,    fmt, ## __VA_ARGS__)
#define C4_LOGFC_ERR(channel, fmt, ...)    c4::log.printfcl(channel, c4::Log::ERR,    fmt, ## __VA_ARGS__)
#define C4_LOGPC_WARN(channel, fmt, ...)   c4::log.printcl (channel, c4::Log::WARN,   fmt, ## __VA_ARGS__)
#define C4_LOGFC_WARN(channel, fmt, ...)   c4::log.printfcl(channel, c4::Log::WARN,   fmt, ## __VA_ARGS__)
#define C4_LOGPC_INFO(channel, fmt, ...)   c4::log.printcl (channel, c4::Log::INFO,   fmt, ## __VA_ARGS__)
#define C4_LOGFC_INFO(channel, fmt, ...)   c4::log.printfcl(channel, c4::Log::INFO,   fmt, ## __VA_ARGS__)
#define C4_LOGPC_DEBUG(channel, fmt, ...)  c4::log.printcl (channel, c4::Log::DEBUG,  fmt, ## __VA_ARGS__)
#define C4_LOGFC_DEBUG(channel, fmt, ...)  c4::log.printfcl(channel, c4::Log::DEBUG,  fmt, ## __VA_ARGS__)
#define C4_LOGPC_TRACE1(channel, fmt, ...) c4::log.printcl (channel, c4::Log::TRACE1, fmt, ## __VA_ARGS__)
#define C4_LOGFC_TRACE1(channel, fmt, ...) c4::log.printfcl(channel, c4::Log::TRACE1, fmt, ## __VA_ARGS__)
#define C4_LOGPC_TRACE2(channel, fmt, ...) c4::log.printcl (channel, c4::Log::TRACE2, fmt, ## __VA_ARGS__)
#define C4_LOGFC_TRACE2(channel, fmt, ...) c4::log.printfcl(channel, c4::Log::TRACE2, fmt, ## __VA_ARGS__)
#define C4_LOGPC_TRACE3(channel, fmt, ...) c4::log.printcl (channel, c4::Log::TRACE3, fmt, ## __VA_ARGS__)
#define C4_LOGFC_TRACE3(channel, fmt, ...) c4::log.printfcl(channel, c4::Log::TRACE3, fmt, ## __VA_ARGS__)

#ifdef __clang__
#   pragma clang diagnostic pop
#endif

C4_END_NAMESPACE(c4)

#endif /* _C4_LOG_HPP_ */
