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
