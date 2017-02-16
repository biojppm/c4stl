#include "c4/error.hpp"
#include "c4/sstream.hpp"
#include "c4/log.hpp"

#include <stdlib.h>

#if defined(C4_XBOX) || (defined(C4_WIN) && defined(C4_MSVC))
#   include "c4/windows.hpp"
#elif defined(C4_PS4)
#   include <libdbg.h>
#elif defined(C4_UNIX) || defined(C4_LINUX)
#   include <sys/stat.h>
#   include <cstring>
#   include <fcntl.h>
#   include <unistd.h>
#endif

#ifdef C4_ERROR_THROWS_EXCEPTION
#   include <exception>
#endif

//-----------------------------------------------------------------------------
C4_BEGIN_NAMESPACE(c4)

void _handle_error()
{
#ifdef C4_ERROR_THROWS_EXCEPTION
    throw Exception();
#else
    std::terminate();
#endif
}

static error_callback_type _error_callback = &_handle_error;

//-----------------------------------------------------------------------------

/** Defaults to abort() */
error_callback_type get_error_callback()
{
    return _error_callback;
}
/** Set the function which is called when an error occurs. */
void set_error_callback(error_callback_type cb)
{
    _error_callback = cb;
}

//-----------------------------------------------------------------------------
void _log_errwarn(const char *what, const char *file, int line, const char* func,
                  const char *fmt, va_list args)
{
    sstream ss;
    ss.vprintf(fmt, args);
    C4_LOGF_ERR("\n");
    if(file == nullptr && line == 0 && func == nullptr)
    {
        C4_LOGF_ERR("%s: %s\n", what, ss.c_str());
    }
    else if(file != nullptr && line != 0 && func == nullptr)
    {
        C4_LOGF_ERR("%s: %s:%d: %s\n", what, file, line, ss.c_str());
    }
    else
    {
        C4_LOGF_ERR("%s: %s:%d: %s\n", what, file, line, ss.c_str());
        C4_LOGF_ERR("%s: %s:%d: here: %s\n", what, file, line, func);
    }
    c4::log.flush();
}

//-----------------------------------------------------------------------------
/** Raise an error, and report a printf-formatted message.
 * If an error callback was set, it will be called.
 * @see set_error_callback() */
void report_error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    _log_errwarn("ERROR", nullptr, 0, nullptr, fmt, args);
    va_end(args);
    auto fn = get_error_callback();
    if(fn)
    {
        fn();
    }
}

void report_warning(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    _log_errwarn("WARNING", nullptr, 0, nullptr, fmt, args);
    va_end(args);
}

/** Raise an error, and report a printf-formatted message.
 * If an error callback was set, it will be called.
 * @see set_error_callback() */
void report_error_fl(const char *file, int line, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    _log_errwarn("ERROR", file, line, nullptr, fmt, args);
    va_end(args);
    auto fn = get_error_callback();
    if(fn)
    {
        fn();
    }
}

/** Report a printf-formatted warning message. */
void report_warning_fl(const char *file, int line, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    _log_errwarn("WARNING", file, line, nullptr, fmt, args);
    va_end(args);
}

/** Raise an error, and report a printf-formatted message.
 * If an error callback was set, it will be called.
 * @see set_error_callback() */
void report_error_flf(const char *file, int line, const char *func, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    _log_errwarn("ERROR", file, line, func, fmt, args);
    va_end(args);
    auto fn = get_error_callback();
    if(fn)
    {
        fn();
    }
}

/** Report a printf-formatted warning message. */
void report_warning_flf(const char *file, int line, const char *func, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    _log_errwarn("ERROR", file, line, func, fmt, args);
    va_end(args);
}

//-----------------------------------------------------------------------------
bool is_debugger_attached()
{
#if defined(C4_PS4)
    return (sceDbgIsDebuggerAttached() != 0);
#elif defined(C4_XBOX) || (defined(C4_WIN) && defined(C4_MSVC))
    return IsDebuggerPresent();
#elif defined(C4_UNIX) || defined(C4_LINUX)
    static bool first_call = true;
    static bool first_call_result = false;
    if(first_call)
    {
        first_call = true;
        //! @see http://stackoverflow.com/questions/3596781/how-to-detect-if-the-current-process-is-being-run-by-gdb
        //! (this answer: http://stackoverflow.com/a/24969863/3968589 )
        char buf[1024] = "";

        int status_fd = open("/proc/self/status", O_RDONLY);
        if (status_fd == -1)
        {
            return 0;
        }

        ssize_t num_read = ::read(status_fd, buf, sizeof(buf));

        if (num_read > 0)
        {
            static const char TracerPid[] = "TracerPid:";
            char *tracer_pid;

            buf[num_read] = 0;
            tracer_pid    = strstr(buf, TracerPid);
            if (tracer_pid)
            {
                first_call_result = !!atoi(tracer_pid + sizeof(TracerPid) - 1);
            }
        }
    }
    return first_call_result;
#else
    C4_NOT_IMPLEMENTED();
    return false;
#endif
}

C4_END_NAMESPACE(c4)
