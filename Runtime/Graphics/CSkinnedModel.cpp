#include "CSkinnedModel.hpp"

namespace urde
{

CSkinnedModel::CSkinnedModel(const TLockedToken<CModel>& model,
                             const TLockedToken<CSkinRules>& skinRules,
                             const TLockedToken<CCharLayoutInfo>& layoutInfo)
{

}

void CSkinnedModel::Calculate(const CPoseAsTransforms& pose,
                              const std::experimental::optional<CVertexMorphEffect>&)
{
}

}
