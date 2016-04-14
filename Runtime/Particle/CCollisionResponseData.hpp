#ifndef __URDE_CCOLLISIONRESPONSEDATA_HPP__
#define __URDE_CCOLLISIONRESPONSEDATA_HPP__

#include "RetroTypes.hpp"
#include "CFactoryMgr.hpp"
#include "IObj.hpp"
#include "CToken.hpp"
#include "IOStreams.hpp"
#include "optional.hpp"

namespace urde
{
class CSimplePool;
class CGenDescription;
class CDecalDescription;

enum class EWeaponCollisionResponseTypes
{
    Unknown0, Unknown1, Unknown2, Unknown3, Unknown4,
    Unknown5, Unknown6, Unknown7, Unknown8, Unknown9,
    Unknown10,Unknown11,Unknown12,Unknown13,Unknown14,
    Unknown15,Unknown16,Unknown17,Unknown18,Unknown19,
    Unknown20,Unknown21,Unknown22,Unknown23,Unknown24,
    Unknown25,Unknown26,Unknown27,Unknown28,Unknown29,
    Unknown30,Unknown31,Unknown32,Unknown33,Unknown34,
    Unknown35,Unknown36,Unknown37,Unknown38,Unknown39,
    Unknown40,Unknown41,Unknown42,Unknown43,Unknown44,
    Unknown45,Unknown46,Unknown47,Unknown48,Unknown49,
    Unknown50,Unknown51,Unknown52,Unknown53,Unknown54,
    Unknown55,Unknown56,Unknown57,Unknown58,Unknown59,
    Unknown60,Unknown61,Unknown62,Unknown63,Unknown64,
    Unknown65,Unknown66,Unknown67,Unknown68,Unknown69,
    Unknown70,Unknown71,Unknown72,Unknown73,Unknown74,
    Unknown75,Unknown76,Unknown77,Unknown78,Unknown79,
    Unknown80,Unknown81,Unknown82,Unknown83,Unknown84,
    Unknown85,Unknown86,Unknown87,Unknown88,Unknown89,
    Unknown90,Unknown91,Unknown92,Unknown93
};

class CCollisionResponseData
{

    std::vector<std::experimental::optional<TLockedToken<CGenDescription>>> x0_generators;
    std::vector<ResId> x10_sfx;
    std::vector<std::experimental::optional<TLockedToken<CDecalDescription>>> x20_decals;
    float x30_RNGE;
    float x34_FOFF;

    void AddParticleSystemToResponse(EWeaponCollisionResponseTypes type, CInputStream& in, CSimplePool* resPool);
    bool CheckAndAddDecalToResponse(FourCC clsId, CInputStream& in, CSimplePool* resPool);
    bool CheckAndAddSoundFXToResponse(FourCC clsId, CInputStream& in);
    bool CheckAndAddParticleSystemToResponse(FourCC clsId, CInputStream& in, CSimplePool* resPool);
    bool CheckAndAddResourceToResponse(FourCC clsId, CInputStream& in, CSimplePool* resPool);
public:
    CCollisionResponseData(CInputStream& in, CSimplePool* resPool);
};

CFactoryFnReturn FCollisionResponseDataFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms);
}

#endif // __URDE_CCOLLISIONRESPONSEDATA_HPP__
