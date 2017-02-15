#ifndef _C4_TIME_HPP_
#define _C4_TIME_HPP_

#include "c4/config.hpp"

/** @macro C4_TIME_TYPE the type of the value to hold time */

C4_BEGIN_NAMESPACE(c4)

using time_type = C4_TIME_TYPE;

/** converts automatically from/to microseconds. */
class Time
{
public:

    C4_ALWAYS_INLINE Time() : m_microsecs(0) {}
    C4_ALWAYS_INLINE Time(time_type microsecs) : m_microsecs(microsecs) {}

    C4_ALWAYS_INLINE operator time_type () const { return m_microsecs; }

    C4_ALWAYS_INLINE void operator= (time_type t) { m_microsecs = t; }

    C4_ALWAYS_INLINE void m(time_type minutes) { m_microsecs = minutes * 60.e6; }
    C4_ALWAYS_INLINE time_type m() const { return m_microsecs / 60.e6; }

    C4_ALWAYS_INLINE void s(time_type seconds) { m_microsecs = seconds * 1.e6; }
    C4_ALWAYS_INLINE time_type s() const { return m_microsecs * 1.e-6; }

    C4_ALWAYS_INLINE void ms(time_type miliseconds) { m_microsecs = miliseconds * 1.e3; }
    C4_ALWAYS_INLINE time_type ms() const { return m_microsecs * 1.e-3; }

    C4_ALWAYS_INLINE void us(time_type microseconds) { m_microsecs = microseconds; }
    C4_ALWAYS_INLINE time_type us() const { return m_microsecs; }

    C4_ALWAYS_INLINE void ns(time_type nanoseconds) { m_microsecs = nanoseconds * 1.e-3; }
    C4_ALWAYS_INLINE time_type ns() const { return m_microsecs * 1.e3; }

private:

    time_type m_microsecs;

};

C4_ALWAYS_INLINE Time nsecs(time_type val) { return Time(val * time_type(1.e-3)); }
C4_ALWAYS_INLINE Time usecs(time_type val) { return Time(val); }
C4_ALWAYS_INLINE Time msecs(time_type val) { return Time(val * time_type(1.e3)); }
C4_ALWAYS_INLINE Time  secs(time_type val) { return Time(val * time_type(1.e6)); }
C4_ALWAYS_INLINE Time  mins(time_type val) { return Time(val * time_type(60.e6)); }
C4_ALWAYS_INLINE Time hours(time_type val) { return Time(val * time_type(3600.e6)); }

time_type currtime();

/** execution time */
inline time_type exetime()
{
    static const time_type atstart = currtime();
    time_type now = currtime() - atstart;
    return now;
}

/** do a spin loop for at least the given time */
inline void busy_wait(time_type microsecs)
{
    time_type start = currtime();
    while(currtime() - start < microsecs)
    {
        C4_KEEP_EMPTY_LOOP;
    }
}

C4_END_NAMESPACE(c4)


#endif /* _C4_TIME_HPP_ */
