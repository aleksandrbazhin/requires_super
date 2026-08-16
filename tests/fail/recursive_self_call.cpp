// This file intentionally fails because recursively calling the overriding
// method is not equivalent to calling its annotated base implementation.

class RefreshBase
{
public:
    [[clang::annotate("mv-requires-super")]] virtual void refresh()
    {
    }
};

class RecursiveFailure final : public RefreshBase
{
public:
    // Fails: this resolves to RecursiveFailure::refresh(), not RefreshBase::refresh().
    void refresh() override
    {
        refresh();
    }
};
