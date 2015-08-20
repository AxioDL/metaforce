#ifndef __RETRO_CSFXMANAGER_HPP__
#define __RETRO_CSFXMANAGER_HPP__

#include <vector>
#include "../RetroTypes.hpp"
#include "CSfxHandle.hpp"
#include "CVector3f.hpp"
#include "CAudioSys.hpp"

namespace Retro
{

class CSfxManager
{
    enum ESfxChannels
    {
    };

    enum ESfxAudibility
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
        s16 m_rank;
        s16 m_prio;
        CSfxHandle m_handle;
        TAreaId m_area;
        bool m_useAcoustics:1;
        bool m_available:1;
        bool m_inArea:1;
        bool m_looped:1;
        bool m_playing:1;
        bool m_active:1;
    public:
        virtual ~CBaseSfxWrapper() {}
        virtual void SetActive(bool v) {m_active = v;}
        virtual void SetPlaying(bool v) {m_playing = v;}
        virtual void SetRank(short v) {m_rank = v;}
        virtual void SetInArea(bool v) {m_inArea = v;}
        virtual bool IsLooped() const {return m_looped;}
        virtual bool IsPlaying() const {return m_playing;}
        virtual bool IsActive() const {return m_active;}
        virtual bool IsInArea() const {return m_inArea;}
        virtual bool UseAcoustics() const {return m_useAcoustics;}
        virtual s16 GetRank() const {return m_rank;}
        virtual s16 GetPriority() const {return m_prio;}
        virtual TAreaId GetArea() const {return m_area;}
        virtual CSfxHandle GetSfxHandle() const {return m_handle;}
        virtual void Play()=0;
        virtual void Stop()=0;
        virtual bool Ready()=0;
        virtual ESfxAudibility GetAudible(const CVector3f&)=0;
        virtual u32 GetVoice() const=0;

        void Release() {m_available = true;}
        bool Available() const {return m_available;}

        CBaseSfxWrapper(bool looped, s16 prio, const CSfxHandle& handle, bool useAcoustics, TAreaId area)
        : m_rank(0), m_prio(prio), m_handle(handle), m_area(area), m_useAcoustics(useAcoustics),
          m_inArea(0), m_looped(looped), m_playing(0), m_active(0) {}
    };

    class CSfxEmitterWrapper : public CBaseSfxWrapper
    {
        CAudioSys::C3DEmitterParmData m_parmData;
        u32 m_emitterHandle = -1;
    public:
        bool IsPlaying() const;
        void Play();
        void Stop();
        bool Ready();
        ESfxAudibility GetAudible(const CVector3f&);
        u32 GetVoice() const;

        u32 GetHandle() const {return m_emitterHandle;}

        CSfxEmitterWrapper(bool looped, s16 prio, const CAudioSys::C3DEmitterParmData& data,
                           const CSfxHandle& handle, bool useAcoustics, TAreaId area)
        : CBaseSfxWrapper(looped, prio, handle, useAcoustics, area), m_parmData(data) {}
    };

    class CSfxWrapper : public CBaseSfxWrapper
    {
        u16 m_sfxId;
        u32 m_voiceHandle = -1;
        s16 m_vol;
        s16 m_pan;
    public:
        bool IsPlaying() const;
        void Play();
        void Stop();
        bool Ready();
        ESfxAudibility GetAudible(const CVector3f&) {return Aud3;}
        u32 GetVoice() const {return m_voiceHandle;}

        void SetVolume(s16 vol) {m_vol = vol;}

        CSfxWrapper(bool looped, s16 prio, u16 sfxId, s16 vol, s16 pan,
                    const CSfxHandle& handle, bool useAcoustics, TAreaId area)
        : CBaseSfxWrapper(looped, prio, handle, useAcoustics, area),
          m_sfxId(sfxId), m_vol(vol), m_pan(pan) {}
    };

    static CSfxChannel m_channels[4];
    static rstl::reserved_vector<CSfxEmitterWrapper, 128> m_emitterWrapperPool;
    static rstl::reserved_vector<CSfxWrapper, 128> m_wrapperPool;
    static ESfxChannels m_currentChannel;
    static bool m_doUpdate;
    static void* m_sfxTranslationTable;
    static void* m_usedSounds;
    static bool m_muted;
    static bool m_auxProcessingEnabled;

    static u16 kMaxPriority;
    static u16 kMedPriority;
    static u16 kInternalInvalidSfxId;
    static u32 kAllAreas;

    static ESfxChannels GetCurrentChannel() {return m_currentChannel;}
};

}

#endif // __RETRO_CSFXMANAGER_HPP__
