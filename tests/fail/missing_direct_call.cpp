// This file intentionally fails because DirectFailure::run() does not call
// the annotated DirectBase::run() implementation.

class DirectBase
{
public:
    [[clang::annotate("mv-requires-super")]] virtual void run()
    {
    }
};

class DirectFailure final : public DirectBase
{
public:
    // Fails: the required DirectBase::run() call is missing.
    void run() override
    {
    }
};
