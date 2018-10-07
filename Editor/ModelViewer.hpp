#pragma once

#include "Space.hpp"
#include "ViewManager.hpp"
#include "Camera.hpp"

namespace urde
{
class ModelViewer : public ViewerSpace
{
    struct State : Space::State
    {
        AT_DECL_DNA_YAML
        AT_DECL_DNAV
        enum class Mode
        {
            Solid,
            Material,
            Wireframe
        };

        Value<Mode>    renderMode = Mode::Material;
        Value<zeus::CVector3f>   cameraPosition;
        Value<zeus::CQuaternion> cameraOrientation;

    } m_state;

    const Space::State& spaceState() const { return m_state; }
    std::unique_ptr<urde::CLineRenderer> m_lineRenderer;
    struct View : specter::View
    {
        ModelViewer& m_mv;
        boo::SWindowRect m_scissorRect;

        View(ModelViewer& mv, specter::ViewResources& res)
        : specter::View(res, mv.m_vm.rootView()), m_mv(mv) {}

        void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
    };

    Camera m_camera;
    std::unique_ptr<View> m_view;

public:
    ModelViewer(ViewManager& vm, Space* parent)
        : ViewerSpace(vm, Class::ModelViewer, parent)
    {
        reloadState();
        m_lineRenderer.reset(new urde::CLineRenderer(urde::CLineRenderer::EPrimitiveMode::LineStrip, 4, nullptr, true));
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
    {
        m_camera.setPosition(m_state.cameraPosition);
        m_camera.setOrientation(m_state.cameraOrientation);
    }

    Space* copy(Space *parent) const
    {
        return new ModelViewer(m_vm, parent, *this);
    }

    virtual specter::View* buildContentView(specter::ViewResources& res)
    {
        m_view.reset(new View(*this, res));
        return m_view.get();
    }

    bool usesToolbar() const { return true; }
};

}

