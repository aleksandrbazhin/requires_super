# `mv-requires-super`

A Clang-Tidy 14 module that requires overriding methods to call an annotated
base implementation:

```cpp
class Base
{
public:
    [[clang::annotate("mv-requires-super")]] virtual void update() {}
};

class Child : public Base
{
public:
    void update() override
    {
        Base::update();
    }
};
```

The nearest explicitly annotated method is required. Unannotated intermediate
overrides may be called or skipped.

Like Clang's Objective-C `objc_requires_super` behavior:

- the check requires only the existence of a call, not a call on every path;
- repeated base calls are accepted;
- calls inside lambdas do not count, even if the lambda is immediately invoked;
- the base implementation must be called on the current object.

C++ multiple inheritance is also supported: every required override branch
must receive a call.

## Requirements

- CMake 3.22+
- C++20 compiler
- Clang-Tidy 14
- LLVM and Clang 14 development packages
- Catch2 2.x

The Clang-Tidy executable and development libraries must have the same major
version.

## Build and test

```sh
cmake -S . -B build -DCMAKE_CXX_COMPILER=g++
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Catch2 runs the module against the fixtures in `tests/pass` and `tests/fail`.

## CMake integration

Copy `clang-tidy/` into the consuming project, then configure the targets that
already use Clang-Tidy:

```cmake
find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy REQUIRED)
add_subdirectory(clang-tidy)

add_executable(my_app main.cpp)
set_target_properties(
  my_app
  PROPERTIES CXX_CLANG_TIDY
             "${CLANG_TIDY_EXECUTABLE};--load=${MV_REQUIRES_SUPER_PLUGIN_PATH}"
)
add_dependencies(my_app MvRequiresSuperTidyModule)
```

Enable the check in `.clang-tidy`:

```yaml
Checks: '-*,mv-requires-super'
WarningsAsErrors: 'mv-requires-super'
```
