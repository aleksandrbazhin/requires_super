// This file intentionally produces two mv-requires-super diagnostics.

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
    // Fails: this override never calls the annotated DirectBase::run().
    void run() override
    {
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

class GrandchildFailure final : public Parent
{
public:
    // Fails: Parent::step() is annotated, so it cannot be skipped in favor of Root::step().
    void step() override
    {
        Root::step();
    }
};
