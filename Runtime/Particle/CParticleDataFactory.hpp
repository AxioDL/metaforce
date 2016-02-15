#ifndef __PSHAG_CPARTICLEDATAFACTORY_HPP__
#define __PSHAG_CPARTICLEDATAFACTORY_HPP__

#include "RetroTypes.hpp"
#include "IObj.hpp"
#include "CToken.hpp"
#include "IOStreams.hpp"

namespace pshag
{
class CGenDescription;
class CSwooshDescription;
class CElectricDescription;
class CSimplePool;
class CVParamTransfer;
class CModel;
class CUVElement;
class CColorElement;
class CModVectorElement;
class CEmitterElement;
class CVectorElement;
class CRealElement;
class CIntElement;

struct SParticleModel
{
    TLockedToken<CModel> m_token;
    bool m_found = false;
    CModel* m_model = nullptr;
};

struct SChildGeneratorDesc
{
    TLockedToken<CGenDescription> m_token;
    bool m_found = false;
    CGenDescription* m_gen = nullptr;
};

struct SSwooshGeneratorDesc
{
    TLockedToken<CSwooshDescription> m_token;
    bool m_found = false;
    CSwooshDescription* m_swoosh = nullptr;
};

struct SElectricGeneratorDesc
{
    TLockedToken<CElectricDescription> m_token;
    bool m_found = false;
    CElectricDescription* m_electric = nullptr;
};

class CParticleDataFactory
{
    friend class CDecalDataFactory;
    friend class CParticleElectricDataFactory;
    friend class CParticleSwooshDataFactory;
    friend class CProjectileWeaponDataFactory;

    static SParticleModel GetModel(CInputStream& in, CSimplePool* resPool);
    static SChildGeneratorDesc GetChildGeneratorDesc(TResId res, CSimplePool* resPool, const std::vector<TResId>& tracker);
    static SChildGeneratorDesc GetChildGeneratorDesc(CInputStream& in, CSimplePool* resPool, const std::vector<TResId>& tracker);
    static SSwooshGeneratorDesc GetSwooshGeneratorDesc(CInputStream& in, CSimplePool* resPool);
    static SElectricGeneratorDesc GetElectricGeneratorDesc(CInputStream& in, CSimplePool* resPool);
    static CUVElement* GetTextureElement(CInputStream& in, CSimplePool* resPool);
    static CColorElement* GetColorElement(CInputStream& in);
    static CModVectorElement* GetModVectorElement(CInputStream& in);
    static CEmitterElement* GetEmitterElement(CInputStream& in);
    static CVectorElement* GetVectorElement(CInputStream& in);
    static CRealElement* GetRealElement(CInputStream& in);
    static CIntElement* GetIntElement(CInputStream& in);

    static float GetReal(CInputStream& in);
    static int32_t GetInt(CInputStream& in);
    static bool GetBool(CInputStream& in);
    static FourCC GetClassID(CInputStream& in);
    static CGenDescription* CreateGeneratorDescription(CInputStream& in, std::vector<TResId>& tracker,
                                                       TResId resId, CSimplePool* resPool);
    static bool CreateGPSM(CGenDescription* fillDesc, CInputStream& in,
                           std::vector<TResId>& tracker, CSimplePool* resPool);
    static void LoadGPSMTokens(CGenDescription* desc);
public:
    static CGenDescription* GetGeneratorDesc(CInputStream& in, CSimplePool* resPool);
};

std::unique_ptr<IObj> FParticleFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms);

}

#endif // __PSHAG_CPARTICLEDATAFACTORY_HPP__
