// This file intentionally fails because every annotated override branch must
// receive its own base call.

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

class PartialFailure final : public LeftBase, public RightBase
{
public:
    // Fails: RightBase::reset() is missing even though LeftBase::reset() is called.
    void reset() override
    {
        LeftBase::reset();
    }
};
