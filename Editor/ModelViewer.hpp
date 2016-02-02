#ifndef URDE_MODEL_VIEWER_HPP
#define URDE_MODEL_VIEWER_HPP

#include "Space.hpp"
#include "ViewManager.hpp"
#include "CVector3f.hpp"
#include "CProjection.hpp"

namespace URDE
{
class ModelViewer : public ViewerSpace
{
    struct State : Space::State
    {
        DECL_YAML
        enum class Mode
        {
            Solid,
            Material,
            Wireframe
        };

        Value<Mode>    renderMode = Mode::Material;
        Value<Zeus::CVector3f>   cameraPosition;
        Value<Zeus::CQuaternion> cameraOrientation;

    } m_state;

    const Space::State& spaceState() const { return m_state; }

    struct View : Specter::View
    {
    };

    virtual Specter::View* buildContentView(Specter::ViewResources& res)
    {
        return nullptr;
    }

public:
    ModelViewer(ViewManager& vm, Space* parent)
        : ViewerSpace(vm, Class::ModelViewer, parent)
    {
        reloadState();
    }

    ModelViewer(ViewManager& vm, Space* parent, const ModelViewer& other)
        : ModelViewer(vm, parent)
    {
        m_state = other.m_state;
        reloadState();
    }

    ModelViewer(ViewManager& vm, Space* parent, ConfigReader& r)
        : ModelViewer(vm, parent)
    {
        m_state.read(r);
        reloadState();
    }

    void reloadState()
    {}

    Space* copy(Space *parent) const
    {
        return new ModelViewer(m_vm, parent, *this);
    }

    bool usesToolbar() const { return true; }
};

}

#endif // URDE_MODEL_VIEWER_HPP
