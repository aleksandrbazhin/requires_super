class DirectBase
{
public:
    [[clang::annotate("mv-requires-super")]] virtual void run()
    {
    }
};

class DirectSuccess final : public DirectBase
{
public:
    void run() override
    {
        DirectBase::run();
    }
};

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

class UnrestrictedBase
{
public:
    virtual void execute()
    {
    }
};

class UnrestrictedChild final : public UnrestrictedBase
{
public:
    void execute() override
    {
    }
};
