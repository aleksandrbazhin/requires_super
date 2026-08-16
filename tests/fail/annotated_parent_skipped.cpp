// This file intentionally fails because an explicitly annotated parent may
// not be skipped in favor of an older annotated ancestor.

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
    // Fails: Parent::step() is annotated, but this calls Root::step() instead.
    void step() override
    {
        Root::step();
    }
};
