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
    std::unique_ptr<Specter::SplitView> m_splitView;

    std::unique_ptr<Specter::Space> m_space1;
    std::unique_ptr<Specter::Button> m_butt1;
    std::unique_ptr<Specter::MultiLineTextView> m_textView1;
    std::unique_ptr<Specter::Space> m_space2;
    std::unique_ptr<Specter::Button> m_butt2;
    std::unique_ptr<Specter::MultiLineTextView> m_textView2;

    HECL::CVar* m_cvPixelFactor;

    bool m_updatePf = false;
    float m_reqPf;

    void SetupRootView();
public:
    ViewManager(HECL::Runtime::FileStoreManager& fileMgr, HECL::CVarManager& cvarMgr)
    : m_cvarManager(cvarMgr), m_fontCache(fileMgr), m_setTo1(*this), m_setTo2(*this) {}

    Specter::RootView& rootView() const {return *m_rootView;}
    void RequestPixelFactor(float pf)
    {
        m_reqPf = pf;
        m_updatePf = true;
    }

    void init(boo::IApplication* app);
    bool proc();
    void stop();

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
};

}

#endif // RUDE_VIEW_MANAGER_HPP
