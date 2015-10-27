#ifndef _DNAMP1_HUDMEMO_HPP_
#define _DNAMP1_HUDMEMO_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct HUDMemo : IScriptObject
{
    DECL_YAML
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
};
}
}

#endif
