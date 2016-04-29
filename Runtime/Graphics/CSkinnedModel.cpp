#include "CSkinnedModel.hpp"

namespace urde
{

CSkinnedModel::CSkinnedModel(const TLockedToken<CModel>& model,
                             const TLockedToken<CSkinRules>& skinRules,
                             const TLockedToken<CCharLayoutInfo>& layoutInfo)
: x4_model(model), x10_skinRules(skinRules), x1c_layoutInfo(layoutInfo)
{

}

CSkinnedModel::CSkinnedModel(IObjectStore& store, ResId model,
                             ResId skinRules, ResId layoutInfo,
                             EDataOwnership ownership)
{
}

void CSkinnedModel::Calculate(const CPoseAsTransforms& pose,
                              const rstl::optional_object<CVertexMorphEffect>&)
{
}

CMorphableSkinnedModel::CMorphableSkinnedModel(IObjectStore& store, ResId model,
                                               ResId skinRules, ResId layoutInfo,
                                               EDataOwnership ownership)
: CSkinnedModel(store, model, skinRules, layoutInfo, ownership)
{
}

}
