#pragma once

#include "RetroTypes.hpp"
#include "CToken.hpp"
#include "Graphics/CTexture.hpp"
#include "CFluidUVMotion.hpp"
#include "zeus/CAABox.hpp"
#include "zeus/CFrustum.hpp"
#include "Graphics/Shaders/CFluidPlaneShader.hpp"

namespace urde
{
class CFluidUVMotion;
class CRippleManager;
class CScriptWater;
class CStateManager;
class CRipple;

class CFluidPlaneRender
{
public:
    enum class NormalMode
    {
        None,
        NoNormals,
        Normals,
        NBT
    };

    static int numTilesInHField;
    static int numSubdivisionsInTile;
    static int numSubdivisionsInHField;

    struct SPatchInfo
    {
        u8 x0_xSubdivs, x1_ySubdivs;
        zeus::CVector2f x4_localMin, xc_globalMin;
        float x14_tileSize;
        float x18_rippleResolution;
        float x1c_tileHypRadius;
        float x20_ooTileSize;
        float x24_ooRippleResolution;
        u16 x28_tileX;
        u16 x2a_gridDimX;
        u16 x2c_gridDimY;
        u16 x2e_tileY;
        const bool* x30_gridFlags;
        u8 x34_redShift;
        u8 x35_greenShift;
        u8 x36_blueShift;
        NormalMode x37_normalMode;
        float x38_wavecapIntensityScale;
    public:
        SPatchInfo(const zeus::CVector3f& localMin, const zeus::CVector3f& localMax, const zeus::CVector3f& pos,
                   float rippleResolution, float tileSize, float wavecapIntensityScale, int numSubdivisionsInHField,
                   NormalMode normalMode, int redShift, int greenShift, int blueShift, u32 tileX, u32 gridDimX,
                   u32 gridDimY, u32 tileY, const bool* gridFlags)
        {
            x0_xSubdivs = std::min(s16((localMax.x() - localMin.x()) / rippleResolution + 1.f - FLT_EPSILON) + 2,
                                   numSubdivisionsInHField + 2);
            x1_ySubdivs = std::min(s16((localMax.y() - localMin.y()) / rippleResolution + 1.f - FLT_EPSILON) + 2,
                                   numSubdivisionsInHField + 2);
            float tileHypRadius = tileSize * tileSize * 2 * 0.25f;
            x4_localMin = localMin.toVec2f();
            xc_globalMin = x4_localMin + pos.toVec2f();
            x14_tileSize = tileSize;
            x18_rippleResolution = rippleResolution;
            if (tileHypRadius != 0.f)
                tileHypRadius = std::sqrt(tileHypRadius);
            x1c_tileHypRadius = tileHypRadius;
            x20_ooTileSize = 1.f / x14_tileSize;
            x24_ooRippleResolution = 1.f / x18_rippleResolution;
            x28_tileX = u16(tileX);
            x2a_gridDimX = u16(gridDimX);
            x2c_gridDimY = u16(gridDimY);
            x2e_tileY = u16(tileY);
            x30_gridFlags = gridFlags;
            x34_redShift = u8(redShift);
            x35_greenShift = u8(greenShift);
            x36_blueShift = u8(blueShift);
            x37_normalMode = normalMode;
            x38_wavecapIntensityScale = wavecapIntensityScale;
        }
    };

    struct SRippleInfo
    {
        const CRipple& x0_ripple;
        int x4_fromX;
        int x8_toX;
        int xc_fromY;
        int x10_toY;
        int x14_gfromX;
        int x18_gtoX;
        int x1c_gfromY;
        int x20_gtoY;
    public:
        SRippleInfo(const CRipple& ripple, int fromX, int toX, int fromY, int toY)
            : x0_ripple(ripple), x14_gfromX(fromX), x18_gtoX(toX), x1c_gfromY(fromY), x20_gtoY(toY) {}
    };

    struct SHFieldSample
    {
        float height;
        s8 nx;
        s8 ny;
        s8 nz;
        u8 wavecapIntensity;

        zeus::CVector3f MakeNormal() const { return zeus::CVector3f{nx / 63.f, ny / 63.f, nz / 63.f}.normalized(); }
        zeus::CVector3f MakeBinormal() const { return zeus::CVector3f{nx / 63.f, nz / 63.f, -ny / 63.f}.normalized(); }
        zeus::CVector3f MakeTangent() const { return zeus::CVector3f{nz / 63.f, ny / 63.f, -nx / 63.f}.normalized(); }
        zeus::CColor MakeColor(const CFluidPlaneRender::SPatchInfo& info) const
        {
            return {(wavecapIntensity >> info.x34_redShift) / 255.f,
                    (wavecapIntensity >> info.x35_greenShift) / 255.f,
                    (wavecapIntensity >> info.x36_blueShift) / 255.f};
        }
    };
};

class CFluidPlane
{
protected:
    CAssetId x4_texPattern1Id;
    CAssetId x8_texPattern2Id;
    CAssetId xc_texColorId;
    TLockedToken<CTexture> x10_texPattern1;
    TLockedToken<CTexture> x20_texPattern2;
    TLockedToken<CTexture> x30_texColor;
    float x40_alpha;
    EFluidType x44_fluidType;
    float x48_rippleIntensity;
    CFluidUVMotion x4c_uvMotion;

    mutable std::vector<CFluidPlaneShader::Vertex> m_verts;
    mutable std::vector<CFluidPlaneShader::PatchVertex> m_pVerts;
    mutable std::experimental::optional<CFluidPlaneShader> m_shader;

    float ProjectRippleVelocity(float baseI, float velDot) const;
    float CalculateRippleIntensity(float baseI) const;

    virtual void RenderStripWithRipples(float curY, const CFluidPlaneRender::SHFieldSample (&heights)[46][46],
                                        const u8 (&flags)[9][9], int startYDiv,
                                        const CFluidPlaneRender::SPatchInfo& info,
                                        std::vector<CFluidPlaneShader::Vertex>& vOut,
                                        std::vector<CFluidPlaneShader::PatchVertex>& pvOut) const;
    void RenderPatch(const CFluidPlaneRender::SPatchInfo& info,
                     const CFluidPlaneRender::SHFieldSample (&heights)[46][46],
                     const u8 (&flags)[9][9], bool noRipples, bool flagIs1,
                     std::vector<CFluidPlaneShader::Vertex>& vOut,
                     std::vector<CFluidPlaneShader::PatchVertex>& pvOut) const;

public:
    virtual ~CFluidPlane() = default;
    CFluidPlane(CAssetId texPattern1, CAssetId texPattern2, CAssetId texColor, float alpha,
                EFluidType fluidType, float rippleIntensity, const CFluidUVMotion& motion);

    // Called by CPlayer, CMorphBall, CWeapon, CPuddleSpore, CMagdolite
    virtual void AddRipple(float mag, TUniqueId rippler, const zeus::CVector3f& center,
                           CScriptWater& water, CStateManager& mgr);

    // Called by CAi
    virtual void AddRipple(float intensity, TUniqueId rippler, const zeus::CVector3f& center,
                           const zeus::CVector3f& velocity, const CScriptWater& water, CStateManager& mgr,
                           const zeus::CVector3f& upVec);

    virtual void AddRipple(const CRipple& ripple, const CScriptWater& water, CStateManager& mgr);

    virtual void Render(const CStateManager& mgr, float alpha, const zeus::CAABox& aabb, const zeus::CTransform& xf,
                        const zeus::CTransform& areaXf, bool noNormals, const zeus::CFrustum& frustum,
                        const std::experimental::optional<CRippleManager>& rippleManager, TUniqueId waterId,
                        const bool* gridFlags, u32 gridDimX, u32 gridDimY, const zeus::CVector3f& areaCenter) const {}

    float GetAlpha() const { return x40_alpha; }
    EFluidType GetFluidType() const { return x44_fluidType; }
    const CFluidUVMotion& GetUVMotion() const { return x4c_uvMotion; }
    const CTexture& GetColorTexture() const { return *x30_texColor; }
    bool HasColorTexture() const { return x30_texColor.operator bool(); }
    const CTexture& GetTexturePattern1() const { return *x10_texPattern1; }
    bool HasTexturePattern1() const { return x10_texPattern1.operator bool(); }
    const CTexture& GetTexturePattern2() const { return *x20_texPattern2; }
    bool HasTexturePattern2() const { return x20_texPattern2.operator bool(); }
};
}

