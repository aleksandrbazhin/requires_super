// This file intentionally fails because a call inside a nested lambda belongs
// to that lambda even when the lambda is immediately invoked.

class LambdaBase
{
public:
    [[clang::annotate("mv-requires-super")]] virtual void run()
    {
    }
};

class InvokedLambdaFailure final : public LambdaBase
{
public:
    // Fails: immediate invocation does not make this a call in the outer body.
    void run() override
    {
        [this]()
        {
            LambdaBase::run();
        }();
    }
};
