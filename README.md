# SimpleExpected - Simple Header-Only C++17 Compatible Implementation of std::expected

## Why expected<> is a Clean Error Handling Approach

expected<> is one of the cleanest solution because:

### 1. **Explicit Error Paths**
```cpp
// Traditional error codes - easy to ignore
int result = do_work();
if (result != 0) { /* handle? */ }

// With expected - can't ignore errors
auto result = do_work(); // returns expected<Data, Error>
if (!result) { /* must handle */ }
```

### 2. **Type-Safe Errors**
No more magic numbers or error codes - real types with context:
```cpp
expected<Data, std::string> getData(); // Error contains meaningful message
```

### 3. **No Exception Overhead**
Perfect for performance-critical code where exceptions are too expensive.

### 4. **Clear Intent**
The signature `expected<Value, Error>` documents exactly what can go wrong.

## Why I Chose std::variant Over Alternatives

After evaluating several approaches, I settled on std::variant because:

### **Automatic Lifetime Management**
No more manual destructor calls - the variant handles it all:
```cpp
~expected() {
    // No need for this with variant!
    // if (has_value_) value_.~T(); else error_.~E(); 
}
```

### **Exception Safety Done Right**
The standard library implementation gives me:
- Strong exception guarantees
- No memory leaks
- Proper rollback on failures

### **Trivial Types Stay Trivial**
When used with simple types:
```cpp
expected<int, int> e; // Still trivial copy/move/destruct
```

### **Compiler Optimizations**
std::variant is heavily optimized in modern compilers.

### **Less Bug-Prone**
Other mothods like manual unions or std::aligned_storage are more error-prone.

## Features of this Implementation

1. **Complete C++17 Implementation** - Works anywhere C++17 runs
2. **std::expected Compatibility** - Easy migration when C++26 comes
3. **No Default-Constructible Requirement** - Works with more types
4. **Full Exception Safety** - Verified through rigorous testing
5. **Trivial Type Support** - Zero overhead for simple cases

## How to Use

```cpp
#include "mr/expected.hpp"

mr::expected<Data, ParseError> parseData(std::string_view input) {
    if (input.empty()) {
        return mr::make_unexpected(ParseError::EmptyInput);
    }
    // ... parsing logic ...
    return Data{/*...*/};
}

int main() {
    auto result = parseData("...");
    if (!result) {
        std::cerr << "Error: " << result.error().message();
        return 1;
    }
    process(*result);
}
```

## Building and Testing

I've included comprehensive tests that verify:

```bash
# Build and run all tests
build.sh profiles/linux_gcc_13_Make.Debug
```

## License

MIT License - I want others to benefit from this work freely.

