#include "c4/ctor_dtor.hpp"
#include "c4/test.hpp"

C4_BEGIN_NAMESPACE(c4)

C4_BEGIN_HIDDEN_NAMESPACE
struct subject
{
    static size_t ct_cp, ct_mv, cp, mv;
    static void clear() { ct_cp = ct_mv = cp = mv = 0; }
    subject(Counting<std::string> const& s)
    {
        ++ct_cp;
    }
    subject(Counting<std::string> && s)
    {
        ++ct_mv;
    }
    subject(subject const& s)
    {
        ++cp;
    }
    subject(subject && s)
    {
        ++mv;
    }
};
size_t subject::ct_cp = 0;
size_t subject::ct_mv = 0;
size_t subject::cp = 0;
size_t subject::mv = 0;
C4_END_HIDDEN_NAMESPACE

TEST(ctor_dtor, construct_n)
{
    using T = Counting< subject >;
    char buf1[100 * sizeof(T)];
    T* mem1 = reinterpret_cast<T*>(buf1);

    using cs = Counting< std::string >;

    int num = 10;

    {
        auto chc = T::check_num_ctors_dtors(num, 0);
        auto ch = cs::check_num_ctors_dtors(1, 1);
        cs s("bla");
        construct_n(mem1, num, s);
        EXPECT_EQ(subject::ct_cp, num);
        subject::clear();
    }

    {
        auto chc = T::check_num_ctors_dtors(num, 0);
        auto ch = cs::check_num_ctors_dtors(1, 1);
        construct_n(mem1, num, cs("bla"));  // BAD!!! will call 10 moves
        EXPECT_EQ(subject::ct_cp, num);
        subject::clear();
    }
}

C4_END_NAMESPACE(c4)
