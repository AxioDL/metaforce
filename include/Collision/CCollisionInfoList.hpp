#ifndef _CCOLLISIONINFOLIST
#define _CCOLLISIONINFOLIST

#include "Collision/CCollisionInfo.hpp"

#include "rstl/reserved_vector.hpp"

class CCollisionInfoList {
  rstl::reserved_vector< CCollisionInfo, 32 > mList;

public:
  typedef rstl::reserved_vector< CCollisionInfo, 32 >::iterator iterator;
  typedef rstl::reserved_vector< CCollisionInfo, 32 >::const_iterator const_iterator;

  void Add(const CCollisionInfo& info, bool swap) {
    if (mList.size() == 32) {
      return;
    }
    if (!swap) {
      mList.push_back(info);
    } else {
      mList.push_back(info.GetSwapped(CCollisionInfo::kSM_Swap));
    }
  }
  void Clear() { mList.clear(); }
  int GetCount() const { return mList.size(); }
  rstl::reserved_vector< CCollisionInfo, 32 >& GetList() { return mList; }
  const rstl::reserved_vector< CCollisionInfo, 32 >& GetList() const { return mList; }
  CCollisionInfo& operator[](int idx) { return mList[idx]; }
  const CCollisionInfo& operator[](int idx) const { return mList[idx]; }
  void Swap(const int idx) {
    for (int i = idx; i < GetCount(); ++i) {
      mList[i].Swap();
    }
  }

  iterator End() { return mList.end(); }
  const_iterator End() const { return mList.end(); }
  iterator Begin() { return mList.begin(); }
  const_iterator Begin() const { return mList.begin(); }
};

#endif // _CCOLLISIONINFOLIST
