#ifndef __URDE_CIOWIN_HPP__
#define __URDE_CIOWIN_HPP__

#include <string>
#include <memory>

#include "RetroTypes.hpp"

namespace urde
{
class CArchitectureMessage;
class CArchitectureQueue;

class CIOWin
{
    std::string x4_name;
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
    CIOWin(const char* name) : x4_name(name) {m_nameHash = std::hash<std::string>()(x4_name);}
    virtual EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&)=0;
    virtual bool GetIsContinueDraw() const {return true;}
    virtual void Draw() const {}
    virtual void PreDraw() const {}
    const std::string& GetName() const {return x4_name;}
    size_t GetNameHash() const {return m_nameHash;}
};

static bool operator==(std::shared_ptr<CIOWin> a, std::shared_ptr<CIOWin> b)
{
    return a.get() == b.get();
}

}

#endif // __URDE_CIOWIN_HPP__
