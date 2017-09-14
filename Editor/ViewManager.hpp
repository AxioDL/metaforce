#ifndef URDE_VIEW_MANAGER_HPP
#define URDE_VIEW_MANAGER_HPP

#include "hecl/CVarManager.hpp"
#include "boo/audiodev/IAudioVoiceEngine.hpp"
#include "amuse/BooBackend.hpp"
#include "ProjectManager.hpp"
#include "Space.hpp"

#include "Runtime/CGameHintInfo.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Graphics/CLineRenderer.hpp"
#include "Runtime/Graphics/CMoviePlayer.hpp"
#include "Runtime/Graphics/CModel.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/Character/CAssetFactory.hpp"
#include "Runtime/Graphics/Shaders/CColoredQuadFilter.hpp"
#include "Runtime/Graphics/Shaders/CXRayBlurFilter.hpp"
#include "Runtime/Graphics/Shaders/CCameraBlurFilter.hpp"
#include "Runtime/Audio/CStaticAudioPlayer.hpp"

namespace urde
{
class SplashScreen;

class ViewManager : public specter::IViewManager
{
    friend class ProjectManager;
    friend class Space;
    friend class RootSpace;
    friend class SplitSpace;

    std::shared_ptr<boo::IWindow> m_mainWindow;
    hecl::Runtime::FileStoreManager& m_fileStoreManager;
    hecl::CVarManager& m_cvarManager;
    ProjectManager m_projManager;
    specter::FontCache m_fontCache;
    specter::DefaultThemeData m_themeData;
    specter::ViewResources m_viewResources;
    boo::GraphicsDataToken m_iconsToken;
    boo::GraphicsDataToken m_badgeToken;
    specter::Translator m_translator;
    boo::IGraphicsDataFactory* m_mainBooFactory = nullptr;
    boo::IGraphicsCommandQueue* m_mainCommandQueue = nullptr;
    boo::ITextureR* m_renderTex = nullptr;
    const boo::SystemChar* m_mainPlatformName;

    std::unique_ptr<specter::RootView> m_rootView;
    std::unique_ptr<SplashScreen> m_splash;
    std::unique_ptr<RootSpace> m_rootSpace;
    specter::View* m_rootSpaceView = nullptr;

    class TestGameView : public specter::View
    {
        ViewManager& m_vm;
    public:
        TestGameView(ViewManager& vm, specter::ViewResources& res, specter::View& parent)
        : View(res, parent), m_vm(vm) {}
        void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
        void draw(boo::IGraphicsCommandQueue* gfxQ);

        void mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mkey)
        {
            if (MP1::CMain* m = m_vm.m_projManager.gameMain())
                if (MP1::CGameArchitectureSupport* as = m->GetArchSupport())
                    as->mouseDown(coord, button, mkey);
        }

        void mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mkey)
        {
            if (MP1::CMain* m = m_vm.m_projManager.gameMain())
                if (MP1::CGameArchitectureSupport* as = m->GetArchSupport())
                    as->mouseUp(coord, button, mkey);
        }

        void mouseMove(const boo::SWindowCoord& coord)
        {
            if (MP1::CMain* m = m_vm.m_projManager.gameMain())
                if (MP1::CGameArchitectureSupport* as = m->GetArchSupport())
                    as->mouseMove(coord);
        }

        void scroll(const boo::SWindowCoord& coord, const boo::SScrollDelta& sd)
        {
            if (MP1::CMain* m = m_vm.m_projManager.gameMain())
                if (MP1::CGameArchitectureSupport* as = m->GetArchSupport())
                    as->scroll(coord, sd);
        }

        void charKeyDown(unsigned long cc, boo::EModifierKey mkey, bool repeat)
        {
            if (MP1::CMain* m = m_vm.m_projManager.gameMain())
                if (MP1::CGameArchitectureSupport* as = m->GetArchSupport())
                    as->charKeyDown(cc, mkey, repeat);
        }

        void charKeyUp(unsigned long cc, boo::EModifierKey mkey)
        {
            if (MP1::CMain* m = m_vm.m_projManager.gameMain())
                if (MP1::CGameArchitectureSupport* as = m->GetArchSupport())
                    as->charKeyUp(cc, mkey);
        }

        void specialKeyDown(boo::ESpecialKey skey, boo::EModifierKey mkey, bool repeat)
        {
            if (MP1::CMain* m = m_vm.m_projManager.gameMain())
                if (MP1::CGameArchitectureSupport* as = m->GetArchSupport())
                    as->specialKeyDown(skey, mkey, repeat);
        }

        void specialKeyUp(boo::ESpecialKey skey, boo::EModifierKey mkey)
        {
            if (MP1::CMain* m = m_vm.m_projManager.gameMain())
                if (MP1::CGameArchitectureSupport* as = m->GetArchSupport())
                    as->specialKeyUp(skey, mkey);
        }
    };
    std::unique_ptr<TestGameView> m_testGameView;
    std::unique_ptr<boo::IAudioVoiceEngine> m_voiceEngine;
    std::unique_ptr<boo::IAudioVoice> m_videoVoice;
    std::experimental::optional<amuse::BooBackendVoiceAllocator> m_amuseAllocWrapper;

    hecl::SystemString m_recentProjectsPath;
    std::vector<hecl::SystemString> m_recentProjects;
    hecl::SystemString m_recentFilesPath;
    std::vector<hecl::SystemString> m_recentFiles;

    bool m_updatePf = false;
    float m_reqPf;

    specter::View* BuildSpaceViews();
    specter::RootView* SetupRootView();
    SplashScreen* SetupSplashView();
    void RootSpaceViewBuilt(specter::View* view);
    void ProjectChanged(hecl::Database::Project& proj);
    void SetupEditorView();
    void SetupEditorView(ConfigReader& r);
    void SaveEditorView(ConfigWriter& w);

    bool m_showSplash = false;
    void DismissSplash();

    unsigned m_editorFrames = 120;
    void FadeInEditors() {m_editorFrames = 0;}

    void BuildTestPART();
    void InitMP1(MP1::CMain& main);

    Space* m_deferSplit = nullptr;
    specter::SplitView::Axis m_deferSplitAxis;
    int m_deferSplitThisSlot;
    boo::SWindowCoord m_deferSplitCoord;
    hecl::SystemString m_deferedProject;

public:
    ViewManager(hecl::Runtime::FileStoreManager& fileMgr, hecl::CVarManager& cvarMgr);
    ~ViewManager();

    specter::RootView& rootView() const {return *m_rootView;}
    void requestPixelFactor(float pf)
    {
        m_reqPf = pf;
        m_updatePf = true;
    }

    ProjectManager& projectManager() {return m_projManager;}
    hecl::Database::Project* project() {return m_projManager.project();}
    const specter::Translator* getTranslator() const {return &m_translator;}

    void deferSpaceSplit(specter::ISpaceController* split, specter::SplitView::Axis axis, int thisSlot,
                         const boo::SWindowCoord& coord)
    {
        m_deferSplit = static_cast<Space*>(split);
        m_deferSplitAxis = axis;
        m_deferSplitThisSlot = thisSlot;
        m_deferSplitCoord = coord;
    }

    const std::vector<hecl::SystemString>* recentProjects() const {return &m_recentProjects;}
    void pushRecentProject(const hecl::SystemString& path);

    const std::vector<hecl::SystemString>* recentFiles() const {return &m_recentFiles;}
    void pushRecentFile(const hecl::SystemString& path);

    void init(boo::IApplication* app);
    const boo::SystemChar* platformName() { return m_mainPlatformName; }
    bool proc();
    void stop();

    void deferOpenProject(const hecl::SystemString& path) { m_deferedProject = path; }
};

}

#endif // URDE_VIEW_MANAGER_HPP
