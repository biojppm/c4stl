#include "c4/test/list.hpp"

C4_BEGIN_NAMESPACE(c4)

#define _c4adaptor(tyname, ty) \
    _C4_CALL_FLAT_FWD_LIST_TESTS_FOR_STORAGE(tyname, ty, raw_paged, _C4_RAW_PAGED_FLAT_FWD_LIST)

CALL_FOR_CONTAINEE_ARCHETYPES(_c4adaptor)

C4_END_NAMESPACE(c4)
