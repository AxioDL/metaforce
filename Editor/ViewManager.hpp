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

    HECL::CVar* m_cvDPI;
    HECL::CVar* m_test1;
    HECL::CVar* m_test2;

    void SetupRootView();
public:
    ViewManager(HECL::Runtime::FileStoreManager& fileMgr, HECL::CVarManager& cvarMgr)
    : m_cvarManager(cvarMgr), m_fontCache(fileMgr) {}

    void init(boo::IApplication* app);
    bool proc();
    void stop();
};

}

#endif // RUDE_VIEW_MANAGER_HPP
