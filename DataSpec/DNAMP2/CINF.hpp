#ifndef _DNAMP2_CINF_HPP_
#define _DNAMP2_CINF_HPP_

#include "BlenderConnection.hpp"
#include "../DNACommon/DNACommon.hpp"

namespace DataSpec
{
namespace DNAMP2
{

struct CINF : BigDNA
{
    DECL_DNA
    Value<atUint32> boneCount;
    struct Bone : BigDNA
    {
        DECL_DNA
        Value<atUint32> id;
        Value<atUint32> parentId;
        Value<atVec3f> origin;
        Value<float> skinMetrics[8];
        Value<atUint32> linkedCount;
        Vector<atUint32, DNA_COUNT(linkedCount)> linked;
    };
    Vector<Bone, DNA_COUNT(boneCount)> bones;

    Value<atUint32> boneIdCount;
    Vector<atUint32, DNA_COUNT(boneIdCount)> boneIds;

    Value<atUint32> nameCount;
    struct Name : BigDNA
    {
        DECL_DNA
        String<-1> name;
        Value<atUint32> boneId;
    };
    Vector<Name, DNA_COUNT(nameCount)> names;
    
    atUint32 getInternalBoneIdxFromId(atUint32 id) const
    {
        atUint32 idx = 0;
        for (const Bone& b : bones)
        {
            if (b.id == id)
                return idx;
            ++idx;
        }
        return -1;
    }

    atUint32 getBoneIdxFromId(atUint32 id) const
    {
        atUint32 idx = 0;
        for (atUint32 bid : boneIds)
        {
            if (bid == id)
                return idx;
            ++idx;
        }
        return 0;
    }

    const std::string* getBoneNameFromId(atUint32 id) const
    {
        for (const Name& name : names)
            if (id == name.boneId)
                return &name.name;
        return nullptr;
    }

    void sendVertexGroupsToBlender(hecl::BlenderConnection::PyOutStream& os) const
    {
        for (atUint32 bid : boneIds)
        {
            for (const Name& name : names)
            {
                if (name.boneId == bid)
                {
                    os.format("obj.vertex_groups.new('%s')\n", name.name.c_str());
                    break;
                }
            }
        }
    }

    void sendCINFToBlender(hecl::BlenderConnection::PyOutStream& os, const UniqueID32& cinfId) const
    {
        os.format("arm = bpy.data.armatures.new('CINF_%08X')\n"
                  "arm_obj = bpy.data.objects.new(arm.name, arm)\n"
                  "bpy.context.scene.objects.link(arm_obj)\n"
                  "bpy.context.scene.objects.active = arm_obj\n"
                  "bpy.ops.object.mode_set(mode='EDIT')\n"
                  "arm_bone_table = {}\n",
                  cinfId.toUint32());

        for (const Bone& bone : bones)
            os.format("bone = arm.edit_bones.new('%s')\n"
                      "bone.head = (%f,%f,%f)\n"
                      "bone.tail = bone.head\n"
                      "bone.tail[1] += 0.5\n"
                      "bone.use_inherit_scale = False\n"
                      "arm_bone_table[%u] = bone\n", getBoneNameFromId(bone.id)->c_str(),
                      bone.origin.vec[0], bone.origin.vec[1], bone.origin.vec[2], bone.id);

        for (const Bone& bone : bones)
            if (bone.parentId != 97)
                os.format("arm_bone_table[%u].parent = arm_bone_table[%u]\n", bone.id, bone.parentId);

        os << "bpy.ops.object.mode_set(mode='OBJECT')\n";
    }

    static std::string GetCINFArmatureName(const UniqueID32& cinfId)
    {
        return hecl::Format("CINF_%08X", cinfId.toUint32());
    }
};

}
}

#endif // _DNAMP2_CINF_HPP_
