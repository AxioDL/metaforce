#ifndef CTOOL_BASE
#define CTOOL_BASE

class CToolBase
{
protected:
    const std::vector<std::string>& m_args;
public:
    CToolBase(const std::vector<std::string>& args)
    : m_args(args) {}
    virtual ~CToolBase() {}
};

#endif // CTOOL_BASE
