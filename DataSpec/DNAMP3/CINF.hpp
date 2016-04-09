#ifndef _DNAMP3_CINF_HPP_
#define _DNAMP3_CINF_HPP_

#include "BlenderConnection.hpp"
#include "../DNACommon/DNACommon.hpp"
#include "../DNAMP2/CINF.hpp"

namespace DataSpec
{
namespace DNAMP3
{

struct CINF : DNAMP2::CINF
{
    Delete expl;
    void sendCINFToBlender(hecl::BlenderConnection::PyOutStream& os, const UniqueID64& cinfId) const
    {
        DNAANIM::RigInverter<CINF> inverter(*this);

        os.format("arm = bpy.data.armatures.new('CINF_%016" PRIX64 "')\n"
                  "arm_obj = bpy.data.objects.new(arm.name, arm)\n"
                  "bpy.context.scene.objects.link(arm_obj)\n"
                  "bpy.context.scene.objects.active = arm_obj\n"
                  "bpy.ops.object.mode_set(mode='EDIT')\n"
                  "arm_bone_table = {}\n",
                  cinfId.toUint64());

        for (const DNAANIM::RigInverter<CINF>::Bone& bone : inverter.getBones())
            os.format("bone = arm.edit_bones.new('%s')\n"
                      "bone.head = (%f,%f,%f)\n"
                      "bone.tail = (%f,%f,%f)\n"
                      "bone.use_inherit_scale = False\n"
                      "arm_bone_table[%u] = bone\n", getBoneNameFromId(bone.m_origBone.id)->c_str(),
                      bone.m_origBone.origin.vec[0], bone.m_origBone.origin.vec[1], bone.m_origBone.origin.vec[2],
                      bone.m_tail[0], bone.m_tail[1], bone.m_tail[2],
                      bone.m_origBone.id);

        if (bones.size())
        {
            atUint32 nullId = bones[0].parentId;
            for (const Bone& bone : bones)
                if (bone.parentId != nullId)
                    os.format("arm_bone_table[%u].parent = arm_bone_table[%u]\n", bone.id, bone.parentId);
        }

        os << "bpy.ops.object.mode_set(mode='OBJECT')\n";
    }

    static std::string GetCINFArmatureName(const UniqueID64& cinfId)
    {
        return hecl::Format("CINF_%016" PRIX64, cinfId.toUint64());
    }
};

}
}

#endif // _DNAMP3_CINF_HPP_
