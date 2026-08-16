cmake_minimum_required(VERSION 3.22)

project(mv_requires_super_clang_tidy VERSION 1.0.0 LANGUAGES C CXX)

option(ENABLE_CLANG_TIDY "Run clang-tidy while compiling C++ sources" ON)
option(
  BUILD_MV_REQUIRES_SUPER_MODULE
  "Build the custom mv-requires-super clang-tidy module"
  ON
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(ENABLE_CLANG_TIDY OR BUILD_MV_REQUIRES_SUPER_MODULE)
  find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy REQUIRED)
endif()

if(ENABLE_CLANG_TIDY)
  set(CLANG_TIDY_COMMAND "${CLANG_TIDY_EXECUTABLE}")

  # Supply the include paths detected for the selected compiler. This keeps
  # clang-tidy working on systems with more than one GCC installation.
  foreach(SYSTEM_INCLUDE_DIR IN LISTS CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES)
    list(
      APPEND
      CLANG_TIDY_COMMAND
      "--extra-arg-before=-isystem${SYSTEM_INCLUDE_DIR}"
    )
  endforeach()

endif()

if(BUILD_MV_REQUIRES_SUPER_MODULE)
  add_subdirectory(clang-tidy)
endif()

if(ENABLE_CLANG_TIDY)
  if(TARGET MvRequiresSuperTidyModule)
    list(APPEND CLANG_TIDY_COMMAND "--load=${MV_REQUIRES_SUPER_PLUGIN_PATH}")
  endif()
  set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
endif()

include(CTest)
if(BUILD_TESTING)
  if(TARGET MvRequiresSuperTidyModule)
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

if(ENABLE_CLANG_TIDY)
  set(
    MV_TIDY_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/clang-tidy/RequiresSuperCheck.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/clang-tidy/RequiresSuperModule.cpp"
  )
  if(TARGET MvRequiresSuperTests)
    list(APPEND MV_TIDY_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tests/ClangTidyTests.cpp")
  endif()

  add_custom_target(
    tidy
    COMMAND ${CLANG_TIDY_COMMAND} -p "${CMAKE_BINARY_DIR}" ${MV_TIDY_SOURCES}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Running clang-tidy on all C++ source files"
    VERBATIM
  )
  if(TARGET MvRequiresSuperTidyModule)
    add_dependencies(tidy MvRequiresSuperTidyModule)
  endif()
endif()
