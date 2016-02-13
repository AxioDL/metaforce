#ifndef _DNAMP1_TEAMAIMGR_HPP_
#define _DNAMP1_TEAMAIMGR_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct TeamAIMgr : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atUint32> unknown1;
    Value<atUint32> unknown2;
    Value<atUint32> unknown3;
    Value<atUint32> unknown4;
    Value<atUint32> unknown5;
    Value<atUint32> unknown6;
    Value<atUint32> unknown7;
    Value<float> unknown8;
    Value<float> unknown9;
};
}
}

#endif
