#include "CINF.hpp"
#include "hecl/Blender/Connection.hpp"

namespace DataSpec::DNAMP3 {

void CINF::sendCINFToBlender(hecl::blender::PyOutStream& os, const UniqueID64& cinfId) const {
  DNAANIM::RigInverter<CINF> inverter(*this);

  os.format(fmt("arm = bpy.data.armatures.new('CINF_{}')\n"
            "arm_obj = bpy.data.objects.new(arm.name, arm)\n"
            "bpy.context.scene.collection.objects.link(arm_obj)\n"
            "bpy.context.view_layer.objects.active = arm_obj\n"
            "bpy.ops.object.mode_set(mode='EDIT')\n"
            "arm_bone_table = {{}}\n"),
            cinfId);

  for (const DNAANIM::RigInverter<CINF>::Bone& bone : inverter.getBones()) {
    zeus::simd_floats originF(bone.m_origBone.origin.simd);
    zeus::simd_floats tailF(bone.m_tail.mSimd);
    os.format(fmt(
        "bone = arm.edit_bones.new('{}')\n"
        "bone.head = ({},{},{})\n"
        "bone.tail = ({},{},{})\n"
        "bone.use_inherit_scale = False\n"
        "arm_bone_table[{}] = bone\n"),
        *getBoneNameFromId(bone.m_origBone.id), originF[0], originF[1], originF[2], tailF[0], tailF[1],
        tailF[2], bone.m_origBone.id);
  }

  if (bones.size()) {
    atUint32 nullId = bones[0].parentId;
    for (const Bone& bone : bones)
      if (bone.parentId != nullId)
        os.format(fmt("arm_bone_table[{}].parent = arm_bone_table[{}]\n"), bone.id, bone.parentId);
  }

  os << "bpy.ops.object.mode_set(mode='OBJECT')\n";

  for (const DNAANIM::RigInverter<CINF>::Bone& bone : inverter.getBones())
    os.format(fmt("arm_obj.pose.bones['{}'].rotation_mode = 'QUATERNION'\n"),
              *getBoneNameFromId(bone.m_origBone.id));
}

std::string CINF::GetCINFArmatureName(const UniqueID64& cinfId) {
  return fmt::format(fmt("CINF_{}"), cinfId);
}

} // namespace DataSpec::DNAMP3
