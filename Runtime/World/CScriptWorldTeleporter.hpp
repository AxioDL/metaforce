#ifndef __CSCRIPTWORLDTELEPORTER_HPP__
#define __CSCRIPTWORLDTELEPORTER_HPP__

#include "CEntity.hpp"
#include "CAnimationParameters.hpp"
#include "zeus/CVector3f.hpp"

namespace urde
{

class CScriptWorldTeleporter : public CEntity
{
    enum class ETeleporterType
    {
        NoTransition,
        Elevator,
        Text
    };

    ResId x34_worldId;
    ResId x38_areaId;
    ETeleporterType x3c_type = ETeleporterType::NoTransition;
    union
    {
        struct
        {
            bool x40_24_upElevator : 1;
            bool x40_25_inTransition : 1;
            bool x40_27_fadeWhite : 1;
        };
        u8 _dummy = 0;
    };

    float x44_charFadeIn = 0.1f;
    float x48_charsPerSecond = 8.0f;
    float x4c_showDelay = 0.0f;
    CAnimationParameters x50_playerAnim;
    zeus::CVector3f x5c_playerScale;
    ResId x68_platformModel = kInvalidResId;
    zeus::CVector3f x6c_platformScale;
    ResId x78_backgroundModel = kInvalidResId;
    zeus::CVector3f x7c_backgroundScale;
    u16 x88_soundId = -1;
    u8 x8a_volume = 0;
    u8 x8b_panning = 0;
    ResId x8c_fontId;
    ResId x90_stringId;
public:
    CScriptWorldTeleporter(TUniqueId, const std::string&, const CEntityInfo&, bool, ResId, ResId);
    CScriptWorldTeleporter(TUniqueId, const std::string&, const CEntityInfo&, bool, ResId, ResId, u16, u8, u8, ResId,
                           ResId, bool, float, float, float);
    CScriptWorldTeleporter(TUniqueId, const std::string&, const CEntityInfo&, bool, ResId, ResId, ResId, u32, u32,
                           const zeus::CVector3f&, ResId, const zeus::CVector3f&, ResId, const zeus::CVector3f&, bool,
                           u16, u8, u8);

    void Accept(IVisitor&);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr);
    void StartTransition(CStateManager&);
};
}

#endif // __CSCRIPTWORLDTELEPORTER_HPP__
