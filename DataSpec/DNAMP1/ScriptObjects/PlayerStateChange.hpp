#ifndef _DNAMP1_PLAYERSTATECHANGE_HPP_
#define _DNAMP1_PLAYERSTATECHANGE_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct PlayerStateChange : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<bool> unknown;
    Value<atUint32> item;
    Value<atUint32> unknown2;
    Value<atUint32> unknown3;
    Value<atUint32> unknown4;
    Value<atUint32> unknown5;
};
}
}

#endif
