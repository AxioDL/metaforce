#ifndef _DNAMP1_DOORAREA_HPP_
#define _DNAMP1_DOORAREA_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct DoorArea : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    AnimationParameters animationParameters;
    ActorParameters actorParameters;
    Value<atVec3f> unknown1;
    Value<atVec3f> collisionExtent;
    Value<atVec3f> collisionOffset;
    Value<bool> unknown4;
    Value<bool> unknown5;
    Value<bool> unknown6;
    Value<float> unknown7;
    Value<bool> unknown8;

    void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
            std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo) const
    {
        actorParameters.addCMDLRigPairs(addTo, animationParameters.getCINF(pakRouter));
    }

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        animationParameters.nameANCS(pakRouter, name + "_animp");
        actorParameters.nameIDs(pakRouter, name + "_actp");
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const
    {
        animationParameters.depANCS(pathsOut);
        actorParameters.depIDs(pathsOut);
    }

    void gatherScans(std::vector<Scan>& scansOut) const
    {
        actorParameters.scanIDs(scansOut);
    }

    zeus::CAABox getVISIAABB(hecl::BlenderToken& btok) const
    {
        hecl::BlenderConnection& conn = btok.getBlenderConnection();
        zeus::CAABox aabbOut;

        if (animationParameters.animationCharacterSet)
        {
            hecl::ProjectPath path = UniqueIDBridge::TranslatePakIdToPath(
                animationParameters.animationCharacterSet);
            conn.openBlend(path.getWithExtension(_S(".blend"), true));
            hecl::BlenderConnection::DataStream ds = conn.beginData();
            auto aabb = ds.getMeshAABB();
            aabbOut = zeus::CAABox(aabb.first, aabb.second);
        }

        if (aabbOut.min.x > aabbOut.max.x)
            return {};

        zeus::CTransform xf = ConvertEditorEulerToTransform4f(scale, orientation, location);
        return aabbOut.getTransformedAABox(xf);
    }
};
}
}

#endif
