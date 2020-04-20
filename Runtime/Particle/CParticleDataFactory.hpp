#pragma once

#include <memory>
#include <vector>

#include "Runtime/CFactoryMgr.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/IOStreams.hpp"
#include "Runtime/IObj.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Graphics/CModel.hpp"

namespace urde {
class CColorElement;
class CElectricDescription;
class CEmitterElement;
class CGenDescription;
class CIntElement;
class CModVectorElement;
class CRealElement;
class CSimplePool;
class CSwooshDescription;
class CUVElement;
class CVParamTransfer;
class CVectorElement;

struct SParticleModel {
  TLockedToken<CModel> m_token;
  bool m_found = false;
  CModel* m_model = nullptr;
  SParticleModel() = default;
  SParticleModel(CToken&& tok, bool found) : m_token(std::move(tok)), m_found(found) {}
  explicit operator bool() const { return m_found; }
};

struct SChildGeneratorDesc {
  TLockedToken<CGenDescription> m_token;
  bool m_found = false;
  CGenDescription* m_gen = nullptr;
  SChildGeneratorDesc() = default;
  SChildGeneratorDesc(CToken&& tok, bool found) : m_token(std::move(tok)), m_found(found) {}
  explicit operator bool() const { return m_found; }
};

struct SSwooshGeneratorDesc {
  TLockedToken<CSwooshDescription> m_token;
  bool m_found = false;
  CSwooshDescription* m_swoosh = nullptr;
  SSwooshGeneratorDesc() = default;
  SSwooshGeneratorDesc(CToken&& tok, bool found) : m_token(std::move(tok)), m_found(found) {}
  explicit operator bool() const { return m_found; }
};

struct SElectricGeneratorDesc {
  TLockedToken<CElectricDescription> m_token;
  bool m_found = false;
  CElectricDescription* m_electric = nullptr;
  SElectricGeneratorDesc() = default;
  SElectricGeneratorDesc(CToken&& tok, bool found) : m_token(std::move(tok)), m_found(found) {}
  explicit operator bool() const { return m_found; }
};

class CParticleDataFactory {
  friend class CDecalDataFactory;
  friend class CCollisionResponseData;
  friend class CParticleElectricDataFactory;
  friend class CParticleSwooshDataFactory;
  friend class CProjectileWeaponDataFactory;

  static SParticleModel GetModel(CInputStream& in, CSimplePool* resPool);
  static SChildGeneratorDesc GetChildGeneratorDesc(CAssetId res, CSimplePool* resPool,
                                                   const std::vector<CAssetId>& tracker);
  static SChildGeneratorDesc GetChildGeneratorDesc(CInputStream& in, CSimplePool* resPool,
                                                   const std::vector<CAssetId>& tracker);
  static SSwooshGeneratorDesc GetSwooshGeneratorDesc(CInputStream& in, CSimplePool* resPool);
  static SElectricGeneratorDesc GetElectricGeneratorDesc(CInputStream& in, CSimplePool* resPool);
  static std::unique_ptr<CUVElement> GetTextureElement(CInputStream& in, CSimplePool* resPool);
  static std::unique_ptr<CColorElement> GetColorElement(CInputStream& in);
  static std::unique_ptr<CModVectorElement> GetModVectorElement(CInputStream& in);
  static std::unique_ptr<CEmitterElement> GetEmitterElement(CInputStream& in);
  static std::unique_ptr<CVectorElement> GetVectorElement(CInputStream& in);
  static std::unique_ptr<CRealElement> GetRealElement(CInputStream& in);
  static std::unique_ptr<CIntElement> GetIntElement(CInputStream& in);

  static float GetReal(CInputStream& in);
  static s32 GetInt(CInputStream& in);
  static bool GetBool(CInputStream& in);
  static FourCC GetClassID(CInputStream& in);
  static std::unique_ptr<CGenDescription> CreateGeneratorDescription(CInputStream& in, std::vector<CAssetId>& tracker,
                                                                     CAssetId resId, CSimplePool* resPool);
  static bool CreateGPSM(CGenDescription* fillDesc, CInputStream& in, std::vector<CAssetId>& tracker,
                         CSimplePool* resPool);
  static void LoadGPSMTokens(CGenDescription* desc);

public:
  static std::unique_ptr<CGenDescription> GetGeneratorDesc(CInputStream& in, CSimplePool* resPool);
};

CFactoryFnReturn FParticleFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms,
                                  CObjectReference* selfRef);

} // namespace urde

// FIXME hacky workaround for MSVC; these need to be complete types
// but introduce circular dependencies if included at the start
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/Particle/CSwooshDescription.hpp"
#include "Runtime/Particle/CElectricDescription.hpp"