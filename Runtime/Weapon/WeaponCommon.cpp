#include "WeaponCommon.hpp"
#include "Character/CAnimData.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "Character/CPrimitive.hpp"

namespace urde
{
namespace NWeaponTypes
{

void primitive_set_to_token_vector(const CAnimData& animData, const std::set<CPrimitive>& primSet,
                                   std::vector<CToken>& tokensOut, bool preLock)
{
    int eventCount = 0;
    for (const CPrimitive& prim : primSet)
        if (animData.GetEventResourceIdForAnimResourceId(prim.GetAnimResId()).IsValid())
            ++eventCount;

    tokensOut.clear();
    tokensOut.reserve(primSet.size() + eventCount);

    SObjectTag atag{FOURCC('ANIM'), 0};
    SObjectTag etag{FOURCC('EVNT'), 0};
    for (const CPrimitive& prim : primSet)
    {
        CAssetId eId = animData.GetEventResourceIdForAnimResourceId(prim.GetAnimResId());
        if (eId.IsValid())
        {
            etag.id = prim.GetAnimResId();
            tokensOut.push_back(g_SimplePool->GetObj(etag));
            if (preLock)
                tokensOut.back().Lock();
        }
        atag.id = prim.GetAnimResId();
        tokensOut.push_back(g_SimplePool->GetObj(atag));
        if (preLock)
            tokensOut.back().Lock();
    }
}

void unlock_tokens(std::vector<CToken>& anims)
{
    for (CToken& tok : anims)
        tok.Unlock();
}

void lock_tokens(std::vector<CToken>& anims)
{
    for (CToken& tok : anims)
        tok.Lock();
}

bool are_tokens_ready(const std::vector<CToken>& anims)
{
    for (const CToken& tok : anims)
        if (!tok.IsLoaded())
            return false;
    return true;
}

void get_token_vector(CAnimData& animData, int begin, int end, std::vector<CToken>& tokensOut, bool preLock)
{
    std::set<CPrimitive> prims;
    for (int i=begin ; i<end ; ++i)
    {
        CAnimPlaybackParms parms(i, -1, 1.f, true);
        animData.GetAnimationPrimitives(parms, prims);
    }
    primitive_set_to_token_vector(animData, prims, tokensOut, preLock);
}

void get_token_vector(CAnimData& animData, int animIdx, std::vector<CToken>& tokensOut, bool preLock)
{
    std::set<CPrimitive> prims;
    CAnimPlaybackParms parms(animIdx, -1, 1.f, true);
    animData.GetAnimationPrimitives(parms, prims);
    primitive_set_to_token_vector(animData, prims, tokensOut, preLock);
}

}
}
