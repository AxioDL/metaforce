#pragma once

#include "boo/graphicsdev/IGraphicsDataFactory.hpp"
#include "RetroTypes.hpp"
#include "CSkinBank.hpp"
#include "CFactoryMgr.hpp"

namespace urde
{
class CPoseAsTransforms;
class CModel;

struct SSkinWeighting
{
    CSegId m_id;
    float m_weight;
    explicit SSkinWeighting(CInputStream& in)
    : m_id(in), m_weight(in.readFloatBig()) {}
};

class CVirtualBone
{
    std::vector<SSkinWeighting> m_weights;
public:
    explicit CVirtualBone(CInputStream& in)
    {
        u32 weightCount = in.readUint32Big();
        m_weights.reserve(weightCount);
        for (u32 i=0 ; i<weightCount ; ++i)
            m_weights.emplace_back(in);
    }

    const std::vector<SSkinWeighting>& GetWeights() const { return m_weights; }
};

class CSkinRules
{
    std::vector<CSkinBank> x0_skinBanks;
    //u32 x10_vertexCount;
    //u32 x14_normalCount;
    std::vector<CVirtualBone> m_virtualBones;
    std::vector<u32> m_poolToSkinIdx;

public:
    explicit CSkinRules(CInputStream& in);

    void GetBankTransforms(std::vector<const zeus::CTransform*>& out,
                           const CPoseAsTransforms& pose, int skinBankIdx) const
    { x0_skinBanks[skinBankIdx].GetBankTransforms(out, pose); }

    void TransformVerticesCPU(std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vnOut,
                              const CPoseAsTransforms& pose, const CModel& model) const;
};

CFactoryFnReturn FSkinRulesFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& params,
                                   CObjectReference* selfRef);

}

