#ifndef _C4_SSTREAM_HPP_
#define _C4_SSTREAM_HPP_

#include "c4/config.hpp"
#include "c4/span.hpp"

#include <limits>
#include <inttypes.h>

C4_BEGIN_NAMESPACE(c4)

//-----------------------------------------------------------------------------
template< class T > struct fmt_tag {};

#define _C4_DEFINE_FMT_TAG(_ty, fmtstr) \
template<>\
struct fmt_tag< _ty >\
{\
    static constexpr const char fmt[] = fmtstr;\
    static constexpr const char fmt_with_num_chars_arg[] = fmtstr "%n";\
}

_C4_DEFINE_FMT_TAG(void *  , "%p"               );
_C4_DEFINE_FMT_TAG(char    , "%c"               );
_C4_DEFINE_FMT_TAG(char *  , "%s"               );
_C4_DEFINE_FMT_TAG(double  , "%lg"              );
_C4_DEFINE_FMT_TAG(float   , "%g"               );
_C4_DEFINE_FMT_TAG( int64_t, /*"%lld"*/"%"PRId64);
_C4_DEFINE_FMT_TAG(uint64_t, /*"%llu"*/"%"PRIu64);
_C4_DEFINE_FMT_TAG( int32_t, /*"%d"  */"%"PRId32);
_C4_DEFINE_FMT_TAG(uint32_t, /*"%u"  */"%"PRIu32);
_C4_DEFINE_FMT_TAG( int16_t, /*"%hd" */"%"PRId16);
_C4_DEFINE_FMT_TAG(uint16_t, /*"%hu" */"%"PRIu16);
_C4_DEFINE_FMT_TAG( int8_t , /*"%hhd"*/"%"PRId8 );
_C4_DEFINE_FMT_TAG(uint8_t , /*"%hhu"*/"%"PRIu8 );

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class sstream
{
private:

    char* m_buf;
    size_t m_size;

    size_t m_putpos;
    size_t m_getpos;

    size_t m_status;

public:

    static constexpr const size_t npos = size_t(-1);

    using  char_type = char;
    using  size_type = size_t;
    using  span_type =  span<char, size_t>;
    using cspan_type = cspan<char, size_t>;

public:

    sstream();
    ~sstream();

    sstream(sstream const&) = delete;
    sstream(sstream     &&) = default;
    sstream& operator=(sstream const&) = delete;
    sstream& operator=(sstream     &&) = default;

    constexpr size_t max_size() const
    {
        return std::numeric_limits< size_t >::max() - 1;
    }

    void reset()
    {
        seekp(0);
        seekg(0);
        clear_err();
    }

public:

    // conversion to/from string

    /* COMING UP

    template< class T >
    const char* to_string(T const& v);

    template< class T >
    void from_string(const char *s, size_t size, T& v);
    */

public:

    // Raw writing of characters

    void write(const char *str, size_t sz); ///< write a sequence of characters
    void read (char *str, size_t sz); ///< read a sequence of characters

    ///< write a null-terminated sequence of characters
    void write(const char *cstr);

    ///< write a sequence of characters
    C4_ALWAYS_INLINE void write(cspan_type const& s) { write(s.data(), s.size()); }
    ///< read a sequence of characters
    C4_ALWAYS_INLINE void read ( span_type & s) { read(s.data(), s.size()); }

public:

    /* C-style formatted print. */

    void printf(const char *fmt, ...) C4_ATTR_FORMAT(printf, 1, 2);
    void vprintf(const char *fmt, va_list args);

    /* Due to how std::*scanf() is implemented, we do not provide public
     * scanf functions, as it is impossible to determine the number
     * of characters read from the buffer unless a %n argument is
     * used as the last format spec. This is something which we have no way
     * of imposing on the clients (and could not act on, either way,
     * because we're blind to the arguments passed by the client). */

public:

    /* python-style type-safe write/read with arguments indicated as {}.
     * This is a minimal version:
     * ---> no positional arguments (eg {0} or {1})
     * ---> no named arguments (eg {foo} or {bar})
     * ---> no formatting of the arguments themselves
     * If any the above are needed then consider the C-style printf/scanf functions.
     * If type-safety is needed, then consider using fmtlib: https://github.com/fmtlib/fmt */

    /** python-style type-safe formatted print. Eg: stream.printp("hello I am {} years old", age); */
    template <class... Args>
    C4_ALWAYS_INLINE void printp(const char* fmt, Args&& ...more)
    {
        printp_(fmt, std::forward<Args>(more)...);
    }
    /** python-style type-safe formatted scan. Eg: stream.scanp("hello I am {} years old", age);
     *
     * @warning the format string is not checked against the stream; it is
     * merely used for skipping (the same number of) characters until the
     * next argument. */
    template <class... Args>
    C4_ALWAYS_INLINE void scanp(const char* fmt, Args&& ...more)
    {
        scanp_(fmt, std::forward<Args>(more)...);
    }

public:

    // R-style: append several arguments, formatting them each as a string
    // and writing them together WITHOUT separation

    /// put the given arguments into the stream, without any separation character
    template <class T, class... MoreArgs>
    void cat(T const& arg, MoreArgs&& ...more)
    {
        *this << arg;
        cat(std::forward< MoreArgs >(more)...);
    }

    /// read the given arguments from the stream, without any separation character
    template <class T, class... MoreArgs>
    void uncat(T & arg, MoreArgs&& ...more)
    {
        *this >> arg;
        uncat(std::forward< MoreArgs >(more)...);
    }

public:

    /** R-style: append several arguments, formatting them each as a string
     * and writing them together separated by the given character */
    template <class... Args>
    C4_ALWAYS_INLINE void catsep(char sep, Args&& ...args)
    {
        catsep_(sep, std::forward<Args>(args)...);
    }
    template <class... Args>
    C4_ALWAYS_INLINE void uncatsep(char sep, Args&& ...args)
    {
        uncatsep_(std::forward<Args>(args)...);
    }

public:

    /// get the full resulting string (from 0 to tellp)
    C4_ALWAYS_INLINE const char* c_str() const { C4_XASSERT(m_buf[m_putpos] == '\0'); return m_buf; }
    /// get the current reading string (from tellg to tellp)
    C4_ALWAYS_INLINE const char* c_strg() const { C4_XASSERT(m_getpos < m_putpos && m_buf[m_putpos] == '\0'); return m_buf + m_getpos; }

    /// get the full resulting span (from 0 to tellp)
    C4_ALWAYS_INLINE cspan_type span() const { return cspan_type(m_buf, m_putpos); }
    /// get the current reading span (from tellg to to tellp)
    C4_ALWAYS_INLINE cspan_type spang() const { return cspan_type(m_buf + m_getpos, m_putpos - m_getpos); }

public:

    /// get the current write (ie, put) position
    C4_ALWAYS_INLINE size_t tellp() const { return m_putpos; }
    /// get the current read (ie, get) position
    C4_ALWAYS_INLINE size_t tellg() const { return m_getpos; }

    /// set the current write (ie, put) position
    C4_ALWAYS_INLINE void seekp(size_t p) { C4_CHECK(p <= m_size); m_putpos = p; }
    /// set the current read (ie, get) position
    C4_ALWAYS_INLINE void seekg(size_t g) { C4_CHECK(g <= m_size && g <= m_putpos); m_getpos = g; }

    /// advance the current write (ie, put) position
    C4_ALWAYS_INLINE void advp(size_t p) { C4_CHECK(m_size - m_putpos >= p); m_putpos += p; }
    /// advance the current read (ie, get) position
    C4_ALWAYS_INLINE void advg(size_t g) { C4_CHECK(m_putpos - m_getpos >= g); m_getpos += g; }

    /// remaining size for writing (ie, put), WITHOUT terminating null character
    C4_ALWAYS_INLINE size_t remp() const { C4_XASSERT(m_putpos <= m_size); return m_size - m_putpos; }
    /// remaining size for reading (ie, get)
    C4_ALWAYS_INLINE size_t remg() const { C4_XASSERT(m_getpos <= m_putpos); return m_putpos - m_getpos; }

    /// true if sz chars can be written (from tellp to the current max_size())
    C4_ALWAYS_INLINE bool okp(size_t sz) const { return sz <= (max_size() - m_putpos); }
    /// true if sz chars can be read (from tellg to the current tellp)
    C4_ALWAYS_INLINE bool okg(size_t sz) const { return sz <= remg(); }

public:

    C4_ALWAYS_INLINE char     get()        { char c; read(&c, 1); return c; }
    C4_ALWAYS_INLINE sstream& get(char &c) { read(&c, 1); return *this; }
    C4_ALWAYS_INLINE sstream& put(char c)  { write(&c, 1); return *this; }

    C4_ALWAYS_INLINE char peek() { return peek(1); }
    char peek(size_t ahead);

public:

    // error handling and recovery

    typedef enum {
        /** The write buffer has been exhausted and cannot be further increased.
         * Trying to write again will cause a C4_ERROR unless IGNORE_ERR is set. */
        EOFP = 0x01,
        /** The read buffer has been exhausted (ie, it has reached the end of the write buffer).
         * Trying to read again will cause a C4_ERROR unless IGNORE_ERR is set. */
        EOFG = 0x01 << 1,
        /** a reading error has occurred: could not convert the string to the argument's
         * intrinsic type. Trying to read further will cause a C4_ERROR unless IGNORE_ERR is set. */
        FAIL = 0x01 << 2,
        /** do not call C4_ERROR when errors occur; silently continue. */
        IGNORE_ERR = 0x01 << 3
    } Status_e;

    C4_ALWAYS_INLINE size_t    stat() const { return m_status; }
    C4_ALWAYS_INLINE void clear_err() { m_status &= ~(EOFG|EOFP|FAIL); }

    C4_ALWAYS_INLINE void ignore_err(bool yes) { if(yes) m_status |= IGNORE_ERR; else m_status &= ~IGNORE_ERR; }
    C4_ALWAYS_INLINE bool ignore_err() const { return m_status & IGNORE_ERR; }

    C4_ALWAYS_INLINE bool eofp() const { return m_status & EOFP; }
    C4_ALWAYS_INLINE bool eofg() const { return m_status & EOFG; }
    C4_ALWAYS_INLINE bool fail() const { return m_status & FAIL; }

private:

    void errp_();
    void errg_();

    void resize_(size_t sz);
    bool growto_(size_t sz);

    template <class T, class... MoreArgs>
    void printp_(const char* fmt, T const& arg, MoreArgs&& ...more);

    template <class T, class... MoreArgs>
    void scanp_(const char* fmt, T & arg, MoreArgs&& ...more);

    static size_t nextarg_(const char *fmt) ;

    template <class T, class... MoreArgs>
    void catsep_(char sep, T const& arg, MoreArgs&& ...more)
    {
        *this << arg << sep;
        catsep_(sep, std::forward<MoreArgs>(more)...);
    }
    template <class T, class... MoreArgs>
    void uncatsep_(T & arg, MoreArgs&& ...more)
    {
        char sep;
        *this >> arg >> sep;
        uncatsep_(std::forward<MoreArgs>(more)...);
    }

    // terminate recursion for variadic templates
    C4_ALWAYS_INLINE void cat() {}
    C4_ALWAYS_INLINE void uncat() {}
    C4_ALWAYS_INLINE void catsep_(char) {}
    C4_ALWAYS_INLINE void uncatsep_() {}
    C4_ALWAYS_INLINE void printp_(const char* fmt)
    {
        write(fmt);
    }
    void scanp_(const char *fmt)
    {
        while(*fmt != '\0')
        {
            ++m_getpos; // skip as many chars as those from the fmt
            ++fmt;
        }
    }

public:

    void scanf____(const char *fmt, void *arg);

};

//-----------------------------------------------------------------------------
template< class T, C4_REQUIRE_T(std::is_scalar< T >::value) >
C4_ALWAYS_INLINE sstream& operator<< (sstream& ss, T const& var)
{
    using tag = fmt_tag< typename std::remove_const<T>::type >;
    ss.printf(tag::fmt, var);
    return ss;
}
template< class T, C4_REQUIRE_T(std::is_scalar< T >::value) >
C4_ALWAYS_INLINE sstream& operator>> (sstream& ss, T & var)
{
    using tag = fmt_tag< typename std::remove_const<T>::type >;
    ss.scanf____(tag::fmt_with_num_chars_arg, (void*)&var);
    return ss;
}
C4_ALWAYS_INLINE sstream& operator<< (sstream& ss, char const& var)
{
    ss.write(&var, 1);
    return ss;
}
C4_ALWAYS_INLINE sstream& operator>> (sstream& ss, char & var)
{
    ss.read(&var, 1);
    return ss;
}

template< size_t N >
C4_ALWAYS_INLINE sstream& operator<< (sstream& ss, const char (&var)[N])
{
    ss.printf("%.*s", (int)(N-1), &var[0]);
    return ss;
}
template< size_t N >
C4_ALWAYS_INLINE sstream& operator>> (sstream& ss, char (&var)[N])
{
    ss.scanf("%.*s", (int)(N-1), &var[0]);
    return ss;
}

C4_END_NAMESPACE(c4)

#include "c4/sstream.def.hpp"

#endif /* _C4_SSTREAM_HPP_ */
