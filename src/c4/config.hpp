#ifndef _C4_UTIL_HPP_
#define _C4_UTIL_HPP_

/** @defgroup basic_headers Basic headers
 * Headers providing basic macros, platform+cpu+compiler information,
 * C++ facilities and basic typedefs. */

/** @file config.hpp Contains configuration defines and includes the basic_headers.
 * @ingroup basic_headers */

//#define C4_DEBUG

#define C4_ERROR_SHOWS_FILELINE
#define C4_ERROR_SHOWS_FUNC
//#define C4_ERROR_THROWS_EXCEPTION
//#define C4_NO_ALLOC_DEFAULTS
//#define C4_REDEFINE_CPPNEW

#ifndef C4_SIZE_TYPE
#   define C4_SIZE_TYPE size_t
#endif

#ifndef C4_STR_SIZE_TYPE
#   define C4_STR_SIZE_TYPE C4_SIZE_TYPE
#endif

#ifndef C4_TIME_TYPE
#   define C4_TIME_TYPE double
#endif

/** Define this to disable string expression templates. */
//#define C4_STR_DISABLE_EXPR_TPL

#define C4_LOG_THREAD_SAFE
#ifndef C4_LOG_MAX_CHANNELS
#   define C4_LOG_MAX_CHANNELS 16
#endif
#ifndef C4_LOG_BUFFER_INITIAL_SIZE
#   define C4_LOG_BUFFER_INITIAL_SIZE 128
#endif
#ifndef C4_LOG_BUFFER_REF_SIZE
#   define C4_LOG_BUFFER_REF_SIZE 256
#endif

#include "c4/preprocessor.hpp"
#include "c4/platform.hpp"
#include "c4/cpu.hpp"
#include "c4/compiler.hpp"
#include "c4/language.hpp"
#include "c4/types.hpp"

#endif //_C4_UTIL_HPP_
