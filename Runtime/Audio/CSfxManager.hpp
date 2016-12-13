#ifndef __URDE_CSFXMANAGER_HPP__
#define __URDE_CSFXMANAGER_HPP__

#include <vector>
#include "../RetroTypes.hpp"
#include "CSfxHandle.hpp"
#include "zeus/CVector3f.hpp"
#include "CAudioSys.hpp"

namespace urde
{

class CSfxManager
{
    static std::vector<s16>* mpSfxTranslationTable;
public:
    enum class ESfxChannels
    {
        Zero,
        One
    };

    enum class ESfxAudibility
    {
        Aud0,
        Aud1,
        Aud2,
        Aud3
    };

    class CSfxChannel
    {
    };

    class CBaseSfxWrapper
    {
        float x4_ = 15.f;
        s16 x8_rank = 0;
        s16 xa_prio;
        CSfxHandle xc_handle;
        TAreaId x10_area;
        union
        {
            struct
            {
                bool x14_24_useAcoustics:1;
                bool x14_25_available:1;
                bool x14_26_inArea:1;
                bool x14_27_looped:1;
                bool x14_28_playing:1;
                bool x14_29_active:1;
            };
            u16 _dummy = 0;
        };
    public:
        virtual ~CBaseSfxWrapper() {}
        virtual void SetActive(bool v) {x14_29_active = v;}
        virtual void SetPlaying(bool v) {x14_28_playing = v;}
        virtual void SetRank(short v) {x8_rank = v;}
        virtual void SetInArea(bool v) {x14_26_inArea = v;}
        virtual bool IsLooped() const {return x14_27_looped;}
        virtual bool IsPlaying() const {return x14_28_playing;}
        virtual bool IsActive() const {return x14_29_active;}
        virtual bool IsInArea() const {return x14_26_inArea;}
        virtual bool UseAcoustics() const {return x14_24_useAcoustics;}
        virtual s16 GetRank() const {return x8_rank;}
        virtual s16 GetPriority() const {return xa_prio;}
        virtual TAreaId GetArea() const {return x10_area;}
        virtual CSfxHandle GetSfxHandle() const {return xc_handle;}
        virtual void Play()=0;
        virtual void Stop()=0;
        virtual bool Ready()=0;
        virtual ESfxAudibility GetAudible(const zeus::CVector3f&)=0;
        virtual const std::shared_ptr<amuse::Voice>& GetVoice() const=0;

        void Release() {x14_25_available = true;}
        bool Available() const {return x14_25_available;}

        CBaseSfxWrapper(bool looped, s16 prio, const CSfxHandle& handle, bool useAcoustics, TAreaId area)
        : x8_rank(0), xa_prio(prio), xc_handle(handle), x10_area(area), x14_24_useAcoustics(useAcoustics),
          x14_26_inArea(0), x14_27_looped(looped), x14_28_playing(0), x14_29_active(0) {}
    };

    class CSfxEmitterWrapper : public CBaseSfxWrapper
    {
        CAudioSys::C3DEmitterParmData x24_parmData;
        std::shared_ptr<amuse::Emitter> x50_emitterHandle;
        bool x54_ready = true;
    public:
        bool IsPlaying() const;
        void Play();
        void Stop();
        bool Ready();
        ESfxAudibility GetAudible(const zeus::CVector3f&);
        const std::shared_ptr<amuse::Voice>& GetVoice() const { return x50_emitterHandle->getVoice(); }

        const std::shared_ptr<amuse::Emitter>& GetHandle() const { return x50_emitterHandle; }

        CSfxEmitterWrapper(bool looped, s16 prio, const CAudioSys::C3DEmitterParmData& data,
                           const CSfxHandle& handle, bool useAcoustics, TAreaId area)
        : CBaseSfxWrapper(looped, prio, handle, useAcoustics, area), x24_parmData(data) {}
    };

    class CSfxWrapper : public CBaseSfxWrapper
    {
        u16 x18_sfxId;
        std::shared_ptr<amuse::Voice> x1c_voiceHandle;
        s16 x20_vol;
        s16 x22_pan;
        bool x24_ready = true;
    public:
        bool IsPlaying() const;
        void Play();
        void Stop();
        bool Ready();
        ESfxAudibility GetAudible(const zeus::CVector3f&) {return ESfxAudibility::Aud3;}
        const std::shared_ptr<amuse::Voice>& GetVoice() const {return x1c_voiceHandle;}

        void SetVolume(s16 vol) {x20_vol = vol;}

        CSfxWrapper(bool looped, s16 prio, u16 sfxId, s16 vol, s16 pan,
                    const CSfxHandle& handle, bool useAcoustics, TAreaId area)
        : CBaseSfxWrapper(looped, prio, handle, useAcoustics, area),
          x18_sfxId(sfxId), x20_vol(vol), x22_pan(pan) {}
    };

    static CSfxChannel m_channels[4];
    static rstl::reserved_vector<CSfxEmitterWrapper, 128> m_emitterWrapperPool;
    static rstl::reserved_vector<CSfxWrapper, 128> m_wrapperPool;
    static ESfxChannels m_currentChannel;
    static bool m_doUpdate;
    static void* m_usedSounds;
    static bool m_muted;
    static bool m_auxProcessingEnabled;

    static u16 kMaxPriority;
    static u16 kMedPriority;
    static u16 kInternalInvalidSfxId;
    static u32 kAllAreas;

    static void SetChannel(ESfxChannels) {}
    static void KillAll(ESfxChannels) {}
    static void TurnOnChannel(ESfxChannels) {}
    static ESfxChannels GetCurrentChannel() {return m_currentChannel;}
    static void AddListener(ESfxChannels,
                            const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                            const zeus::CVector3f& heading, const zeus::CVector3f& up,
                            float frontRadius, float surroundRadius, float soundSpeed,
                            u32 flags /* 0x1 for doppler */, u8 vol);
    static void UpdateListener(const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                               const zeus::CVector3f& heading, const zeus::CVector3f& up,
                               u8 vol);

    static void RemoveEmitter(const CSfxHandle&) {}
    static void PitchBend(const CSfxHandle&, s32) {}
    static u16 TranslateSFXID(u16);
    static CSfxHandle SfxStop(const CSfxHandle& handle);
    static CSfxHandle SfxStart(u16 id, float vol, float pan, bool active, s16 prio, bool inArea, s32 areaId);
};

}

#endif // __URDE_CSFXMANAGER_HPP__
