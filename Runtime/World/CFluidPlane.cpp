#include <Runtime/GameGlobalObjects.hpp>
#include "CFluidPlane.hpp"
#include "CSimplePool.hpp"
#include "CRipple.hpp"
#include "CScriptWater.hpp"
#include "CStateManager.hpp"

namespace urde
{

CFluidPlane::CFluidPlane(CAssetId texPattern1, CAssetId texPattern2, CAssetId texColor, float alpha,
                         EFluidType fluidType, float rippleIntensity, const CFluidUVMotion& motion)
: x4_texPattern1Id(texPattern1), x8_texPattern2Id(texPattern2), xc_texColorId(texColor), x40_alpha(alpha),
  x44_fluidType(fluidType), x48_rippleIntensity(rippleIntensity), x4c_uvMotion(motion)
{
    if (g_ResFactory->GetResourceTypeById(texPattern1) == FOURCC('TXTR'))
        x10_texPattern1 = g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), texPattern1});
    if (g_ResFactory->GetResourceTypeById(texPattern2) == FOURCC('TXTR'))
        x20_texPattern2 = g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), texPattern2});
    if (g_ResFactory->GetResourceTypeById(texColor) == FOURCC('TXTR'))
        x30_texColor = g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), texColor});
}

float CFluidPlane::ProjectRippleVelocity(float baseI, float velDot) const
{
    float tmp = 0.5f * baseI * velDot * velDot;
    if (tmp != 0.f)
        tmp = std::sqrt(tmp);
    if (tmp >= 160.f)
        return 1.f;
    return tmp / 160.f;
}

float CFluidPlane::CalculateRippleIntensity(float baseI) const
{
    float mul;
    switch (x44_fluidType)
    {
    case EFluidType::NormalWater:
        mul = g_tweakGame->GetRippleIntensityNormal();
        break;
    case EFluidType::PoisonWater:
        mul = g_tweakGame->GetRippleIntensityPoison();
        break;
    case EFluidType::Lava:
        mul = g_tweakGame->GetRippleIntensityLava();
        break;
    case EFluidType::PhazonFluid:
    case EFluidType::Four:
        mul = 0.8f;
        break;
    case EFluidType::ThickLava:
        mul = 1.f;
        break;
    }

    return zeus::clamp(0.f, baseI * mul * (1.f - x48_rippleIntensity + 0.5f), 1.f);
}

void CFluidPlane::AddRipple(float mag, TUniqueId rippler, const zeus::CVector3f& center,
                            CScriptWater& water, CStateManager& mgr)
{
    if (!water.CanRippleAtPoint(center))
        return;

    mag = CalculateRippleIntensity(mag);
    mgr.GetFluidPlaneManager()->RippleManager().AddRipple(CRipple(rippler, center, mag));
}

void CFluidPlane::AddRipple(float intensity, TUniqueId rippler, const zeus::CVector3f& center,
                            const zeus::CVector3f& velocity, const CScriptWater& water, CStateManager& mgr,
                            const zeus::CVector3f& upVec)
{
    if (!water.CanRippleAtPoint(center))
        return;

    intensity = CalculateRippleIntensity(ProjectRippleVelocity(intensity, upVec.dot(velocity)));
    mgr.GetFluidPlaneManager()->RippleManager().AddRipple(CRipple(rippler, center, intensity));
}

void CFluidPlane::AddRipple(const CRipple& ripple, const CScriptWater& water, CStateManager& mgr)
{
    if (!water.CanRippleAtPoint(ripple.GetCenter()))
        return;
    mgr.GetFluidPlaneManager()->RippleManager().AddRipple(ripple);
}

void CFluidPlane::RenderStripWithRipples(float curY,
                                         const CFluidPlaneRender::SHFieldSample (&heights)[46][46],
                                         const u8 (&flags)[9][9], int startYDiv,
                                         const CFluidPlaneRender::SPatchInfo& info,
                                         std::vector<CFluidPlaneShader::Vertex>& vOut,
                                         std::vector<CFluidPlaneShader::PatchVertex>& pvOut) const
{
    m_shader->bindRegular();

    int yTile = (startYDiv + CFluidPlaneRender::numSubdivisionsInTile - 1) /
                CFluidPlaneRender::numSubdivisionsInTile;
    int endXTile = (info.x0_xSubdivs + CFluidPlaneRender::numSubdivisionsInTile - 4) /
                   CFluidPlaneRender::numSubdivisionsInTile;

    int midDiv = CFluidPlaneRender::numSubdivisionsInTile / 2;
    float tileMid = info.x18_rippleResolution * midDiv;
    float yMin = curY;
    float yMid = curY + tileMid;

    float curX = info.x4_localMin.x();
    int gridCell = info.x28_tileX + info.x2a_gridDimX * (info.x2e_tileY + yTile - 1);
    int xTile = 1;
    int tileSpan;
    for (int i = 1 ; i < info.x0_xSubdivs - 2 ;
         i += CFluidPlaneRender::numSubdivisionsInTile * tileSpan, gridCell += tileSpan,
         xTile += tileSpan, curX += info.x14_tileSize * tileSpan)
    {
        tileSpan = 1;
        if (info.x30_gridFlags && !info.x30_gridFlags[gridCell])
            continue;

        if ((flags[yTile][xTile] & 0x1f) == 0x1f)
        {
            for (; xTile+tileSpan<=endXTile ; ++tileSpan)
            {
                if ((flags[yTile][xTile+tileSpan] & 0x1f) != 0x1f)
                    break;
                if (info.x30_gridFlags && !info.x30_gridFlags[gridCell+tileSpan])
                    break;
            }

            int stripDivCount = tileSpan * CFluidPlaneRender::numSubdivisionsInTile + 1;
            int remSubdivs = CFluidPlaneRender::numSubdivisionsInTile;
            std::function<void(float x, float y, const CFluidPlaneRender::SHFieldSample& samp)> func;

            switch (info.x37_normalMode)
            {
            case CFluidPlaneRender::NormalMode::None:
                func = [&](float x, float y, const CFluidPlaneRender::SHFieldSample& samp)
                {
                    vOut.emplace_back(zeus::CVector3f(x, y, samp.height));
                };
                break;
            case CFluidPlaneRender::NormalMode::NoNormals:
                func = [&](float x, float y, const CFluidPlaneRender::SHFieldSample& samp)
                {
                    vOut.emplace_back(zeus::CVector3f(x, y, samp.height), samp.MakeColor(info));
                };
                break;
            case CFluidPlaneRender::NormalMode::Normals:
                func = [&](float x, float y, const CFluidPlaneRender::SHFieldSample& samp)
                {
                    vOut.emplace_back(zeus::CVector3f(x, y, samp.height), samp.MakeNormal(),
                                      samp.MakeColor(info));
                };
                break;
            case CFluidPlaneRender::NormalMode::NBT:
                func = [&](float x, float y, const CFluidPlaneRender::SHFieldSample& samp)
                {
                    vOut.emplace_back(zeus::CVector3f(x, y, samp.height), samp.MakeNormal(),
                                      samp.MakeBinormal(), samp.MakeTangent(), samp.MakeColor(info));
                };
                break;
            }

            float curTileY = yMin;
            int curYDiv = startYDiv;
            for (; remSubdivs>0 ; --remSubdivs, ++curYDiv, curTileY+=info.x18_rippleResolution)
            {
                size_t start = vOut.size();
                float curTileX = curX;
                for (int v=0 ; v<stripDivCount ; ++v)
                {
                    func(curTileX, curTileY, heights[curYDiv][i+v]);
                    func(curTileX, curTileY + info.x18_rippleResolution, heights[curYDiv+1][i+v]);
                    curTileX += info.x18_rippleResolution;
                }
                CGraphics::DrawArray(start, vOut.size() - start);
            }
        }
        else
        {
            bool r19 = (flags[yTile+1][xTile] & 0x2) != 0; // North
            bool r16 = (flags[yTile][xTile-1] & 0x8) != 0; // West
            bool r18 = (flags[yTile][xTile+1] & 0x4) != 0; // East
            bool r17 = (flags[yTile-1][xTile] & 0x1) != 0; // South

            int r6 = (r19 ? CFluidPlaneRender::numSubdivisionsInTile : 1) + 2;
            r6 += r18 ? CFluidPlaneRender::numSubdivisionsInTile : 1;
            r6 += r17 ? CFluidPlaneRender::numSubdivisionsInTile : 1;
            r6 += r16 ? CFluidPlaneRender::numSubdivisionsInTile : 1;

            if (r6 == 6 && (info.x37_normalMode == CFluidPlaneRender::NormalMode::Normals ||
                            info.x37_normalMode == CFluidPlaneRender::NormalMode::NBT))
            {
                for (; xTile+tileSpan<=endXTile ; ++tileSpan)
                {
                    if ((flags[yTile][xTile+tileSpan] & 0x1f) == 0x1f)
                        break;
                    if (info.x30_gridFlags && !info.x30_gridFlags[gridCell+tileSpan])
                        break;
                    if ((flags[yTile+1][xTile+tileSpan] & 0x2) != 0x0)
                        break;
                    if ((flags[yTile][xTile+tileSpan+1] & 0x4) != 0x0)
                        break;
                    if ((flags[yTile-1][xTile+tileSpan] & 0x1) != 0x0)
                        break;
                }

                int stripDivCount = tileSpan + 1;
                size_t start = vOut.size();
                switch (info.x37_normalMode)
                {
                case CFluidPlaneRender::NormalMode::Normals:
                {
                    int curYDiv0 = startYDiv;
                    int curYDiv1 = startYDiv + CFluidPlaneRender::numSubdivisionsInTile;
                    float curTileX = curX;
                    for (int v=0 ; v<stripDivCount ; ++v)
                    {
                        int curXDiv = v * CFluidPlaneRender::numSubdivisionsInTile + i;
                        const CFluidPlaneRender::SHFieldSample& samp0 = heights[curYDiv0][curXDiv];
                        const CFluidPlaneRender::SHFieldSample& samp1 = heights[curYDiv1][curXDiv];
                        vOut.emplace_back(zeus::CVector3f(curTileX, yMin, samp0.height),
                                          samp0.MakeNormal(), samp0.MakeColor(info));
                        vOut.emplace_back(zeus::CVector3f(curTileX, yMin + info.x14_tileSize, samp1.height),
                                          samp1.MakeNormal(), samp1.MakeColor(info));
                        curTileX += info.x14_tileSize;
                    }
                    break;
                }
                case CFluidPlaneRender::NormalMode::NBT:
                {
                    int curYDiv0 = startYDiv;
                    int curYDiv1 = startYDiv + CFluidPlaneRender::numSubdivisionsInTile;
                    float curTileX = curX;
                    for (int v=0 ; v<stripDivCount ; ++v)
                    {
                        int curXDiv = v * CFluidPlaneRender::numSubdivisionsInTile + i;
                        const CFluidPlaneRender::SHFieldSample& samp0 = heights[curYDiv0][curXDiv];
                        const CFluidPlaneRender::SHFieldSample& samp1 = heights[curYDiv1][curXDiv];
                        vOut.emplace_back(zeus::CVector3f(curTileX, yMin, samp0.height),
                                          samp0.MakeNormal(), samp0.MakeBinormal(), samp0.MakeTangent(),
                                          samp0.MakeColor(info));
                        vOut.emplace_back(zeus::CVector3f(curTileX, yMin + info.x14_tileSize, samp1.height),
                                          samp1.MakeNormal(), samp1.MakeBinormal(), samp1.MakeTangent(),
                                          samp1.MakeColor(info));
                        curTileX += info.x14_tileSize;
                    }
                    break;
                }
                default:
                    break;
                }
                CGraphics::DrawArray(start, vOut.size() - start);
            }
            else
            {
                TriFanToStrip<CFluidPlaneShader::Vertex> toStrip(vOut);
                std::function<void(float x, float y, const CFluidPlaneRender::SHFieldSample& samp)> func;

                switch (info.x37_normalMode)
                {
                case CFluidPlaneRender::NormalMode::None:
                    func = [&](float x, float y, const CFluidPlaneRender::SHFieldSample& samp)
                    {
                        toStrip.EmplaceVert(zeus::CVector3f(x, y, samp.height));
                    };
                    break;
                case CFluidPlaneRender::NormalMode::NoNormals:
                    func = [&](float x, float y, const CFluidPlaneRender::SHFieldSample& samp)
                    {
                        toStrip.EmplaceVert(zeus::CVector3f(x, y, samp.height), samp.MakeColor(info));
                    };
                    break;
                case CFluidPlaneRender::NormalMode::Normals:
                    func = [&](float x, float y, const CFluidPlaneRender::SHFieldSample& samp)
                    {
                        toStrip.EmplaceVert(zeus::CVector3f(x, y, samp.height), samp.MakeNormal(),
                                            samp.MakeColor(info));
                    };
                    break;
                case CFluidPlaneRender::NormalMode::NBT:
                    func = [&](float x, float y, const CFluidPlaneRender::SHFieldSample& samp)
                    {
                        toStrip.EmplaceVert(zeus::CVector3f(x, y, samp.height), samp.MakeNormal(),
                                            samp.MakeBinormal(), samp.MakeTangent(), samp.MakeColor(info));
                    };
                    break;
                }

                func(tileMid + curX, yMid, heights[startYDiv+midDiv][i+midDiv]);

                int curXDiv = i;
                int curYDiv = startYDiv + CFluidPlaneRender::numSubdivisionsInTile;
                float curTileX = curX;
                float curTileY = yMin + info.x14_tileSize;
                for (int v=0 ; v<(r19 ? CFluidPlaneRender::numSubdivisionsInTile : 1) ; ++v)
                {
                    const CFluidPlaneRender::SHFieldSample& samp = heights[curYDiv][curXDiv+v];
                    func(curTileX, curTileY, samp);
                    curTileX += info.x18_rippleResolution;
                }

                curXDiv = i + CFluidPlaneRender::numSubdivisionsInTile;
                curYDiv = startYDiv + CFluidPlaneRender::numSubdivisionsInTile;
                curTileX = curX + info.x14_tileSize;
                curTileY = yMin + info.x14_tileSize;
                for (int v=0 ; v<(r18 ? CFluidPlaneRender::numSubdivisionsInTile : 1) ; ++v)
                {
                    const CFluidPlaneRender::SHFieldSample& samp = heights[curYDiv-v][curXDiv];
                    func(curTileX, curTileY, samp);
                    curTileY -= info.x18_rippleResolution;
                }

                curXDiv = i + CFluidPlaneRender::numSubdivisionsInTile;
                curYDiv = startYDiv;
                curTileX = curX + info.x14_tileSize;
                curTileY = yMin;
                for (int v=0 ; v<(r17 ? CFluidPlaneRender::numSubdivisionsInTile : 1) ; ++v)
                {
                    const CFluidPlaneRender::SHFieldSample& samp = heights[curYDiv][curXDiv-v];
                    func(curTileX, curTileY, samp);
                    curTileX -= info.x18_rippleResolution;
                }

                curXDiv = i;
                curYDiv = startYDiv;
                curTileX = curX;
                curTileY = yMin;
                if (r16)
                {
                    for (int v=0 ; v<CFluidPlaneRender::numSubdivisionsInTile+1 ; ++v)
                    {
                        const CFluidPlaneRender::SHFieldSample& samp = heights[curYDiv+v][curXDiv];
                        func(curTileX, curTileY, samp);
                        curTileY += info.x18_rippleResolution;
                    }
                }
                else
                {
                    {
                        const CFluidPlaneRender::SHFieldSample& samp = heights[curYDiv][curXDiv];
                        func(curTileX, curTileY, samp);
                    }
                    curTileY += info.x14_tileSize;
                    {
                        const CFluidPlaneRender::SHFieldSample& samp =
                            heights[curYDiv+CFluidPlaneRender::numSubdivisionsInTile][curXDiv];
                        func(curTileX, curTileY, samp);
                    }
                }

                toStrip.Draw();
            }
        }
    }
}

void CFluidPlane::RenderPatch(const CFluidPlaneRender::SPatchInfo& info,
                              const CFluidPlaneRender::SHFieldSample (&heights)[46][46],
                              const u8 (&flags)[9][9], bool noRipples, bool flagIs1,
                              std::vector<CFluidPlaneShader::Vertex>& vOut,
                              std::vector<CFluidPlaneShader::PatchVertex>& pvOut) const
{
    if (noRipples)
    {
        m_shader->bindRegular();

        float xMin = info.x4_localMin.x();
        float yMin = info.x4_localMin.y();
        float xMax = info.x18_rippleResolution * (info.x0_xSubdivs - 2) + xMin;
        float yMax = info.x18_rippleResolution * (info.x1_ySubdivs - 2) + yMin;

        switch (info.x37_normalMode)
        {
        case CFluidPlaneRender::NormalMode::None:
        {
            size_t start = vOut.size();
            vOut.emplace_back(zeus::CVector3f(xMin, yMin, 0.f));
            vOut.emplace_back(zeus::CVector3f(xMin, yMax, 0.f));
            vOut.emplace_back(zeus::CVector3f(xMax, yMin, 0.f));
            vOut.emplace_back(zeus::CVector3f(xMax, yMax, 0.f));
            CGraphics::DrawArray(start, 4);
            break;
        }
        case CFluidPlaneRender::NormalMode::NoNormals:
        {
            size_t start = vOut.size();
            vOut.emplace_back(zeus::CVector3f(xMin, yMin, 0.f), zeus::CColor::skBlack);
            vOut.emplace_back(zeus::CVector3f(xMin, yMax, 0.f), zeus::CColor::skBlack);
            vOut.emplace_back(zeus::CVector3f(xMax, yMin, 0.f), zeus::CColor::skBlack);
            vOut.emplace_back(zeus::CVector3f(xMax, yMax, 0.f), zeus::CColor::skBlack);
            CGraphics::DrawArray(start, 4);
            break;
        }
        case CFluidPlaneRender::NormalMode::Normals:
        {
            int yTiles = (info.x1_ySubdivs - 3) / CFluidPlaneRender::numSubdivisionsInTile + 1;
            int xTiles = (info.x0_xSubdivs - 3) / CFluidPlaneRender::numSubdivisionsInTile + 1;
            int xTileStart = info.x28_tileX + info.x2e_tileY * info.x2a_gridDimX;
            yMax = yMin;
            for (int curYTile=yTiles ; curYTile>0 ; --curYTile,
                yMax += info.x14_tileSize, xTileStart += info.x2a_gridDimX)
            {
                xMax = xMin;
                int nextXTile;
                for (int curXTile=0 ; curXTile<xTiles ; curXTile=nextXTile)
                {
                    if (!info.x30_gridFlags || info.x30_gridFlags[xTileStart+curXTile])
                    {
                        if (curYTile == yTiles || curYTile == 1 || curXTile == 0 || xTiles - 1 == curXTile)
                        {
                            TriFanToStrip<CFluidPlaneShader::Vertex> toStrip(vOut);

                            toStrip.EmplaceVert(zeus::CVector3f(xMax + 0.5f * info.x14_tileSize,
                                                                yMax + 0.5f * info.x14_tileSize, 0.f),
                                                zeus::CVector3f::skUp, zeus::CColor::skBlack);

                            float tmp = xMax;
                            for (int v=0 ; v<((curYTile == 1) ?
                                              CFluidPlaneRender::numSubdivisionsInTile : 1) ; ++v)
                            {
                                toStrip.EmplaceVert(zeus::CVector3f(tmp, yMax + info.x14_tileSize, 0.f),
                                                    zeus::CVector3f::skUp, zeus::CColor::skBlack);
                                tmp += info.x18_rippleResolution;
                            }

                            tmp = yMax + info.x14_tileSize;
                            for (int v=0 ; v<((xTiles - 1 == curXTile) ?
                                              CFluidPlaneRender::numSubdivisionsInTile : 1) ; ++v)
                            {
                                toStrip.EmplaceVert(zeus::CVector3f(xMax + info.x14_tileSize, tmp, 0.f),
                                                    zeus::CVector3f::skUp, zeus::CColor::skBlack);
                                tmp -= info.x18_rippleResolution;
                            }

                            tmp = xMax + info.x14_tileSize;
                            for (int v=0 ; v<((curYTile == yTiles) ?
                                              CFluidPlaneRender::numSubdivisionsInTile : 1) ; ++v)
                            {
                                toStrip.EmplaceVert(zeus::CVector3f(tmp, yMax, 0.f),
                                                    zeus::CVector3f::skUp, zeus::CColor::skBlack);
                                tmp -= info.x18_rippleResolution;
                            }

                            tmp = yMax;
                            for (int v=0 ; v<((curXTile == 0) ?
                                              CFluidPlaneRender::numSubdivisionsInTile : 1) ; ++v)
                            {
                                toStrip.EmplaceVert(zeus::CVector3f(xMax, tmp, 0.f),
                                                    zeus::CVector3f::skUp, zeus::CColor::skBlack);
                                tmp += info.x18_rippleResolution;
                            }

                            toStrip.EmplaceVert(zeus::CVector3f(xMax, yMax + info.x14_tileSize, 0.f),
                                                zeus::CVector3f::skUp, zeus::CColor::skBlack);

                            toStrip.Draw();

                            nextXTile = curXTile + 1;
                            xMax += info.x14_tileSize;
                        }
                        else
                        {
                            nextXTile = curXTile + 1;
                            while (nextXTile < xTiles - 1 &&
                                   (!info.x30_gridFlags || info.x30_gridFlags[xTileStart+nextXTile]))
                                ++nextXTile;

                            size_t start = vOut.size();
                            for (int v = 0 ; v < nextXTile - curXTile + 1 ; ++v)
                            {
                                vOut.emplace_back(zeus::CVector3f(xMax, yMax, 0.f),
                                                  zeus::CVector3f::skUp, zeus::CColor::skBlack);
                                vOut.emplace_back(zeus::CVector3f(xMax, yMax + info.x14_tileSize, 0.f),
                                                  zeus::CVector3f::skUp, zeus::CColor::skBlack);
                                xMax += info.x14_tileSize;
                            }
                            CGraphics::DrawArray(start, vOut.size() - start);

                            ++nextXTile;
                            if (nextXTile == xTiles)
                            {
                                --nextXTile;
                                xMax -= info.x14_tileSize;
                            }
                        }
                    }
                    else
                    {
                        nextXTile = curXTile + 1;
                        xMax += info.x14_tileSize;
                        while (nextXTile < xTiles && !info.x30_gridFlags[xTileStart+nextXTile])
                        {
                            xMax += info.x14_tileSize;
                            ++nextXTile;
                        }
                    }
                }
            }
            break;
        }
        case CFluidPlaneRender::NormalMode::NBT:
        {
            if (flagIs1 || !info.x30_gridFlags)
            {
                size_t start = vOut.size();
                vOut.emplace_back(zeus::CVector3f(xMin, yMin, 0.f), zeus::CVector3f::skUp,
                                  zeus::CVector3f::skForward, zeus::CVector3f::skRight, zeus::CColor::skBlack);
                vOut.emplace_back(zeus::CVector3f(xMin, yMax, 0.f), zeus::CVector3f::skUp,
                                  zeus::CVector3f::skForward, zeus::CVector3f::skRight, zeus::CColor::skBlack);
                vOut.emplace_back(zeus::CVector3f(xMax, yMin, 0.f), zeus::CVector3f::skUp,
                                  zeus::CVector3f::skForward, zeus::CVector3f::skRight, zeus::CColor::skBlack);
                vOut.emplace_back(zeus::CVector3f(xMax, yMax, 0.f), zeus::CVector3f::skUp,
                                  zeus::CVector3f::skForward, zeus::CVector3f::skRight, zeus::CColor::skBlack);
                CGraphics::DrawArray(start, 4);
            }
            else
            {
                int xTiles = (info.x0_xSubdivs - 3) / CFluidPlaneRender::numSubdivisionsInTile + 1;
                int yTiles = (info.x1_ySubdivs - 3) / CFluidPlaneRender::numSubdivisionsInTile + 1;
                int xTileStart = info.x28_tileX + info.x2e_tileY * info.x2a_gridDimX;
                for (; yTiles>0 ; --yTiles, yMin += info.x14_tileSize, xTileStart += info.x2a_gridDimX)
                {
                    xMax = xMin;
                    int nextXTile;
                    for (int curXTile=0 ; curXTile<xTiles ; curXTile=nextXTile)
                    {
                        if (info.x30_gridFlags[xTileStart+curXTile])
                        {
                            nextXTile = curXTile + 1;
                            int tile = xTileStart + nextXTile;
                            while (nextXTile < xTiles && info.x30_gridFlags[tile])
                            {
                                ++nextXTile;
                                ++tile;
                            }

                            size_t start = vOut.size();
                            for (int v = 0 ; v < nextXTile - curXTile + 1 ; ++v)
                            {
                                vOut.emplace_back(zeus::CVector3f(xMax, yMin, 0.f), zeus::CVector3f::skUp,
                                                  zeus::CVector3f::skForward, zeus::CVector3f::skRight,
                                                  zeus::CColor::skBlack);
                                vOut.emplace_back(zeus::CVector3f(xMax, yMin + info.x14_tileSize, 0.f),
                                                  zeus::CVector3f::skUp, zeus::CVector3f::skForward,
                                                  zeus::CVector3f::skRight, zeus::CColor::skBlack);
                                xMax += info.x14_tileSize;
                            }
                            CGraphics::DrawArray(start, vOut.size() - start);
                        }
                        else
                        {
                            nextXTile = curXTile + 1;
                            xMax += info.x14_tileSize;
                            int tile = xTileStart + nextXTile;
                            while (nextXTile < xTiles && !info.x30_gridFlags[tile])
                            {
                                xMax += info.x14_tileSize;
                                ++nextXTile;
                                ++tile;
                            }
                        }
                    }
                }
            }
            break;
        }
        }
    }
    else
    {
        float curY = info.x4_localMin.y();
        for (int startYDiv=1 ; startYDiv<info.x1_ySubdivs-2 ;
             startYDiv += CFluidPlaneRender::numSubdivisionsInTile, curY += info.x14_tileSize)
            RenderStripWithRipples(curY, heights, flags, startYDiv, info, vOut, pvOut);
    }
}

}
