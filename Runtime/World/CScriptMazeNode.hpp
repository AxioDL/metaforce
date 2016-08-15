#ifndef __URDE_CSCRIPTMAZE_NODE_HPP__
#define __URDE_CSCRIPTMAZE_NODE_HPP__

#include "CActor.hpp"

namespace urde
{
class CScriptMazeNode : CActor
{
    s32 xe8_;
    s32 xec_;
    s32 xf0_;
    TUniqueId xf4_ = kInvalidUniqueId;
    float xf8_ = 0.f;
    TUniqueId xfc_ = kInvalidUniqueId;
    zeus::CVector3f x100_;
    TUniqueId x10c_ = kInvalidUniqueId;
    zeus::CVector3f x110_;
    TUniqueId x11c_ = kInvalidUniqueId;
    zeus::CVector3f x120_;
    s32 x130_ = 0;
    s32 x134_ = 0;
    s32 x138_ = 0;
    union
    {
        struct
        {
            bool x13c_24_ : 1;
            bool x13c_25_ : 1;
            bool x13c_26_ : 1;
        };
        u8 dummy = 0;
    };
public:
    CScriptMazeNode(TUniqueId, const std::string&, const CEntityInfo&, const zeus::CTransform&, bool, s32, s32, s32,
                    const zeus::CVector3f&, const zeus::CVector3f&, const zeus::CVector3f&);

    void LoadSeeds();
};
}

#endif
