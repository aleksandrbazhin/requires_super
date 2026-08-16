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
