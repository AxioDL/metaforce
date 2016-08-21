#include "CSkinnedModel.hpp"

namespace urde
{

CSkinnedModel::CSkinnedModel(TLockedToken<CModel> model,
                             TLockedToken<CSkinRules> skinRules,
                             TLockedToken<CCharLayoutInfo> layoutInfo,
                             int shaderIdx)
: x4_model(model), x10_skinRules(skinRules), x1c_layoutInfo(layoutInfo)
{
    m_modelInst = model->MakeNewInstance(shaderIdx);
}

CSkinnedModel::CSkinnedModel(IObjectStore& store, ResId model,
                             ResId skinRules, ResId layoutInfo,
                             int shaderIdx)
: CSkinnedModel(store.GetObj(SObjectTag{FOURCC('CMDL'), model}),
                store.GetObj(SObjectTag{FOURCC('CSKR'), skinRules}),
                store.GetObj(SObjectTag{FOURCC('CINF'), layoutInfo}),
                shaderIdx)
{
}

void CSkinnedModel::Calculate(const CPoseAsTransforms& pose,
                              const CModelFlags& drawFlags,
                              const rstl::optional_object<CVertexMorphEffect>& morphEffect,
                              const float* morphMagnitudes)
{
    m_modelInst->UpdateUniformData(drawFlags, x10_skinRules.GetObj(), &pose);
}

void CSkinnedModel::Draw(const CModelFlags& drawFlags) const
{
    if (m_modelInst->TryLockTextures())
        m_modelInst->DrawSurfaces(drawFlags);
}

CMorphableSkinnedModel::CMorphableSkinnedModel(IObjectStore& store, ResId model,
                                               ResId skinRules, ResId layoutInfo,
                                               int shaderIdx)
: CSkinnedModel(store, model, skinRules, layoutInfo, shaderIdx)
{
}

}
