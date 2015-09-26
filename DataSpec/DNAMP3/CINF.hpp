#ifndef _DNAMP3_CINF_HPP_
#define _DNAMP3_CINF_HPP_

#include "BlenderConnection.hpp"
#include "../DNACommon/DNACommon.hpp"
#include "../DNAMP2/CINF.hpp"

namespace Retro
{
namespace DNAMP3
{

struct CINF : DNAMP2::CINF
{
    Delete expl;
    void sendCINFToBlender(HECL::BlenderConnection::PyOutStream& os, const UniqueID64& cinfId) const
    {
        os.format("arm = bpy.data.armatures.new('CINF_%016" PRIX64 "')\n"
                  "arm_obj = bpy.data.objects.new(arm.name, arm)\n"
                  "bpy.context.scene.objects.link(arm_obj)\n"
                  "bpy.context.scene.objects.active = arm_obj\n"
                  "bpy.ops.object.mode_set(mode='EDIT')\n"
                  "arm_bone_table = {}\n",
                  cinfId.toUint64());

        for (const Bone& bone : bones)
            os.format("bone = arm.edit_bones.new('%s')\n"
                      "bone.head = (%f,%f,%f)\n"
                      "bone.tail = bone.head\n"
                      "bone.tail[1] += 0.5\n"
                      "arm_bone_table[%u] = bone\n", getBoneNameFromId(bone.id)->c_str(),
                      bone.origin.vec[0], bone.origin.vec[1], bone.origin.vec[2], bone.id);

        for (const Bone& bone : bones)
            if (bone.parentId != 97 && bone.parentId != 147)
                os.format("arm_bone_table[%u].parent = arm_bone_table[%u]\n", bone.id, bone.parentId);

        os << "bpy.ops.object.mode_set(mode='OBJECT')\n";
    }
};

}
}

#endif // _DNAMP3_CINF_HPP_
