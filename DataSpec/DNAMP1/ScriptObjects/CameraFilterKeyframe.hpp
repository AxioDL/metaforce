#ifndef _DNAMP1_CAMERAFILTERKEYFRAME_HPP_
#define _DNAMP1_CAMERAFILTERKEYFRAME_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct CameraFilterKeyframe : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<bool> unknown1;
    Value<atUint32> unkown2;
    Value<atUint32> unkown3;
    Value<atUint32> unkown4;
    Value<atUint32> unkown5;
    Value<atVec4f> unknown6; //CColor
    Value<float> unknown7;
    Value<float> unknown8;
    UniqueID32 texture;

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        if (texture)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(texture);
            ent->name = name + "_texture";
        }
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const
    {
        g_curSpec->flattenDependencies(texture, pathsOut);
    }
};
}
}

#endif
