#ifndef _C4_LIBTEST_SUPPRWARN_PUSH_HPP_
#define _C4_LIBTEST_SUPPRWARN_PUSH_HPP_

/** @file supprwarn_push.hpp this file contains directives to make the
 * compiler ignore warnings in test code. It should NOT be used for c4stl
 * itself, but only in test code. */

#ifdef __clang__
#   pragma clang diagnostic push
    /* NOTE: using , ## __VA_ARGS__ to deal with zero-args calls to
     * variadic macros is not portable, but works in clang, gcc, msvc, icc.
     * clang requires switching off compiler warnings for pedantic mode.
     * @see http://stackoverflow.com/questions/32047685/variadic-macro-without-arguments */
#   pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments" // warning: token pasting of ',' and __VA_ARGS__ is a GNU extension
#   pragma clang diagnostic ignored "-Wunused-local-typedef"
#   pragma clang diagnostic ignored "-Wsign-compare" // warning: comparison of integers of different signs: 'const unsigned long' and 'const int'
#   pragma clang diagnostic ignored "-Wfloat-equal" // warning: comparing floating point with == or != is unsafe
#   pragma clang diagnostic ignored "-Wwritable-strings" // ISO C++11 does not allow conversion from string literal to char*
#   pragma clang diagnostic ignored "-Wunused-variable"
#   pragma clang diagnostic ignored "-Wunused-parameter"
#elif defined(__GNUC__)
    /* GCC also issues a warning for zero-args calls to variadic macros.
     * This warning is switched on with -pedantic and apparently there is no
     * easy way to turn it off as with clang. But marking this as a system
     * header works.
     * @see https://gcc.gnu.org/onlinedocs/cpp/System-Headers.html
     * @see http://stackoverflow.com/questions/35587137/ */
#   pragma GCC system_header
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wwrite-strings" // ISO C++ forbids converting a string constant to ‘C* {aka char*}’
#   pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#endif /* _C4_LIBTEST_SUPPRWARN_PUSH_HPP_ */