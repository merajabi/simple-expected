
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
	}

	TestValue(const TestValue& t)
		:myVector(t.myVector)
	{ 
		myCopyConstructorCounter++;
	}

	TestValue(TestValue&& t)
		:myVector(std::move(t.myVector))
	{ 
		myMoveConstructorCounter++;
	}

	~TestValue()
	{ 
		myDestructorCounter++;
	}

	TestValue& operator=(const TestValue& t) = delete;
	TestValue& operator=(TestValue&& t) = delete;

	static void ResetCounters()
	{
		myDefaultConstructorCounter = 0;
		myCopyConstructorCounter = 0;
		myMoveConstructorCounter = 0;
		myDestructorCounter = 0;
	}

	std::vector<int> myVector;

	static int myDefaultConstructorCounter;
	static int myCopyConstructorCounter;
	static int myMoveConstructorCounter;
	static int myDestructorCounter;
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
		TestValue::ResetCounters();
	}

	void TearDown() override 
	{
	}
};

SimpleExpected<void,int>
VoidPositive(int x)
{
	if( x < 0)
	{
		return SimpleUnexpected(x);
	}
	return SimpleExpected<void,int>{};
}

TEST_F(SimpleExpectedTests, ShouldBail_WhenVoidPositiveCalledWithNegative) 
{
	auto expexted = VoidPositive(-1);
	if(!expexted)
	{
		const auto c = expexted.error();
		EXPECT_EQ(c,-1);
		EXPECT_THROW({ expexted.value();}, std::logic_error);
	}
}

TEST_F(SimpleExpectedTests, ShouldSucceed_WhenVoidPositiveCalledWithPositive) 
{
   	auto expexted = VoidPositive(1);
	if(expexted)
	{
		expexted.value();
		EXPECT_THROW({ expexted.error();}, std::logic_error);
	}
}


SimpleExpected<int,int>
IntPositive(int x)
{
	if( x < 0)
	{
		return SimpleUnexpected(x);
	}
	return SimpleExpected<int,int>{};
}

TEST_F(SimpleExpectedTests, ShouldBail_WhenIntPositiveCalledWithNegative) 
{
	auto expexted = IntPositive(-1);
	EXPECT_FALSE(expexted);
	if(!expexted)
	{
		const auto c = expexted.error();
		EXPECT_EQ(c,-1);
		EXPECT_THROW({ expexted.value();}, std::logic_error);
	}
}

TEST_F(SimpleExpectedTests, ShouldSucceed_WhenIntPositiveCalledWithPositive) 
{
	auto expexted = IntPositive(1);
	EXPECT_TRUE(expexted);
	if(expexted)
	{
		const auto c = expexted.value();
		EXPECT_EQ(c,0);
		EXPECT_THROW({ expexted.error();}, std::logic_error);
	}
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

TEST_F(SimpleExpectedTests, ShouldBail_WhenDivisionByZero) 
{
	auto expexted = Div(4,0);
	EXPECT_FALSE(expexted);
	if(!expexted)
	{
		const auto c = expexted.error();
		EXPECT_EQ(c,TestError::DIVISION_BY_ZERO);
		EXPECT_THROW({ expexted.value();}, std::logic_error);
	}
}

TEST_F(SimpleExpectedTests, ShouldSucceed_WhenOrValueCalledWithError) 
{
	auto expexted = Div(4,0);
	EXPECT_FALSE(expexted);
	if(!expexted)
	{
		const auto c = expexted.value_or(1);
		EXPECT_EQ(c,1);
	}
}

TEST_F(SimpleExpectedTests, ShouldSucceed_WhenOrValueCalledOnDivisible) 
{
	auto expexted = Div(4,2);
	EXPECT_TRUE(expexted);
	if(expexted)
	{
		const auto c = expexted.value_or(1);
		EXPECT_EQ(c,2);
		EXPECT_THROW({ expexted.error();}, std::logic_error);
	}
}

SimpleExpected<TestValue,TestError>
Concatenate(TestValue x,TestValue y)
{
	if(x.myVector.size() != y.myVector.size())
	{
		return SimpleUnexpected(TestError::NO_EQUAL_SIZE);
	}

	TestValue z = x;
	z.myVector.insert(z.myVector.end(),y.myVector.begin(),y.myVector.end());
	return z;
}

TEST_F(SimpleExpectedTests, ShouldSucceed_WhenReturnValueByCopy) 
{
	{
		TestValue a;
		a.myVector.push_back(1);
		TestValue b;
		b.myVector.push_back(2);

		auto expexted = Concatenate(a,b);
		EXPECT_TRUE(expexted);
		if(expexted)
		{
			const auto c = expexted.value();
			EXPECT_EQ(c.myVector,(std::vector{1,2}));
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
		a.myVector.push_back(1);
		TestValue b;
		b.myVector.push_back(2);

		auto expexted = Concatenate(a,b);
		EXPECT_TRUE(expexted);
		if(expexted)
		{
			const auto c = std::move(expexted).value();
			EXPECT_EQ(c.myVector,(std::vector{1,2}));
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
		a.myVector.push_back(1);
		TestValue b;
		b.myVector.push_back(2);

		auto expexted = Concatenate(a,b);
		EXPECT_TRUE(expexted);
		if(expexted)
		{
			const auto c = std::move(expexted).value();
			EXPECT_EQ(c.myVector,(std::vector{1,2}));

			EXPECT_THROW({ expexted.value();}, std::logic_error);
			EXPECT_THROW({ std::move(expexted).value();}, std::logic_error);
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
		a.myVector.push_back(1);
		TestValue b;
		b.myVector.push_back(2);

		auto functor = [](){
				TestValue x;
				x.myVector.push_back(1);
				return x;
			};

		auto expexted = Concatenate(a,b);
		EXPECT_TRUE(expexted);
		if(expexted)
		{
			const auto c = std::move(expexted).value_or(functor());
			EXPECT_EQ(c.myVector,(std::vector{1,2}));

			EXPECT_THROW({ expexted.value_or(functor());}, std::logic_error);
			EXPECT_THROW({ std::move(expexted).value_or(functor());}, std::logic_error);
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
		b.myVector.push_back(2);

		auto functor = [](){
				TestValue x;
				x.myVector.push_back(1);
				return x;
			};

		auto expexted = Concatenate(a,b);
		EXPECT_FALSE(expexted);
		if(!expexted)
		{
			const auto c = std::move(expexted).value_or(functor());
			EXPECT_EQ(c.myVector,(std::vector{1}));

			const auto d = std::move(expexted).value_or(functor());
			EXPECT_EQ(d.myVector,(std::vector{1}));
		}
	}
	EXPECT_EQ(TestValue::myDefaultConstructorCounter,5);
	EXPECT_EQ(TestValue::myCopyConstructorCounter,2);
	EXPECT_EQ(TestValue::myMoveConstructorCounter,2);
	EXPECT_EQ(TestValue::myDestructorCounter,TestValue::myDefaultConstructorCounter + TestValue::myCopyConstructorCounter + TestValue::myMoveConstructorCounter);
}



