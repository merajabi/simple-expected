# SimpleExpected - Header-Only C++17 Compatible Implementation of std::expected

## Features of this Implementation

1. **Complete C++17 Implementation** - Works anywhere C++17 runs
2. **std::expected Compatibility** - Easy migration when std::expected comes
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

```bash
# Build and run all tests
build.sh profiles/linux_gcc_13_Make.Debug
```

## License

MIT License

