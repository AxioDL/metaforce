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

    struct StateHead : Athena::io::DNAYaml<Athena::BigEndian>
    {
        DECL_YAML
        Value<Class> cls;
    };
    struct State : Athena::io::DNAYaml<Athena::BigEndian> {Delete _d;};

    static Space* NewSpaceFromYAMLStream(ViewManager& vm, Athena::io::YAMLDocReader& r);

protected:
    friend class ViewManager;
    ViewManager& m_vm;
    Class m_class = Class::None;
    std::unique_ptr<Specter::Space> m_space;
    Space(ViewManager& vm, Class cls) : m_vm(vm), m_class(cls) {}

    /* Allows common Space code to access DNA-encoded state */
    virtual Space::State& spaceState()=0;

    /* Structural control */
    virtual bool usesToolbar() const {return false;}
    virtual void buildToolbarView(Specter::ViewResources& res, Specter::Toolbar& tb) {}
    virtual Specter::View* buildContentView(Specter::ViewResources& res)=0;
    Specter::Space* buildSpaceView(Specter::ViewResources& res);
public:
};

class SplitSpace : public Space
{
    friend class ViewManager;
    std::unique_ptr<Space> m_a;
    std::unique_ptr<Space> m_b;
    std::unique_ptr<Specter::SplitView> m_splitView;
    struct State : Space::State
    {
        DECL_YAML
        Value<float> split;
    } m_state;
    Space::State& spaceState() {return m_state;}

public:
    SplitSpace(ViewManager& vm) : Space(vm, Class::SplitSpace) {}
    SplitSpace(ViewManager& vm, Athena::io::YAMLDocReader& r)
    : SplitSpace(vm)
    {
        m_state.read(r);
        m_a.reset(NewSpaceFromYAMLStream(vm, r));
        m_b.reset(NewSpaceFromYAMLStream(vm, r));
    }

    Specter::View* buildContentView(Specter::ViewResources& res);
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

    struct State : Space::State
    {
        DECL_YAML
    } m_state;
    Space::State& spaceState() {return m_state;}

    bool usesToolbar() const {return true;}
    void buildToolbarView(Specter::ViewResources& res, Specter::Toolbar& tb)
    {
        m_button.reset(new Specter::Button(res, tb, m_binding, m_buttonStr));
        tb.push_back(m_button.get());
    }

    Specter::View* buildContentView(Specter::ViewResources& res)
    {
        m_textView.reset(new Specter::MultiLineTextView(res, *m_space, res.m_heading14));
        m_textView->setBackground(res.themeData().viewportBackground());
        m_textView->typesetGlyphs(m_contentStr, res.themeData().uiText());
        return m_textView.get();
    }
};

}

#endif // RUDE_SPACE_HPP
