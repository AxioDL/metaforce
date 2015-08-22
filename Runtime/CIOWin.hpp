#ifndef __RETRO_CIOWIN_HPP__
#define __RETRO_CIOWIN_HPP__

#include <string>

namespace Retro
{
class CArchitectureMessage;
class CArchitectureQueue;

class CIOWin
{
    const char* m_name;
public:
    virtual ~CIOWin() {}
    CIOWin(const char* name) : m_name(name) {}
    virtual bool OnMessage(const CArchitectureMessage&, CArchitectureQueue&)=0;
    virtual bool GetIsContinueDraw() const {return true;}
    virtual void Draw() const {}
    virtual void PreDraw() const {}
};

}

#endif // __RETRO_CIOWIN_HPP__
