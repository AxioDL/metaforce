#ifndef RUDE_VIEW_MANAGER_HPP
#define RUDE_VIEW_MANAGER_HPP

#include <Specter/Specter.hpp>
#include <HECL/CVarManager.hpp>

namespace RUDE
{

class ViewManager : Specter::IViewManager
{
    HECL::CVarManager& m_cvarManager;
    Specter::FontCache m_fontCache;
    Specter::ViewResources m_viewResources;
    std::unique_ptr<boo::IWindow> m_mainWindow;
    std::unique_ptr<Specter::RootView> m_rootView;

    Specter::Space* m_space1;
    Specter::Space* m_space2;

    HECL::CVar* m_cvPixelFactor;

    bool m_updatePf = false;
    float m_reqPf;

    void SetupRootView();
public:
    ViewManager(HECL::Runtime::FileStoreManager& fileMgr, HECL::CVarManager& cvarMgr)
    : m_cvarManager(cvarMgr), m_fontCache(fileMgr) {}

    Specter::RootView& rootView() const {return *m_rootView;}
    void RequestPixelFactor(float pf)
    {
        m_reqPf = pf;
        m_updatePf = true;
    }

    void init(boo::IApplication* app);
    bool proc();
    void stop();
};

}

#endif // RUDE_VIEW_MANAGER_HPP
