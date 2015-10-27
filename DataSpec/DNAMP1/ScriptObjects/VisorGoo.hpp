#ifndef _DNAMP1_VISORGOO_HPP_
#define _DNAMP1_VISORGOO_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct VisorGoo : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> position;
    UniqueID32 particle;
    Value<atUint32> unknown1; // always FF
    Value<float> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
    Value<atVec4f> unknown6; // CColor
    Value<atUint32> unknown7;
    Value<bool> unknown8;

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        if (particle)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle);
            ent->name = name + "_part";
        }
    }
};
}
}

#endif
