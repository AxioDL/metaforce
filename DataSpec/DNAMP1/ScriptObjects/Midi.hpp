#ifndef _DNAMP1_MIDI_HPP_
#define _DNAMP1_MIDI_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct Midi : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<bool> unknown1;
    UniqueID32 song;
    Value<float> unknown2;
    Value<float> unknown3;
    Value<atUint32> unknown4;

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        if (song)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(song);
            ent->name = name + "_song";
        }
    }
};
}
}

#endif
