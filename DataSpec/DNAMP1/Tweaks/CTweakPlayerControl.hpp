#pragma once

#include "../../DNACommon/Tweaks/ITweakPlayerControl.hpp"

namespace DataSpec::DNAMP1
{

struct CTweakPlayerControl final : ITweakPlayerControl
{
    AT_DECL_DNA_YAML
    Vector<atUint32, AT_DNA_COUNT(67)> m_mappings;
    atUint32 GetMapping(atUint32 command) const {return m_mappings[command];}
    CTweakPlayerControl() = default;
    CTweakPlayerControl(athena::io::IStreamReader& reader) {this->read(reader);}
};

}

