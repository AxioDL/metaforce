#ifndef __COMMON_RIGINVERTER_HPP__
#define __COMMON_RIGINVERTER_HPP__

#include "zeus/CVector3f.hpp"
#include "zeus/CMatrix3f.hpp"
#include "zeus/CQuaternion.hpp"
#include "hecl/hecl.hpp"

namespace DataSpec::DNAANIM
{

/** One-shot process to invert CINF armature into connected rig,
 *  inverting rotations/translations of ANIM data to match */
template <class CINFType>
class RigInverter
{
public:
    struct Bone
    {
        const typename CINFType::Bone& m_origBone;
        zeus::CMatrix3f m_inverter;
        zeus::CMatrix3f m_restorer;
        zeus::CVector3f m_tail;
        zeus::CVector3f m_parentDelta;
        Bone(const CINFType& cinf, const typename CINFType::Bone& origBone);
    };
private:
    const CINFType& m_cinf;
    std::vector<Bone> m_bones;
public:
    RigInverter(const CINFType& cinf);
    RigInverter(const CINFType& cinf,
                const std::unordered_map<std::string, hecl::blender::Matrix3f>& matrices);
    const CINFType& getCINF() const {return m_cinf;}
    const std::vector<Bone>& getBones() const {return m_bones;}

    zeus::CQuaternion invertRotation(atUint32 boneId, const zeus::CQuaternion& origRot) const;
    zeus::CVector3f invertPosition(atUint32 boneId, const zeus::CVector3f& origPos, bool subDelta) const;

    zeus::CQuaternion restoreRotation(atUint32 boneId, const zeus::CQuaternion& origRot) const;
    zeus::CVector3f restorePosition(atUint32 boneId, const zeus::CVector3f& origPos, bool subDelta) const;
};

}

#endif // __COMMON_RIGINVERTER_HPP__
