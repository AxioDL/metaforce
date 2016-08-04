#ifndef __URDE_CDECAL_HPP__
#define __URDE_CDECAL_HPP__

#include "RetroTypes.hpp"
#include "CToken.hpp"
#include "zeus/CTransform.hpp"
#include "CDecalDescription.hpp"
#include "CRandom16.hpp"

namespace urde
{
class CDecal
{
public:
    struct CQuadDecal
    {
        union
        {
            struct
            {
                bool x0_24_ : 1;
            };
            u32 _dummy = 0;
        };
        s32 x4_lifetime = 0;
        float x8_rotation = 0.f;
        CQuadDecal(s32 i, float f)
            : x4_lifetime(i),
              x8_rotation(f)
        {
            x0_24_ = true;
        }
    };
private:
    static bool sMoveRedToAphaBuffer;
    static CRandom16 sDecalRandom;

    TLockedToken<CDecalDescription> x0_description;
    zeus::CTransform xc_transform;
    CQuadDecal x3c_decalQuad1;
    CQuadDecal x48_decalQuad2;
    u32 x54_ = 0;
    u32 x58_ = 0;
    u32 x5c_ = 0;
    zeus::CVector3f x60_;
    void InitQuad(CQuadDecal&, const CDecalDescription::SQuadDescr&);
public:
    CDecal(const TToken<CDecalDescription>&, const zeus::CTransform&);
    bool IsDone() const;
    void RenderQuad(CQuadDecal&, const CDecalDescription::SQuadDescr&) const;
    void RenderMdl() const;
    void ProcessQuad(CQuadDecal&, const CDecalDescription::SQuadDescr&, s32) const;
    void Update(float);
    void CheckTime(s32, s32);

    static void SetGlobalSeed(u16);
    static void SetMoveRedToAlphaBuffer(bool);
};
}

#endif // __URDE_CDECAL_HPP__
