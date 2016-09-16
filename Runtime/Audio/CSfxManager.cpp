#include "CSfxManager.hpp"

namespace urde
{
std::vector<s16>* CSfxManager::mpSfxTranslationTable = nullptr;

void CSfxManager::AddListener(ESfxChannels,
                              const zeus::CVector3f& vec1, const zeus::CVector3f& vec2,
                              const zeus::CVector3f& right, const zeus::CVector3f& up,
                              float, float, float, u32, u8)
{
}

void CSfxManager::UpdateListener(const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                                 const zeus::CVector3f& heading, const zeus::CVector3f& up,
                                 u8 vol)
{
}

u16 CSfxManager::TranslateSFXID(u16 id)
{
    if (mpSfxTranslationTable == nullptr)
        return 0;

    u16 index = id;
    if (index >= mpSfxTranslationTable->size())
        return 0;

    s16 ret = mpSfxTranslationTable->at(index);
    if (ret == -1)
        return 0;
    return ret;
}

CSfxHandle CSfxManager::SfxStop(const CSfxHandle& handle)
{
}

CSfxHandle CSfxManager::SfxStart(u16 id, s16 vol, s16 pan, bool active, s16 prio, bool inArea, s32 areaId)
{
}

}
