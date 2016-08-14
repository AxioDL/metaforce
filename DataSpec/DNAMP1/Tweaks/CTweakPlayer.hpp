#ifndef _DNAMP1_CTWEAKPLAYER_HPP_
#define _DNAMP1_CTWEAKPLAYER_HPP_

#include "../../DNACommon/Tweaks/ITweakPlayer.hpp"

namespace DataSpec
{
namespace DNAMP1
{

struct CTweakPlayer : ITweakPlayer
{
    DECL_YAML
    Value<float> m_playerHeight;
    Value<float> m_playerXYHalfExtent;
    Value<float> m_unk1;
    Value<float> m_unk2;
    Value<float> m_unk3;
    Value<float> m_leftDiv;
    Value<float> m_rightDiv;
    float GetPlayerHeight() const { return m_playerHeight; }
    float GetPlayerXYHalfExtent() const { return m_playerXYHalfExtent; }
    float GetPlayerSomething1() const { return m_unk1; }
    float GetPlayerSomething2() const { return m_unk2; }
    float GetPlayerSomething3() const { return m_unk3; }
    float GetLeftLogicalThreshold() const {return m_leftDiv;}
    float GetRightLogicalThreshold() const {return m_rightDiv;}
    CTweakPlayer(athena::io::IStreamReader& reader) {this->read(reader);}
};

}
}

#endif // _DNAMP1_CTWEAKPLAYER_HPP_
