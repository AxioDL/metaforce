#ifndef SPECTER_CONTROL_HPP
#define SPECTER_CONTROL_HPP

#include "View.hpp"
#include "HECL/CVar.hpp"

namespace Specter
{

struct IControlBinding
{
    virtual const char* name() const=0;
    virtual const char* help() const {return nullptr;}
};

struct IButtonBinding : IControlBinding
{
    /** Pressed/Released while Hovering action,
     *  cancellable by holding the button and releasing outside */
    virtual void activated(const boo::SWindowCoord& coord)=0;
};

struct IFloatBinding : IControlBinding
{
    virtual float getDefault() const {return 0.0;}
    virtual std::pair<float,float> getBounds() const {return std::make_pair(FLT_MIN, FLT_MAX);}
    virtual void changed(float val)=0;
};

struct IIntBinding : IControlBinding
{
    virtual int getDefault() const {return 0;}
    virtual std::pair<int,int> getBounds() const {return std::make_pair(INT_MIN, INT_MAX);}
    virtual void changed(int val)=0;
};

struct IStringBinding : IControlBinding
{
    virtual std::string getDefault() const {return "";}
    virtual void changed(const std::string& val)=0;
};

struct CVarControlBinding : IControlBinding
{
    HECL::CVar* m_cvar;
    CVarControlBinding(HECL::CVar* cvar)
    : m_cvar(cvar) {}
    const char* name() const {return m_cvar->name().c_str();}
    const char* help() const {return m_cvar->rawHelp().c_str();}
};

class Control : public View
{
protected:
    IControlBinding* m_controlBinding = nullptr;
public:
    Control(ViewResources& res, View& parentView, IControlBinding* controlBinding);
    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseEnter(const boo::SWindowCoord&);
    void mouseLeave(const boo::SWindowCoord&);

    IControlBinding* setControlBinding(IControlBinding* controlBinding);
};
    
class ITextInputView : public Control, public boo::ITextInputCallback
{
protected:
    static std::recursive_mutex m_textInputLk;
    ITextInputView(ViewResources& res, View& parentView,
                   IControlBinding* controlBinding)
    : Control(res, parentView, controlBinding) {}
public:
    virtual void clipboardCopy() {}
    virtual void clipboardCut() {}
    virtual void clipboardPaste() {}
};

}

#endif // SPECTER_CONTROL_HPP
