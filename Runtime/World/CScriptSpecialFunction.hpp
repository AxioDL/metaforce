#ifndef __URDE_CSCRIPTSPECIALFUNCTION_HPP__
#define __URDE_CSCRIPTSPECIALFUNCTION_HPP__

#include "CActor.hpp"
#include "CDamageInfo.hpp"
#include "zeus/CTransform.hpp"
#include "zeus/CColor.hpp"

namespace urde
{
class CScriptSpecialFunction : public CActor
{
public:
    enum class ESpecialFunction
    {
        What,
        PlayerFollowLocator,
        SpinnerController,
        ObjectFollowLocator,
        Four,
        InventoryActivator,
        MapStation,
        SaveStation,
        IntroBossRingController,
        ViewFrustumTester,
        ShotSpinnerController,
        EscapeSequence,
        BossEnergyBar,
        EndGame,
        HUDFadeIn,
        CinematicSkip,
        ScriptLayerController,
        RainSimulator,
        AreaDamage,
        ObjectFollowObject,
        RedundantHintSystem,
        DropBomb,
        TwentyTwo,
        MissileStation,
        Billboard,
        PlayerInAreaRelay,
        HUDTarget,
        FogFader,
        EnterLogbook,
        PowerBombStation,
        Ending,
        FusionRelay,
        WeaponSwitch,
        FogVolume = 47,
        RadialDamage = 48,
        EnvFxDensityController = 49,
        RumbleEffect = 50
    };

private:
public:
    CScriptSpecialFunction(TUniqueId, const std::string&, const CEntityInfo&, const zeus::CTransform&, ESpecialFunction,
                           const std::string&, float, float, float, float, const zeus::CVector3f&, const zeus::CColor&,
                           bool, const CDamageInfo&, u32, u32, u32, u16, u16, u16);
};
}

#endif // CSCRIPTSPECIALFUNCTION_HPP
