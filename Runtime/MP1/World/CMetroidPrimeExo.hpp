#ifndef CMETROIDPRIMEEXO_HPP
#define CMETROIDPRIMEEXO_HPP

#include "World/CPatterned.hpp"
#include "World/CPatternedInfo.hpp"
#include "World/CActorParameters.hpp"
#include "Camera/CCameraShakeData.hpp"
#include "Weapon/CBeamInfo.hpp"

namespace urde
{
class CCameraShakeData;

namespace MP1
{

struct SPrimeStruct2B
{
    u32 x0_propertyCount;
    CAssetId x4_particle1;
    CAssetId x8_particle2;
    CAssetId xc_particle3;
    CDamageInfo x10_dInfo;
    float x2c_;
    float x30_;
    CAssetId x34_texture;
    u16 x38_;
    u16 x3a_;
    explicit SPrimeStruct2B(CInputStream& in);
};

struct SPrimeStruct5
{
    u32 x0_propertyCount;
    CAssetId x4_;
    u32 x8_;
    CAssetId xc_;
    CAssetId x10_;
    u32 x14_;
    u32 x18_;
    u32 x1c_;
    u32 x20_;
    explicit SPrimeStruct5(CInputStream& in);
};

struct SPrimeStruct4
{
    CBeamInfo x0_beamInfo;
    u32 x44_;
    CDamageInfo x48_dInfo1;
    SPrimeStruct5 x64_struct5;
    float x88_;
    CDamageInfo x8c_dInfo2;
    explicit SPrimeStruct4(CInputStream& in);
};

struct SPrimeStruct5B
{
    u32 x0_propertyCount;
    TToken<CGenDescription> x4_particle;
    CDamageInfo xc_dInfo;
    float x28_;
    float x2c_;
    float x30_;
    CAssetId x34_texture;
    bool x38_24_ : 1;
    bool x38_25_ : 1;
    bool x38_26_ : 1;
    bool x38_27_ : 1;
    explicit SPrimeStruct5B(CInputStream& in);
};

struct SPrimeStruct6
{
    u32 x0_propertyCount;
    CDamageVulnerability x4_damageVulnerability;
    zeus::CColor x6c_color;
    u32 x70_;
    u32 x74_;
    explicit SPrimeStruct6(CInputStream& in);
};

struct SPrimeExoParameters
{
    u32 x0_propertyCount;
    CPatternedInfo x4_patternedInfo;
    CActorParameters x13c_actorParms;
    u32 x1a4_;
    CCameraShakeData x1a8_;
    CCameraShakeData x27c_;
    CCameraShakeData x350_;
    SPrimeStruct2B x424_;
    CAssetId x460_particle1;
    rstl::reserved_vector<SPrimeStruct4, 4> x464_;
    CAssetId x708_wpsc1;
    CDamageInfo x70c_dInfo1;
    CCameraShakeData x728_shakeData1;
    CAssetId x7fc_wpsc2;
    CDamageInfo x800_dInfo2;
    CCameraShakeData x81c_shakeData2;
    SPrimeStruct5B x8f0_;
    CDamageInfo x92c_;
    CCameraShakeData x948_;
    CAssetId xa1c_particle2;
    CAssetId xa20_swoosh;
    CAssetId xa24_particle3;
    CAssetId xa28_particle4;
    rstl::reserved_vector<SPrimeStruct6, 4> xa2c_;
    explicit SPrimeExoParameters(CInputStream& in);
};

struct SPrimeExoRoomParameters
{
    rstl::reserved_vector<float, 14> x0_;
    explicit SPrimeExoRoomParameters(CInputStream& in);
};

class CMetroidPrimeExo : public CPatterned
{
public:
    static constexpr ECharacter CharacterType = ECharacter::MetroidPrimeExo;
    CMetroidPrimeExo(TUniqueId uid, const std::string& name, const CEntityInfo& info, const zeus::CTransform& xf,
                     CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& aParms,
                     u32 pw1, const CCameraShakeData& shakeData1, const CCameraShakeData& shakeData2,
                     const CCameraShakeData& shakeData3, const SPrimeStruct2B& struct2b, CAssetId particle1,
                     const rstl::reserved_vector<SPrimeStruct4, 4>& struct4s, CAssetId wpsc1, const CDamageInfo& dInfo1,
                     const CCameraShakeData& shakeData4, CAssetId wpsc2, const CDamageInfo& dInfo2,
                     const CCameraShakeData& shakeData5, const SPrimeStruct5B& struct5b, const CDamageInfo& dInfo3,
                     const CCameraShakeData& shakeData6, CAssetId particle2, CAssetId swoosh, CAssetId particle3,
                     CAssetId particle4, const rstl::reserved_vector<SPrimeStruct6, 4>& struct6s);
};

}
}

#endif // CMETROIDPRIMEEXO_HPP
