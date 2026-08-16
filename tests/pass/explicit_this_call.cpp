class ExplicitThisBase
{
public:
    [[clang::annotate("mv-requires-super")]] virtual void run()
    {
    }
};

class ExplicitThisSuccess final : public ExplicitThisBase
{
public:
    void run() override
    {
        this->ExplicitThisBase::run();
    }
};
