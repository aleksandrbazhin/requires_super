extern bool condition;

class ConditionalBase
{
public:
    [[clang::annotate("mv-requires-super")]] virtual void update()
    {
    }
};

class ConditionalSuccess final : public ConditionalBase
{
public:
    void update() override
    {
        if (condition)
        {
            ConditionalBase::update();
        }
    }
};
