#ifndef _C4_SZCONV_HPP_
#define _C4_SZCONV_HPP_

/** @file szconv.hpp utilities to deal safely with narrowing conversions */

#include "c4/config.hpp"
#include "c4/error.hpp"

C4_BEGIN_NAMESPACE(c4)

/** use SFINAE to overload szconv() based on whether SizeOut is narrower than SizeIn */
#define _C4FOR(op)                                                  \
    typename std::enable_if< sizeof(SizeOut) op sizeof(SizeIn), SizeOut >::type

/** when SizeOut is wider than SizeIn, assignment can occur without reservations */
template< class SizeOut, class SizeIn >
C4_ALWAYS_INLINE _C4FOR(>=) szconv(SizeIn sz)
{
    return sz;
}

/** when SizeOut is narrower than SizeIn, narrowing will occur, so add a check for overflow */
template< class SizeOut, class SizeIn >
C4_ALWAYS_INLINE _C4FOR(<) szconv(SizeIn sz)
{
    C4_XASSERT(sz <= std::numeric_limits< SizeOut >::max());
    SizeOut szo = (SizeOut)sz;
    return szo;
}

#undef _C4FOR

C4_END_NAMESPACE(c4)

#endif /* _C4_SZCONV_HPP_ */
