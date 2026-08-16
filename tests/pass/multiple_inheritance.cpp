class LeftBase
{
public:
    [[clang::annotate("mv-requires-super")]] virtual void reset()
    {
    }
};

class RightBase
{
public:
    [[clang::annotate("mv-requires-super")]] virtual void reset()
    {
    }
};

class CompleteSuccess final : public LeftBase, public RightBase
{
public:
    void reset() override
    {
        LeftBase::reset();
        RightBase::reset();
    }
};
