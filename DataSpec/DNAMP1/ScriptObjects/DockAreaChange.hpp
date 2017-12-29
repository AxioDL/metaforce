#ifndef _DNAMP1_DOCKAREACHANGE_HPP_
#define _DNAMP1_DOCKAREACHANGE_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct DockAreaChange : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atUint32> unknown1;
    Value<bool> unknown2;
};
}

#endif
