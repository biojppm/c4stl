#include "c4/test/storage/contiguous.hpp"

C4_BEGIN_NAMESPACE(c4)
C4_BEGIN_NAMESPACE(stg)

REGISTER_TYPED_TEST_CASE_P(test_fixed_size,
                           ctor_empty, ctor_copy, ctor_move, ctor_copy_span, ctor_aggregate,
                           get_span, span_conversion,
                           get_spanrs, spanrs_conversion,
                           get_etched_span, etched_span_conversion);

REGISTER_TYPED_TEST_CASE_P(test_fixed_capacity,
                           contiguous_class_layout,
                           ctor_empty, ctor_copy, ctor_move, ctor_copy_span, ctor_aggregate,
                           get_span, span_conversion,
                           get_spanrs, spanrs_conversion,
                           get_etched_span, etched_span_conversion);

INSTANTIATE_TYPED_TEST_CASE_P(containers, test_fixed_size, c4::archetypes::containees);
//INSTANTIATE_TYPED_TEST_CASE_P(containers, test_fixed_capacity, c4::archetypes::containees);

C4_END_NAMESPACE(stg)
C4_END_NAMESPACE(c4)
