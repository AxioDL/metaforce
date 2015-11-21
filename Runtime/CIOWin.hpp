#ifndef __RETRO_CIOWIN_HPP__
#define __RETRO_CIOWIN_HPP__

#include <string>
#include "rstl.hpp"

namespace Retro
{
class CArchitectureMessage;
class CArchitectureQueue;

class CIOWin
{
    std::string m_name;
    size_t m_nameHash;
public:
    enum class EMessageReturn
    {
        Normal = 0,
        Exit = 1,
        RemoveIOWinAndExit = 2,
        RemoveIOWin = 3
    };
    virtual ~CIOWin() {}
    CIOWin(const char* name) : m_name(name) {m_nameHash = std::hash<std::string>()(m_name);}
    virtual EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&)=0;
    virtual bool GetIsContinueDraw() const {return true;}
    virtual void Draw() const {}
    virtual void PreDraw() const {}
    const std::string& GetName() const {return m_name;}
    size_t GetNameHash() const {return m_nameHash;}
};

static bool operator==(rstl::rc_ptr<CIOWin> a, rstl::rc_ptr<CIOWin> b)
{
    return a.get() == b.get();
}

}

#endif // __RETRO_CIOWIN_HPP__
