#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct CameraFilterKeyframe : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1> name;
    Value<bool> active;
    Value<atUint32> ftype;
    Value<atUint32> shape;
    Value<atUint32> filterIdx;
    Value<atUint32> unk;
    DNAColor color;
    Value<float> timeIn;
    Value<float> timeOut;
    UniqueID32 texture;

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        if (texture)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(texture);
            ent->name = name + "_texture";
        }
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                            std::vector<hecl::ProjectPath>& lazyOut) const
    {
        g_curSpec->flattenDependencies(texture, pathsOut);
    }
};
}

