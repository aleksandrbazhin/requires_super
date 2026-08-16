class RepeatedBase
{
public:
    [[clang::annotate("mv-requires-super")]] virtual void synchronize()
    {
    }
};

class RepeatedSuccess final : public RepeatedBase
{
public:
    void synchronize() override
    {
        RepeatedBase::synchronize();
        RepeatedBase::synchronize();
    }
};
