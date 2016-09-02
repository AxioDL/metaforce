#ifndef __DNAMP1_SCRIPTOBJECT_HPP
#define __DNAMP1_SCRIPTOBJECT_HPP
#include "../../DNACommon/DNACommon.hpp"
#include "../DNAMP1.hpp"
#include <stdio.h>

namespace DataSpec
{
namespace DNAMP1
{
struct IScriptObject : BigYAML
{
    DECL_YAML
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
    virtual ~IScriptObject() = default;

    virtual void addCMDLRigPairs(PAKRouter<PAKBridge>&,
                                 std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>&) const
    {
    }
    virtual void nameIDs(PAKRouter<PAKBridge>& pakRouter) const {}
};
}
}

#endif
