#ifndef __PSHAG_MP1_HPP__
#define __PSHAG_MP1_HPP__

#include <boo/boo.hpp>
#include "CMemory.hpp"
#include "CTweaks.hpp"
#include "CPlayMovie.hpp"
#include "IOStreams.hpp"

namespace urde
{
class CStopwatch;

namespace MP1
{

enum class EGameplayResult
{
    None,
    Win,
    Lose,
    Playing
};

class WindowCallback : public boo::IWindowCallback
{
};

class CMain : public boo::IApplicationCallback
{
    boo::IWindow* mainWindow;
    int appMain(boo::IApplication* app);
    void appQuitting(boo::IApplication*)
    { xe8_b24_finished = true; }
    void appFilesOpen(boo::IApplication*, const std::vector<std::string>& paths)
    {
        fprintf(stderr, "OPENING: ");
        for (const std::string& path : paths)
            fprintf(stderr, "%s ", path.c_str());
        fprintf(stderr, "\n");
    }

    CMemorySys x6c_memSys;
    CTweaks x70_tweaks;
    EGameplayResult xe4_gameplayResult;
    bool xe8_b24_finished = false;

public:
    CMain();
    void RegisterResourceTweaks();
    void ResetGameState();
    void StreamNewGameState(CInputStream&);
    void CheckTweakManagerDebugOptions() {}
    void AddWorldPaks();
    int RsMain(int argc, const boo::SystemChar* argv[]);
    bool CheckReset();
    bool CheckTerminate() {return false;}
    void DrawDebugMetrics(double, CStopwatch&) {}
    void DoPredrawMetrics() {}
    void FillInAssetIDs();
    void LoadAudio();
    void ShutdownSubsystems();
    void InitializeSubsystems();
    EGameplayResult GetGameplayResult() const {return xe4_gameplayResult;}
    void SetGameplayResult(EGameplayResult wl) {xe4_gameplayResult = wl;}
};

extern CMain* g_main;
}
}

#endif // __PSHAG_MP1_HPP__
