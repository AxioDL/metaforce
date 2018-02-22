#ifndef _DNAMP1_DOCK_HPP_
#define _DNAMP1_DOCK_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct Dock : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1> name;
    Value<bool> active;
    Value<atVec3f> location;
    Value<atVec3f> volume;
    Value<atUint32> dock;
    Value<atUint32> room;
    Value<bool> loadConnected;
};
}

#endif
