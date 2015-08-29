#ifndef _DNAMP1_CTWEAKPLAYER_HPP_
#define _DNAMP1_CTWEAKPLAYER_HPP_

#include "../../DNACommon/Tweaks/ITweakPlayer.hpp"

namespace Retro
{
namespace DNAMP1
{

struct CTweakPlayer : ITweakPlayer
{
    DECL_YAML
    Value<float> m_leftDiv;
    Value<float> m_rightDiv;
    float GetLeftLogicalThreshold() const {return m_leftDiv;}
    float GetRightLogicalThreshold() const {return m_rightDiv;}
    CTweakPlayer(Athena::io::IStreamReader& reader) {this->read(reader);}
};

}
}

#endif // _DNAMP1_CTWEAKPLAYER_HPP_
