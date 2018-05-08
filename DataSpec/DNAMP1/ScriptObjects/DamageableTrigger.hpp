#ifndef _DNAMP1_DAMAGABLETRIGGER_HPP_
#define _DNAMP1_DAMAGABLETRIGGER_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct DamageableTrigger : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> volume;
    HealthInfo healthInfo;
    DamageVulnerability damageVulnerabilty;
    Value<atUint32> faceFlag;
    UniqueID32 patternTex1;
    UniqueID32 patternTex2;
    UniqueID32 colorTex;
    Value<bool> lockOn;
    Value<bool> active;
    VisorParameters visorParameters;

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        if (patternTex1)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(patternTex1);
            ent->name = name + "_patternTex1";
        }
        if (patternTex2)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(patternTex2);
            ent->name = name + "_patternTex2";
        }
        if (colorTex)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(colorTex);
            ent->name = name + "_colorTex";
        }
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                            std::vector<hecl::ProjectPath>& lazyOut) const
    {
        g_curSpec->flattenDependencies(patternTex1, pathsOut);
        g_curSpec->flattenDependencies(patternTex2, pathsOut);
        g_curSpec->flattenDependencies(colorTex, pathsOut);
    }

    zeus::CAABox getVISIAABB(hecl::blender::Token& btok) const
    {
        zeus::CVector3f halfExtent = zeus::CVector3f(volume) / 2.f;
        zeus::CVector3f loc(location);
        return zeus::CAABox(loc - halfExtent, loc + halfExtent);
    }
};
}

#endif
