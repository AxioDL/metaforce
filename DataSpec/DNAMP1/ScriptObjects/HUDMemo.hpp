#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct HUDMemo : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1> name;
    Value<float> firstMessageTimer;
    Value<bool> unknown1;
    Value<atUint32> memoType;
    UniqueID32 message;
    Value<bool> active;

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        if (message)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(message);
            ent->name = name + "_message";
        }
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                            std::vector<hecl::ProjectPath>& lazyOut) const
    {
        g_curSpec->flattenDependencies(message, pathsOut);
    }
};
}

