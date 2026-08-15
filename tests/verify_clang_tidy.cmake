if(NOT DEFINED CLANG_TIDY OR NOT DEFINED PLUGIN OR NOT DEFINED SOURCE)
  message(FATAL_ERROR "CLANG_TIDY, PLUGIN, and SOURCE are required")
endif()

execute_process(
  COMMAND
    "${CLANG_TIDY}"
    "--load=${PLUGIN}"
    "--checks=-*,mv-requires-super"
    "--warnings-as-errors=mv-requires-super"
    "${SOURCE}"
    --
    -std=c++20
  RESULT_VARIABLE tidy_result
  OUTPUT_VARIABLE tidy_stdout
  ERROR_VARIABLE tidy_stderr
)

set(tidy_output "${tidy_stdout}${tidy_stderr}")

if(EXPECT_SUCCESS)
  if(NOT tidy_result EQUAL 0)
    message(FATAL_ERROR "Expected clang-tidy to pass:\n${tidy_output}")
  endif()
else()
  if(tidy_result EQUAL 0)
    message(FATAL_ERROR "Expected clang-tidy to report an error")
  endif()

  foreach(expected_index RANGE 1 2)
    set(expected_variable "EXPECT_TEXT_${expected_index}")
    if(DEFINED ${expected_variable} AND
       NOT tidy_output MATCHES "${${expected_variable}}")
      message(
        FATAL_ERROR
        "Missing expected diagnostic '${${expected_variable}}':\n${tidy_output}"
      )
    endif()
  endforeach()
endif()
