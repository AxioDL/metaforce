#include "CFluidPlaneDoor.hpp"
#include "CFluidPlaneCPU.hpp"
#include "CStateManager.hpp"

namespace urde
{

CFluidPlaneDoor::CFluidPlaneDoor(CAssetId patternTex1, CAssetId patternTex2, CAssetId colorTex, float tileSize,
                                 u32 tileSubdivisions, EFluidType fluidType, float alpha,
                                 const CFluidUVMotion& uvMotion)
: CFluidPlane(patternTex1, patternTex2, colorTex, alpha, fluidType, 0.5f, uvMotion), xa0_tileSize(tileSize),
  xa4_tileSubdivisions(tileSubdivisions & ~0x1), xa8_rippleResolution(xa0_tileSize / float(xa4_tileSubdivisions))
{}

CFluidPlaneShader::RenderSetupInfo
CFluidPlaneDoor::RenderSetup(const CStateManager& mgr, float alpha, const zeus::CTransform& xf,
                             const zeus::CAABox& aabb, bool noNormals) const
{
    CFluidPlaneShader::RenderSetupInfo out;

    float uvT = mgr.GetFluidPlaneManager()->GetUVT();
    CGraphics::SetModelMatrix(xf);

    float fluidUVs[3][2];
    x4c_uvMotion.CalculateFluidTextureOffset(uvT, fluidUVs);

    out.texMtxs[0][0][0] = x4c_uvMotion.GetFluidLayers()[1].GetUVScale();
    out.texMtxs[0][1][1] = x4c_uvMotion.GetFluidLayers()[1].GetUVScale();
    out.texMtxs[0][3][0] = fluidUVs[1][0];
    out.texMtxs[0][3][1] = fluidUVs[1][1];

    out.texMtxs[1][0][0] = x4c_uvMotion.GetFluidLayers()[2].GetUVScale();
    out.texMtxs[1][1][1] = x4c_uvMotion.GetFluidLayers()[2].GetUVScale();
    out.texMtxs[1][3][0] = fluidUVs[2][0];
    out.texMtxs[1][3][1] = fluidUVs[2][1];

    out.texMtxs[2][0][0] = x4c_uvMotion.GetFluidLayers()[0].GetUVScale();
    out.texMtxs[2][1][1] = x4c_uvMotion.GetFluidLayers()[0].GetUVScale();
    out.texMtxs[2][3][0] = fluidUVs[0][0];
    out.texMtxs[2][3][1] = fluidUVs[0][1];

    out.kColors[0] = zeus::CColor(1.f, alpha);

    if (!m_shader)
    {
        auto gridDimX = u32((xa0_tileSize + aabb.max.x() - aabb.min.x() - 0.01f) / xa0_tileSize);
        auto gridDimY = u32((xa0_tileSize + aabb.max.y() - aabb.min.y() - 0.01f) / xa0_tileSize);
        u32 gridCellCount = (gridDimX + 1) * (gridDimY + 1);
        u32 maxVerts = gridCellCount * ((std::max(2, xa4_tileSubdivisions) * 4 + 2) * 4);
        m_shader.emplace(x10_texPattern1, x20_texPattern2, x30_texColor, maxVerts);
    }

    return out;
}

/* Used to be part of locked cache
 * These are too big for stack allocation */
static CFluidPlaneRender::SHFieldSample lc_heights[46][46] = {};
static u8 lc_flags[9][9] = {};

void CFluidPlaneDoor::Render(const CStateManager& mgr, float alpha, const zeus::CAABox& aabb, const zeus::CTransform& xf,
                             const zeus::CTransform& areaXf, bool noNormals, const zeus::CFrustum& frustum,
                             const std::experimental::optional<CRippleManager>& rippleManager, TUniqueId waterId,
                             const bool* gridFlags, u32 gridDimX, u32 gridDimY, const zeus::CVector3f& areaCenter) const
{
    CFluidPlaneShader::RenderSetupInfo setupInfo = RenderSetup(mgr, alpha, xf, aabb, noNormals);
    CFluidPlaneRender::numSubdivisionsInTile = xa4_tileSubdivisions;
    CFluidPlaneRender::numTilesInHField = 42 / xa4_tileSubdivisions;
    CFluidPlaneRender::numSubdivisionsInHField = CFluidPlaneRender::numTilesInHField * xa4_tileSubdivisions;
    zeus::CVector2f centerPlane(aabb.center().x(), aabb.center().y());
    float patchSize = xa8_rippleResolution * CFluidPlaneRender::numSubdivisionsInHField;
    float ooSubdivSize = 1.f / xa8_rippleResolution;

    m_verts.clear();
    m_pVerts.clear();
    m_shader->prepareDraw(setupInfo);

    for (float curX = aabb.min.x() ; curX < aabb.max.x() ; curX += patchSize)
    {
        float remSubdivsX = (aabb.max.x() - curX) * ooSubdivSize;
        for (float curY = aabb.min.y() ; curY < aabb.max.y() ; curY += patchSize)
        {
            float remSubdivsY = (aabb.max.y() - curY) * ooSubdivSize;
            int remSubdivsXi = std::min(CFluidPlaneRender::numSubdivisionsInHField, int(remSubdivsX));
            int remSubdivsYi = std::min(CFluidPlaneRender::numSubdivisionsInHField, int(remSubdivsY));
            zeus::CAABox aabb2(aabb.min, zeus::CVector3f(xa8_rippleResolution * remSubdivsXi + curX,
                                                         xa8_rippleResolution * remSubdivsYi + curY,
                                                         aabb.max.z()));
            if (frustum.aabbFrustumTest(aabb2.getTransformedAABox(xf)))
            {
                CFluidPlaneRender::SPatchInfo patchInfo(zeus::CVector3f(curX, curY, aabb.min.z()),
                                                        aabb2.max, xf.origin, xa8_rippleResolution,
                                                        xa0_tileSize, 0.f,
                                                        CFluidPlaneRender::numSubdivisionsInHField,
                                                        CFluidPlaneRender::NormalMode::None,
                                                        0, 0, 0, 0, 0, 0, 0, nullptr);

                RenderPatch(patchInfo, lc_heights, lc_flags, true, true, m_verts, m_pVerts);
            }
        }
    }

    m_shader->loadVerts(m_verts, m_pVerts);
    m_shader->doneDrawing();
}

}