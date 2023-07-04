#include <stdexcept>
#include <iostream>

template <class E>
class SimpleUnexpected
{
public:
	SimpleUnexpected() = default;
	SimpleUnexpected(const E& e):myError(e){}

	const E& error() const
	{
		return myError;
	}

private:
	const E myError;
};

template< class T, class E >
class SimpleExpected {
public:
	SimpleExpected() = delete;
	SimpleExpected(const T& x):myHasError(false),myError(E{}),myIsMoved(false),myValue(x){}
	SimpleExpected(const SimpleUnexpected<E>& e):myHasError(true),myError(e),myIsMoved(false),myValue(T{}){}
	SimpleExpected(T&& x):myHasError(false),myError(E{}),myIsMoved(false),myValue(std::move(x)){}

	operator bool () const
	{
		return !myHasError;
	}

	bool has_value() const
	{
		return !myHasError;
	}

	const T& operator->() const
	{
		if(myHasError || myIsMoved)
		{
			 throw std::logic_error("No Value");
		}
		return myValue;
	}

	const T& operator*() const
	{
		if(myHasError || myIsMoved)
		{
			 throw std::logic_error("No Value");
		}
		return myValue;
	}

	const T& value() const&
	{
		if(myHasError || myIsMoved)
		{
			 throw std::logic_error("No Value");
		}
		return myValue;
	}

	T&& value() &&
	{
		if(myHasError || myIsMoved)
		{
			 throw std::logic_error("No Value");
		}
		myIsMoved = true;
		return std::move(myValue);
	}
	  
	const T& value_or(const T& tempValue) const&
	{
		if(myIsMoved)
		{
			 throw std::logic_error("No Value");
		}

		if(myHasError)
		{
			return tempValue;
		}

		return myValue;		
	}
/*
	T&& value_or(const T& tempValue) &&
	{
		if(myIsMoved)
		{
			 throw std::logic_error("No Value");
		}

		if(myHasError)
		{
			T temp{tempValue};
			return std::move(temp);
		}

		myIsMoved = true;
		return std::move(myValue);		
	}
*/
	T&& value_or(T&& tempValue) &&
	{
		if(myIsMoved)
		{
			 throw std::logic_error("No Value");
		}

		if(myHasError)
		{
			return std::move(tempValue);
		}

		myIsMoved = true;
		return std::move(myValue);		
	}

	const E& error() const
	{
		if(!myHasError)
		{
			 throw std::logic_error("No Error");
		}
		return myError.error();
	}

private:
	const bool myHasError;
	const SimpleUnexpected<E> myError;
	bool myIsMoved;
	T myValue;
};
