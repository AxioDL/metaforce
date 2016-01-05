#ifndef URDE_SPACE_HPP
#define URDE_SPACE_HPP

#include <Athena/DNAYaml.hpp>
#include <Specter/Specter.hpp>
#include "ProjectManager.hpp"

namespace Specter
{
class View;
class SplitView;
class ViewResources;
class Toolbar;
}
namespace URDE
{
class ViewManager;

class Space
{
    friend class SplitSpace;
public:
    Space(const Space& other) = delete;
    Space& operator=(const Space& other) = delete;

    enum class Class
    {
        None,
        SplitSpace,
        TestSpace,
        ResourceOutliner,
    };

    struct State : Athena::io::DNAYaml<Athena::BigEndian> {Delete _d;};
    static Space* NewSpaceFromConfigStream(ViewManager& vm, ConfigReader& r);

protected:
    friend class ViewManager;
    ViewManager& m_vm;
    Class m_class = Class::None;
    std::unique_ptr<Specter::Space> m_space;
    Space(ViewManager& vm, Class cls) : m_vm(vm), m_class(cls) {}

    /* Allows common Space code to access DNA-encoded state */
    virtual const Space::State& spaceState() const=0;

    /* Structural control */
    virtual bool usesToolbar() const {return false;}
    virtual void buildToolbarView(Specter::ViewResources& res, Specter::Toolbar& tb) {}
    virtual Specter::View* buildContentView(Specter::ViewResources& res)=0;
    virtual Specter::View* buildSpaceView(Specter::ViewResources& res);

public:
    virtual void saveState(Athena::io::IStreamWriter& w) const
    {
        w.writeUint32Big(atUint32(m_class));
        spaceState().write(w);
    }

    virtual void saveState(Athena::io::YAMLDocWriter& w) const
    {
        w.writeUint32("class", atUint32(m_class));
        spaceState().write(w);
    }

    virtual void think() {}
};

class SplitSpace : public Space
{
    friend class ViewManager;
    std::unique_ptr<Space> m_slots[2];
    std::unique_ptr<Specter::SplitView> m_splitView;
    struct State : Space::State
    {
        DECL_YAML
        Value<float> split;
    } m_state;
    const Space::State& spaceState() const {return m_state;}

public:
    SplitSpace(ViewManager& vm) : Space(vm, Class::SplitSpace) {}
    SplitSpace(ViewManager& vm, ConfigReader& r)
    : SplitSpace(vm)
    {
        m_state.read(r);
#ifdef URDE_BINARY_CONFIGS
        m_slots[0].reset(NewSpaceFromConfigStream(vm, r));
        m_slots[1].reset(NewSpaceFromConfigStream(vm, r));
#else
        r.enterSubRecord("slot0");
        m_slots[0].reset(NewSpaceFromConfigStream(vm, r));
        r.leaveSubRecord();
        r.enterSubRecord("slot1");
        m_slots[1].reset(NewSpaceFromConfigStream(vm, r));
        r.leaveSubRecord();
#endif
    }

    void saveState(Athena::io::IStreamWriter& w) const
    {
        w.writeUint32Big(atUint32(m_class));
        m_state.write(w);

        if (m_slots[0])
            m_slots[0]->saveState(w);
        else
            w.writeUint32Big(0);


        if (m_slots[1])
            m_slots[1]->saveState(w);
        else
            w.writeUint32Big(0);
    }

    void saveState(Athena::io::YAMLDocWriter& w) const
    {
        w.writeUint32("class", atUint32(m_class));
        m_state.write(w);

        w.enterSubRecord("slot0");
        if (m_slots[0])
            m_slots[0]->saveState(w);
        else
            w.writeUint32("class", 0);
        w.leaveSubRecord();

        w.enterSubRecord("slot1");
        if (m_slots[1])
            m_slots[1]->saveState(w);
        else
            w.writeUint32("class", 0);
        w.leaveSubRecord();
    }

    void setSpaceSlot(unsigned slot, std::unique_ptr<Space>&& space);

    Specter::View* buildSpaceView(Specter::ViewResources& res) {return buildContentView(res);}
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

#endif // URDE_SPACE_HPP
