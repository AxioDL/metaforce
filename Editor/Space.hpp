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
class RootSpace;

class Space : public Specter::ISpaceController
{
    friend class SplitSpace;
public:
    virtual ~Space() = default;
    Space(const Space& other) = delete;
    Space& operator=(const Space& other) = delete;

    /** Common encoded-enumeration for all space classes */
    enum class Class
    {
        None,
        RootSpace,
        SplitSpace,
        TestSpace,
        ResourceBrowser,
        ModelViewer,
        EffectEditor
    };

    struct State : Athena::io::DNAYaml<Athena::BigEndian> {Delete _d;};
    static Space* NewSpaceFromConfigStream(ViewManager& vm, Space* parent, ConfigReader& r);
    static RootSpace* NewRootSpaceFromConfigStream(ViewManager& vm, ConfigReader& r);

    struct SpaceMenuNode : Specter::IMenuNode
    {
        struct SubNodeData : Specter::IMenuNode
        {
            Class m_cls;
            std::string m_key;
            std::string m_text;
            Specter::Icon& m_icon;
            Zeus::CColor m_color;
            const std::string* text() const {return &m_text;}
            void activated(const boo::SWindowCoord& coord) {}

            SubNodeData(Class cls, const char* key, const char* text, Specter::Icon& icon, const Zeus::CColor& color)
            : m_cls(cls), m_key(key), m_text(text), m_icon(icon), m_color(color) {}
        };
        static std::vector<SubNodeData> s_subNodeDats;

        struct SubNode : Specter::IMenuNode
        {
            Space& m_space;
            const SubNodeData& m_data;
            const std::string* text() const {return &m_data.m_text;}
            void activated(const boo::SWindowCoord& coord) {}

            SubNode(Space& space, const SubNodeData& data) : m_space(space), m_data(data) {}
        };
        std::vector<SubNode> m_subNodes;

        SpaceMenuNode(Space& space)
        {
            m_subNodes.reserve(s_subNodeDats.size());
            for (const SubNodeData& sn : s_subNodeDats)
                m_subNodes.emplace_back(space, sn);
        }

        static std::string s_text;
        const std::string* text() const {return &s_text;}

        size_t subNodeCount() const {return m_subNodes.size();}
        IMenuNode* subNode(size_t idx) {return &m_subNodes[idx];}

        static void InitializeStrings(ViewManager& vm);
        static const std::string* LookupClassString(Class cls)
        {
            for (const SubNodeData& sn : s_subNodeDats)
                if (sn.m_cls == cls)
                    return &sn.m_text;
            return nullptr;
        }
        static Specter::Icon* LookupClassIcon(Class cls)
        {
            for (SubNodeData& sn : s_subNodeDats)
                if (sn.m_cls == cls)
                    return &sn.m_icon;
            return nullptr;
        }
        static const Zeus::CColor* LookupClassColor(Class cls)
        {
            for (SubNodeData& sn : s_subNodeDats)
                if (sn.m_cls == cls)
                    return &sn.m_color;
            return nullptr;
        }
    } m_spaceMenuNode;

    struct SpaceSelectBind : Specter::IButtonBinding
    {
        Space& m_space;
        const char* name(const Specter::Control* control) const {return SpaceMenuNode::s_text.c_str();}

        MenuStyle menuStyle(const Specter::Button* button) const {return MenuStyle::Primary;}
        std::unique_ptr<Specter::View> buildMenu(const Specter::Button* button);

        SpaceSelectBind(Space& space) : m_space(space) {}
    } m_spaceSelectBind;
    std::unique_ptr<Specter::Button> m_spaceSelectButton;

protected:
    friend class ViewManager;
    friend class RootSpace;
    ViewManager& m_vm;
    Class m_class = Class::None;
    Space* m_parent;
    std::unique_ptr<Specter::Space> m_spaceView;
    Space(ViewManager& vm, Class cls, Space* parent);

    /* Allows common Space code to access DNA-encoded state */
    virtual const Space::State& spaceState() const=0;

    /* Structural control */
    virtual bool usesToolbar() const {return false;}
    virtual unsigned toolbarUnits() const {return 1;}
    virtual void buildToolbarView(Specter::ViewResources& res, Specter::Toolbar& tb) {}
    virtual Specter::View* buildContentView(Specter::ViewResources& res)=0;
    virtual Specter::View* buildSpaceView(Specter::ViewResources& res);

public:
    virtual void saveState(Athena::io::IStreamWriter& w) const;
    virtual void saveState(Athena::io::YAMLDocWriter& w) const;
    virtual void reloadState() {}

    virtual void think() {}

    virtual Space* copy(Space* parent) const=0;
    bool spaceSplitAllowed() const {return true;}

    Specter::ISplitSpaceController* spaceSplit(Specter::SplitView::Axis axis, int thisSlot);
    virtual std::unique_ptr<Space> exchangeSpaceSplitJoin(Space* removeSpace, std::unique_ptr<Space>&& keepSpace)
    {return std::unique_ptr<Space>();}

    virtual Specter::View* basisView() {return m_spaceView.get();}
};

class RootSpace : public Space
{
    friend class ViewManager;
    std::unique_ptr<Specter::RootView> m_rootView;
    std::unique_ptr<Space> m_spaceTree;
    struct State : Space::State
    {
        DECL_YAML
    } m_state;
    const Space::State& spaceState() const {return m_state;}

public:
    RootSpace(ViewManager& vm) : Space(vm, Class::RootSpace, nullptr) {}
    RootSpace(ViewManager& vm, ConfigReader& r)
    : RootSpace(vm)
    {
        m_state.read(r);
#ifdef URDE_BINARY_CONFIGS
        m_spaceTree.reset(NewSpaceFromConfigStream(vm, this, r));
#else
        r.enterSubRecord("spaceTree");
        m_spaceTree.reset(NewSpaceFromConfigStream(vm, this, r));
        r.leaveSubRecord();
#endif
    }

    void saveState(Athena::io::IStreamWriter& w) const
    {
        w.writeUint32Big(atUint32(m_class));
        m_state.write(w);

        if (m_spaceTree)
            m_spaceTree->saveState(w);
        else
            w.writeUint32Big(0);
    }

    void saveState(Athena::io::YAMLDocWriter& w) const
    {
        w.writeUint32("class", atUint32(m_class));
        m_state.write(w);

        w.enterSubRecord("spaceTree");
        if (m_spaceTree)
            m_spaceTree->saveState(w);
        else
            w.writeUint32("class", 0);
        w.leaveSubRecord();
    }

    void setChild(std::unique_ptr<Space>&& space)
    {
        m_spaceTree = std::move(space);
        m_spaceTree->m_parent = this;
    }

    Space* copy(Space* parent) const {return nullptr;}
    bool spaceSplitAllowed() const {return false;}

    Specter::View* buildSpaceView(Specter::ViewResources& res);
    Specter::View* buildContentView(Specter::ViewResources& res) {return m_spaceTree->buildSpaceView(res);}

    std::unique_ptr<Space> exchangeSpaceSplitJoin(Space* removeSpace, std::unique_ptr<Space>&& keepSpace);

    Specter::View* basisView();
};

class SplitSpace : public Space, public Specter::ISplitSpaceController
{
    friend class ViewManager;
    std::unique_ptr<Space> m_slots[2];
    std::unique_ptr<Specter::SplitView> m_splitView;
    struct State : Space::State
    {
        DECL_YAML
        Value<Specter::SplitView::Axis> axis = Specter::SplitView::Axis::Horizontal;
        Value<float> split = 0.5;
    } m_state;
    const Space::State& spaceState() const {return m_state;}

public:
    SplitSpace(ViewManager& vm, Space* parent, Specter::SplitView::Axis axis) : Space(vm, Class::SplitSpace, parent)
    {
        m_state.axis = axis;
        reloadState();
    }
    SplitSpace(ViewManager& vm, Space* parent, ConfigReader& r)
    : SplitSpace(vm, parent, Specter::SplitView::Axis::Horizontal)
    {
        m_state.read(r);
#ifdef URDE_BINARY_CONFIGS
        m_slots[0].reset(NewSpaceFromConfigStream(vm, this, r));
        m_slots[1].reset(NewSpaceFromConfigStream(vm, this, r));
#else
        r.enterSubRecord("slot0");
        m_slots[0].reset(NewSpaceFromConfigStream(vm, this, r));
        r.leaveSubRecord();
        r.enterSubRecord("slot1");
        m_slots[1].reset(NewSpaceFromConfigStream(vm, this, r));
        r.leaveSubRecord();
#endif
        reloadState();
    }

    void reloadState()
    {
        m_state.split = std::min(1.f, std::max(0.f, m_state.split));
        if (m_state.axis != Specter::SplitView::Axis::Horizontal &&
            m_state.axis != Specter::SplitView::Axis::Vertical)
            m_state.axis = Specter::SplitView::Axis::Horizontal;
        if (m_splitView)
        {
            m_splitView->setSplit(m_state.split);
            m_splitView->setAxis(m_state.axis);
        }
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

    void setChildSlot(unsigned slot, std::unique_ptr<Space>&& space);

    Specter::View* buildSpaceView(Specter::ViewResources& res) {return buildContentView(res);}
    Specter::View* buildContentView(Specter::ViewResources& res);

    Space* copy(Space* parent) const {return nullptr;}
    bool spaceSplitAllowed() const {return false;}

    ISpaceController* spaceJoin(int keepSlot)
    {
        if (m_parent)
        {
            ISpaceController* ret = m_slots[keepSlot].get();
            m_parent->exchangeSpaceSplitJoin(this, std::move(m_slots[keepSlot]));
            return ret;
        }
        return nullptr;
    }

    std::unique_ptr<Space> exchangeSpaceSplitJoin(Space* removeSpace, std::unique_ptr<Space>&& keepSpace);

    Specter::SplitView* splitView() {return m_splitView.get();}
    void updateSplit(float split) {m_state.split = split;}
    void joinViews(Specter::SplitView* thisSplit, int thisSlot, Specter::SplitView* otherSplit, int otherSlot);

    void setAxis(Specter::SplitView::Axis axis)
    {
        m_state.axis = axis;
        reloadState();
    }

    Specter::SplitView::Axis axis() const {return m_state.axis;}
    float split() const {return m_state.split;}

    Specter::View* basisView() {return m_splitView.get();}
};

class ViewerSpace : public Space
{
public:
    ViewerSpace(ViewManager& vm, Class cls, Space* parent)
    : Space(vm, cls, parent) {}
};

class EditorSpace : public Space
{
public:
    EditorSpace(ViewManager& vm, Class cls, Space* parent)
    : Space(vm, cls, parent) {}
};

class TestSpace : public Space
{
    std::unique_ptr<Specter::Button> m_button;
    std::unique_ptr<Specter::MultiLineTextView> m_textView;

    std::string m_contentStr;
    std::string m_buttonStr;

    Specter::IButtonBinding* m_binding;

public:
    TestSpace(ViewManager& vm, Space* parent, const std::string& content, const std::string& button,
              Specter::IButtonBinding* binding)
    : Space(vm, Class::TestSpace, parent), m_contentStr(content), m_buttonStr(button), m_binding(binding)
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
        tb.push_back(m_button.get(), 0);
    }

    Specter::View* buildContentView(Specter::ViewResources& res)
    {
        m_textView.reset(new Specter::MultiLineTextView(res, *m_spaceView, res.m_heading14));
        m_textView->setBackground(res.themeData().viewportBackground());
        m_textView->typesetGlyphs(m_contentStr, res.themeData().uiText());
        return m_textView.get();
    }
};

}

#endif // URDE_SPACE_HPP
