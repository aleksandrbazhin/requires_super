#include "RequiresSuperCheck.hpp"

#include "clang-tidy/ClangTidyModule.h"
#include "clang-tidy/ClangTidyModuleRegistry.h"

namespace clang::tidy::mv
{
namespace
{

class RequiresSuperModule final : public ClangTidyModule
{
public:
    void addCheckFactories(ClangTidyCheckFactories &factories) override
    {
        factories.registerCheck<RequiresSuperCheck>("mv-requires-super");
    }
};

const ClangTidyModuleRegistry::Add<RequiresSuperModule> registered_module{
    "mv-requires-super-module", "Adds the mv-requires-super check."};

}  // namespace

// Keep an anchor available when this module is linked into another library.
volatile int RequiresSuperModuleAnchorSource = 0;

}  // namespace clang::tidy::mv
