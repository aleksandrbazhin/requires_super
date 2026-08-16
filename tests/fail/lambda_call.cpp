// This file intentionally fails because calls inside nested lambdas do not
// satisfy the enclosing overriding method's requirement.

class LambdaBase
{
public:
    [[clang::annotate("mv-requires-super")]] virtual void run()
    {
    }
};

class LambdaFailure final : public LambdaBase
{
public:
    // Fails: LambdaBase::run() appears only inside an uninvoked lambda.
    void run() override
    {
        const auto call_base = [this]()
        {
            LambdaBase::run();
        };
        (void)call_base;
    }
};
