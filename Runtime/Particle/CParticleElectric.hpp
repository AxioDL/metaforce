#ifndef __URDE_CPARTICLEELECTRIC_HPP__
#define __URDE_CPARTICLEELECTRIC_HPP__

#include "CParticleGen.hpp"
#include "CToken.hpp"
#include "CRandom16.hpp"
#include "Graphics/CLineRenderer.hpp"

namespace urde
{
class CElectricDescription;
class CParticleSwoosh;
class CElementGen;

class CParticleElectric : public CParticleGen
{
    static u16 g_GlobalSeed;
public:
    static void SetGlobalSeed(u16 seed) { g_GlobalSeed = seed; }
    class CLineManager
    {
        friend class CParticleElectric;
        std::vector<zeus::CVector3f> x0_verts;
        float x10_widths[3] = {1.f, 2.f, 3.f};
        zeus::CColor x1c_colors[3];
        zeus::CAABox x28_aabb = zeus::CAABox::skInvertedBox;
    };

    class CParticleElectricManager
    {
        friend class CParticleElectric;
        u32 x0_idx;
        u32 x4_slif;
        u32 x8_startFrame;
        u32 xc_endFrame;
        int x10_gpsmIdx = -1;
        int x14_epsmIdx = -1;
    public:
        CParticleElectricManager(u32 idx, u32 slif, u32 startFrame)
        : x0_idx(idx), x4_slif(slif), x8_startFrame(startFrame), xc_endFrame(startFrame + slif) {}
    };

private:
    TLockedToken<CElectricDescription> x1c_elecDesc;
    int              x28_currentFrame = 0;
    int              x2c_LIFE;
    double           x30_curTime = 0.0;
    zeus::CVector3f  x38_translation;
    zeus::CTransform x44_orientation;
    zeus::CTransform x74_invOrientation;
    zeus::CVector3f  xa4_globalTranslation;
    zeus::CTransform xb0_globalOrientation;
    zeus::CVector3f  xe0_globalScale;
    zeus::CVector3f  xec_localScale;
    zeus::CTransform xf8_cachedXf;
    float            x128 = 0.f;
    float            x12c = 0.f;
    zeus::CAABox     x130_buildBounds = zeus::CAABox::skInvertedBox;
    CRandom16        x14c_randState;
    int              x150_SSEG = 8;
    int              x154_SCNT = 1;
    int              x158 = 0;
    float            x15c_genRem = 0.f;
    zeus::CAABox     x160_systemBounds = zeus::CAABox::skInvertedBox;
    std::experimental::optional<zeus::CVector3f> x178_overrideIPos;
    std::experimental::optional<zeus::CVector3f> x188_overrideIVel;
    std::experimental::optional<zeus::CVector3f> x198_overrideFPos;
    std::experimental::optional<zeus::CVector3f> x1a8_overrideFVel;
    zeus::CColor     x1b8_moduColor;
    rstl::reserved_vector<bool, 32> x1bc_allocated;
    rstl::reserved_vector<std::unique_ptr<CParticleSwoosh>, 32> x1e0_swooshGenerators;
    rstl::reserved_vector<std::unique_ptr<CLineManager>, 32> x2e4_lineManagers;
    std::list<CParticleElectricManager> x3e8_electricManagers;
    std::vector<std::unique_ptr<CElementGen>> x400_gpsmGenerators;
    std::vector<std::unique_ptr<CElementGen>> x410_epsmGenerators;
    std::vector<zeus::CVector3f> x420_calculatedVerts;
    std::vector<float> x430_fractalMags;
    std::vector<zeus::CVector3f> x440_fractalOffsets;

    size_t m_nextLineRenderer = 0;
    std::vector<std::unique_ptr<CLineRenderer>> m_lineRenderers;

    union
    {
        struct
        {
            bool x450_24_emitting : 1; bool x450_25_haveGPSM : 1; bool x450_26_haveEPSM : 1;
            bool x450_27_haveSSWH : 1; bool x450_28_haveLWD: 1; bool x450_29_transformDirty : 1;
        };
        u32 dummy = 0;
    };

    void SetupLineGXMaterial();
    void DrawLineStrip(const std::vector<zeus::CVector3f>& verts, float width, const zeus::CColor& color);
    void RenderLines();
    void RenderSwooshes();
    void UpdateCachedTransform();
    void UpdateLine(int idx, int frame);
    void UpdateElectricalEffects();
    void CalculateFractal(int start, int end, float ampl, float ampd);
    void CalculatePoints();
    void CreateNewParticles(int count);
    void AddElectricalEffects();
    void BuildBounds();

public:
    CParticleElectric(const TToken<CElectricDescription>& desc);

    bool Update(double);
    void Render();
    void SetOrientation(const zeus::CTransform&);
    void SetTranslation(const zeus::CVector3f&);
    void SetGlobalOrientation(const zeus::CTransform&);
    void SetGlobalTranslation(const zeus::CVector3f&);
    void SetGlobalScale(const zeus::CVector3f&);
    void SetLocalScale(const zeus::CVector3f&);
    void SetParticleEmission(bool);
    void SetModulationColor(const zeus::CColor&);
    void SetOverrideIPos(const zeus::CVector3f& vec) { x178_overrideIPos = vec; }
    void SetOverrideIVel(const zeus::CVector3f& vec) { x188_overrideIVel = vec; }
    void SetOverrideFPos(const zeus::CVector3f& vec) { x198_overrideFPos = vec; }
    void SetOverrideFVel(const zeus::CVector3f& vec) { x1a8_overrideFVel = vec; }
    const zeus::CTransform& GetOrientation() const;
    const zeus::CVector3f& GetTranslation() const;
    const zeus::CTransform& GetGlobalOrientation() const;
    const zeus::CVector3f& GetGlobalTranslation() const;
    const zeus::CVector3f& GetGlobalScale() const;
    const zeus::CColor& GetModulationColor() const;
    bool IsSystemDeletable() const;
    rstl::optional_object<zeus::CAABox> GetBounds() const;
    u32 GetParticleCount() const;
    bool SystemHasLight() const;
    CLight GetLight() const;
    bool GetParticleEmission() const;
    void DestroyParticles();
    void Reset() {}
    FourCC Get4CharId() const { return FOURCC('ELSC'); }
};

}

#endif // __URDE_CPARTICLEELECTRIC_HPP__
