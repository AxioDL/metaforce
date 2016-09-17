#ifndef __URDE_IMAIN_HPP__
#define __URDE_IMAIN_HPP__
#include <boo/boo.hpp>
#include <boo/graphicsdev/GL.hpp>
#include <boo/audiodev/IAudioVoiceEngine.hpp>
#include <hecl/Runtime.hpp>
#include <amuse/amuse.hpp>
#include "RetroTypes.hpp"

namespace urde
{
class CStopWatch;
enum class EGameplayResult
{
    None,
    Win,
    Lose,
    Playing
};

class IMain
{
public:
    enum class EFlowState
    {
        Zero,
        One,
        Two,
        Three,
        Four,
        Five,
        Six,
    };

    virtual void RegisterResourceTweaks() {}
    virtual void ResetGameState()=0;
    virtual void StreamNewGameState(CInputStream&) {}
    virtual void CheckTweakManagerDebugOptions() {}
    virtual void Init(const hecl::Runtime::FileStoreManager& storeMgr,
              boo::IAudioVoiceEngine* voiceEngine,
              amuse::IBackendVoiceAllocator& backend)=0;
    virtual void Draw()=0;
    virtual bool Proc()=0;
    virtual void Shutdown()=0;
    virtual bool CheckReset()=0;
    virtual bool CheckTerminate()=0;
    virtual void DrawDebugMetrics(double, CStopWatch&) {}
    virtual void DoPreDrawMetrics(){}
    virtual void FillInAssetIDs()=0;
    virtual void LoadAudio()=0;
    virtual void ShutdownSubsystems()=0;
    virtual EGameplayResult GetGameplayResult() const=0;
    virtual void SetGameplayResult(EGameplayResult wl)=0;
    virtual EFlowState GetFlowState() const=0;
};
}

#endif // __URDE_IMAIN_HPP__
