# `mv-requires-super` Clang-Tidy module

This CMake project builds a loadable Clang-Tidy module containing the
`mv-requires-super` check. It has no unrelated application or library sources;
the C++ files under `clang-tidy/` implement the module, and `tests/` contains
its fixtures.

The check enforces calls to explicitly annotated base implementations:

```cpp
class Base
{
public:
    [[clang::annotate("mv-requires-super")]] virtual void update()
    {
    }
};

class Child : public Base
{
public:
    void update() override
    {
        Base::update(); // Required.
    }
};
```

Annotations are not treated as inherited. The check walks each override chain
to its nearest explicitly annotated method. A grandchild may call an
unannotated intermediate implementation or skip it and call the annotated
grandparent directly. If the intermediate override is explicitly annotated,
that nearer implementation must be called.

The rule verifies that a matching qualified base call appears somewhere in the
overriding body. It does not prove that every runtime control-flow path makes
the call.

## Requirements

- CMake 3.22 or newer
- A C++20 compiler
- Clang-Tidy 14
- LLVM 14 development files (`llvm-14-dev` on Ubuntu 22.04)
- Clang 14 development files (`libclang-14-dev` on Ubuntu 22.04)

The Clang-Tidy executable and development libraries must use the same major
LLVM version. A plugin built for one major version is not ABI-compatible with
another.

## Build and test

```sh
cmake -S . -B build -DCMAKE_CXX_COMPILER=g++
cmake --build build --parallel
ctest --test-dir build --output-on-failure
cmake --build build --target tidy
```

Passing fixtures are under `tests/pass`. Intentionally failing fixtures are
under `tests/fail`, with a comment beside every expected diagnostic explaining
the exact violated requirement. The failing CTest succeeds only when all
expected diagnostics are produced.

Use `-DENABLE_CLANG_TIDY=OFF` to build the plugin without creating the explicit
`tidy` target. Use `-DBUILD_MV_REQUIRES_SUPER_MODULE=OFF` to omit the plugin.

## Integrate into another CMake project

Copy the `clang-tidy` directory into the other project. Add it before creating
the targets that should be analyzed:

```cmake
find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy REQUIRED)

add_subdirectory(clang-tidy)

set(PROJECT_CLANG_TIDY
    "${CLANG_TIDY_EXECUTABLE}"
    "--load=${MV_REQUIRES_SUPER_PLUGIN_PATH}")

add_executable(my_app main.cpp)
set_target_properties(
  my_app
  PROPERTIES CXX_CLANG_TIDY "${PROJECT_CLANG_TIDY}"
)

# Build the module before clang-tidy is launched for my_app.
add_dependencies(my_app MvRequiresSuperTidyModule)
```

Enable the check in that project's `.clang-tidy` file:

```yaml
Checks: '-*,mv-requires-super'
WarningsAsErrors: 'mv-requires-super'
```

For code also compiled by non-Clang compilers, a portability macro can hide
the Clang-only annotation from the normal compiler while remaining visible to
Clang-Tidy:

```cpp
#if defined(__clang__)
#define MV_REQUIRES_SUPER [[clang::annotate("mv-requires-super")]]
#else
#define MV_REQUIRES_SUPER
#endif
```

Use `--load=/absolute/path/to/MvRequiresSuperTidyModule.so` when invoking
Clang-Tidy outside CMake. The extension is `.dylib` on macOS and `.dll` on
Windows.
