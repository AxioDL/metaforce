#ifndef RUDE_SPACE_HPP
#define RUDE_SPACE_HPP

#include <Athena/DNAYaml.hpp>
#include <Specter/Specter.hpp>

namespace Specter
{
class View;
class SplitView;
class ViewResources;
class Toolbar;
}
namespace RUDE
{
class ViewManager;

struct SpaceState : Athena::io::DNAYaml<Athena::BigEndian> {Delete _d;};

class Space
{
public:
    enum class Class
    {
        None,
        SplitSpace,
        TestSpace,
        ResourceOutliner,
    };
protected:
    friend class ViewManager;
    ViewManager& m_vm;
    Class m_class = Class::None;
    std::unique_ptr<Specter::Space> m_space;
    Space(ViewManager& vm, Class cls) : m_vm(vm), m_class(cls) {}
    Space(ViewManager& vm, Class cls, Athena::io::IStreamReader& r) : m_vm(vm), m_class(cls)
    {if (spaceState()) spaceState()->read(r);}
    void writeState(Athena::io::IStreamWriter& w) const;

    /* Allows common Space code to access DNA-encoded state */
    virtual SpaceState* spaceState() {return nullptr;}

    /* Structural control */
    virtual bool usesToolbar() const {return false;}
    virtual void buildToolbar(Specter::ViewResources& res, Specter::Toolbar& tb) {}
    virtual Specter::View* buildContent(Specter::ViewResources& res)=0;
    Specter::Space* buildSpace(Specter::ViewResources& res);
public:
};

class SplitSpace : public Space
{
    friend class ViewManager;
    std::unique_ptr<Specter::SplitView> m_splitView;
    struct State : SpaceState
    {
        DECL_YAML
        Value<float> m_split;
    } m_state;
    SpaceState* spaceState() {return &m_state;}

public:
    SplitSpace(ViewManager& vm)
    : Space(vm, Class::SplitSpace) {}
    Specter::View* buildContent(Specter::ViewResources& res);
};

class TestSpace : public Space
{
    std::unique_ptr<Specter::Button> m_button;
    std::unique_ptr<Specter::MultiLineTextView> m_textView;

    std::string m_contentStr;
    std::string m_buttonStr;

    Specter::IButtonBinding* m_binding;

public:
    TestSpace(ViewManager& vm, const std::string& content, const std::string& button,
              Specter::IButtonBinding* binding)
    : Space(vm, Class::TestSpace), m_contentStr(content), m_buttonStr(button), m_binding(binding)
    {}

    struct State : SpaceState
    {
        DECL_YAML
    } m_state;
    SpaceState* spaceState() {return &m_state;}

    bool usesToolbar() const {return true;}
    void buildToolbar(Specter::ViewResources& res, Specter::Toolbar& tb)
    {
        m_button.reset(new Specter::Button(res, tb, m_binding, m_buttonStr));
        tb.push_back(m_button.get());
    }

    Specter::View* buildContent(Specter::ViewResources& res)
    {
        m_textView.reset(new Specter::MultiLineTextView(res, *m_space, res.m_heading14));
        m_textView->setBackground(res.themeData().viewportBackground());
        m_textView->typesetGlyphs(m_contentStr, res.themeData().uiText());
        return m_textView.get();
    }
};

}

#endif // RUDE_SPACE_HPP
