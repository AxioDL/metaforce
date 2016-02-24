#ifndef URDE_MODEL_VIEWER_HPP
#define URDE_MODEL_VIEWER_HPP

#include "Space.hpp"
#include "ViewManager.hpp"
#include "Camera.hpp"

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
    std::unique_ptr<pshag::CLineRenderer> m_lineRenderer;
    struct View : Specter::View
    {
        ModelViewer& m_mv;
        boo::SWindowRect m_scissorRect;

        View(ModelViewer& mv, Specter::ViewResources& res)
            : Specter::View(res, mv.m_vm.rootView()), m_mv(mv)
        {
            commitResources(res);
        }

        void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
        void draw(boo::IGraphicsCommandQueue *gfxQ);
    };

    Camera m_camera;
    std::unique_ptr<View> m_view;

public:
    ModelViewer(ViewManager& vm, Space* parent)
        : ViewerSpace(vm, Class::ModelViewer, parent)
    {
        reloadState();
        m_lineRenderer.reset(new pshag::CLineRenderer(pshag::CLineRenderer::EPrimitiveMode::LineStrip, 4, nullptr, true));
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

    virtual Specter::View* buildContentView(Specter::ViewResources& res)
    {
        m_view.reset(new View(*this, res));
        return m_view.get();
    }

    bool usesToolbar() const { return true; }
};

}

#endif // URDE_MODEL_VIEWER_HPP
