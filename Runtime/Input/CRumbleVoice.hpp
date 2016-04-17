#ifndef CRUMBLEVOICE_HPP
#define CRUMBLEVOICE_HPP

#include "RetroTypes.hpp"

namespace urde
{
enum class ERumbleFxId
{
    Seven = 7
};
enum class ERumblePriority
{
    None,
    Two = 2
};

struct SAdsrData;
class CRumbleVoice
{
public:
    CRumbleVoice() {}
    CRumbleVoice(const SAdsrData& data);
};

struct SAdsrData
{
    float x0 = 0.f;
    float x4 = 0.f;
    float x8 = 0.f;
    float xc = 0.f;
    float x10 = 0.f;
    float x14 = 0.f;
    union
    {
        struct { bool x18_24 : 1; bool x18_25 : 1; };
        u8 dummy = 0;
    };

    SAdsrData() = default;
    SAdsrData(float a, float b, float c, float d, float e, float f, bool g, bool h)
        : x0(a), x4(b), x8(c), xc(d), x10(e), x14(f)
    {
        x18_24 = g;
        x18_25 = h;
    }
};

struct SAdsrDelta
{
    enum class EPhase
    {
        Stop,
        Start,
    };

    float x0 = 0.f;
    float x4 = 0.f;
    float x8 = 0.f;
    float xc = 0.f;
    float x10 = 0.f;
    ERumblePriority x1c_priority;
    EPhase          x20_phase;

    SAdsrDelta(EPhase phase, ERumblePriority priority)
        : x1c_priority(priority), x20_phase(phase)
    {}
    SAdsrDelta(EPhase phase)
        : x1c_priority(ERumblePriority::None), x20_phase(phase)
    {}

    static SAdsrDelta Stopped() { return SAdsrDelta(EPhase::Stop); }
    static SAdsrDelta Start(ERumblePriority priority) { return SAdsrDelta(EPhase::Start, priority); }
};
}

#endif // CRUMBLEVOICE_HPP
