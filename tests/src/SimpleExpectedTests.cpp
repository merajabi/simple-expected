#include <gtest/gtest.h>

#include "mr/expected.hpp"

#include <iostream>
#include <vector>
#include <string>

namespace {
struct Point {
    std::vector<int> coordinates;
    std::string name;
    
    template <class... Args>
    Point(std::initializer_list<int> coords, Args&&... name_args)
        : coordinates(coords),
          name(std::forward<Args>(name_args)...) {}
};

struct move_detector {
    move_detector() = default;
    move_detector(move_detector&& rhs) {
        rhs.been_moved = true;
    }
    bool been_moved = false;
};

} // namespace

TEST(ConstructorsTest, DefaultConstructor) {
    mr::expected<int, int> e;
    EXPECT_TRUE(e.has_value());
    EXPECT_EQ(e.value(), 0);
}

TEST(ConstructorsTest, MakeUnexpected) {
    mr::expected<int, int> e = mr::make_unexpected(0);
    EXPECT_FALSE(e.has_value());
    EXPECT_EQ(e.error(), 0);
}

TEST(ConstructorsTest, InPlaceInitializerList) {
    mr::expected<std::vector<int>, int> e(mr::in_place, {0, 1});
    EXPECT_TRUE(e.has_value());
    EXPECT_EQ((*e)[0], 0);
    EXPECT_EQ((*e)[1], 1);
}

TEST(ConstructorsTest, VoidIntDefault) {
    mr::expected<void, int> e;
    EXPECT_TRUE(e.has_value());
}

TEST(ConstructorsTest, VoidIntUnexpected) {
    mr::expected<void, int> e(mr::unexpect, 42);
    EXPECT_FALSE(e.has_value());
    EXPECT_EQ(e.error(), 42);
}

TEST(AssignmentTest, SimpleAssignment) {
    mr::expected<int, int> e1 = 42;
    mr::expected<int, int> e4 = mr::make_unexpected(42);
    
    // Value to error
    e1 = e4;
    EXPECT_FALSE(e1.has_value());
    EXPECT_EQ(e1.error(), 42);
    
    // Error to value
    e1 = 17;
    EXPECT_TRUE(e1.has_value());
    EXPECT_EQ(e1.value(), 17);
}

TEST(ExpectedObserverTests, ValueAndErrorAccessors) {
    mr::expected<int, int> o1 = 42;
    mr::expected<int, int> o2{mr::unexpect, 0};
    EXPECT_EQ(*o1, 42);
    EXPECT_EQ(o1.value(), 42);
    EXPECT_EQ(o2.value_or(42), 42);
    EXPECT_EQ(o2.error(), 0);
}

TEST(ExpectedObserverTests, MoveDetectorBehavior) {
    mr::expected<move_detector, int> o4{mr::in_place};
    move_detector o5 = std::move(o4).value();
    EXPECT_TRUE(o4->been_moved);
}

TEST(ExpectedEmplaceTests, EmplacePoint) {
    mr::expected<Point, int> e = mr::make_unexpected(0);
    e.emplace({1, 2}, "origin");  // init list + variadic args
    
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(e->coordinates[0], 1);
    ASSERT_EQ(e->coordinates[1], 2); 
    ASSERT_EQ(e->name, "origin");
}

TEST(ExpectedEmplaceTests, EmplacePointError) {
    mr::expected<Point, int> e2 = mr::make_unexpected(42);
    ASSERT_FALSE(e2.has_value());
    ASSERT_EQ(e2.error(), 42);
    
    // Test emplace after error
    e2.emplace({0, 0}, "origin");
    ASSERT_TRUE(e2.has_value());
}

TEST(TrivialityTest, ExpectedIntInt) {
    EXPECT_TRUE((std::is_trivially_copy_constructible<mr::expected<int,int>>::value));
    EXPECT_TRUE((std::is_trivially_move_constructible<mr::expected<int,int>>::value));
}

TEST(DeletionTest, CopyDeletedMoveDefaulted) {
    struct T {
        T(const T&)=delete;
        T(T&&)=default;
        T& operator=(const T&)=delete;
        T& operator=(T&&)=default;
    };
    EXPECT_FALSE((std::is_copy_constructible<mr::expected<T,int>>::value));
    EXPECT_TRUE((std::is_move_constructible<mr::expected<T,int>>::value));
}

TEST(DeletionTest, NotDefaultConstructible) {
    struct T { T(int){} };
    EXPECT_FALSE((std::is_default_constructible<mr::expected<T,int>>::value));
}

TEST(ExpectedAssertionTests, AccessErrorOnValueThrows) {
    mr::expected<int, int> o1 = 42;
    EXPECT_THROW({ (void)o1.error(); }, std::logic_error);
}

TEST(ExpectedAssertionTests, AccessValueOnErrorThrows) {
    mr::expected<int, int> o2{mr::unexpect, 0};
    EXPECT_THROW({ (void)*o2; }, std::logic_error);
}


