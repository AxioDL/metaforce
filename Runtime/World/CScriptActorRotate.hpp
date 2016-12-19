#ifndef CSCRIPTACTORROTATE_HPP
#define CSCRIPTACTORROTATE_HPP

#include "CEntity.hpp"
#include "zeus/CTransform.hpp"

namespace urde
{
class CScriptActorRotate : public CEntity
{
    zeus::CVector3f x34_rotation;
    float x40_;
    float x44_;
    std::map<TUniqueId, zeus::CTransform> x48_actors;

    union {
        struct
        {
            bool x58_24_ : 1;
            bool x58_25_ : 1;
            bool x58_26_ : 1;
            bool x58_27_ : 1;
        };
        u32 dummy = 0;
    };

public:
    CScriptActorRotate(TUniqueId, const std::string&, const CEntityInfo&, const zeus::CVector3f&, float, bool, bool,
                       bool);
};
}

#endif // CSCRIPTACTORROTATE_HPP
