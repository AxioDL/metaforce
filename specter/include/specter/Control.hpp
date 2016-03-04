#ifndef SPECTER_CONTROL_HPP
#define SPECTER_CONTROL_HPP

#include "View.hpp"

namespace specter
{
class Control;
class Button;

struct IControlBinding
{
    virtual const char* name(const Control* control) const=0;
    virtual const char* help(const Control* control) const {return nullptr;}
};

struct IButtonBinding : IControlBinding
{
    /** Pressed/Released while Hovering action,
     *  cancellable by holding the button and releasing outside */
    virtual void activated(const Button* button, const boo::SWindowCoord& coord) {}

    /** Pass-through down action */
    virtual void down(const Button* button, const boo::SWindowCoord& coord) {}

    /** Pass-through up action */
    virtual void up(const Button* button, const boo::SWindowCoord& coord) {}

    /** Optional style of menu to bind to button */
    enum class MenuStyle
    {
        None, /**< No menu; normal button */
        Primary, /**< Menu button replaces normal button */
        Auxiliary /**< Menu button placed alongside normal button */
    };

    /** Informs button which MenuStyle to present to user */
    virtual MenuStyle menuStyle(const specter::Button* button) const {return MenuStyle::None;}

    /** Called when user requests menu, Button assumes modal ownership */
    virtual std::unique_ptr<View> buildMenu(const specter::Button* button) {return std::unique_ptr<View>();}
};

struct IFloatBinding : IControlBinding
{
    virtual float getDefault(const Control* control) const {return 0.0;}
    virtual std::pair<float,float> getBounds(const Control* control) const {return std::make_pair(FLT_MIN, FLT_MAX);}
    virtual void changed(const Control* control, float val)=0;
};

struct IIntBinding : IControlBinding
{
    virtual int getDefault(const Control* control) const {return 0;}
    virtual std::pair<int,int> getBounds(const Control* control) const {return std::make_pair(INT_MIN, INT_MAX);}
    virtual void changed(const Control* control, int val)=0;
};

struct IStringBinding : IControlBinding
{
    virtual std::string getDefault(const Control* control) const {return "";}
    virtual void changed(const Control* control, const std::string& val)=0;
};

struct CVarControlBinding : IControlBinding
{
    hecl::CVar* m_cvar;
    CVarControlBinding(hecl::CVar* cvar)
    : m_cvar(cvar) {}
    const char* name(const Control* control) const {return m_cvar->name().c_str();}
    const char* help(const Control* control) const {return m_cvar->rawHelp().c_str();}
};

class Control : public View
{
protected:
    IControlBinding* m_controlBinding = nullptr;
public:
    Control(ViewResources& res, View& parentView, IControlBinding* controlBinding);
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
