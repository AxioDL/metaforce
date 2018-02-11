#include "CMetroidPrimeExo.hpp"
#include "World/ScriptLoader.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "DataSpec/DNAMP1/ScriptObjects/MetroidPrimeStage1.hpp"

namespace urde::MP1
{

SPrimeStruct2B::SPrimeStruct2B(CInputStream& in)
: x0_propertyCount(in.readUint32Big()),
  x4_particle1(in.readUint32Big()),
  x8_particle2(in.readUint32Big()),
  xc_particle3(in.readUint32Big()),
  x10_dInfo(in),
  x2c_(in.readFloatBig()),
  x30_(in.readFloatBig()),
  x34_texture(in.readUint32Big()),
  x38_(CSfxManager::TranslateSFXID(u16(in.readUint32Big()))),
  x3a_(CSfxManager::TranslateSFXID(u16(in.readUint32Big())))
{}

SPrimeStruct5::SPrimeStruct5(CInputStream& in)
: x0_propertyCount(in.readUint32Big()),
  x4_(in.readUint32Big()),
  x8_(in.readUint32Big()),
  xc_(in.readUint32Big()),
  x10_(in.readUint32Big()),
  x14_(in.readUint32Big()),
  x18_(in.readUint32Big()),
  x1c_(in.readUint32Big()),
  x20_(in.readUint32Big())
{}

SPrimeStruct4::SPrimeStruct4(CInputStream& in)
: x0_beamInfo(in),
  x44_(in.readUint32Big()),
  x48_dInfo1(in),
  x64_struct5(in),
  x88_(in.readFloatBig()),
  x8c_dInfo2(in)
{}

SPrimeStruct6::SPrimeStruct6(CInputStream& in)
: x0_propertyCount(in.readUint32Big()),
  x4_damageVulnerability(in),
  x6c_color(zeus::CColor::ReadRGBABig(in)),
  x70_(in.readUint32Big()),
  x74_(in.readUint32Big())
{}

static CPatternedInfo LoadPatternedInfo(CInputStream& in)
{
    std::pair<bool, u32> pcount = CPatternedInfo::HasCorrectParameterCount(in);
    return CPatternedInfo(in, pcount.second);
}

using CameraShakeData = DataSpec::DNAMP1::MetroidPrimeStage1::MassivePrimeStruct::CameraShakeData;

static SCameraShakePoint BuildCameraShakePoint(CameraShakeData::CameraShakerComponent::CameraShakePoint& sp)
{
    return SCameraShakePoint(false, sp.attackTime, sp.sustainTime, sp.duration, sp.magnitude);
}

static CCameraShakerComponent BuildCameraShakerComponent(CameraShakeData::CameraShakerComponent& comp)
{
    return CCameraShakerComponent(comp.useModulation, BuildCameraShakePoint(comp.am),
                                  BuildCameraShakePoint(comp.fm));
}

static CCameraShakeData LoadCameraShakeData(CInputStream& in)
{
    CameraShakeData shakeData;
    shakeData.read(in);
    return CCameraShakeData(shakeData.duration, shakeData.sfxDist, u32(shakeData.useSfx),
                            zeus::CVector3f::skZero,
                            BuildCameraShakerComponent(shakeData.shakerComponents[0]),
                            BuildCameraShakerComponent(shakeData.shakerComponents[1]),
                            BuildCameraShakerComponent(shakeData.shakerComponents[2]));
}

static rstl::reserved_vector<SPrimeStruct4, 4> LoadPrimeStruct4s(CInputStream& in)
{
    rstl::reserved_vector<SPrimeStruct4, 4> ret;
    for (int i=0 ; i<4 ; ++i)
        ret.emplace_back(in);
    return ret;
}

static rstl::reserved_vector<SPrimeStruct6, 4> LoadPrimeStruct6s(CInputStream& in)
{
    rstl::reserved_vector<SPrimeStruct6, 4> ret;
    for (int i=0 ; i<4 ; ++i)
        ret.emplace_back(in);
    return ret;
}

SPrimeExoParameters::SPrimeExoParameters(CInputStream& in)
: x0_propertyCount(in.readUint32Big()),
  x4_patternedInfo(LoadPatternedInfo(in)),
  x13c_actorParms(ScriptLoader::LoadActorParameters(in)),
  x1a4_(in.readUint32Big()),
  x1a8_(LoadCameraShakeData(in)),
  x27c_(LoadCameraShakeData(in)),
  x350_(LoadCameraShakeData(in)),
  x424_(in),
  x460_particle1(in.readUint32Big()),
  x464_(LoadPrimeStruct4s(in)),
  x708_wpsc1(in.readUint32Big()),
  x70c_dInfo1(in),
  x728_shakeData1(LoadCameraShakeData(in)),
  x7fc_wpsc2(in.readUint32Big()),
  x800_dInfo2(in),
  x81c_shakeData2(LoadCameraShakeData(in)),
  x8f0_(in),
  x92c_(in),
  x948_(LoadCameraShakeData(in)),
  xa1c_particle2(in.readUint32Big()),
  xa20_swoosh(in.readUint32Big()),
  xa24_particle3(in.readUint32Big()),
  xa28_particle4(in.readUint32Big()),
  xa2c_(LoadPrimeStruct6s(in))
{}

SPrimeExoRoomParameters::SPrimeExoRoomParameters(CInputStream& in)
{
    u32 propCount = std::min(u32(14), in.readUint32Big());
    for (int i=0 ; i<propCount ; ++i)
        x0_.push_back(in.readFloatBig());
}

CMetroidPrimeExo::CMetroidPrimeExo(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                   const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo,
                                   const CActorParameters& aParms, u32 pw1, const CCameraShakeData& shakeData1,
                                   const CCameraShakeData& shakeData2, const CCameraShakeData& shakeData3,
                                   const SPrimeStruct2B& struct2b, CAssetId particle1,
                                   const rstl::reserved_vector<SPrimeStruct4, 4>& struct4s, CAssetId wpsc1,
                                   const CDamageInfo& dInfo1, const CCameraShakeData& shakeData4, CAssetId wpsc2,
                                   const CDamageInfo& dInfo2, const CCameraShakeData& shakeData5,
                                   const SPrimeProjectileInfo& projectileInfo, const CDamageInfo& dInfo3,
                                   const CCameraShakeData& shakeData6, CAssetId particle2, CAssetId swoosh,
                                   CAssetId particle3, CAssetId particle4,
                                   const rstl::reserved_vector<SPrimeStruct6, 4>& struct6s)
: CPatterned(ECharacter::MetroidPrimeExo, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Flyer, EColliderType::One, EBodyType::Flyer, aParms, 2)
{

}

}
