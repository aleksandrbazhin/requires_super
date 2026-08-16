#include "RequiresSuperCheck.hpp"

#include "clang/AST/Attr.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Basic/Diagnostic.h"
#include "llvm/ADT/SmallPtrSet.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace clang::tidy::mv
{
namespace
{

constexpr llvm::StringLiteral RequiredAnnotation{"mv-requires-super"};

const AnnotateAttr *explicitRequirement(const CXXMethodDecl &method)
{
    for (const FunctionDecl *redeclared_method : method.redecls())
    {
        for (const AnnotateAttr *annotation : redeclared_method->specific_attrs<AnnotateAttr>())
        {
            if (!annotation->isInherited() && annotation->getAnnotation() == RequiredAnnotation)
            {
                return annotation;
            }
        }
    }
    return nullptr;
}

struct RequiredBase
{
    const CXXMethodDecl *annotated_method;
    const AnnotateAttr *annotation;
    std::vector<const CXXMethodDecl *> callable_methods;
};

void findNearestRequiredBases(const CXXMethodDecl &method, std::vector<RequiredBase> &required_bases,
                              std::vector<const CXXMethodDecl *> callable_methods,
                              llvm::SmallPtrSetImpl<const CXXMethodDecl *> &active_path)
{
    const CXXMethodDecl *canonical_method = method.getCanonicalDecl();
    if (!active_path.insert(canonical_method).second)
    {
        return;
    }
    callable_methods.push_back(canonical_method);

    if (const AnnotateAttr *annotation = explicitRequirement(method))
    {
        auto existing_requirement =
            std::find_if(required_bases.begin(), required_bases.end(), [canonical_method](const RequiredBase &required_base)
                         { return required_base.annotated_method == canonical_method; });
        if (existing_requirement == required_bases.end())
        {
            required_bases.push_back({canonical_method, annotation, std::move(callable_methods)});
        }
        else
        {
            for (const CXXMethodDecl *callable_method : callable_methods)
            {
                if (std::find(existing_requirement->callable_methods.begin(),
                              existing_requirement->callable_methods.end(), callable_method) ==
                    existing_requirement->callable_methods.end())
                {
                    existing_requirement->callable_methods.push_back(callable_method);
                }
            }
        }
        active_path.erase(canonical_method);
        return;
    }

    for (const CXXMethodDecl *overridden_method : method.overridden_methods())
    {
        findNearestRequiredBases(*overridden_method, required_bases, callable_methods, active_path);
    }
    active_path.erase(canonical_method);
}

class BaseCallVisitor final : public RecursiveASTVisitor<BaseCallVisitor>
{
public:
    explicit BaseCallVisitor(const CXXMethodDecl &required_base)
        : required_base_(required_base.getCanonicalDecl())
    {
    }

    bool VisitCXXMemberCallExpr(const CXXMemberCallExpr *call)
    {
        const CXXMethodDecl *called_method = call->getMethodDecl();
        const Expr *object = call->getImplicitObjectArgument();
        const bool calls_on_current_object =
            object != nullptr && isa<CXXThisExpr>(object->IgnoreParenImpCasts());
        if (called_method != nullptr && called_method->getCanonicalDecl() == required_base_ && calls_on_current_object)
        {
            found_ = true;
        }
        return !found_;
    }

    // Match Clang's Objective-C requires-super behavior: a super call in a
    // nested block or lambda belongs to that nested callable, not this method.
    bool TraverseLambdaExpr(LambdaExpr *)
    {
        return true;
    }

    [[nodiscard]] bool found() const
    {
        return found_;
    }

private:
    const CXXMethodDecl *required_base_;
    bool found_{false};
};

bool callsRequiredBase(const CXXMethodDecl &method, const RequiredBase &required_base)
{
    const Stmt *body = method.getBody();
    if (body == nullptr)
    {
        return false;
    }

    for (const CXXMethodDecl *callable_method : required_base.callable_methods)
    {
        BaseCallVisitor visitor{*callable_method};
        visitor.TraverseStmt(const_cast<Stmt *>(body));
        if (visitor.found())
        {
            return true;
        }
    }
    return false;
}

}  // namespace

bool RequiresSuperCheck::isLanguageVersionSupported(const LangOptions &language_options) const
{
    return language_options.CPlusPlus;
}

void RequiresSuperCheck::registerMatchers(ast_matchers::MatchFinder *finder)
{
    using namespace ast_matchers;
    finder->addMatcher(cxxMethodDecl(isDefinition(), isOverride(), unless(isImplicit())).bind("overriding-method"), this);
}

void RequiresSuperCheck::check(const ast_matchers::MatchFinder::MatchResult &result)
{
    const auto *method = result.Nodes.getNodeAs<CXXMethodDecl>("overriding-method");
    if (method == nullptr || method->getBody() == nullptr)
    {
        return;
    }

    std::vector<RequiredBase> required_bases;
    llvm::SmallPtrSet<const CXXMethodDecl *, 8> active_path;
    for (const CXXMethodDecl *overridden_method : method->overridden_methods())
    {
        findNearestRequiredBases(*overridden_method, required_bases, {}, active_path);
    }

    for (const RequiredBase &required_base : required_bases)
    {
        if (callsRequiredBase(*method, required_base))
        {
            continue;
        }

        diag(method->getLocation(),
             "overriding method '%0' must call a base implementation required by annotated method '%1'")
            << method->getQualifiedNameAsString() << required_base.annotated_method->getQualifiedNameAsString();
        diag(required_base.annotation->getLocation(), "base method is marked 'mv-requires-super' here",
             DiagnosticIDs::Note);
    }
}

}  // namespace clang::tidy::mv
