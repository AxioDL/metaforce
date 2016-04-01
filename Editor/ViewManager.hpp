#ifndef URDE_VIEW_MANAGER_HPP
#define URDE_VIEW_MANAGER_HPP

#include "hecl/CVarManager.hpp"
#include "boo/audiodev/IAudioVoiceEngine.hpp"
#include "ProjectManager.hpp"
#include "Space.hpp"

#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Graphics/CLineRenderer.hpp"
#include "Runtime/Graphics/CMoviePlayer.hpp"
#include "Runtime/Graphics/CModel.hpp"

namespace urde
{
class SplashScreen;

class ViewManager : public specter::IViewManager
{
    friend class ProjectManager;
    friend class Space;
    friend class RootSpace;
    friend class SplitSpace;

    hecl::Runtime::FileStoreManager& m_fileStoreManager;
    std::experimental::optional<hecl::Runtime::ShaderCacheManager> m_shaderCacheManager;
    hecl::CVarManager& m_cvarManager;
    ProjectManager m_projManager;
    specter::FontCache m_fontCache;
    specter::DefaultThemeData m_themeData;
    specter::ViewResources m_viewResources;
    boo::GraphicsDataToken m_iconsToken;
    specter::Translator m_translator;
    std::unique_ptr<boo::IWindow> m_mainWindow;

    std::unique_ptr<specter::RootView> m_rootView;
    std::unique_ptr<SplashScreen> m_splash;
    std::unique_ptr<RootSpace> m_rootSpace;
    specter::View* m_rootSpaceView = nullptr;

    class ParticleView : public specter::View
    {
        ViewManager& m_vm;
    public:
        ParticleView(ViewManager& vm, specter::ViewResources& res, specter::View& parent)
        : View(res, parent), m_vm(vm) {}
        void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
        void draw(boo::IGraphicsCommandQueue* gfxQ);
    };
    std::unique_ptr<ParticleView> m_particleView;
    urde::TCachedToken<CModel> m_modelTest;
    urde::TCachedToken<CGenDescription> m_partGenDesc;
    std::unique_ptr<CElementGen> m_partGen;
    std::unique_ptr<CLineRenderer> m_lineRenderer;
    std::unique_ptr<CMoviePlayer> m_moviePlayer;
    std::unique_ptr<u8[]> m_rsfBuf;
    std::unique_ptr<boo::IAudioVoiceEngine> m_voiceEngine;
    std::unique_ptr<boo::IAudioVoice> m_videoVoice;
    struct AudioVoiceCallback : boo::IAudioVoiceCallback
    {
        ViewManager& m_vm;
        size_t supplyAudio(boo::IAudioVoice& voice, size_t frames, int16_t* data)
        {
            if (m_vm.m_moviePlayer)
                m_vm.m_moviePlayer->MixAudio(data, nullptr, frames);
            CMoviePlayer::MixStaticAudio(data, data, frames);
            return frames;
        }
        AudioVoiceCallback(ViewManager& vm) : m_vm(vm) {}
    } m_voiceCallback;

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

    void BuildTestPART(urde::IObjectStore& objStore);

    Space* m_deferSplit = nullptr;
    specter::SplitView::Axis m_deferSplitAxis;
    int m_deferSplitThisSlot;
    boo::SWindowCoord m_deferSplitCoord;

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
    bool proc();
    void stop();
};

}

#endif // URDE_VIEW_MANAGER_HPP
