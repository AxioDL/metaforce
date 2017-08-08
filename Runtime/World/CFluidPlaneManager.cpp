#include "CFluidPlaneManager.hpp"

namespace urde
{

CFluidPlaneManager::CFluidProfile CFluidPlaneManager::sProfile = {};

CFluidPlaneManager::CFluidPlaneManager()
: x0_rippleManager(20, 0.5f)
{
    sProfile.Clear();
    SetupRippleMap();
}

void CFluidPlaneManager::CFluidProfile::Clear()
{

}

void CFluidPlaneManager::StartFrame(bool b)
{
    x121_ = b;
    sProfile.Clear();
}

void CFluidPlaneManager::Update(float dt)
{

}

float CFluidPlaneManager::GetLastRippleDeltaTime(TUniqueId rippler) const
{
    return x0_rippleManager.GetLastRippleDeltaTime(rippler);
}

float CFluidPlaneManager::GetLastSplashDeltaTime(TUniqueId splasher) const
{
    return 0.f;
}

void CFluidPlaneManager::CreateSplash(TUniqueId splasher, CStateManager& mgr, const CScriptWater& water,
                                      const zeus::CVector3f& pos, float factor, bool)
{

}

u8 CFluidPlaneManager::RippleValues[64][64] = {};
u8 CFluidPlaneManager::RippleMins[64] = {};
u8 CFluidPlaneManager::RippleMaxs[64] = {};

void CFluidPlaneManager::SetupRippleMap()
{
    float curX = 0.f;
    for (int i=0 ; i<64 ; ++i)
    {
        float curY = 0.f;
        float minY = 1.f;
        float maxY = 0.f;
        for (int j=0 ; j<64 ; ++j)
        {
            float rVal = 1.f - curY;
            float minX = curY;
            float maxX = 1.25f * (0.25f * rVal + 0.1f) + curY;
            if (curY < 0.f)
                minX = 0.f;
            else if (maxX > 1.f)
                maxX = 1.f;

            float val = 0.f;
            if (curX >= minX && curX <= maxX)
            {
                float t = (curX - minX) / (maxX - minX);
                if (t < 0.4f)
                    val = 2.5f * t;
                else if (t > 0.75f)
                    val = 4.f * (1.f - t);
                else
                    val = 1.f;
            }

            auto valA = u8(std::max(int(255.f * val * rVal * rVal) - 1, 0));
            RippleValues[i][j] = valA;
            if (valA != 0 && curY < minY)
                minY = curY;
            if (valA != 0 && curY > maxY)
                maxY = curY;

            curY += (1.f / 63.f);
        }

        auto valB = u8(std::max(int(255.f * minY) - 1, 0));
        auto valC = u8(std::min(int(255.f * maxY) + 1, 255));
        RippleMins[i] = valB;
        RippleMaxs[i] = valC;
        curX += (1.f / 63.f);
    }
}

}
