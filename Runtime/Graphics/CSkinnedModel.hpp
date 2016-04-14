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
    std::unique_ptr<CBooModel> m_modelInst;
    TLockedToken<CModel> x4_model;
    TLockedToken<CSkinRules> x10_skinRules;
    TLockedToken<CCharLayoutInfo> x1c_layoutInfo;
public:
    CSkinnedModel(const TLockedToken<CModel>& model,
                  const TLockedToken<CSkinRules>& skinRules,
                  const TLockedToken<CCharLayoutInfo>& layoutInfo);
    CSkinnedModel(IObjectStore& store, ResId model, ResId skinRules, ResId layoutInfo);

    const TLockedToken<CModel>& GetModel() const {return x4_model;}
    const TLockedToken<CSkinRules>& GetSkinRules() const {return x10_skinRules;}
    const TLockedToken<CCharLayoutInfo>& GetLayoutInfo() const {return x1c_layoutInfo;}

    void Calculate(const CPoseAsTransforms& pose, const std::experimental::optional<CVertexMorphEffect>&);
    void Draw(const CModelFlags& drawFlags) const;
};

}

#endif // __URDE_CSKINNEDMODEL_HPP__
