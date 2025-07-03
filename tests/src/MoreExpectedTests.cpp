#include "mr/expected.hpp"

#include <gtest/gtest.h>

using namespace mr;

// Test fixture for expected tests
class ExpectedTest : public ::testing::Test {
public:
    ExpectedTest():val_(42),err_(unexpected<int>{-1}){}

protected:
    void SetUp() override {
        val_ = 42;
        err_ = unexpected<int>{-1};
    }

    int val_;
    unexpected<int> err_;
};

// unexpected tests
TEST(UnexpectedTest, Construction) {
    unexpected<int> u(42);
    EXPECT_EQ(u.error(), 42);

    unexpected<std::string> s("error");
    EXPECT_EQ(s.error(), "error");
}

TEST(UnexpectedTest, MakeUnexpected) {
    auto u = make_unexpected(42);
    EXPECT_EQ(u.error(), 42);

    auto s = make_unexpected(std::string("error"));
    EXPECT_EQ(s.error(), "error");
}

TEST(UnexpectedTest, CopyMove) {
    unexpected<int> u1(42);
    unexpected<int> u2 = u1;
    EXPECT_EQ(u2.error(), 42);

    unexpected<int> u3 = std::move(u1);
    EXPECT_EQ(u3.error(), 42);
}

// expected<T,E> tests
TEST_F(ExpectedTest, DefaultConstructor) {
    expected<int, int> e;
    EXPECT_TRUE(e.has_value());
    EXPECT_EQ(*e, 0);
}

TEST_F(ExpectedTest, ValueConstructor) {
    expected<int, int> e(val_);
    EXPECT_TRUE(e.has_value());
    EXPECT_EQ(*e, val_);
}

TEST_F(ExpectedTest, UnexpectedConstructor) {
    expected<int, int> e(err_);
    EXPECT_FALSE(e.has_value());
    EXPECT_EQ(e.error(), err_.error());
}

TEST_F(ExpectedTest, InPlaceConstructor) {
    expected<std::pair<int, int>, int> e(in_place, 1, 2);
    EXPECT_TRUE(e.has_value());
    EXPECT_EQ(e->first, 1);
    EXPECT_EQ(e->second, 2);
}

TEST_F(ExpectedTest, InPlaceInitializerList) {
    expected<std::vector<int>, int> e(in_place, {1, 2, 3});
    EXPECT_TRUE(e.has_value());
    ASSERT_EQ(e->size(), 3);
    EXPECT_EQ((*e)[0], 1);
    EXPECT_EQ((*e)[1], 2);
    EXPECT_EQ((*e)[2], 3);
}

TEST_F(ExpectedTest, CopyMove) {
    expected<int, int> e1(val_);
    expected<int, int> e2 = e1;
    EXPECT_EQ(*e2, val_);

    expected<int, int> e3 = std::move(e1);
    EXPECT_EQ(*e3, val_);
}

TEST_F(ExpectedTest, ValueAccess) {
    expected<int, int> e(val_);
    EXPECT_EQ(*e, val_);
    EXPECT_EQ(e.value(), val_);
    EXPECT_EQ(e.operator->(), &*e);
}

TEST_F(ExpectedTest, ErrorAccess) {
    expected<int, int> e(err_);
    EXPECT_EQ(e.error(), err_.error());
}

TEST_F(ExpectedTest, ValueOr) {
    expected<int, int> e1(val_);
    EXPECT_EQ(e1.value_or(0), val_);

    expected<int, int> e2(err_);
    EXPECT_EQ(e2.value_or(0), 0);
}

TEST_F(ExpectedTest, Emplace) {
    expected<std::pair<int, int>, int> e;
    e.emplace(1, 2);
    EXPECT_TRUE(e.has_value());
    EXPECT_EQ(e->first, 1);
    EXPECT_EQ(e->second, 2);
}

TEST_F(ExpectedTest, EmplaceInitializerList) {
    expected<std::vector<int>, int> e;
    e.emplace({1, 2, 3});
    EXPECT_TRUE(e.has_value());
    ASSERT_EQ(e->size(), 3);
    EXPECT_EQ((*e)[0], 1);
    EXPECT_EQ((*e)[1], 2);
    EXPECT_EQ((*e)[2], 3);
}

TEST_F(ExpectedTest, BoolOperator) {
    expected<int, int> e1(val_);
    EXPECT_TRUE(static_cast<bool>(e1));

    expected<int, int> e2(err_);
    EXPECT_FALSE(static_cast<bool>(e2));
}

// expected<void,E> tests
TEST(ExpectedVoidTest, DefaultConstructor) {
    expected<void, int> e;
    EXPECT_TRUE(e.has_value());
}

TEST(ExpectedVoidTest, UnexpectedConstructor) {
    expected<void, int> e(unexpect, -1);
    EXPECT_FALSE(e.has_value());
    EXPECT_EQ(e.error(), -1);
}

TEST(ExpectedVoidTest, ValueAccess) {
    expected<void, int> e;
    EXPECT_NO_THROW(e.value());
}

TEST(ExpectedVoidTest, ErrorAccess) {
    expected<void, int> e(unexpect, -1);
    EXPECT_EQ(e.error(), -1);
}

// Exception tests
TEST(ExpectedExceptionTest, ValueAccessOnError) {
    expected<int, int> e(unexpect, -1);
    EXPECT_THROW({ (void)*e; }, std::logic_error);
    EXPECT_THROW({ (void)e.value(); }, std::logic_error);
    EXPECT_THROW({ (void)e.operator->(); }, std::logic_error);
}

TEST(ExpectedExceptionTest, ErrorAccessOnValue) {
    expected<int, int> e(42);
    EXPECT_THROW({ (void)e.error(); }, std::logic_error);
}

TEST(ExpectedExceptionTest, VoidValueAccessOnError) {
    expected<void, int> e(unexpect, -1);
    EXPECT_THROW({ e.value(); }, std::logic_error);
}

// Non-default-constructible tests
TEST(ExpectedNonDefaultTest, NoDefaultConstructor) {
    struct NoDefault { NoDefault(int) {} };
    expected<NoDefault, int> e(in_place, 42);
    EXPECT_TRUE(e.has_value());
    
    // Should not compile:
    //expected<NoDefault, int> e2;
}

// Move-only tests
TEST(ExpectedMoveOnlyTest, MoveOnlyType) {
    struct MoveOnly {
        MoveOnly() = default;
        MoveOnly(const MoveOnly&) = delete;
        MoveOnly(MoveOnly&& rhs){rhs.moved = true;}
        bool moved = false;
    };
    
    expected<MoveOnly, int> e1;
    expected<MoveOnly, int> e2 = std::move(e1);
    EXPECT_TRUE(e2.has_value());
    EXPECT_TRUE(e2->moved == false);
    EXPECT_TRUE(e1->moved == true);
}

/*
// Constexpr tests (compile-time checks)
TEST(ExpectedConstexprTest, ConstexprConstruction) {
    constexpr expected<int, int> e1(42);
    static_assert(e1.has_value());
    static_assert(*e1 == 42);
    
    constexpr expected<int, int> e2(unexpect, -1);
    static_assert(!e2.has_value());
    static_assert(e2.error() == -1);
}
*/

// Edge case tests
TEST(ExpectedEdgeCaseTest, SelfAssignment) {
    expected<int, int> e(42);
    e = e;
    EXPECT_EQ(*e, 42);
    
    expected<int, int> e2(unexpect, -1);
    e2 = e2;
    EXPECT_EQ(e2.error(), -1);
}

// value_or with different convertible types
TEST(ExpectedValueOrTest, ConvertibleTypes) {
    expected<int, int> e(unexpect, -1);
    EXPECT_EQ(e.value_or(42.5), 42); // double converts to int
    
    expected<std::string, int> e2(unexpect, -1);
    EXPECT_EQ(e2.value_or("default"), "default");
}

// Test with non-trivial types
TEST(ExpectedNonTrivialTest, StringValues) {
    expected<std::string, std::string> e1("hello");
    EXPECT_EQ(*e1, "hello");
    
    expected<std::string, std::string> e2(unexpect, "error");
    EXPECT_EQ(e2.error(), "error");
    EXPECT_EQ(e2.value_or("default"), "default");
}
