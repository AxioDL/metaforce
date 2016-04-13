#ifndef _DNAMP1_CINF_HPP_
#define _DNAMP1_CINF_HPP_

#include "BlenderConnection.hpp"
#include "../DNACommon/DNACommon.hpp"
#include "../DNACommon/RigInverter.hpp"

namespace DataSpec
{
namespace DNAMP1
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
        DNAANIM::RigInverter<CINF> inverter(*this);

        os.format("arm = bpy.data.armatures.new('CINF_%08X')\n"
                  "arm_obj = bpy.data.objects.new(arm.name, arm)\n"
                  "bpy.context.scene.objects.link(arm_obj)\n"
                  "bpy.context.scene.objects.active = arm_obj\n"
                  "bpy.ops.object.mode_set(mode='EDIT')\n"
                  "arm_bone_table = {}\n",
                  cinfId.toUint32());

        for (const DNAANIM::RigInverter<CINF>::Bone& bone : inverter.getBones())
            os.format("bone = arm.edit_bones.new('%s')\n"
                      "bone.head = (%f,%f,%f)\n"
                      "bone.tail = (%f,%f,%f)\n"
                      "bone.use_inherit_scale = False\n"
                      "arm_bone_table[%u] = bone\n", getBoneNameFromId(bone.m_origBone.id)->c_str(),
                      bone.m_origBone.origin.vec[0], bone.m_origBone.origin.vec[1], bone.m_origBone.origin.vec[2],
                      bone.m_tail[0], bone.m_tail[1], bone.m_tail[2],
                      bone.m_origBone.id);

        for (const Bone& bone : bones)
            if (bone.parentId != 2)
                os.format("arm_bone_table[%u].parent = arm_bone_table[%u]\n", bone.id, bone.parentId);

        os << "bpy.ops.object.mode_set(mode='OBJECT')\n";
    }

    static std::string GetCINFArmatureName(const UniqueID32& cinfId)
    {
        return hecl::Format("CINF_%08X", cinfId.toUint32());
    }

    CINF() = default;
    using Armature = hecl::BlenderConnection::DataStream::Actor::Armature;

    int RecursiveAddArmatureBone(const Armature& armature, const Armature::Bone* bone, int parent, int& curId,
                                 std::unordered_map<std::string, atInt32>& idMap, std::map<std::string, int>& nameMap)
    {
        int selId;
        auto search = idMap.find(bone->name);
        if (search == idMap.end())
        {
            selId = curId++;
            idMap.emplace(std::make_pair(bone->name, selId));
        }
        else
            selId = search->second;

        bones.emplace_back();
        Bone& boneOut = bones.back();
        nameMap[bone->name] = selId;
        boneOut.id = selId;
        boneOut.parentId = parent;
        boneOut.origin = bone->origin;
        boneOut.linkedCount = bone->children.size() + 1;
        boneOut.linked.reserve(boneOut.linkedCount);

        const Armature::Bone* child;
        boneOut.linked.push_back(parent);
        for (size_t i=0 ; (child = armature.getChild(bone, i)) ; ++i)
            boneOut.linked.push_back(RecursiveAddArmatureBone(armature, child, boneOut.id, selId, idMap, nameMap));

        return boneOut.id;
    }

    CINF(const Armature& armature, std::unordered_map<std::string, atInt32>& idMap)
    {
        idMap.reserve(armature.bones.size());
        bones.reserve(armature.bones.size());

        std::map<std::string, int> nameMap;

        const Armature::Bone* bone = armature.getRoot();
        if (bone)
        {
            if (bone->children.size())
            {
                int curId = 4;
                RecursiveAddArmatureBone(armature, armature.getChild(bone, 0), 3, curId, idMap, nameMap);
            }

            bones.emplace_back();
            Bone& boneOut = bones.back();
            nameMap[bone->name] = 3;
            boneOut.id = 3;
            boneOut.parentId = 2;
            boneOut.origin = bone->origin;

            if (bone->children.size())
            {
                boneOut.linkedCount = 2;
                boneOut.linked = {2, 4};
            }
            else
            {
                boneOut.linkedCount = 1;
                boneOut.linked = {2};
            }
        }

        boneCount = bones.size();

        names.reserve(nameMap.size());
        nameCount = nameMap.size();
        for (const auto& name : nameMap)
        {
            names.emplace_back();
            Name& nameOut = names.back();
            nameOut.name = name.first;
            nameOut.boneId = name.second;
        }

        boneIdCount = boneCount;
        boneIds.reserve(boneIdCount);
        for (auto it=bones.crbegin() ; it != bones.crend() ; ++it)
            boneIds.push_back(it->id);
    }
};

}
}

#endif // _DNAMP1_CINF_HPP_
