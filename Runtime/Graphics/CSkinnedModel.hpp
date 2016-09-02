#ifndef __URDE_CSKINNEDMODEL_HPP__
#define __URDE_CSKINNEDMODEL_HPP__

#include "CToken.hpp"
#include "CModel.hpp"
#include "optional.hpp"

namespace urde
{
class CModel;
class CSkinRules;
class CCharLayoutInfo;
class CPoseAsTransforms;
class CVertexMorphEffect;
class IObjectStore;

class CSkinnedModel
{
    friend class CBooModel;
    std::unique_ptr<CBooModel> m_modelInst;
    TLockedToken<CModel> x4_model;
    TLockedToken<CSkinRules> x10_skinRules;
    TLockedToken<CCharLayoutInfo> x1c_layoutInfo;
public:
    enum class EDataOwnership
    {
        Zero,
        One
    };
    CSkinnedModel(TLockedToken<CModel> model,
                  TLockedToken<CSkinRules> skinRules,
                  TLockedToken<CCharLayoutInfo> layoutInfo,
                  int shaderIdx);
    CSkinnedModel(IObjectStore& store, ResId model, ResId skinRules,
                  ResId layoutInfo, int shaderIdx);

    TLockedToken<CModel>& GetModel() {return x4_model;}
    std::unique_ptr<CBooModel>& GetModelInst() {return m_modelInst;}
    TLockedToken<CSkinRules>& GetSkinRules() {return x10_skinRules;}
    TLockedToken<CCharLayoutInfo>& GetLayoutInfo() {return x1c_layoutInfo;}

    void Calculate(const CPoseAsTransforms& pose,
                   const CModelFlags& drawFlags,
                   const std::experimental::optional<CVertexMorphEffect>& morphEffect,
                   const float* morphMagnitudes);
    void Draw(const CModelFlags& drawFlags) const;
};

class CMorphableSkinnedModel : public CSkinnedModel
{
public:
    CMorphableSkinnedModel(IObjectStore& store, ResId model, ResId skinRules,
                           ResId layoutInfo, int shaderIdx);
};

}

#endif // __URDE_CSKINNEDMODEL_HPP__
