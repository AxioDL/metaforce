#ifndef RUDE_VIEW_MANAGER_HPP
#define RUDE_VIEW_MANAGER_HPP

#include <HECL/CVarManager.hpp>
#include "Space.hpp"

namespace RUDE
{
class SplashScreen;

class ViewManager : Specter::IViewManager
{
    HECL::CVarManager& m_cvarManager;
    Specter::FontCache m_fontCache;
    Specter::ViewResources m_viewResources;
    std::unique_ptr<boo::IWindow> m_mainWindow;
    std::unique_ptr<Specter::RootView> m_rootView;
    std::unique_ptr<SplashScreen> m_splash;

    HECL::CVar* m_cvPixelFactor;

    bool m_updatePf = false;
    float m_reqPf;

    Specter::View* BuildSpaceViews(RUDE::Space* space);
    void SetupRootView();
    void SetupSplashView();
    void SetupEditorView();

    bool m_showSplash = false;
public:
    struct SetTo1 : Specter::IButtonBinding
    {
        ViewManager& m_vm;
        std::string m_name = "SetTo1";
        std::string m_help = "Sets scale factor to 1.0";
        SetTo1(ViewManager& vm) : m_vm(vm) {}

        const std::string& name() const {return m_name;}
        const std::string& help() const {return m_help;}
        void pressed(const boo::SWindowCoord& coord)
        {
            m_vm.RequestPixelFactor(1.0);
        }
    };
    SetTo1 m_setTo1;

    struct SetTo2 : Specter::IButtonBinding
    {
        ViewManager& m_vm;
        std::string m_name = "SetTo2";
        std::string m_help = "Sets scale factor to 2.0";
        SetTo2(ViewManager& vm) : m_vm(vm) {}

        const std::string& name() const {return m_name;}
        const std::string& help() const {return m_help;}
        void pressed(const boo::SWindowCoord& coord)
        {
            m_vm.RequestPixelFactor(2.0);
        }
    };
    SetTo2 m_setTo2;

    SplitSpace m_split;
    TestSpace m_space1;
    TestSpace m_space2;

    ViewManager(HECL::Runtime::FileStoreManager& fileMgr, HECL::CVarManager& cvarMgr);
    ~ViewManager();

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
