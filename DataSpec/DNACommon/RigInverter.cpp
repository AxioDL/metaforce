#include "DataSpec/DNACommon/RigInverter.hpp"

#include "DataSpec/DNAMP1/CINF.hpp"
#include "DataSpec/DNAMP2/CINF.hpp"
#include "DataSpec/DNAMP3/CINF.hpp"

#include <hecl/Blender/Connection.hpp>

namespace DataSpec::DNAANIM {

template <class CINFType>
RigInverter<CINFType>::Bone::Bone(const CINFType& cinf, const typename CINFType::Bone& origBone)
: m_origBone(origBone) {
  atUint32 parentIdx = cinf.getInternalBoneIdxFromId(origBone.parentId);
  zeus::CVector3f boneOrigin(origBone.origin);
  zeus::CVector3f naturalTail = boneOrigin + zeus::CVector3f{0.f, 0.5f, 0.f};
  if (parentIdx != UINT32_MAX) {
    const typename CINFType::Bone& pBone = cinf.bones[parentIdx];
    m_parentDelta = boneOrigin - zeus::CVector3f(pBone.origin);
  }

  size_t actualChildren = 0;
  for (atUint32 chId : origBone.linked) {
    if (chId == origBone.parentId)
      continue;
    atUint32 chIdx = cinf.getInternalBoneIdxFromId(chId);
    if (chIdx != UINT32_MAX)
      ++actualChildren;
  }

  const std::string* bName = cinf.getBoneNameFromId(origBone.id);
  bool isLCTR = false;
  if (bName)
    isLCTR = bName->find("_LCTR") != std::string::npos;

  if (parentIdx == UINT32_MAX) {
    /* Root will always use +Y tail */
    m_tail = naturalTail;
  } else if (actualChildren) {
    /* Position tail to average of children */
    for (atUint32 chId : origBone.linked) {
      if (chId == origBone.parentId)
        continue;
      atUint32 chIdx = cinf.getInternalBoneIdxFromId(chId);
      if (chIdx != UINT32_MAX) {
        const typename CINFType::Bone& chBone = cinf.bones[chIdx];
        m_tail += chBone.origin;
      }
    }
    m_tail /= zeus::CVector3f(float(actualChildren));
    if ((m_tail - boneOrigin).magSquared() < 0.001f)
      m_tail = naturalTail;
    else if (isLCTR)
      m_tail = boneOrigin + zeus::CVector3f{0.f, 1.0f, 0.f} * (m_tail - boneOrigin).magnitude();
  } else {
    /* Extrapolate by delta with parent */
    m_tail = boneOrigin + m_parentDelta;
    float deltaMag = m_parentDelta.magnitude();
    if (deltaMag < 0.001f) {
      deltaMag = 0.5f;
      m_tail = naturalTail;
    } else if (deltaMag > 0.5f) {
      /* Extreme bones capped to +0.5 value */
      deltaMag = 0.5f;
      m_tail = boneOrigin + m_parentDelta.normalized() * 0.5f;
    }

    if (isLCTR)
      m_tail = boneOrigin + zeus::CVector3f{0.f, 1.0f, 0.f} * deltaMag;
  }
}

template <class CINFType>
RigInverter<CINFType>::RigInverter(const CINFType& cinf) : m_cinf(cinf) {
  m_bones.reserve(cinf.bones.size());
  for (const typename CINFType::Bone& b : cinf.bones)
    m_bones.emplace_back(cinf, b);
}

template <class CINFType>
RigInverter<CINFType>::RigInverter(const CINFType& cinf,
                                   const std::unordered_map<std::string, hecl::blender::Matrix3f>& matrices)
: m_cinf(cinf) {
  m_bones.reserve(cinf.bones.size());
  for (const typename CINFType::Bone& b : cinf.bones) {
    m_bones.emplace_back(cinf, b);

    const std::string* name = cinf.getBoneNameFromId(b.id);
    if (name) {
      auto search = matrices.find(*name);
      if (search != matrices.cend()) {
        zeus::CMatrix3f boneMtx(search->second[0], search->second[1], search->second[2]);
        m_bones.back().m_restorer = boneMtx;
        m_bones.back().m_inverter = m_bones.back().m_restorer.inverse();
      }
    }
  }
}

template <class CINFType>
zeus::CQuaternion RigInverter<CINFType>::invertRotation(atUint32 boneId, const zeus::CQuaternion& origRot) const {
  for (const Bone& b : m_bones)
    if (b.m_origBone.id == boneId)
      return b.m_restorer * origRot * b.m_inverter;
  return origRot;
}

template <class CINFType>
zeus::CVector3f RigInverter<CINFType>::invertPosition(atUint32 boneId, const zeus::CVector3f& origPos,
                                                      bool subDelta) const {
  for (const Bone& b : m_bones)
    if (b.m_origBone.id == boneId) {
      zeus::CVector3f localPos = origPos;
      if (subDelta)
        localPos -= b.m_parentDelta;
      return b.m_restorer.transform(localPos);
    }
  return origPos;
}

template <class CINFType>
zeus::CQuaternion RigInverter<CINFType>::restoreRotation(atUint32 boneId, const zeus::CQuaternion& origRot) const {
  for (const Bone& b : m_bones)
    if (b.m_origBone.id == boneId)
      return b.m_inverter * origRot * b.m_restorer;
  return origRot;
}

template <class CINFType>
zeus::CVector3f RigInverter<CINFType>::restorePosition(atUint32 boneId, const zeus::CVector3f& origPos,
                                                       bool subDelta) const {
  for (const Bone& b : m_bones)
    if (b.m_origBone.id == boneId) {
      zeus::CVector3f localPos = b.m_inverter.transform(origPos);
      if (subDelta)
        localPos += b.m_parentDelta;
      return localPos;
    }
  return origPos;
}

template class RigInverter<DNAMP1::CINF>;
template class RigInverter<DNAMP2::CINF>;
template class RigInverter<DNAMP3::CINF>;

} // namespace DataSpec::DNAANIM
