#include "c4/list.test.hpp"

C4_BEGIN_NAMESPACE(c4)

#define _c4adaptor(tyname, ty) \
    _C4_CALL_SPLIT_FWD_LIST_TESTS_FOR_STORAGE(tyname, ty, raw_fixed, _C4_RAW_FIXED, 1024)

CALL_FOR_CONTAINEE_ARCHETYPES(_c4adaptor)

C4_END_NAMESPACE(c4)
