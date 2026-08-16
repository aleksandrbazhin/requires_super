// This file intentionally fails because the base implementation must be
// invoked on the current object, like Objective-C's super receiver.

class ObjectBase
{
public:
    [[clang::annotate("mv-requires-super")]] virtual void run()
    {
    }
};

class OtherObjectFailure final : public ObjectBase
{
public:
    // Fails: ObjectBase::run() is called on another object, not this object.
    void run() override
    {
        ObjectBase other;
        other.run();
    }
};
