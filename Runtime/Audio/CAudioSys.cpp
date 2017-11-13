#include "CAudioSys.hpp"
#include "CSimplePool.hpp"
#include "CAudioGroupSet.hpp"

namespace urde
{

CAudioSys* CAudioSys::g_SharedSys = nullptr;
static std::unordered_map<std::string, TLockedToken<CAudioGroupSet>> mpGroupSetDB;
static std::unordered_map<CAssetId, std::string> mpGroupSetResNameDB;
static const std::string mpDefaultInvalidString = "NULL";

TLockedToken<CAudioGroupSet> CAudioSys::FindGroupSet(std::string_view name)
{
    auto search = mpGroupSetDB.find(name.data());
    if (search == mpGroupSetDB.cend())
        return {};
    return search->second;
}

std::string_view CAudioSys::SysGetGroupSetName(CAssetId id)
{
    auto search = mpGroupSetResNameDB.find(id);
    if (search == mpGroupSetResNameDB.cend())
        return mpDefaultInvalidString;
    return search->second;
}

bool CAudioSys::SysLoadGroupSet(CSimplePool* pool, CAssetId id)
{
    if (!FindGroupSet(SysGetGroupSetName(id)))
    {
        TLockedToken<CAudioGroupSet> set = pool->GetObj(SObjectTag{FOURCC('AGSC'), id});
        mpGroupSetDB.emplace(std::make_pair(set->GetName(), set));
        mpGroupSetResNameDB.emplace(std::make_pair(id, set->GetName()));
        return false;
    }
    else
    {
        return true;
    }
}

bool CAudioSys::SysLoadGroupSet(const TLockedToken<CAudioGroupSet>& set, std::string_view name, CAssetId id)
{
    if (!FindGroupSet(name))
    {
        mpGroupSetDB.emplace(std::make_pair(set->GetName(), set));
        mpGroupSetResNameDB.emplace(std::make_pair(id, set->GetName()));
        return false;
    }
    else
    {
        return true;
    }
}

void CAudioSys::SysUnloadAudioGroupSet(std::string_view name)
{
    auto set = FindGroupSet(name);
    if (!set)
        return;

    mpGroupSetDB.erase(name.data());
    mpGroupSetResNameDB.erase(set.GetObjectTag()->id);
}

bool CAudioSys::SysIsGroupSetLoaded(std::string_view name)
{
    return FindGroupSet(name).operator bool();
}

void CAudioSys::SysAddGroupIntoAmuse(std::string_view name)
{
    if (auto set = FindGroupSet(name))
        AddAudioGroup(set->GetAudioGroupData());
}

void CAudioSys::SysRemoveGroupFromAmuse(std::string_view name)
{
    if (auto set = FindGroupSet(name))
        RemoveAudioGroup(set->GetAudioGroupData());
}

static float s_MasterVol = 1.f;
static float s_SfxVol = 1.f;
void CAudioSys::_UpdateVolume()
{
    GetAmuseEngine().setVolume(s_MasterVol * s_SfxVol);
}

void CAudioSys::SysSetVolume(u8 volume)
{
    s_MasterVol = volume / 127.f;
    _UpdateVolume();
}

void CAudioSys::SysSetSfxVolume(u8 volume, u16 time, bool music, bool fx)
{
    s_SfxVol = volume / 127.f;
    _UpdateVolume();
}

static s16 s_VolumeScale = 0x7f;
static s16 s_DefaultVolumeScale = 0x7f;

s16 CAudioSys::GetDefaultVolumeScale()
{
    return s_DefaultVolumeScale;
}

void CAudioSys::SetDefaultVolumeScale(s16 scale)
{
    s_DefaultVolumeScale = scale;
}

void CAudioSys::SetVolumeScale(s16 scale)
{
    s_VolumeScale = scale;
}

}
