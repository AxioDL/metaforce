#ifndef CTOOL_BASE
#define CTOOL_BASE

#include <string>
#include <vector>
#include <HECLDatabase.hpp>

struct SToolPassInfo
{
    std::string pname;
    std::vector<std::string> args;
    std::string output;
    unsigned verbosityLevel = 0;
    bool force = false;
};

class CToolBase
{
protected:
    const SToolPassInfo& m_info;
public:
    CToolBase(const SToolPassInfo& info)
    : m_info(info) {}
    virtual ~CToolBase() {}
    virtual std::string toolName() const=0;
    virtual int run()=0;
};

#endif // CTOOL_BASE
