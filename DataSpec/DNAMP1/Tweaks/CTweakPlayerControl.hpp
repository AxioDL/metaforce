#ifndef _DNAMP1_CTWEAKPLAYERCONTROL_HPP_
#define _DNAMP1_CTWEAKPLAYERCONTROL_HPP_

#include "../../DNACommon/Tweaks/ITweakPlayerControl.hpp"

namespace DataSpec
{
namespace DNAMP1
{

struct CTweakPlayerControl : ITweakPlayerControl
{
    DECL_YAML
    Vector<atUint32, DNA_COUNT(65)> m_mappings;
    atUint32 GetMapping(atUint32 command) const {return m_mappings[command];}
    CTweakPlayerControl(athena::io::IStreamReader& reader) {this->read(reader);}
};

}
}

#endif // _DNAMP1_CTWEAKPLAYERCONTROL_HPP_
