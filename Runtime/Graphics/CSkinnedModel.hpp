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
public:
    CSkinnedModel(const TLockedToken<CModel>& model,
                  const TLockedToken<CSkinRules>& skinRules,
                  const TLockedToken<CCharLayoutInfo>& layoutInfo);
    CSkinnedModel(IObjectStore& store, TResId model, TResId skinRules, TResId layoutInfo);

    void Calculate(const CPoseAsTransforms& pose, const std::experimental::optional<CVertexMorphEffect>&);
};

}

#endif // __URDE_CSKINNEDMODEL_HPP__
