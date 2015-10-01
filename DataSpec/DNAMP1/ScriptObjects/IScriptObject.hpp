#ifndef __DNAMP1_SCRIPTOBJECT_HPP
#define __DNAMP1_SCRIPTOBJECT_HPP
#include "../../DNACommon/DNACommon.hpp"
#include <stdio.h>

namespace Retro
{
namespace DNAMP1
{
struct IScriptObject : BigYAML
{
    DECL_YAML
    IScriptObject() {}
    atUint32 type;
    Value<atUint32> id;
    struct Connection : BigYAML
    {
        DECL_YAML
        Value<atUint32> state;
        Value<atUint32> msg;
        Value<atUint32> target;

    };

    Value<atUint32> connectionCount;
    Vector<Connection, DNA_COUNT(connectionCount)> connections;
    Value<atUint32> propertyCount;
    virtual ~IScriptObject()
    {
    }
};
}
}

#endif
