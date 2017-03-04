#ifndef _C4_ERROR_HPP_
#define _C4_ERROR_HPP_

/** @file error.hpp Facilities for error reporting and runtime assertions. */

#include "c4/config.hpp"

/** @def C4_ERROR_THROWS_EXCEPTION if this is defined and exceptions are
 * enabled, then calls to C4_ERROR() will throw an exception */

/** @def C4_NOEXCEPT evaluates to noexcept when C4_ERROR might be called
 * and exceptions are disabled. Otherwise, defaults to nothing. */

/** @def C4_NOEXCEPT_A evaluates to noexcept when C4_ASSERT is disabled.
 * Otherwise, defaults to nothing. */

/** @def C4_NOEXCEPT_X evaluates to noexcept when C4_XASSERT is disabled.
 * Otherwise, defaults to nothing.  */

#if defined(C4_EXCEPTIONS_ENABLED) && defined(C4_ERROR_THROWS_EXCEPTION)
#   define C4_NOEXCEPT
#else
#   define C4_NOEXCEPT noexcept
#endif

//-----------------------------------------------------------------------------

#ifdef NDEBUG
#   define C4_DEBUG_BREAK()
#else
#   include <debugbreak/debugbreak.h>
#   define C4_DEBUG_BREAK() if(c4::is_debugger_attached()) { ::debug_break(); }
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

bool is_debugger_attached();


typedef enum {
    /** when an error happens and the debugger is attached, call C4_DEBUG_BREAK().
     * Without effect otherwise. */
    ON_ERROR_DEBUGBREAK = 0x01 << 0,
    /** when an error happens log a message. */
    ON_ERROR_LOG = 0x01 << 1,
    /** when an error happens invoke a callback if it was set with
     * set_error_callback(). */
    ON_ERROR_CALLBACK = 0x01 << 2,
    /** when an error happens call std::terminate(). */
    ON_ERROR_ABORT = 0x01 << 3,
    /** when an error happens and exceptions are enabled throw an exception.
     * Without effect otherwise. */
    ON_ERROR_THROW = 0x01 << 4,
    /** the default flags. */
    ON_ERROR_DEFAULTS = ON_ERROR_DEBUGBREAK|ON_ERROR_LOG|ON_ERROR_CALLBACK|ON_ERROR_ABORT
} ErrorFlags_e;
using error_flags = int;
void set_error_flags(error_flags f);
error_flags get_error_flags();


using error_callback_type = void (*)(const char* msg, size_t msg_size);
void set_error_callback(error_callback_type cb);
error_callback_type get_error_callback();


//-----------------------------------------------------------------------------
/** RAII to control the error settings inside a scope. */
struct ScopedErrorSettings
{
    error_flags m_flags;
    error_callback_type m_callback;

    explicit ScopedErrorSettings(error_callback_type cb)
    :   m_flags(get_error_flags()),
        m_callback(get_error_callback())
    {
        set_error_callback(cb);
    }
    explicit ScopedErrorSettings(error_flags flags)
    :   m_flags(get_error_flags()),
        m_callback(get_error_callback())
    {
        set_error_flags(flags);
    }
    explicit ScopedErrorSettings(error_flags flags, error_callback_type cb)
    :   m_flags(get_error_flags()),
        m_callback(get_error_callback())
    {
        set_error_flags(flags);
        set_error_callback(cb);
    }
    ~ScopedErrorSettings()
    {
        set_error_flags(m_flags);
        set_error_callback(m_callback);
    }
};

//-----------------------------------------------------------------------------

#if defined(C4_ERROR_SHOWS_FILELINE) && defined(C4_ERROR_SHOWS_FUNC)

void handle_error(const char *file, int line, const char *func, const char *fmt, ...);
void handle_warning(const char *file, int line, const char *func, const char *fmt, ...);

#   define C4_ERROR(msg, ...)                             \
    if(c4::get_error_flags() & ON_ERROR_DEBUGBREAK) { C4_DEBUG_BREAK() } \
    c4::handle_error(__FILE__, __LINE__, C4_PRETTY_FUNC, msg, ## __VA_ARGS__)

#   define C4_WARNING(msg, ...)                                         \
    c4::handle_warning(__FILE__, __LINE__, C4_PRETTY_FUNC, msg, ## __VA_ARGS__)

#elif defined(C4_ERROR_SHOWS_FILELINE)

void handle_error(const char *file, int line, const char *fmt, ...);
void handle_warning(const char *file, int line, const char *fmt, ...);

#   define C4_ERROR(msg, ...)                             \
    if(c4::get_error_flags() & ON_ERROR_DEBUGBREAK) { C4_DEBUG_BREAK() } \
    c4::handle_error(__FILE__, __LINE__, msg, ## __VA_ARGS__)

#   define C4_WARNING(msg, ...)                                 \
    c4::handle_warning(__FILE__, __LINE__, msg, ## __VA_ARGS__)

#elif ! defined(C4_ERROR_SHOWS_FUNC)

void handle_error(const char *fmt, ...);
void handle_warning(const char *fmt, ...);

#   define C4_ERROR(msg, ...)                             \
    if(c4::get_error_flags() & ON_ERROR_DEBUGBREAK) { C4_DEBUG_BREAK() } \
    c4::handle_error(msg, ## __VA_ARGS__)

#   define C4_WARNING(msg, ...)                 \
    c4::handle_warning(msg, ## __VA_ARGS__)

#else
#   error not implemented
#endif

//-----------------------------------------------------------------------------
// assertions - only in debug builds
#ifdef NDEBUG // turn off assertions
#   define C4_ASSERT(cond)
#   define C4_ASSERT_MSG(cond, fmt, ...)
#   define C4_NOEXCEPT_A noexcept
#else
#   define C4_ASSERT(cond) C4_CHECK(cond)
#   define C4_ASSERT_MSG(cond, fmt, ...) C4_CHECK_MSG(cond, fmt, ## __VA_ARGS__)
#   define C4_NOEXCEPT_A C4_NOEXCEPT
#endif

// Extreme assertion: can be switched off independently of the regular assertion.
// Use eg for bounds checking in hot code.
#ifdef C4_USE_XASSERT
#   define C4_XASSERT(cond) C4_CHECK(cond)
#   define C4_XASSERT_MSG(cond, fmt, ...) C4_CHECK_MSG(cond, fmt, ## __VA_ARGS__)
#   define C4_NOEXCEPT_X C4_NOEXCEPT
#else
#   define C4_XASSERT(cond)
#   define C4_XASSERT_MSG(cond, fmt, ...))
#   define C4_NOEXCEPT_X noexcept
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
