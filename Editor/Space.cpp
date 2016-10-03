#include "Space.hpp"
#include "ViewManager.hpp"
#include "ResourceBrowser.hpp"
#include "ParticleEditor.hpp"
#include "ModelViewer.hpp"
#include "InformationCenter.hpp"
#include "icons/icons.hpp"

namespace urde
{
static logvisor::Module Log("URDE::Space");

Space::Space(ViewManager& vm, Class cls, Space* parent)
: m_spaceMenuNode(*this), m_spaceSelectBind(*this),
  m_vm(vm), m_class(cls), m_parent(parent) {}

specter::View* Space::buildSpaceView(specter::ViewResources& res)
{
    if (usesToolbar())
    {
        m_spaceView.reset(new specter::Space(res, *m_parent->basisView(), *this, specter::Toolbar::Position::Bottom, toolbarUnits()));
        specter::View* sview = buildContentView(res);
        m_spaceView->setContentView(sview);
        specter::Toolbar& tb = *m_spaceView->toolbar();
        const std::string* classStr = SpaceMenuNode::LookupClassString(m_class);
        specter::Icon* classIcon = SpaceMenuNode::LookupClassIcon(m_class);
        const zeus::CColor* classColor = SpaceMenuNode::LookupClassColor(m_class);
        m_spaceSelectButton.reset(new specter::Button(res, tb, &m_spaceSelectBind, "", classIcon,
                                                      specter::Button::Style::Block,
                                                      classColor?*classColor:zeus::CColor::skWhite));
        tb.push_back(m_spaceSelectButton.get(), 0);
        buildToolbarView(res, tb);
        return m_spaceView.get();
    }
    else
    {
        m_spaceView.reset(new specter::Space(res, *m_parent->basisView(), *this, specter::Toolbar::Position::None, 0));
        specter::View* sview = buildContentView(res);
        m_spaceView->setContentView(sview);
        return m_spaceView.get();
    }
}

std::vector<Space::SpaceMenuNode::SubNodeData> Space::SpaceMenuNode::s_subNodeDats =
{
    {Class::ResourceBrowser, "resource_browser", "Resource Browser", GetIcon(SpaceIcon::ResourceBrowser), {0.0f, 1.0f, 0.0f, 1.0f}},
    {Class::EffectEditor, "effect_editor", "Effect Editor", GetIcon(SpaceIcon::ParticleEditor), {1.0f, 0.5f, 0.0f, 1.0f}},
    {Class::ModelViewer, "model_viewer", "Model Viewer", GetIcon(SpaceIcon::ModelViewer), {0.95f, 0.95f, 0.95f, 1.0f}},
    {Class::InformationCenter, "information_center", "Information Center", GetIcon(SpaceIcon::InformationCenter), {0.0f, 1.0f, 1.0f, 1.0f}}
};
std::string Space::SpaceMenuNode::s_text = "Space Types";

void Space::SpaceMenuNode::InitializeStrings(ViewManager& vm)
{
    s_text = vm.translateOr("space_types", s_text.c_str());
    for (SubNodeData& sn : s_subNodeDats)
        sn.m_text = vm.translateOr(sn.m_key, sn.m_text.c_str());
}

std::unique_ptr<specter::View> Space::SpaceSelectBind::buildMenu(const specter::Button* button)
{
    return std::unique_ptr<specter::View>(new specter::Menu(m_space.m_vm.rootView().viewRes(),
                                                            *m_space.m_spaceView, &m_space.m_spaceMenuNode));
}

specter::View* RootSpace::buildSpaceView(specter::ViewResources& res)
{
    specter::View* newRoot = buildContentView(res);
    m_vm.RootSpaceViewBuilt(newRoot);
    return newRoot;
}

specter::View* RootSpace::basisView() {return &m_vm.rootView();}

specter::View* SplitSpace::buildContentView(specter::ViewResources& res)
{
    int clearance = res.pixelFactor() * SPECTER_TOOLBAR_GAUGE;
    m_splitView.reset(new specter::SplitView(res, *m_parent->basisView(), this, m_state.axis,
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
        Log.report(logvisor::Fatal, "invalid slot %u for SplitView", slot);
    m_slots[slot] = std::move(space);
    m_slots[slot]->m_parent = this;
}

void SplitSpace::joinViews(specter::SplitView* thisSplit, int thisSlot, specter::SplitView* otherSplit, int otherSlot)
{
    if (thisSplit == otherSplit)
    {
        SplitSpace* thisSS = dynamic_cast<SplitSpace*>(m_slots[thisSlot].get());
        if (thisSS)
        {
            int ax = thisSS->m_state.axis == specter::SplitView::Axis::Horizontal ? 1 : 0;
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
                int ax = m_state.axis == specter::SplitView::Axis::Horizontal ? 1 : 0;
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

specter::ISplitSpaceController* Space::spaceSplit(specter::SplitView::Axis axis, int thisSlot)
{
    if (m_parent)
    {
        /* Reject split operations with insufficient clearance */
        int clearance = m_vm.m_viewResources.pixelFactor() * SPECTER_TOOLBAR_GAUGE;
        if (axis == specter::SplitView::Axis::Horizontal)
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
        ss->setChildSlot(thisSlot, m_parent->exchangeSpaceSplitJoin(this, std::unique_ptr<Space>(ss)));
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
        Log.report(logvisor::Fatal, "RootSpace::exchangeSpaceSplitJoin() failure");

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
        Log.report(logvisor::Fatal, "SplitSpace::exchangeSpaceSplitJoin() failure");

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

void Space::saveState(athena::io::IStreamWriter& w) const
{
    w.writeUint32Big(atUint32(m_class));
    spaceState().write(w);
}

void Space::saveState(athena::io::YAMLDocWriter& w) const
{
    w.writeUint32("class", atUint32(m_class));
    spaceState().write(w);
}

Space* Space::NewSpaceFromConfigStream(ViewManager& vm, Space* parent, ConfigReader& r)
{
    Class cls = Class(r.readUint32("class"));
    return BuildNewSpace(vm, cls, parent, r);
}

RootSpace* Space::NewRootSpaceFromConfigStream(ViewManager& vm, ConfigReader& r)
{
    Class cls = Class(r.readUint32("class"));
    if (cls != Class::RootSpace)
        return nullptr;
    return new RootSpace(vm, r);
}

void Space::SpaceMenuNode::SubNode::activated(const boo::SWindowCoord &coord)
{
    std::unique_ptr<Space> newSpace;
    switch(m_data.m_cls)
    {
    case Class::InformationCenter:
        if (typeid(InformationCenter) != typeid(m_space))
            newSpace.reset(new InformationCenter(m_space.m_parent->m_vm, m_space.m_parent));
        break;
    case Class::EffectEditor:
        if (typeid(EffectEditor) != typeid(m_space))
            newSpace.reset(new EffectEditor(m_space.m_parent->m_vm, m_space.m_parent));
        break;
    case Class::ResourceBrowser:
        if (typeid(ResourceBrowser) != typeid(m_space))
            newSpace.reset(new ResourceBrowser(m_space.m_parent->m_vm, m_space.m_parent));
        break;
    case Class::ModelViewer:
        if (typeid(ModelViewer) != typeid(m_space))
            newSpace.reset(new ModelViewer(m_space.m_parent->m_vm, m_space.m_parent));
        break;
    default: break;
    }
    if (newSpace)
    {
        Space* parent = m_space.m_parent;
        m_space.m_parent->exchangeSpaceSplitJoin(&m_space, std::move(newSpace));
        parent->m_vm.BuildSpaceViews();
    }
}

}
