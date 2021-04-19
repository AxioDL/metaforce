#include "Runtime/Weapon/CWeaponMgr.hpp"

namespace metaforce {

void CWeaponMgr::Add(TUniqueId uid, EWeaponType type) {
  auto iter = x0_weapons.emplace(uid, rstl::reserved_vector<s32, 15>()).first;
  iter->second.resize(15);
  ++iter->second[size_t(type)];
}

void CWeaponMgr::Remove(TUniqueId uid) {
  const auto& weapon = x0_weapons[uid];

  s32 totalActive = 0;
  for (size_t i = 0; i < 10; ++i) {
    totalActive += weapon[i];
  }

  if (totalActive != 0) {
    return;
  }

  x0_weapons.erase(uid);
}

void CWeaponMgr::IncrCount(TUniqueId uid, EWeaponType type) {
  if (GetIndex(uid) < 0) {
    Add(uid, type);
  } else {
    x0_weapons[uid][size_t(type)]++;
  }
}

void CWeaponMgr::DecrCount(TUniqueId uid, EWeaponType type) {
  if (GetIndex(uid) < 0) {
    return;
  }

  auto& weapon = x0_weapons[uid];
  weapon[size_t(type)]--;
  if (weapon[size_t(type)] > 0) {
    return;
  }

  Remove(uid);
}

s32 CWeaponMgr::GetNumActive(TUniqueId uid, EWeaponType type) const {
  if (GetIndex(uid) < 0) {
    return 0;
  }

  return x0_weapons.at(uid)[size_t(type)];
}

s32 CWeaponMgr::GetIndex(TUniqueId uid) const {
  const auto iter = x0_weapons.find(uid);

  if (iter == x0_weapons.cend()) {
    return -1;
  }

  return s32(std::distance(x0_weapons.cbegin(), iter));
}

} // namespace metaforce
