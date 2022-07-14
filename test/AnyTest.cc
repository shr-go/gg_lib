// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/any.h"
#include <memory>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

struct Foo {
    std::string foo_;
    char bar_[256];

    Foo() : foo_(std::string(128, 'F')), bar_() {
        strncpy(bar_, foo_.c_str(), foo_.size());
    }
};

using namespace gg_lib;

TEST(AnyTest, TestAssign) {
    any x = 4;
    EXPECT_EQ(x.type(), typeid(int));
    EXPECT_EQ(any_cast<int>(x), 4);
    any foo = Foo();
    EXPECT_EQ(foo.type(), typeid(Foo));
    EXPECT_EQ(any_cast<Foo>(foo).foo_, std::string(128, 'F'));
    Foo &bar = any_cast<Foo &>(foo);
    bar.foo_ = std::string(128, 'G');
    EXPECT_EQ(any_cast<Foo>(foo).foo_, std::string(128, 'G'));

}

TEST(AnyTest, TestCast) {
    any foo = Foo();
    EXPECT_THROW(any_cast<int>(foo), bad_any_cast);
    EXPECT_THROW(any_cast<Foo *>(foo), bad_any_cast);
    EXPECT_THROW(any_cast<const Foo *>(foo), bad_any_cast);
    EXPECT_NO_THROW(any_cast<Foo>(foo));
    EXPECT_NO_THROW(any_cast<const Foo>(foo));
    EXPECT_NO_THROW(any_cast<Foo &>(foo));
    EXPECT_NO_THROW(any_cast<const Foo &>(foo));
}

TEST(AnyTest, TestPointer) {
    any foo = Foo();
    EXPECT_NE(any_cast<Foo>(&foo), nullptr);
    EXPECT_EQ(any_cast<int>(&foo), nullptr);

    std::shared_ptr<int> ptr_count(new int);
    std::weak_ptr<int> weak = ptr_count;
    any p0 = 0;

    EXPECT_TRUE(weak.use_count() == 1);
    any p1 = ptr_count;
    EXPECT_TRUE(weak.use_count() == 2);
    any p2 = p1;
    EXPECT_TRUE(weak.use_count() == 3);
    p0 = p1;
    EXPECT_TRUE(weak.use_count() == 4);
    p0 = 0;
    EXPECT_TRUE(weak.use_count() == 3);
    p0 = std::move(p1);
    EXPECT_TRUE(weak.use_count() == 3);
    p0.swap(p1);
    EXPECT_TRUE(weak.use_count() == 3);
    p0 = 0;
    EXPECT_TRUE(weak.use_count() == 3);
    p1.clear();
    EXPECT_TRUE(weak.use_count() == 2);
    p2 = any(Foo());
    EXPECT_TRUE(weak.use_count() == 1);
    p1 = ptr_count;
    EXPECT_TRUE(weak.use_count() == 2);
    ptr_count = nullptr;
    EXPECT_TRUE(weak.use_count() == 1);
    p1 = any();
    EXPECT_TRUE(weak.use_count() == 0);
}

TEST(AnyTest, MakeAny) {
    auto a1 = make_any<std::string>("aaaa");
    EXPECT_EQ(any_cast<std::string>(a1), std::string("aaaa"));
    auto a2 = make_any<std::vector<int>>({1, 2, 3, 4, 5});
    EXPECT_THAT(any_cast<std::vector<int>>(a2), testing::ElementsAre(1, 2, 3, 4, 5));
}
