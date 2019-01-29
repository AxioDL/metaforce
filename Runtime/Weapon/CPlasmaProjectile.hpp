#pragma once

#include "Weapon/CBeamProjectile.hpp"
#include "Weapon/CBeamInfo.hpp"
#include "World/CDamageInfo.hpp"
#include "Graphics/Shaders/CColoredStripShader.hpp"

namespace urde {
class CPlasmaProjectile : public CBeamProjectile {
public:
  struct PlayerEffectResoures : rstl::reserved_vector<u64, 8> {
    PlayerEffectResoures(u64 a = UINT64_MAX, u64 b = UINT64_MAX, u64 c = UINT64_MAX, u64 d = UINT64_MAX,
                         u64 e = UINT64_MAX, u64 f = UINT64_MAX, u64 g = UINT64_MAX, u64 h = UINT64_MAX)
    : rstl::reserved_vector<u64, 8>({a, b, c, d, e, f, g, h}) {}
  };
private:
  std::vector<TUniqueId> x468_lights;
  s32 x478_beamAttributes;
  float x47c_lifeTime;
  float x480_pulseSpeed;
  float x484_shutdownTime;
  float x488_expansionSpeed;
  float x48c_;
  zeus::CColor x490_innerColor;
  zeus::CColor x494_outerColor;
  CDamageInfo x498_phazonDamage;
  enum class EExpansionState {
    Inactive,
    Attack,
    Sustain,
    Release,
    Done
  };
  EExpansionState x4b4_expansionState = EExpansionState::Inactive;
  float x4b8_beamWidth = 0.f;
  float x4bc_lifeTimer = 0.f;
  float x4c0_expansionT = 0.f;
  float x4c4_expansion = 0.f;
  float x4c8_beamAngle = 0.f;
  float x4cc_energyPulseStartY = 0.f;
  float x4d0_shutdownTimer = 0.f;
  float x4d4_contactPulseTimer = 0.f;
  float x4d8_energyPulseTimer = 0.f;
  float x4dc_playerEffectPulseTimer = 0.f;
  float x4e0_playerDamageDuration = 0.f;
  float x4e4_playerDamageTimer = 0.f;
  TLockedToken<CTexture> x4e8_texture;
  TLockedToken<CTexture> x4f4_glowTexture;
  TCachedToken<CGenDescription> x500_contactFxDesc;
  TCachedToken<CGenDescription> x50c_pulseFxDesc;
  std::unique_ptr<CElementGen> x518_contactGen;
  std::unique_ptr<CElementGen> x51c_pulseGen;
  std::unique_ptr<CElementGen> x520_weaponGen;
  CAssetId x524_freezeSteamTxtr;
  CAssetId x528_freezeIceTxtr;
  TToken<CElectricDescription> x52c_visorElectric; // Used to be optional
  TToken<CGenDescription> x538_visorParticle; // Used to be optional
  u16 x544_freezeSfx;
  u16 x546_electricSfx;
  union {
    struct {
      bool x548_24_ : 1;
      bool x548_25_enableEnergyPulse : 1;
      bool x548_26_firing : 1;
      bool x548_27_texturesLoaded : 1;
      bool x548_28_drawOwnerFirst : 1;
      bool x548_29_activePlayerPhazon : 1;
    };
    u32 _dummy3 = 0;
  };

  struct RenderObjects {
    CColoredStripShader m_beamStrip1;
    CColoredStripShader m_beamStrip2;
    CColoredStripShader m_beamStrip3;
    CColoredStripShader m_beamStrip4;
    CColoredStripShader m_beamStrip1Sub;
    CColoredStripShader m_beamStrip2Sub;
    CColoredStripShader m_beamStrip3Sub;
    CColoredStripShader m_beamStrip4Sub;
    CColoredStripShader m_motionBlurStrip;
    RenderObjects(boo::IGraphicsDataFactory::Context& ctx,
                  boo::ObjToken<boo::ITexture> tex,
                  boo::ObjToken<boo::ITexture> glowTex);
  };
  mutable rstl::optional<RenderObjects> m_renderObjs;

  void SetLightsActive(bool active, CStateManager& mgr);
  void CreatePlasmaLights(u32 sourceId, const CLight& l, CStateManager& mgr);
  void DeletePlasmaLights(CStateManager& mgr);
  void UpdateLights(float expansion, float dt, CStateManager& mgr);
  void UpdateEnergyPulse(float dt);
  void RenderMotionBlur() const;
  void RenderBeam(s32 subdivs, float width, const zeus::CColor& color, s32 flags,
                  CColoredStripShader& shader) const;
  float UpdateBeamState(float dt, CStateManager& mgr);
  void MakeBillboardEffect(const rstl::optional<TToken<CGenDescription>>& particle,
                           const rstl::optional<TToken<CElectricDescription>>& electric,
                           std::string_view name, CStateManager& mgr);
  void UpdatePlayerEffects(float dt, CStateManager& mgr);
public:
  CPlasmaProjectile(const TToken<CWeaponDescription>& wDesc, std::string_view name, EWeaponType wType,
                    const CBeamInfo& bInfo, const zeus::CTransform& xf, EMaterialTypes matType,
                    const CDamageInfo& dInfo, TUniqueId uid, TAreaId aid, TUniqueId owner,
                    const PlayerEffectResoures& res, bool growingBeam, EProjectileAttrib attribs);

  void Accept(IVisitor& visitor);
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr);
  void ResetBeam(CStateManager& mgr, bool fullReset);
  void UpdateFx(const zeus::CTransform& xf, float dt, CStateManager& mgr);
  void Fire(const zeus::CTransform& xf, CStateManager& mgr, bool b);
  void Touch(CActor& other, CStateManager& mgr);
  bool CanRenderUnsorted(const CStateManager& mgr) const;
  void AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr) const;
  void Render(const CStateManager& mgr) const;
};
} // namespace urde
