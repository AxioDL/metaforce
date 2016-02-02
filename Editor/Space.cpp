#include "Space.hpp"
#include "ViewManager.hpp"
#include "ResourceBrowser.hpp"
#include "ParticleEditor.hpp"
#include "ModelViewer.hpp"
#include "InformationCenter.hpp"
#include "icons/icons.hpp"

namespace URDE
{
static LogVisor::LogModule Log("URDE::Space");

Space::Space(ViewManager& vm, Class cls, Space* parent)
: m_spaceMenuNode(*this), m_spaceSelectBind(*this),
  m_vm(vm), m_class(cls), m_parent(parent) {}

Specter::View* Space::buildSpaceView(Specter::ViewResources& res)
{
    if (usesToolbar())
    {
        m_spaceView.reset(new Specter::Space(res, *m_parent->basisView(), *this, Specter::Toolbar::Position::Bottom, toolbarUnits()));
        Specter::View* sview = buildContentView(res);
        m_spaceView->setContentView(sview);
        Specter::Toolbar& tb = *m_spaceView->toolbar();
        const std::string* classStr = SpaceMenuNode::LookupClassString(m_class);
        Specter::Icon* classIcon = SpaceMenuNode::LookupClassIcon(m_class);
        const Zeus::CColor* classColor = SpaceMenuNode::LookupClassColor(m_class);
        m_spaceSelectButton.reset(new Specter::Button(res, tb, &m_spaceSelectBind, "", classIcon,
                                                      Specter::Button::Style::Block,
                                                      classColor?*classColor:Zeus::CColor::skWhite));
        tb.push_back(m_spaceSelectButton.get(), 0);
        buildToolbarView(res, tb);
        return m_spaceView.get();
    }
    else
    {
        m_spaceView.reset(new Specter::Space(res, *m_parent->basisView(), *this, Specter::Toolbar::Position::None, 0));
        Specter::View* sview = buildContentView(res);
        m_spaceView->setContentView(sview);
        return m_spaceView.get();
    }
}

std::vector<Space::SpaceMenuNode::SubNodeData> Space::SpaceMenuNode::s_subNodeDats =
{
    {Class::ResourceBrowser, "resource_browser", "Resource Browser", GetIcon(SpaceIcon::ResourceBrowser), {0.0,1.0,0.0,1.0}},
    {Class::EffectEditor, "effect_editor", "Effect Editor", GetIcon(SpaceIcon::ParticleEditor), {1.0,0.5,0.0,1.0}},
    {Class::ModelViewer, "model_viewer", "Model Viewer", GetIcon(SpaceIcon::ModelViewer), {0.95, 0.95, 0.95, 1.0}},
    {Class::InformationCenter, "information_center", "Information Center", GetIcon(SpaceIcon::InformationCenter), {0.0, 1.0, 1.0, 1.0}}
};
std::string Space::SpaceMenuNode::s_text = "Space Types";

void Space::SpaceMenuNode::InitializeStrings(ViewManager& vm)
{
    s_text = vm.translateOr("space_types", s_text.c_str());
    for (SubNodeData& sn : s_subNodeDats)
        sn.m_text = vm.translateOr(sn.m_key, sn.m_text.c_str());
}

std::unique_ptr<Specter::View> Space::SpaceSelectBind::buildMenu(const Specter::Button* button)
{
    return std::unique_ptr<Specter::View>(new Specter::Menu(m_space.m_vm.rootView().viewRes(),
                                                            *m_space.m_spaceView, &m_space.m_spaceMenuNode));
}

Specter::View* RootSpace::buildSpaceView(Specter::ViewResources& res)
{
    Specter::View* newRoot = buildContentView(res);
    m_vm.RootSpaceViewBuilt(newRoot);
    return newRoot;
}

Specter::View* RootSpace::basisView() {return &m_vm.rootView();}

Specter::View* SplitSpace::buildContentView(Specter::ViewResources& res)
{
    int clearance = res.pixelFactor() * SPECTER_TOOLBAR_GAUGE;
    m_splitView.reset(new Specter::SplitView(res, *m_parent->basisView(), this, m_state.axis,
                                             m_state.split, clearance, clearance));
    if (m_slots[0])
        m_splitView->setContentView(0, m_slots[0]->buildSpaceView(res));
    if (m_slots[1])
        m_splitView->setContentView(1, m_slots[1]->buildSpaceView(res));
    return m_splitView.get();
}

void SplitSpace::setChildSlot(unsigned slot, std::unique_ptr<Space>&& space)
{
    if (slot > 1)
        Log.report(LogVisor::FatalError, "invalid slot %u for SplitView", slot);
    m_slots[slot] = std::move(space);
    m_slots[slot]->m_parent = this;
}

void SplitSpace::joinViews(Specter::SplitView* thisSplit, int thisSlot, Specter::SplitView* otherSplit, int otherSlot)
{
    if (thisSplit == otherSplit)
    {
        SplitSpace* thisSS = dynamic_cast<SplitSpace*>(m_slots[thisSlot].get());
        if (thisSS)
        {
            int ax = thisSS->m_state.axis == Specter::SplitView::Axis::Horizontal ? 1 : 0;
            const boo::SWindowRect& thisRect = m_splitView->subRect();
            const boo::SWindowRect& subRect = thisSS->m_splitView->subRect();
            int splitPx = subRect.location[ax] + subRect.size[ax] * thisSS->m_state.split -
                          thisRect.location[ax];
            thisSS->m_state.split = splitPx / float(thisRect.size[ax]);
        }
        m_parent->exchangeSpaceSplitJoin(this, std::move(m_slots[thisSlot]));
        m_vm.BuildSpaceViews();
    }
    else
    {
        for (int i=0 ; i<2 ; ++i)
        {
            SplitSpace* otherSS = dynamic_cast<SplitSpace*>(m_slots[i].get());
            if (otherSS && otherSS->m_splitView.get() == otherSplit)
            {
                int ax = m_state.axis == Specter::SplitView::Axis::Horizontal ? 1 : 0;
                const boo::SWindowRect& thisRect = m_splitView->subRect();
                const boo::SWindowRect& subRect = otherSS->m_splitView->subRect();
                int splitPx = subRect.location[ax] + subRect.size[ax] * otherSS->m_state.split -
                              thisRect.location[ax];
                m_state.split = splitPx / float(thisRect.size[ax]);
                exchangeSpaceSplitJoin(otherSS, std::move(otherSS->m_slots[otherSlot ^ 1]));
                m_vm.BuildSpaceViews();
                break;
            }
        }
    }
}

Specter::ISplitSpaceController* Space::spaceSplit(Specter::SplitView::Axis axis, int thisSlot)
{
    if (m_parent)
    {
        /* Reject split operations with insufficient clearance */
        int clearance = m_vm.m_viewResources.pixelFactor() * SPECTER_TOOLBAR_GAUGE;
        if (axis == Specter::SplitView::Axis::Horizontal)
        {
            if (m_spaceView->subRect().size[1] <= clearance)
                return nullptr;
        }
        else
        {
            if (m_spaceView->subRect().size[0] <= clearance)
                return nullptr;
        }

        SplitSpace* ss = new SplitSpace(m_vm, m_parent, axis);
        ss->setChildSlot(thisSlot, std::move(m_parent->exchangeSpaceSplitJoin(this, std::unique_ptr<Space>(ss))));
        ss->setChildSlot(thisSlot ^ 1, std::unique_ptr<Space>(copy(ss)));
        m_vm.BuildSpaceViews();
        return ss;
    }
    return nullptr;
}

std::unique_ptr<Space> RootSpace::exchangeSpaceSplitJoin(Space* removeSpace, std::unique_ptr<Space>&& keepSpace)
{
    std::unique_ptr<Space> ret = std::move(keepSpace);

    if (removeSpace == m_spaceTree.get())
    {
        m_spaceTree.swap(ret);
        m_spaceTree->m_parent = this;
    }
    else
        Log.report(LogVisor::FatalError, "RootSpace::exchangeSpaceSplitJoin() failure");

    return ret;
}

std::unique_ptr<Space> SplitSpace::exchangeSpaceSplitJoin(Space* removeSpace, std::unique_ptr<Space>&& keepSpace)
{
    std::unique_ptr<Space> ret = std::move(keepSpace);

    if (removeSpace == m_slots[0].get())
    {
        m_slots[0].swap(ret);
        m_slots[0]->m_parent = this;
    }
    else if (removeSpace == m_slots[1].get())
    {
        m_slots[1].swap(ret);
        m_slots[1]->m_parent = this;
    }
    else
        Log.report(LogVisor::FatalError, "SplitSpace::exchangeSpaceSplitJoin() failure");

    return ret;
}

template <class Reader>
static Space* BuildNewSpace(ViewManager& vm, Space::Class cls, Space* parent, Reader& r)
{
    using Class = Space::Class;
    switch (cls)
    {
    case Class::SplitSpace:
        return new SplitSpace(vm, parent, r);
    case Class::ResourceBrowser:
        return new ResourceBrowser(vm, parent, r);
    case Class::EffectEditor:
        return new EffectEditor(vm, parent, r);
    case Class::ModelViewer:
        return new ModelViewer(vm, parent, r);
    case Class::InformationCenter:
        return new InformationCenter(vm, parent, r);
    default: break;
    }
    return nullptr;
}

void Space::saveState(Athena::io::IStreamWriter& w) const
{
    w.writeUint32Big(atUint32(m_class));
    spaceState().write(w);
}

void Space::saveState(Athena::io::YAMLDocWriter& w) const
{
    w.writeUint32("class", atUint32(m_class));
    spaceState().write(w);
}

Space* Space::NewSpaceFromConfigStream(ViewManager& vm, Space* parent, ConfigReader& r)
{
#ifdef URDE_BINARY_CONFIGS
    Class cls = Class(r.readUint32Big());
#else
    Class cls = Class(r.readUint32("class"));
#endif
    return BuildNewSpace(vm, cls, parent, r);
}

RootSpace* Space::NewRootSpaceFromConfigStream(ViewManager& vm, ConfigReader& r)
{
#ifdef URDE_BINARY_CONFIGS
    Class cls = Class(r.readUint32Big());
#else
    Class cls = Class(r.readUint32("class"));
#endif
    if (cls != Class::RootSpace)
        return nullptr;
    return new RootSpace(vm, r);
}

}
