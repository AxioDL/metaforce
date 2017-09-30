#ifndef CMETROIDBETA_HPP
#define CMETROIDBETA_HPP

#include "World/CPatterned.hpp"
#include "CMetroid.hpp"

namespace urde
{
namespace MP1
{

class CMetroidBeta : public CPatterned
{
public:
    static constexpr ECharacter CharacterType = ECharacter::MetroidBeta;
    CMetroidBeta(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                 const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo,
                 const CActorParameters& aParms, const CMetroidData& metroidData);
    void RenderHitGunEffect() const;
    void RenderHitBallEffect() const;
};

}
}

#endif // CMETROIDBETA_HPP
