#ifndef _DNAMP1_CTWEAKPLAYERCONTROL_HPP_
#define _DNAMP1_CTWEAKPLAYERCONTROL_HPP_

#include "../../DNACommon/Tweaks/ITweakPlayerControl.hpp"

namespace DataSpec::DNAMP1
{

struct CTweakPlayerControl final : ITweakPlayerControl
{
    DECL_YAML
    Vector<atUint32, DNA_COUNT(67)> m_mappings;
    atUint32 GetMapping(atUint32 command) const {return m_mappings[command];}
    CTweakPlayerControl() = default;
    CTweakPlayerControl(athena::io::IStreamReader& reader) {this->read(reader);}
};

}

#endif // _DNAMP1_CTWEAKPLAYERCONTROL_HPP_
