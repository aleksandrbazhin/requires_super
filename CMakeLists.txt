cmake_minimum_required(VERSION 3.22)

project(mv_requires_super_clang_tidy VERSION 1.0.0 LANGUAGES C CXX)

option(
  BUILD_MV_REQUIRES_SUPER_MODULE
  "Build the custom mv-requires-super clang-tidy module"
  ON
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(BUILD_MV_REQUIRES_SUPER_MODULE)
  add_subdirectory(clang-tidy)
endif()

include(CTest)
if(BUILD_TESTING)
  if(TARGET MvRequiresSuperTidyModule)
    find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy REQUIRED)
    find_package(Catch2 2 REQUIRED CONFIG)

    set(MV_TEST_SOURCE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    configure_file(
      "${MV_TEST_SOURCE_DIRECTORY}/TestConfig.hpp.in"
      "${CMAKE_CURRENT_BINARY_DIR}/generated/TestConfig.hpp"
      @ONLY
    )

    add_executable(MvRequiresSuperTests tests/ClangTidyTests.cpp)
    target_include_directories(
      MvRequiresSuperTests
      PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated"
    )
    target_link_libraries(
      MvRequiresSuperTests
      PRIVATE Catch2::Catch2WithMain MvLlvmSupport
    )
    add_dependencies(MvRequiresSuperTests MvRequiresSuperTidyModule)

    include(Catch)
    catch_discover_tests(MvRequiresSuperTests)
  endif()
endif()
