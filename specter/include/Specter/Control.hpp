#ifndef SPECTER_CONTROL_HPP
#define SPECTER_CONTROL_HPP

#include "View.hpp"
#include "HECL/CVar.hpp"

namespace Specter
{

struct IControlBinding
{
    virtual const std::string& name() const=0;
    virtual const std::string& help() const=0;
};

struct CVarControlBinding : IControlBinding
{
    HECL::CVar* m_cvar;
    CVarControlBinding(HECL::CVar* cvar)
    : m_cvar(cvar) {}
    const std::string& name() const {return m_cvar->name();}
    const std::string& help() const {return m_cvar->rawHelp();}
};

class Control : public View
{
protected:
    std::unique_ptr<IControlBinding> m_controlBinding;
public:
    Control(ViewResources& res, View& parentView, std::unique_ptr<IControlBinding>&& controlBinding);
    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseEnter(const boo::SWindowCoord&);
    void mouseLeave(const boo::SWindowCoord&);

    void setControlBinding(std::unique_ptr<IControlBinding>&& controlBinding);
};

}

#endif // SPECTER_CONTROL_HPP
