#ifndef CRUMBLEVOICE_HPP
#define CRUMBLEVOICE_HPP

#include "RetroTypes.hpp"

namespace urde
{
enum class ERumbleFxId
{
    CameraShake = 6,
    EscapeSequenceShake = 7,
    PlayerBump = 11,
    PlayerGunCharge = 12,
    PlayerMissileFire = 13,
    PlayerGrappleFire = 14,
    PlayerLand = 15,
    PlayerGrappleSwoosh = 17
};
enum class ERumblePriority
{
    None = 0,
    One = 1,
    Two = 2,
    Three = 3
};

struct SAdsrData
{
    float x0_attackGain = 0.f;
    float x4_autoReleaseDur = 0.f;
    float x8_attackDur = 0.f;
    float xc_decayDur = 0.f;
    float x10_sustainGain = 0.f;
    float x14_releaseDur = 0.f;
    union
    {
        struct { bool x18_24_hasSustain : 1; bool x18_25_autoRelease : 1; };
        u8 dummy = 0;
    };

    SAdsrData() = default;
    SAdsrData(float attackGain, float autoReleaseDur, float attackDur, float decayDur,
              float sustainGain, float releaseDur, bool hasSustain, bool autoRelease)
    : x0_attackGain(attackGain), x4_autoReleaseDur(autoReleaseDur), x8_attackDur(attackDur),
      xc_decayDur(decayDur), x10_sustainGain(sustainGain), x14_releaseDur(releaseDur)
    {
        x18_24_hasSustain = hasSustain;
        x18_25_autoRelease = autoRelease;
    }
};

struct SAdsrDelta
{
    enum class EPhase
    {
        Stop,
        Queued,
        Attack,
        Decay,
        Sustain,
        Release
    };

    float x0_curLevel = 0.f;
    float x4_attackTime = 0.f;
    float x8_decayTime = 0.f;
    float xc_releaseTime = 0.f;
    float x10_autoReleaseTime = 0.f;
    float x14_attackLevel;
    float x18_sustainLevel;
    ERumblePriority x1c_priority;
    EPhase x20_phase;

    SAdsrDelta(EPhase phase, ERumblePriority priority)
        : x1c_priority(priority), x20_phase(phase)
    {}
    SAdsrDelta(EPhase phase)
        : x1c_priority(ERumblePriority::None), x20_phase(phase)
    {}

    static SAdsrDelta Stopped() { return SAdsrDelta(EPhase::Stop); }
    static SAdsrDelta Start(ERumblePriority priority, bool preQueue)
    { return SAdsrDelta(preQueue ? EPhase::Queued : EPhase::Attack, priority); }
};

class CRumbleVoice
{
    std::vector<SAdsrData> x0_datas;
    std::vector<SAdsrDelta> x10_deltas;
    rstl::reserved_vector<s16, 4> x20_handleIds;
    s16 x2c_usedChannels = 0;
    u8 x2e_lastId = 0;
    bool UpdateChannel(SAdsrDelta& delta, const SAdsrData& data, float dt);
public:
    CRumbleVoice();
    s16 CreateRumbleHandle(s16 idx);
    bool OwnsSustained(s16 id) const;
    s16 GetFreeChannel() const;
    float GetIntensity() const;
    bool Update(float dt);
    void HardReset();
    s16 Activate(const SAdsrData& data, s16 idx, float gain, ERumblePriority prio);
    void Deactivate(s16 id, bool b1);
    ERumblePriority GetPriority(s16 idx) const { return x10_deltas[idx].x1c_priority; }
};
}

#endif // CRUMBLEVOICE_HPP
