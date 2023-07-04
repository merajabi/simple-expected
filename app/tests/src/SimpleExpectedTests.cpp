
#include "SimpleExpected.h"

#include <gtest/gtest.h>

#include <iostream>
#include <vector>

enum class TestError
{
    NO_EQUAL_SIZE,
	DIVISION_BY_ZERO
};

class TestValue {
public:
	TestValue()
	{ 
		myDefaultConstructorCounter++; 
		std::cout << "TestValue()" <<std::endl;
	}

	TestValue(const TestValue& t)
		:v(t.v)
	{ 
		myCopyConstructorCounter++;
		std::cout << "TestValue(const TestValue& t)" <<std::endl;
	}

	TestValue(TestValue&& t)
		:v(std::move(t.v))
	{ 
		myMoveConstructorCounter++;
		std::cout << "TestValue(TestValue&& t)" <<std::endl;
	}

	~TestValue()
	{ 
		myDestructorCounter++;
		std::cout << "~TestValue()" <<std::endl;
	}

	TestValue& operator=(const TestValue& t) = delete; //{ std::cout << "operator=(const TestValue& t)" <<std::endl;};
	TestValue& operator=(TestValue&& t) = delete; //{ std::cout << "operator=(TestValue&& t)" <<std::endl;};

	static void ResetCounters()
	{
		myDefaultConstructorCounter = 0;
		myCopyConstructorCounter = 0;
		myMoveConstructorCounter = 0;
		myDestructorCounter = 0;
	}

	std::vector<int> v;

	static int myDefaultConstructorCounter;
	static int myCopyConstructorCounter;
	static int myMoveConstructorCounter;
	static int myDestructorCounter;
private:
};

int TestValue::myDefaultConstructorCounter = 0;
int TestValue::myCopyConstructorCounter = 0;
int TestValue::myMoveConstructorCounter = 0;
int TestValue::myDestructorCounter = 0;

class SimpleExpectedTests : public ::testing::Test 
{

protected:
	void SetUp() override 
	{
		std::cout << "Hello!" << std::endl;
		TestValue::ResetCounters();
	}

	void TearDown() override 
	{
		std::cout << "Bye!" << std::endl;
	}

};

SimpleExpected<TestValue,TestError>
Concatenate(TestValue x,TestValue y)
{
	std::cout << "Concatenate(TestValue x,TestValue y)" <<std::endl;
	if(x.v.size() != y.v.size())
	{
		return SimpleUnexpected(TestError::NO_EQUAL_SIZE);
	}

	TestValue z = x;
	z.v.insert(z.v.end(),y.v.begin(),y.v.end());
	return z;
}

TEST_F(SimpleExpectedTests, ShouldSucceed_WhenReturnValueByCopy) 
{
	{
		TestValue a;
		a.v.push_back(1);
		TestValue b;
		b.v.push_back(2);

		auto r = Concatenate(a,b);
		EXPECT_TRUE(r);
		if(r)
		{
			std::cout << "r.value()" <<std::endl;
			const auto c = r.value();
			EXPECT_EQ(c.v,(std::vector{1,2}));
		}
	}
	EXPECT_EQ(TestValue::myDefaultConstructorCounter,2);
	EXPECT_EQ(TestValue::myCopyConstructorCounter,4);
	EXPECT_EQ(TestValue::myMoveConstructorCounter,1);
	EXPECT_EQ(TestValue::myDestructorCounter,TestValue::myDefaultConstructorCounter + TestValue::myCopyConstructorCounter + TestValue::myMoveConstructorCounter);
}

TEST_F(SimpleExpectedTests, ShouldSucceed_WhenReturnValueByMove) 
{
	{
		TestValue a;
		a.v.push_back(1);
		TestValue b;
		b.v.push_back(2);

		auto r = Concatenate(a,b);
		EXPECT_TRUE(r);
		if(r)
		{
			std::cout << "r.value()" <<std::endl;
			const auto c = std::move(r).value();
			EXPECT_EQ(c.v,(std::vector{1,2}));
		}
	}
	EXPECT_EQ(TestValue::myDefaultConstructorCounter,2);
	EXPECT_EQ(TestValue::myCopyConstructorCounter,3);
	EXPECT_EQ(TestValue::myMoveConstructorCounter,2);
	EXPECT_EQ(TestValue::myDestructorCounter,TestValue::myDefaultConstructorCounter + TestValue::myCopyConstructorCounter + TestValue::myMoveConstructorCounter);
}

TEST_F(SimpleExpectedTests, ShouldBail_WhenReturnValueAfterMove) 
{
	{
		TestValue a;
		a.v.push_back(1);
		TestValue b;
		b.v.push_back(2);

		auto r = Concatenate(a,b);
		EXPECT_TRUE(r);
		if(r)
		{
			std::cout << "r.value()" <<std::endl;
			const auto c = std::move(r).value();
			EXPECT_EQ(c.v,(std::vector{1,2}));

			EXPECT_THROW({ r.value();}, std::logic_error);
			EXPECT_THROW({ std::move(r).value();}, std::logic_error);
		}
	}
	EXPECT_EQ(TestValue::myDefaultConstructorCounter,2);
	EXPECT_EQ(TestValue::myCopyConstructorCounter,3);
	EXPECT_EQ(TestValue::myMoveConstructorCounter,2);
	EXPECT_EQ(TestValue::myDestructorCounter,TestValue::myDefaultConstructorCounter + TestValue::myCopyConstructorCounter + TestValue::myMoveConstructorCounter);
}

TEST_F(SimpleExpectedTests, ShouldBail_WhenReturnOrValueAfterMove) 
{
	{
		TestValue a;
		a.v.push_back(1);
		TestValue b;
		b.v.push_back(2);

		auto functor = [](){
				TestValue x;
				x.v.push_back(1);
				return x;
			};

		auto r = Concatenate(a,b);
		EXPECT_TRUE(r);
		if(r)
		{
			std::cout << "r.value()" <<std::endl;
			const auto c = std::move(r).value_or(functor());
			EXPECT_EQ(c.v,(std::vector{1,2}));

			EXPECT_THROW({ r.value_or(functor());}, std::logic_error);
			EXPECT_THROW({ std::move(r).value_or(functor());}, std::logic_error);
		}
	}
	EXPECT_EQ(TestValue::myDefaultConstructorCounter,5);
	EXPECT_EQ(TestValue::myCopyConstructorCounter,3);
	EXPECT_EQ(TestValue::myMoveConstructorCounter,2);
	EXPECT_EQ(TestValue::myDestructorCounter,TestValue::myDefaultConstructorCounter + TestValue::myCopyConstructorCounter + TestValue::myMoveConstructorCounter);
}
TEST_F(SimpleExpectedTests, ShouldSucceed_WhenReturnOrValueAfterMoveWithError) 
{
	{
		TestValue a;
		TestValue b;
		b.v.push_back(2);

		auto functor = [](){
				TestValue x;
				x.v.push_back(1);
				return x;
			};

		auto r = Concatenate(a,b);
		EXPECT_FALSE(r);
		if(!r)
		{
			std::cout << "r.value()" <<std::endl;
			const auto c = std::move(r).value_or(functor());
			EXPECT_EQ(c.v,(std::vector{1}));

			const auto d = std::move(r).value_or(functor());
			EXPECT_EQ(d.v,(std::vector{1}));
		}
	}
	EXPECT_EQ(TestValue::myDefaultConstructorCounter,5);
	EXPECT_EQ(TestValue::myCopyConstructorCounter,2);
	EXPECT_EQ(TestValue::myMoveConstructorCounter,2);
	EXPECT_EQ(TestValue::myDestructorCounter,TestValue::myDefaultConstructorCounter + TestValue::myCopyConstructorCounter + TestValue::myMoveConstructorCounter);
}



SimpleExpected<int,TestError>
Div(int x,int y)
{
	if(y == 0)
	{
		return SimpleUnexpected(TestError::DIVISION_BY_ZERO);
	}
	return x/y;
}

TEST_F(SimpleExpectedTests, ShouldSucceed_WhenReturnNoError) 
{
	auto r = Div(4,2);
	EXPECT_TRUE(r);
	if(r)
	{
		const auto c = r.value_or(1);
		EXPECT_EQ(c,2);
	}
}

TEST_F(SimpleExpectedTests, ShouldBail_WhenError) 
{
	auto r = Div(4,0);
	EXPECT_FALSE(r);
	if(!r)
	{
		const auto c = r.error();
		EXPECT_EQ(c,TestError::DIVISION_BY_ZERO);
	}
}

TEST_F(SimpleExpectedTests, ShouldSucceed_WhenReturnOrValueWithError) 
{
	auto r = Div(4,0);
	EXPECT_FALSE(r);
	if(!r)
	{
		const auto c = r.value_or(1);
		EXPECT_EQ(c,1);
	}
}


