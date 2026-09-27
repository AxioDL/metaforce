#define WEAK_DELETE_HACK
#include "MetroidPrime/CWeaponMgr.hpp"

CWeaponMgr::CWeaponMgr() {}

void CWeaponMgr::Remove(TUniqueId uid) {
  rstl::map< TUniqueId, Vec >::iterator iter = mWeapons.find(uid);
  if (iter != mWeapons.end()) {
    mWeapons.erase(iter);
  }
}

void CWeaponMgr::IncrCount(TUniqueId uid, EWeaponType type) {
  Vec* vec = GetIndex(uid);
  if (vec == nullptr) {
    Add(uid, type);
  } else {
    (*vec)[type]++;
  }
}

void CWeaponMgr::DecrCount(TUniqueId uid, EWeaponType type) {
  Vec* vecP = GetIndex(uid);
  if (!vecP) {
    return;
  }

  Vec& vec = *vecP;
  vec[type]--;

  bool found = true;
  rstl::reserved_vector< int, 15 >::iterator vit = vec.begin(), end = vec.end();
  for (; vit != end; ++vit) {
    if (*vit > 0) {
      found = false;
      break;
    }
  }
  if (found) {
    Remove(uid);
  }
}

int CWeaponMgr::GetNumActive(TUniqueId uid, EWeaponType type) const {
  Vec* vecP = GetIndex(uid);
  if (vecP) {
    return (*vecP)[type];
  } else {
    return 0;
  }
}
