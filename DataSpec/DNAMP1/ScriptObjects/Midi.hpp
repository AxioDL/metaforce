#ifndef _DNAMP1_MIDI_HPP_
#define _DNAMP1_MIDI_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct Midi : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1> name;
    Value<bool> active;
    UniqueID32 song;
    Value<float> fadeInTime;
    Value<float> fadeOutTime;
    Value<atUint32> volume;

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        if (song)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(song);
            ent->name = name + "_song";
        }
    }

    void gatherDependencies(std::vector<hecl::ProjectPath> &pathsOut) const
    {
        // Dedicated PAK for this
        //g_curSpec->flattenDependencies(song, pathsOut);
    }
};
}

#endif
