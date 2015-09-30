#ifndef _DNAMP1_DOCK_HPP_
#define _DNAMP1_DOCK_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct Dock : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<bool> active;
    Value<atVec3f> location;
    Value<atVec3f> volume;
    Value<atUint32> dock;
    Value<atUint32> room;
    Value<bool> autoLoad;
};
}
}

#endif
