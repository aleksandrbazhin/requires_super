find_package(LLVM 14 REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

if(NOT LLVM_VERSION_MAJOR EQUAL 14)
  message(
    FATAL_ERROR
    "This module targets LLVM/Clang 14, but LLVM ${LLVM_PACKAGE_VERSION} was found"
  )
endif()

add_library(
  MvRequiresSuperTidyModule
  MODULE
  RequiresSuperCheck.cpp
  RequiresSuperCheck.hpp
  RequiresSuperModule.cpp
)

target_include_directories(
  MvRequiresSuperTidyModule
  SYSTEM PRIVATE
  "${LLVM_INCLUDE_DIRS}"
  "${CLANG_INCLUDE_DIRS}"
)
target_compile_definitions(MvRequiresSuperTidyModule PRIVATE ${LLVM_DEFINITIONS})
target_compile_features(MvRequiresSuperTidyModule PRIVATE cxx_std_17)
# Use Clang's shared library so the plugin and clang-tidy share one module
# registry. Linking the static clangTidy target here creates a second registry.
target_link_libraries(MvRequiresSuperTidyModule PRIVATE clang-cpp)

set(MV_REQUIRES_SUPER_PLUGIN_DIRECTORY "${CMAKE_BINARY_DIR}/clang-tidy")
set(MV_REQUIRES_SUPER_PLUGIN_PATH
    "${MV_REQUIRES_SUPER_PLUGIN_DIRECTORY}/MvRequiresSuperTidyModule${CMAKE_SHARED_MODULE_SUFFIX}"
    PARENT_SCOPE)

set_target_properties(
  MvRequiresSuperTidyModule
  PROPERTIES
    CXX_CLANG_TIDY ""
    LIBRARY_OUTPUT_DIRECTORY "${MV_REQUIRES_SUPER_PLUGIN_DIRECTORY}"
    PREFIX ""
)
