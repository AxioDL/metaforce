#ifndef __CSCRIPTWORLDTELEPORTER_HPP__
#define __CSCRIPTWORLDTELEPORTER_HPP__

#include "CEntity.hpp"
#include "zeus/CVector3f.hpp"

namespace urde
{

class CScriptWorldTeleporter : public CEntity
{
    u32 x34_;
    u32 x38_;
    u32 x3c_ = 0;
    union
    {
        struct
        {
            bool x40_24_ : 1;
            bool x40_25_ : 1;
            bool x40_26_ : 1;
            bool x40_27_ : 1;
        };
        u8 _dummy = 0;
    };

    float x44_ = 0.1f;
    float x48_ = 8.0f;
    float x4c_ = 0.0f;
    u32 x50_ = -1;
    u32 x54_ = -1;
    u32 x58_ = 0;
    zeus::CVector3f x5c_;
    u32 x68_ = -1;
    zeus::CVector3f x6c_;
    u32 x78_ = -1;
    zeus::CVector3f x7c_;
    u32 x88_ = -1;

public:
    CScriptWorldTeleporter(TUniqueId, const std::string&, const CEntityInfo&, bool, u32, u32);
};
}

#endif // __CSCRIPTWORLDTELEPORTER_HPP__
