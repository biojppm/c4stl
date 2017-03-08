#include "c4/test.hpp"

#include "c4/type_name.hpp"

class  SomeTypeName {};
struct SomeStructName {};

C4_BEGIN_NAMESPACE(c4)

class  SomeTypeNameInsideANamespace {};
struct SomeStructNameInsideANamespace {};

TEST(type_name, intrinsic_types)
{
    using str = cspan< char >;
    C4_EXPECT_EQ(type_name< int >(), str("int"));
    C4_EXPECT_EQ(type_name< float >(), str("float"));
    C4_EXPECT_EQ(type_name< double >(), str("double"));
}
TEST(type_name, classes)
{
    using str = cspan< char >;
    C4_EXPECT_EQ(type_name< SomeTypeName >(), str("SomeTypeName"));
    C4_EXPECT_EQ(type_name< ::SomeTypeName >(), str("SomeTypeName"));
}
TEST(type_name, structs)
{
    using str = cspan< char >;
    C4_EXPECT_EQ(type_name< SomeStructName >(), str("SomeStructName"));
    C4_EXPECT_EQ(type_name< ::SomeStructName >(), str("SomeStructName"));
}
TEST(type_name, inside_namespace)
{
    using str = cspan< char >;
    C4_EXPECT_EQ(type_name< SomeTypeNameInsideANamespace >(), str("c4::SomeTypeNameInsideANamespace"));
    C4_EXPECT_EQ(type_name< c4::SomeTypeNameInsideANamespace >(), str("c4::SomeTypeNameInsideANamespace"));
    C4_EXPECT_EQ(type_name< ::c4::SomeTypeNameInsideANamespace >(), str("c4::SomeTypeNameInsideANamespace"));

    C4_EXPECT_EQ(type_name< SomeStructNameInsideANamespace >(), str("c4::SomeStructNameInsideANamespace"));
    C4_EXPECT_EQ(type_name< c4::SomeStructNameInsideANamespace >(), str("c4::SomeStructNameInsideANamespace"));
    C4_EXPECT_EQ(type_name< ::c4::SomeStructNameInsideANamespace >(), str("c4::SomeStructNameInsideANamespace"));
}

C4_END_NAMESPACE(c4)
