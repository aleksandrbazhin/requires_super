class Root
{
public:
    [[clang::annotate("mv-requires-super")]] virtual void step()
    {
    }
};

class Parent : public Root
{
public:
    [[clang::annotate("mv-requires-super")]] void step() override
    {
        Root::step();
    }
};

class GrandchildSuccess final : public Parent
{
public:
    void step() override
    {
        Parent::step();
    }
};
