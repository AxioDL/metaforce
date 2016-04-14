#include "CSkinnedModel.hpp"

namespace urde
{

CSkinnedModel::CSkinnedModel(const TLockedToken<CModel>& model,
                             const TLockedToken<CSkinRules>& skinRules,
                             const TLockedToken<CCharLayoutInfo>& layoutInfo)
: x4_model(model), x10_skinRules(skinRules), x1c_layoutInfo(layoutInfo)
{

}

void CSkinnedModel::Calculate(const CPoseAsTransforms& pose,
                              const std::experimental::optional<CVertexMorphEffect>&)
{
}

}
