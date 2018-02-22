#include "Oculus.hpp"

namespace DataSpec::DNAMP1
{

template <class Op>
void Oculus::Enumerate(typename Op::StreamT& s)
{
    IScriptObject::Enumerate<Op>(s);
    Do<Op>({"name"}, name, s);
    Do<Op>({"location"}, location, s);
    Do<Op>({"orientation"}, orientation, s);
    Do<Op>({"scale"}, scale, s);
    Do<Op>({"patternedInfo"}, patternedInfo, s);
    Do<Op>({"actorParameters"}, actorParameters, s);
    Do<Op>({"unknown1"}, unknown1, s);
    Do<Op>({"unknown2"}, unknown2, s);
    Do<Op>({"unknown3"}, unknown3, s);
    Do<Op>({"unknown4"}, unknown4, s);
    Do<Op>({"unknown5"}, unknown5, s);
    Do<Op>({"unknown6"}, unknown6, s);
    Do<Op>({"damageVulnerabilty"}, damageVulnerabilty, s);
    Do<Op>({"unknown7"}, unknown7, s);
    Do<Op>({"damageInfo"}, damageInfo, s);
    if (propertyCount == 16)
        Do<Op>({"unknown8"}, unknown8, s);
    else
        unknown8 = 0.f;
}

const char* Oculus::DNAType()
{
    return "urde::DNAMP1::Oculus";
}

AT_SPECIALIZE_DNA_YAML(Oculus)

}