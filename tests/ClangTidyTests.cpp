#include "TestConfig.hpp"

#include <catch2/catch.hpp>

#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Program.h"

#include <fstream>
#include <iterator>
#include <string>
#include <system_error>
#include <utility>

namespace
{

struct TidyResult
{
    int exit_code;
    std::string output;
};

struct FailureExpectation
{
    llvm::StringRef fixture;
    llvm::StringRef overriding_method;
    llvm::StringRef required_method;
};

std::string readFile(const llvm::StringRef path)
{
    std::ifstream input{path.str()};
    return {std::istreambuf_iterator<char>{input}, std::istreambuf_iterator<char>{}};
}

TidyResult runClangTidy(const llvm::StringRef relative_fixture)
{
    llvm::SmallString<128> stdout_path;
    llvm::SmallString<128> stderr_path;
    const std::error_code stdout_error = llvm::sys::fs::createTemporaryFile("mv-requires-super", "stdout", stdout_path);
    const std::error_code stderr_error = llvm::sys::fs::createTemporaryFile("mv-requires-super", "stderr", stderr_path);
    REQUIRE_FALSE(stdout_error);
    REQUIRE_FALSE(stderr_error);

    const std::string fixture_path =
        (llvm::Twine{mv::test_config::FixtureDirectory} + "/" + relative_fixture).str();
    const std::string load_argument = (llvm::Twine{"--load="} + mv::test_config::PluginPath).str();

    const llvm::SmallVector<llvm::StringRef, 8> arguments{
        mv::test_config::ClangTidyExecutable,
        load_argument,
        "--checks=-*,mv-requires-super",
        "--warnings-as-errors=mv-requires-super",
        fixture_path,
        "--",
        "-std=c++20",
    };
    const llvm::SmallVector<llvm::Optional<llvm::StringRef>, 3> redirects{
        llvm::StringRef{""},
        llvm::StringRef{stdout_path},
        llvm::StringRef{stderr_path},
    };

    std::string execution_error;
    const int exit_code = llvm::sys::ExecuteAndWait(mv::test_config::ClangTidyExecutable, arguments, llvm::None,
                                                     redirects, 30, 0, &execution_error);
    std::string output = readFile(stdout_path) + readFile(stderr_path) + execution_error;
    llvm::sys::fs::remove(stdout_path);
    llvm::sys::fs::remove(stderr_path);
    return {exit_code, std::move(output)};
}

void requirePass(const llvm::StringRef fixture)
{
    const TidyResult result = runClangTidy(fixture);
    INFO(result.output);
    REQUIRE(result.exit_code == 0);
    REQUIRE(result.output.find("[mv-requires-super") == std::string::npos);
}

void requireFailure(const FailureExpectation &expectation)
{
    const TidyResult result = runClangTidy(expectation.fixture);
    INFO(result.output);
    REQUIRE(result.exit_code != 0);
    REQUIRE(result.output.find(expectation.overriding_method.str()) != std::string::npos);
    REQUIRE(result.output.find(expectation.required_method.str()) != std::string::npos);
    REQUIRE(result.output.find("[mv-requires-super,-warnings-as-errors]") != std::string::npos);
}

}  // namespace

TEST_CASE("a direct override calls its annotated base", "[pass]")
{
    requirePass("pass/direct_call.cpp");
}

TEST_CASE("an explicit this base call satisfies the requirement", "[pass]")
{
    requirePass("pass/explicit_this_call.cpp");
}

TEST_CASE("a conditional base call satisfies the existence check", "[pass][control-flow]")
{
    requirePass("pass/conditional_call.cpp");
}

TEST_CASE("multiple base calls satisfy the requirement", "[pass]")
{
    requirePass("pass/multiple_calls.cpp");
}

TEST_CASE("an override without an annotation requirement needs no base call", "[pass]")
{
    requirePass("pass/unannotated_override.cpp");
}

TEST_CASE("a grandchild may call an unannotated parent", "[pass][hierarchy]")
{
    requirePass("pass/unannotated_parent_call.cpp");
}

TEST_CASE("a grandchild may skip an unannotated parent", "[pass][hierarchy]")
{
    requirePass("pass/unannotated_parent_skip.cpp");
}

TEST_CASE("a grandchild calls an explicitly annotated parent", "[pass][hierarchy]")
{
    requirePass("pass/annotated_parent_call.cpp");
}

TEST_CASE("an override calls every annotated base in multiple inheritance", "[pass][multiple-inheritance]")
{
    requirePass("pass/multiple_inheritance.cpp");
}

TEST_CASE("a missing direct base call is diagnosed", "[fail]")
{
    requireFailure({"fail/missing_direct_call.cpp", "DirectFailure::run", "DirectBase::run"});
}

TEST_CASE("an annotated parent cannot be skipped", "[fail][hierarchy]")
{
    requireFailure({"fail/annotated_parent_skipped.cpp", "GrandchildFailure::step", "Parent::step"});
}

TEST_CASE("calling only one annotated base is diagnosed", "[fail][multiple-inheritance]")
{
    requireFailure({"fail/multiple_inheritance_partial.cpp", "PartialFailure::reset", "RightBase::reset"});
}

TEST_CASE("a recursive self-call is not a base call", "[fail]")
{
    requireFailure({"fail/recursive_self_call.cpp", "RecursiveFailure::refresh", "RefreshBase::refresh"});
}

TEST_CASE("a base call inside a stored lambda does not count", "[fail][lambda]")
{
    requireFailure({"fail/lambda_call.cpp", "LambdaFailure::run", "LambdaBase::run"});
}

TEST_CASE("a base call inside an immediately invoked lambda does not count", "[fail][lambda]")
{
    requireFailure({"fail/invoked_lambda_call.cpp", "InvokedLambdaFailure::run", "LambdaBase::run"});
}

TEST_CASE("a base call on another object does not count", "[fail]")
{
    requireFailure({"fail/other_object_call.cpp", "OtherObjectFailure::run", "ObjectBase::run"});
}
