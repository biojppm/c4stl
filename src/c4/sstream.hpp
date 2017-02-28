#ifndef _C4_SSTREAM_HPP_
#define _C4_SSTREAM_HPP_

#include "c4/config.hpp"
#include "c4/span.hpp"
#include "c4/string_fwd.hpp"

#include "c4/char_traits.hpp"
#include <inttypes.h>
#include <stdio.h> // vsnprintf, sscanf
#include <wchar.h>
#include <stdarg.h>

C4_BEGIN_NAMESPACE(c4)

template< class StringType = c4::string > class sstream;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** a string stream. */
template< class StringType >
class sstream
{

    StringType                     m_string;
    typename StringType::size_type m_putpos;
    typename StringType::size_type m_getpos;
    typename StringType::size_type m_status;

public:

    static constexpr const size_t npos = typename StringType::size_type(-1);

    using string_type = StringType;
    using traits_type = typename StringType::traits_type;
    using  char_type  = typename traits_type::char_type;
    using value_type  = typename StringType::value_type;
    using  size_type  = typename StringType::size_type;
    using  span_type  =  span<typename StringType::value_type, typename StringType::size_type>;
    using cspan_type  = cspan<typename StringType::value_type, typename StringType::size_type>;

    typedef enum {
        /** The write buffer has been exhausted and cannot be further increased.
         * Trying to write again will cause a C4_ERROR unless IGNORE_ERR is set. */
        EOFP = 0x01 << 0,
        /** The read buffer has been exhausted (ie, it has reached the end of the write buffer).
         * Trying to read again will cause a C4_ERROR unless IGNORE_ERR is set. */
        EOFG = 0x01 << 1,
        /** a reading error has occurred: could not convert the string to the argument's
         * intrinsic type. Trying to read further will cause a C4_ERROR unless IGNORE_ERR is set. */
        FAIL = 0x01 << 2,
        /** do not call C4_ERROR when errors occur; silently continue. Actions which
         * would overflow the buffer are silently skippedd. */
        IGNORE_ERR = 0x01 << 3,
    } Status_e;

public:

    template< class... Args >
    sstream(Args&&... string_ctor_args);
    ~sstream() = default;

    sstream(sstream const& that) = default;
    sstream(sstream     && that) = default;
    sstream& operator=(sstream const& that) = default;
    sstream& operator=(sstream     && that) = default;

    void move_in(StringType &&str);
    StringType&& move_out();

    void reset()
    {
        seekp(0);
        seekg(0);
        clear_err();
    }

    void reserve(size_type cap);
    C4_ALWAYS_INLINE size_type capacity() const { return m_string.capacity(); }

    size_type max_size() const { return m_string.max_size() - 1; }

public:

    /// get the full resulting span (from 0 to tellp)
    C4_ALWAYS_INLINE cspan_type strp() const noexcept { return cspan_type(m_string.data(), m_putpos); }
    /// get the current reading span (from tellg to to tellp)
    C4_ALWAYS_INLINE cspan_type strg() const C4_NOEXCEPT_X { C4_XASSERT(m_putpos >= m_getpos); return cspan_type(m_string.data() + m_getpos, m_putpos - m_getpos); }

    /// get the full resulting string (from 0 to tellp)
    C4_ALWAYS_INLINE const char* c_strp() const { C4_XASSERT(m_string[m_putpos] == '\0'); return m_string.data(); }
    /// get the current reading string (from tellg to tellp)
    C4_ALWAYS_INLINE const char* c_strg() const { C4_XASSERT(m_getpos < m_putpos && m_string[m_putpos] == '\0'); return m_string.data() + m_getpos; }

public:

    // Raw writing of characters

    ///< write a sequence of characters
    void write(const char *str, size_type sz);
    ///< write a sequence of characters
    C4_ALWAYS_INLINE void write(const wchar_t *str, size_type sz) { write(reinterpret_cast< const char* >(str), sz * sizeof(wchar_t)); } // invoke the char version
    ///< write a sequence of characters
    C4_ALWAYS_INLINE void write(cspan_type const& s) { write(s.data(), s.size()); }

    ///< read a sequence of characters
    void read(char *str, size_type sz);
    ///< read a sequence of characters
    C4_ALWAYS_INLINE void read(wchar_t *str, size_type sz) { read(reinterpret_cast< char* >(str), sz * sizeof(wchar_t)); } // invoke the char version
    ///< read a sequence of characters
    C4_ALWAYS_INLINE void read ( span_type & s) { read(s.data(), s.size()); }

public:

    /* C-style formatted print. */

    void printf(const char_type *fmt, ...) C4_ATTR_FORMAT(printf, 1, 2);
    void vprintf(const char_type *fmt, va_list args);

    /* Due to how std::*scanf() is implemented, we do not provide public
     * scanf functions, as it is impossible to determine the number
     * of characters read from the buffer unless a %n argument is
     * used as the last format spec. This is something which we have no way
     * of imposing on the clients (and could not act on, either way,
     * because we're blind to the arguments passed by the client, so we
     * cannot access them). */

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
    C4_ALWAYS_INLINE void printp(const char_type* fmt, Args&& ...more)
    {
        printp_(fmt, std::forward<Args>(more)...);
    }
    /** python-style type-safe formatted scan. Eg: stream.scanp("hello I am {} years old", age);
     *
     * @warning the format string is not checked against the stream; it is
     * merely used for skipping (the same number of) characters until the
     * next argument. */
    template <class... Args>
    C4_ALWAYS_INLINE void scanp(const char_type* fmt, Args&& ...more)
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
    C4_ALWAYS_INLINE void catsep(char_type sep, Args&& ...args)
    {
        catsep_(sep, std::forward<Args>(args)...);
    }
    template <class... Args>
    C4_ALWAYS_INLINE void uncatsep(char_type sep, Args&& ...args)
    {
        uncatsep_(std::forward<Args>(args)...);
    }

public:

    /// get the current write (ie, put) position
    C4_ALWAYS_INLINE size_type tellp() const { return m_putpos; }
    /// get the current read (ie, get) position
    C4_ALWAYS_INLINE size_type tellg() const { return m_getpos; }

    /// set the current write (ie, put) position
    C4_ALWAYS_INLINE void seekp(size_type p) { C4_CHECK(p <= capacity()); m_putpos = p; }
    /// set the current read (ie, get) position
    C4_ALWAYS_INLINE void seekg(size_type g) { C4_CHECK(g <= capacity() && g <= m_putpos); m_getpos = g; }

    /// advance the current write (ie, put) position
    C4_ALWAYS_INLINE void advp(size_type p) { C4_CHECK(capacity() - m_putpos >= p); m_putpos += p; }
    /// advance the current read (ie, get) position
    C4_ALWAYS_INLINE void advg(size_type g) { C4_CHECK(m_putpos - m_getpos >= g); m_getpos += g; }

    /// remaining size for writing (ie, put), WITHOUT terminating null character
    C4_ALWAYS_INLINE size_type remp() const { C4_XASSERT(m_putpos <= capacity()); return capacity() - m_putpos; }
    /// remaining size for reading (ie, get)
    C4_ALWAYS_INLINE size_type remg() const { C4_XASSERT(m_getpos <= m_putpos); return m_putpos - m_getpos; }

    /// true if sz chars can be written (from tellp to the current capacity)
    C4_ALWAYS_INLINE bool okp(size_type sz) const { return sz <= remp(); }
    /// true if sz chars can be read
    C4_ALWAYS_INLINE bool okg(size_type sz) const { return sz <= remg(); }

public:

    C4_ALWAYS_INLINE char_type get()       { char_type c; read(&c, 1); return c; }
    C4_ALWAYS_INLINE void put(char_type c) { write(&c, 1); return *this; }

    C4_ALWAYS_INLINE char_type peek() { return peek(1); }
    char_type peek(size_type ahead);

public:

    // error handling and recovery

    C4_ALWAYS_INLINE size_type stat() const { return m_status; }
    C4_ALWAYS_INLINE void clear_err() { m_status &= ~(EOFG|EOFP|FAIL); }

    C4_ALWAYS_INLINE void ignore_err(bool yes) { if(yes) m_status |= IGNORE_ERR; else m_status &= ~IGNORE_ERR; }
    C4_ALWAYS_INLINE bool ignore_err() const { return m_status & IGNORE_ERR; }

    C4_ALWAYS_INLINE bool eofp() const { return m_status & EOFP; }
    C4_ALWAYS_INLINE bool eofg() const { return m_status & EOFG; }
    C4_ALWAYS_INLINE bool fail() const { return m_status & FAIL; }

private:

    void errp_();
    void errg_();
    void errf_();

    void growto_(size_type sz);

    template <class T, class... MoreArgs>
    void printp_(const char_type* fmt, T const& arg, MoreArgs&& ...more);

    template <class T, class... MoreArgs>
    void scanp_(const char_type* fmt, T & arg, MoreArgs&& ...more);

    static size_type nextarg_(const char_type *fmt) ;

    template <class T, class... MoreArgs>
    void catsep_(char_type sep, T const& arg, MoreArgs&& ...more)
    {
        *this << arg << sep;
        catsep_(sep, std::forward<MoreArgs>(more)...);
    }
    template <class T >
    void catsep_(char_type sep, T const& arg)
    {
        *this << arg;
    }
    template <class T, class... MoreArgs>
    void uncatsep_(T & arg, MoreArgs&& ...more)
    {
        char_type sep;
        *this >> arg >> sep;
        uncatsep_(std::forward<MoreArgs>(more)...);
    }
    template <class T>
    void uncatsep_(T & arg)
    {
        *this >> arg;
    }

    // terminate recursion for variadic templates
    C4_ALWAYS_INLINE void cat() {}
    C4_ALWAYS_INLINE void uncat() {}
    C4_ALWAYS_INLINE void printp_(const char_type* fmt)
    {
        write(fmt, szconv< size_type >(traits_type::length(fmt)));
    }
    void scanp_(const char_type *fmt)
    {
        while(*fmt != char_type(0))
        {
            ++m_getpos; // skip as many chars as those from the fmt
            ++fmt;
        }
    }

    C4_ALWAYS_INLINE char_type      * buf_()       { return const_cast< char_type* >(m_string.data()); }
    C4_ALWAYS_INLINE const char_type* buf_() const { return                          m_string.data(); }

public:

    void scanf____(const char_type *fmt, void *arg);

};

//-----------------------------------------------------------------------------
/** a utility class for dispatching template types to their printf/scanf formats */
template< class CharType, class T > struct fmt_tag {};

/** define printf formats for a scalar (intrinsic) type */
#define _C4_DECLARE_FMT_TAG(_ty, fmtstr)                                 \
template<>                                                               \
struct fmt_tag< char, _ty >                                              \
{                                                                        \
    static constexpr const char fmt[] = fmtstr;                          \
    static constexpr const char fmtn[] = fmtstr "%n";                    \
};                                                                       \
template<>                                                               \
struct fmt_tag< wchar_t, _ty >                                           \
{                                                                        \
    static constexpr const wchar_t fmt[] = C4_WIDEN(fmtstr);             \
    static constexpr const wchar_t fmtn[] = C4_WIDEN(fmtstr "%n");       \
}

_C4_DECLARE_FMT_TAG(void*   , "%p"                );
_C4_DECLARE_FMT_TAG(char    , "%c"                );
_C4_DECLARE_FMT_TAG(wchar_t , "%c"                );
_C4_DECLARE_FMT_TAG(char*   , "%s"                );
_C4_DECLARE_FMT_TAG(wchar_t*, "%s"                );
_C4_DECLARE_FMT_TAG(double  , "%lg"               );
_C4_DECLARE_FMT_TAG(float   , "%g"                );
_C4_DECLARE_FMT_TAG( int8_t , "%" PRId8 /*"%hhd"*/);
_C4_DECLARE_FMT_TAG(uint8_t , "%" PRIu8 /*"%hhu"*/);
_C4_DECLARE_FMT_TAG( int16_t, "%" PRId16/*"%hd" */);
_C4_DECLARE_FMT_TAG(uint16_t, "%" PRIu16/*"%hu" */);
_C4_DECLARE_FMT_TAG( int32_t, "%" PRId32/*"%d"  */);
_C4_DECLARE_FMT_TAG(uint32_t, "%" PRIu32/*"%u"  */);
_C4_DECLARE_FMT_TAG( int64_t, "%" PRId64/*"%lld"*/);
_C4_DECLARE_FMT_TAG(uint64_t, "%" PRIu64/*"%llu"*/);


/** define chevron IO operators for a scalar (intrinsic) type */
#define _C4_DEFINE_CHEVRON_IO_OPERATORS(ty)                              \
template<class StringType>                                               \
C4_ALWAYS_INLINE                                                         \
sstream<StringType>& operator<< (sstream<StringType>& ss, ty var)        \
{                                                                        \
    ss.printf(fmt_tag< typename StringType::value_type, ty >::fmt, var); \
    return ss;                                                           \
}                                                                        \
template<class StringType>                                               \
C4_ALWAYS_INLINE                                                         \
sstream<StringType>& operator>> (sstream<StringType>& ss, ty& var)       \
{                                                                        \
    ss.scanf____(fmt_tag< typename StringType::value_type, ty >::fmtn, &var); \
    return ss;                                                           \
}

template< class StringType > C4_ALWAYS_INLINE sstream<StringType>& operator<< (sstream<StringType>& ss, char  var) { ss.write(&var, 1); return ss; }\
template< class StringType > C4_ALWAYS_INLINE sstream<StringType>& operator>> (sstream<StringType>& ss, char& var) { ss.read(&var, 1); return ss; }
template< class StringType > C4_ALWAYS_INLINE sstream<StringType>& operator<< (sstream<StringType>& ss, wchar_t  var) { ss.write(&var, 1); return ss; }\
template< class StringType > C4_ALWAYS_INLINE sstream<StringType>& operator>> (sstream<StringType>& ss, wchar_t& var) { ss.read(&var, 1); return ss; }

_C4_DEFINE_CHEVRON_IO_OPERATORS(void*   )
_C4_DEFINE_CHEVRON_IO_OPERATORS(char*   )
_C4_DEFINE_CHEVRON_IO_OPERATORS(wchar_t*)
_C4_DEFINE_CHEVRON_IO_OPERATORS( int8_t )
_C4_DEFINE_CHEVRON_IO_OPERATORS(uint8_t )
_C4_DEFINE_CHEVRON_IO_OPERATORS( int16_t)
_C4_DEFINE_CHEVRON_IO_OPERATORS(uint16_t)
_C4_DEFINE_CHEVRON_IO_OPERATORS( int32_t)
_C4_DEFINE_CHEVRON_IO_OPERATORS(uint32_t)
_C4_DEFINE_CHEVRON_IO_OPERATORS( int64_t)
_C4_DEFINE_CHEVRON_IO_OPERATORS(uint64_t)
_C4_DEFINE_CHEVRON_IO_OPERATORS(float   )
_C4_DEFINE_CHEVRON_IO_OPERATORS(double  )

template< class StringType, size_t N >
C4_ALWAYS_INLINE sstream<StringType>& operator<< (sstream<StringType>& ss, const char (&var)[N])
{
    using char_type = typename sstream<StringType>::char_type;
    ss.printf(C4_TXTTY("%.*s", char_type), (int)(N-1), &var[0]);
    return ss;
}
template< class StringType, size_t N >
C4_ALWAYS_INLINE sstream<StringType>& operator>> (sstream<StringType>& ss, char (&var)[N])
{
    using char_type = typename sstream<StringType>::char_type;
    ss.scanf(C4_TXTTY("%.*s", char_type), (int)(N-1), &var[0]);
    return ss;
}
template< class StringType, size_t N >
C4_ALWAYS_INLINE sstream<StringType>& operator<< (sstream<StringType>& ss, const wchar_t (&var)[N])
{
    using char_type = typename sstream<StringType>::char_type;
    ss.printf(C4_TXTTY("%.*s", char_type), (int)(N-1), &var[0]);
    return ss;
}
template< class StringType, size_t N >
C4_ALWAYS_INLINE sstream<StringType>& operator>> (sstream<StringType>& ss, wchar_t (&var)[N])
{
    using char_type = typename sstream<StringType>::char_type;
    ss.scanf(C4_TXTTY("%.*s", char_type), (int)(N-1), &var[0]);
    return ss;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// utility overloads for char and wchar_t printf/scanf

C4_ALWAYS_INLINE int vsnprintf(char *buffer, size_t buffer_size, const char* format, va_list list)
{
    return ::vsnprintf(buffer, buffer_size, format, list);
}
C4_ALWAYS_INLINE int vsnprintf(wchar_t *buffer, size_t buffer_size, const wchar_t* format, va_list list)
{
    return ::vswprintf(buffer, buffer_size, format, list);
}
inline int sscanf(const char* buffer, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int ret = ::vsscanf(buffer, format, args);
    va_end(args);
    return ret;
}
inline int sscanf(const wchar_t* buffer, const wchar_t* format, ...)
{
    va_list args;
    va_start(args, format);
    int ret = ::vswscanf(buffer, format, args);
    va_end(args);
    return ret;
}

C4_END_NAMESPACE(c4)

#include "c4/sstream.def.hpp"

#endif /* _C4_SSTREAM_HPP_ */
