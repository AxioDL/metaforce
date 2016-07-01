#include "CGameState.hpp"
#include "IOStreams.hpp"
#include "zeus/Math.hpp"

namespace urde
{

CGameState::CGameState(CBitStreamReader& stream)
{
    for (u32 i = 0; i < 128; i++)
        stream.ReadEncoded(8);
    u32 tmp = stream.ReadEncoded(32);
    double val1 = *(reinterpret_cast<float*>(&tmp));
    bool val2 = stream.ReadEncoded(1);
    stream.ReadEncoded(1);
    tmp = stream.ReadEncoded(32);
    double val3 = *(reinterpret_cast<float*>(&tmp));
    tmp = stream.ReadEncoded(32);
    double val4 = *(reinterpret_cast<float*>(&tmp));
    tmp = stream.ReadEncoded(32);
    double val5 = *(reinterpret_cast<float*>(&tmp));

    CPlayerState tmpPlayer(stream);
    float currentHealth = tmpPlayer.GetHealthInfo().GetHP();
}

void CGameState::SetCurrentWorldId(unsigned int id, const std::string& name)
{
}

void CGameState::SetTotalPlayTime(float time)
{
    xa0_playTime = zeus::clamp<double>(0.0, time, 359999.0);
}

}
