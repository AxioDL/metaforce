#ifndef __DNAMP1_SCRIPTOBJECT_HPP
#define __DNAMP1_SCRIPTOBJECT_HPP
#include "../../DNACommon/DNACommon.hpp"
#include "../DNAMP1.hpp"
#include "../SAVW.hpp"
#include "zeus/CAABox.hpp"
#include "specter/genie.hpp"

#include <cstdio>

#define SO_NAME_SPECPROP() SPECTER_PROPERTY("Name", "Instance name; Used to debug scripting events")
#define SO_LOCATION_SPECPROP() SPECTER_PROPERTY("Location", "World relative location of the Actor instance")
#define SO_ORIENTATION_SPECPROP() SPECTER_PROPERTY("Orientation", "Object local axis-angle")
#define SO_SCALE_SPECPROP() SPECTER_PROPERTY("Scale", "Object local scale")
#define SO_COLLISION_EXTENT_SPECPROP() SPECTER_PROPERTY("Collision Extent", "")
#define SO_COLLISION_OFFSET_SPECPROP() SPECTER_PROPERTY("Collision Offset", "")
#define SO_ACTIVE_SPECPROP() SPECTER_PROPERTY("Active", "If enabled, object instance is drawn and updated")

namespace DataSpec::DNAMP1
{

zeus::CTransform ConvertEditorEulerToTransform4f(const zeus::CVector3f& scale,
                                                 const zeus::CVector3f& orientation,
                                                 const zeus::CVector3f& position);

struct IScriptObject : BigDNAVYaml
{
    AT_DECL_DNA_YAML
    atUint32 type;
    Value<atUint32> id;
    struct Connection : BigDNA
    {
        AT_DECL_DNA_YAML
        Value<atUint32> state;
        Value<atUint32> msg;
        Value<atUint32> target;
    };

    Value<atUint32> connectionCount;
    Vector<Connection, AT_DNA_COUNT(connectionCount)> connections;
    Value<atUint32> propertyCount;
    virtual ~IScriptObject() = default;

    virtual void addCMDLRigPairs(PAKRouter<PAKBridge>&,
                                 std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>&) const {}
    virtual void nameIDs(PAKRouter<PAKBridge>& pakRouter) const {}
    virtual void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                                    std::vector<hecl::ProjectPath>& lazyOut) const {}
    virtual void gatherScans(std::vector<Scan>& scansOut) const {}
    virtual zeus::CAABox getVISIAABB(hecl::blender::Token& btok) const { return {}; }
};
}

#endif
