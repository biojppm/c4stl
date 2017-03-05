#include "c4/sstream.hpp"

C4_BEGIN_NAMESPACE(c4)

#define _C4_DEFINE_FMT_TAG(_ty)                               \
constexpr const char fmt_tag< char, _ty >::fmt_pri  [];       \
constexpr const char fmt_tag< char, _ty >::fmt_pri_n[];       \
constexpr const char fmt_tag< char, _ty >::fmt_scn  [];       \
constexpr const char fmt_tag< char, _ty >::fmt_scn_n[];       \
constexpr const wchar_t fmt_tag< wchar_t, _ty >::fmt_pri  []; \
constexpr const wchar_t fmt_tag< wchar_t, _ty >::fmt_pri_n[]; \
constexpr const wchar_t fmt_tag< wchar_t, _ty >::fmt_scn  []; \
constexpr const wchar_t fmt_tag< wchar_t, _ty >::fmt_scn_n[]

_C4_DEFINE_FMT_TAG(void*   );
_C4_DEFINE_FMT_TAG(char    );
_C4_DEFINE_FMT_TAG(wchar_t );
_C4_DEFINE_FMT_TAG(char*   );
_C4_DEFINE_FMT_TAG(wchar_t*);
_C4_DEFINE_FMT_TAG(double  );
_C4_DEFINE_FMT_TAG(float   );
_C4_DEFINE_FMT_TAG( int8_t );
_C4_DEFINE_FMT_TAG(uint8_t );
_C4_DEFINE_FMT_TAG( int16_t);
_C4_DEFINE_FMT_TAG(uint16_t);
_C4_DEFINE_FMT_TAG( int32_t);
_C4_DEFINE_FMT_TAG(uint32_t);
_C4_DEFINE_FMT_TAG( int64_t);
_C4_DEFINE_FMT_TAG(uint64_t);

#undef _C4_DEFINE_FMT_TAG

C4_END_NAMESPACE(c4)
