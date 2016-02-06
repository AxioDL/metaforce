#ifndef __RETRO_CPARTICLEDATAFACTORY_HPP__
#define __RETRO_CPARTICLEDATAFACTORY_HPP__

#include "RetroTypes.hpp"
#include "IObj.hpp"
#include "CToken.hpp"
#include "IOStreams.hpp"

namespace Retro
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
    TToken<CModel> m_model;
    bool m_found = false;
};

struct SChildGeneratorDesc
{
    TToken<CGenDescription> m_gen;
    bool m_found = false;
};

struct SSwooshGeneratorDesc
{
    TToken<CSwooshDescription> m_swoosh;
    bool m_found = false;
};

struct SElectricGeneratorDesc
{
    TToken<CElectricDescription> m_electric;
    bool m_found = false;
};

class CParticleDataFactory
{
    static SParticleModel GetModel(CInputStream& in, CSimplePool* resPool);
    static SChildGeneratorDesc GetChildGeneratorDesc(TResID res, CSimplePool* resPool, const std::vector<TResID>& tracker);
    static SChildGeneratorDesc GetChildGeneratorDesc(CInputStream& in, CSimplePool* resPool, const std::vector<TResID>& tracker);
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

public:
    static CGenDescription* GetGeneratorDesc(CInputStream& in, CSimplePool* resPool);
    static CGenDescription* CreateGeneratorDescription(CInputStream& in, std::vector<TResID>& tracker,
                                                       TResID resId, CSimplePool* resPool);
    static bool CreateGPSM(CGenDescription* fillDesc, CInputStream& in,
                           std::vector<TResID>& tracker, CSimplePool* resPool);
    static void LoadGPSMTokens(CGenDescription* desc);
};

std::unique_ptr<IObj> FParticleFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms);

}

#endif // __RETRO_CPARTICLEDATAFACTORY_HPP__
