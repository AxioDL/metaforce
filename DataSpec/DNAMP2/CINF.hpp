#ifndef _DNAMP2_CINF_HPP_
#define _DNAMP2_CINF_HPP_

#include "DataSpec/DNACommon/DNACommon.hpp"
#include "DataSpec/DNACommon/RigInverter.hpp"

namespace DataSpec::DNAMP2
{

struct CINF : BigDNA
{
    AT_DECL_DNA
    Value<atUint32> boneCount;
    struct Bone : BigDNA
    {
        AT_DECL_DNA
        Value<atUint32> id;
        Value<atUint32> parentId;
        Value<atVec3f> origin;
        Value<atVec4f> q1;
        Value<atVec4f> q2;
        Value<atUint32> linkedCount;
        Vector<atUint32, AT_DNA_COUNT(linkedCount)> linked;
    };
    Vector<Bone, AT_DNA_COUNT(boneCount)> bones;

    Value<atUint32> boneIdCount;
    Vector<atUint32, AT_DNA_COUNT(boneIdCount)> boneIds;

    Value<atUint32> nameCount;
    struct Name : BigDNA
    {
        AT_DECL_DNA
        String<-1> name;
        Value<atUint32> boneId;
    };
    Vector<Name, AT_DNA_COUNT(nameCount)> names;
    
    atUint32 getInternalBoneIdxFromId(atUint32 id) const;
    atUint32 getBoneIdxFromId(atUint32 id) const;
    const std::string* getBoneNameFromId(atUint32 id) const;
    void sendVertexGroupsToBlender(hecl::blender::PyOutStream& os) const;
    void sendCINFToBlender(hecl::blender::PyOutStream& os, const UniqueID32& cinfId) const;
    static std::string GetCINFArmatureName(const UniqueID32& cinfId);
};

}

#endif // _DNAMP2_CINF_HPP_
