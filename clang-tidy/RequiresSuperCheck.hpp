#ifndef MV_REQUIRES_SUPER_CHECK_HPP
#define MV_REQUIRES_SUPER_CHECK_HPP

#include "clang-tidy/ClangTidyCheck.h"

namespace clang::tidy::mv
{

class RequiresSuperCheck final : public ClangTidyCheck
{
public:
    RequiresSuperCheck(llvm::StringRef name, ClangTidyContext *context)
        : ClangTidyCheck(name, context)
    {
    }

    bool isLanguageVersionSupported(const LangOptions &language_options) const override;
    void registerMatchers(ast_matchers::MatchFinder *finder) override;
    void check(const ast_matchers::MatchFinder::MatchResult &result) override;
};

}  // namespace clang::tidy::mv

#endif  // MV_REQUIRES_SUPER_CHECK_HPP
