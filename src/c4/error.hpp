#ifndef _C4_ERROR_HPP_
#define _C4_ERROR_HPP_

#include "c4/config.hpp"

/** @def C4_ERROR_THROWS_EXCEPTION if this is defined and exceptions are
 * enabled, then calls to C4_ERROR() will throw an exception */

 /** @def C4_NOEXCEPT evaluates to noexcept when C4_ERROR might be called
 * and exceptions are disabled. */

#ifdef C4_ERROR_THROWS_EXCEPTION
#   define C4_NOEXCEPT
#else
#   define C4_NOEXCEPT noexcept
#endif

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

//-----------------------------------------------------------------------------

C4_BEGIN_NAMESPACE(c4)

using error_callback_type = void (*)();

void set_error_callback(error_callback_type cb);
error_callback_type get_error_callback();


void report_error    (                                              const char *fmt, ...);
void report_error_fl (const char *file, int line,                   const char *fmt, ...);
void report_error_flf(const char *file, int line, const char *func, const char *fmt, ...);


void report_warning    (                                              const char *fmt, ...);
void report_warning_fl (const char *file, int line,                   const char *fmt, ...);
void report_warning_flf(const char *file, int line, const char *func, const char *fmt, ...);


//-----------------------------------------------------------------------------
/** RAII sets the error callback inside a scope. */
struct ScopedErrorCallback
{
    error_callback_type m_original;

    explicit ScopedErrorCallback(error_callback_type cb)
        : m_original(get_error_callback())
    {
        set_error_callback(cb);
    }
    ~ScopedErrorCallback()
    {
        set_error_callback(m_original);
    }
};


//-----------------------------------------------------------------------------
/** Raise an error, and report a printf-formatted message.
 * If an error callback was set, it will be called.
 * @see set_error_callback() */
#if defined(C4_ERROR_SHOWS_FILELINE) && defined(C4_ERROR_SHOWS_FUNC)

#   define C4_ERROR(msg, ...)                                           \
    c4::report_error_flf(__FILE__, __LINE__, C4_PRETTY_FUNC, msg, ## __VA_ARGS__)

#elif defined(C4_ERROR_SHOWS_FILELINE)

#   define C4_ERROR(msg, ...)                                       \
    c4::report_error_fl(__FILE__, __LINE__, msg, ## __VA_ARGS__)

#elif ! defined(C4_ERROR_SHOWS_FUNC)

#   define C4_ERROR(msg, ...)                   \
    c4::report_error(msg, ## __VA_ARGS__)

#else
#   error not implemented
#endif


//-----------------------------------------------------------------------------
/** Report a warning with a printf-formatted message. */
#if defined(C4_ERROR_SHOWS_FILELINE) && defined(C4_ERROR_SHOWS_FUNC)

#   define C4_WARNING(msg, ...)                                         \
    c4::report_warning_flf(__FILE__, __LINE__, C4_PRETTY_FUNC, msg, ## __VA_ARGS__)

#elif defined(C4_ERROR_SHOWS_FILELINE)
#   define C4_WARNING(msg, ...)                                 \
    c4::report_warning_fl(__FILE__, __LINE__, msg, ## __VA_ARGS__)

#elif ! defined(C4_ERROR_SHOWS_FUNC)
#   define C4_WARNING(msg, ...)                 \
    c4::report_warning(msg, ## __VA_ARGS__)

#else
#   error not implemented
#endif


//-----------------------------------------------------------------------------
// assertions - only in debug builds
#ifdef NDEBUG // turn off assertions
#   define C4_ASSERT(cond)
#   define C4_ASSERT_MSG(cond, fmt, ...)
#else
#   define C4_ASSERT(cond) C4_CHECK(cond)
#   define C4_ASSERT_MSG(cond, fmt, ...) C4_CHECK_MSG(cond, fmt, ## __VA_ARGS__)
#endif

// Extreme assertion: can be switched off independently of the regular assertion.
// Use eg for bounds checking in hot code.
#ifdef C4_USE_XASSERT
#   define C4_XASSERT(cond) C4_CHECK(cond)
#   define C4_XASSERT_MSG(cond, fmt, ...) C4_CHECK_MSG(cond, fmt, ## __VA_ARGS__)
#else
#   define C4_XASSERT(cond)
#   define C4_XASSERT_MSG(cond, fmt, ...)
#endif


//-----------------------------------------------------------------------------
/** Check that a condition is true, or raise an error when not
 *  true. Unlike C4_ASSERT, this check is not omitted in non-debug
 *  builds.
 *  @see C4_ASSERT */
#define C4_CHECK(cond)                          \
    if(C4_UNLIKELY(!(cond)))                    \
    {                                           \
        C4_ERROR("check failed: %s", #cond);    \
    }

/** like C4_CHECK(), and additionally log a printf-style message.
 * @see C4_CHECK */
#define C4_CHECK_MSG(cond, fmt, ...)                                \
    if(C4_UNLIKELY(!(cond)))                                        \
    {                                                               \
        C4_ERROR("check failed: %s\n" fmt, #cond, ## __VA_ARGS__);  \
    }

//-----------------------------------------------------------------------------
// Common error conditions
#define C4_NOT_IMPLEMENTED() C4_ERROR("NOT IMPLEMENTED")
#define C4_NOT_IMPLEMENTED_MSG(msg, ...) C4_ERROR("NOT IMPLEMENTED: " msg, ## __VA_ARGS__)
#define C4_NOT_IMPLEMENTED_IF(condition) if(C4_UNLIKELY(condition)) { C4_ERROR("NOT IMPLEMENTED"); }
#define C4_NOT_IMPLEMENTED_IF_MSG(condition, msg, ...) if(C4_UNLIKELY(condition)) { C4_ERROR("NOT IMPLEMENTED: " msg, ## __VA_ARGS__); }

#define C4_NEVER_REACH() C4_UNREACHABLE(); C4_ERROR("never reach this point")
#define C4_NEVER_REACH_MSG(msg, ...) C4_UNREACHABLE(); C4_ERROR("never reach this point: " msg, ## __VA_ARGS__)

C4_END_NAMESPACE(c4)

#ifdef __clang__
#   pragma clang diagnostic pop
#endif

#endif /* _C4_ERROR_HPP_ */
