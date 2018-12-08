#include "CWeaponMgr.hpp"

namespace urde {

void CWeaponMgr::Add(TUniqueId uid, EWeaponType type) {
  x0_weapons.insert(std::make_pair(uid, rstl::reserved_vector<s32, 10>()));
  x0_weapons[uid].resize(10);
  ++x0_weapons[uid][u32(type)];
}

void CWeaponMgr::Remove(TUniqueId uid) {
  s32 totalActive = 0;
  for (u32 i = 0; i < 10; ++i)
    totalActive += x0_weapons[uid][i];

  if (totalActive == 0)
    x0_weapons.erase(uid);
}

void CWeaponMgr::IncrCount(TUniqueId uid, EWeaponType type) {
  if (GetIndex(uid) < 0)
    Add(uid, type);
  else
    x0_weapons[uid][u32(type)]++;
}

void CWeaponMgr::DecrCount(TUniqueId uid, EWeaponType type) {
  if (GetIndex(uid) < 0)
    return;

  x0_weapons[uid][u32(type)]--;
  if (x0_weapons[uid][u32(type)] <= 0)
    Remove(uid);
}

s32 CWeaponMgr::GetNumActive(TUniqueId uid, EWeaponType type) const {
  if (GetIndex(uid) < 0)
    return 0;

  return x0_weapons.at(uid)[u32(type)];
}

s32 CWeaponMgr::GetIndex(TUniqueId uid) const {
  if (x0_weapons.find(uid) == x0_weapons.end())
    return -1;
  return s32(std::distance(x0_weapons.begin(), x0_weapons.find(uid)));
}

} // namespace urde
