#ifndef __URDE_CGAMEAREA_HPP__
#define __URDE_CGAMEAREA_HPP__

#include "zeus/CVector2f.hpp"
#include "zeus/CColor.hpp"
#include "zeus/CAABox.hpp"
#include "CToken.hpp"
#include "RetroTypes.hpp"
#include "IGameArea.hpp"
#include "hecl/ClientProcess.hpp"

namespace urde
{

enum class ERglFogMode
{
    Four = 4
};

class CGameArea : public IGameArea
{
    int x4_mlvlVersion;
    ResId x8_nameSTRG;
    zeus::CTransform xc_transform;
    zeus::CTransform x3c_invTransform;
    zeus::CAABox x6c_aabb;
    ResId x84_mrea;
    u32 x88_areaId;
    std::vector<u16> x8c_attachedAreaIndices;
    std::vector<SObjectTag> x9c_deps1;
    std::vector<SObjectTag> xac_deps2;

    std::vector<u32> xbc_;
    std::vector<Dock> xcc_docks;
    std::vector<CToken> xdc_tokens;

    u32 xec_totalResourcesSize = 0;

    union
    {
        struct
        {
            bool xf0_24_ : 1;
            bool xf0_25_active : 1;
            bool xf0_26_ : 1;
            bool xf0_27_ : 1;
            bool xf0_28_ : 1;
        };
        u8 _dummy = 0;
    };

    std::list<std::shared_ptr<const hecl::ClientProcess::BufferTransaction>> xf8_loadTransactions;

public:
    class CAreaFog
    {
        zeus::CVector2f x4_ = {0.f, 1024.f};
        zeus::CVector2f xc_ = {0.f, 1024.f};
        zeus::CVector2f x14_;
        zeus::CVector3f x1c_ = {0.5f};
        zeus::CVector3f x28_ = {0.5f};
        float x34_ = 0.f;
    public:
        void SetFogExplicit(ERglFogMode, const zeus::CColor& color, const zeus::CVector2f& vec);
        void DisableFog();
        void Update(float dt);
    };

    CGameArea(CInputStream& in, int mlvlVersion);

    const zeus::CTransform& GetTransform() const {return xc_transform;}
    const zeus::CTransform& GetInverseTransform() const {return x3c_invTransform;}
    const zeus::CAABox& GetAABB() const {return x6c_aabb;}

};

}

#endif // __URDE_CGAMEAREA_HPP__
