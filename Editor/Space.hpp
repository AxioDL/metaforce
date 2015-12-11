#ifndef RUDE_SPACE_HPP
#define RUDE_SPACE_HPP

#include <Athena/DNAYaml.hpp>
#include <Specter/Space.hpp>

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

struct SpaceState : Athena::io::DNAYaml<Athena::BigEndian> {};

class Space
{
    std::unique_ptr<Specter::Space> m_space;
public:
    enum class Class
    {
        None,
        Split,
        ResourceOutliner,
    };
protected:
    ViewManager& m_vm;
    Class m_class = Class::None;
    Space(ViewManager& vm, Class cls) : m_vm(vm), m_class(cls) {}
    Space(ViewManager& vm, Class cls, Athena::io::IStreamReader& r) : m_vm(vm), m_class(cls) {}
    void writeState(Athena::io::IStreamWriter& w) const;

    /* Allows common Space code to access DNA-encoded state */
    virtual SpaceState& spaceState()=0;

    /* Structural control */
    virtual bool usesToolbar() const {return false;}
    virtual void buildToolbar(Specter::ViewResources& res, Specter::Toolbar& tb) {}
    virtual Specter::View* buildContent(Specter::ViewResources& res)=0;
public:
};

class Split : public Space
{
    std::unique_ptr<Specter::SplitView> m_split;
    struct State : SpaceState
    {
        DECL_YAML
        Value<float> m_split;
    } m_state;
    SpaceState& spaceState() {return m_state;}
};

}

#endif // RUDE_SPACE_HPP
