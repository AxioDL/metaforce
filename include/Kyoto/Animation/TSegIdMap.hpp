#ifndef MP1_TSEGIDMAP_HPP
#define MP1_TSEGIDMAP_HPP

#include <rstl/reserved_vector.hpp>

template < typename T >
class TSegIdMap {
public:
#if VERSION >= VERSION_GM8P_00
  TSegIdMap(uchar count);
#else
  inline TSegIdMap(uchar count);
#endif

  TSegIdMap(CInputStream& in)
  : mBoneCount(0)
  , mCapacity(in.Get< uint >())
  , mIndirectionMap(100, rstl::pair< char, char >(-1, -1))
  , mNodes(reinterpret_cast< T* >(rs_new char[mCapacity * sizeof(T)]))
  , mCurPrevBone(0) {
    for (int i = 0; i < mCapacity; i++) {
      CSegId seg(in);
      T node(in);
      insert(seg, node);
    }
  }

  T& operator[](const CSegId& id) {
    int index = mIndirectionMap[id.val()].second;
    return AccessElement(static_cast< uchar >(index));
  }
  const T& operator[](const CSegId& id) const {
    return AccessElement(static_cast< uchar >(mIndirectionMap[id.val()].second));
  }

  bool ContainsDataFor(const CSegId& id) const {
    return mIndirectionMap[id.val()] != rstl::pair< char, char >(-1, -1);
  }

  T& AccessElement(int index) {
    T* ptr = mNodes + index;
    return *ptr;
  }
  const T& AccessElement(int index) const { return mNodes[index]; }

  void insert(const CSegId& id, const T& value);

  ~TSegIdMap();

private:
  char mBoneCount;
  char mCapacity;
  rstl::reserved_vector< rstl::pair< char, char >, 100 > mIndirectionMap;
  T* mNodes;
  char mCurPrevBone;
};

template < typename T >
TSegIdMap< T >::TSegIdMap(uchar count)
: mBoneCount(0)
, mCapacity(count)
, mIndirectionMap(100, rstl::pair< char, char >(-1, -1))
, mNodes(nullptr)
, mCurPrevBone(0) {
  mNodes = reinterpret_cast< T* >(rs_new uchar[count * sizeof(T)]);
}

template < typename T >
TSegIdMap< T >::~TSegIdMap() {
  CSegId id(mCurPrevBone);
  while (id != CSegId::Null()) {
    (*this)[id].~T();
    id = CSegId(mIndirectionMap[id.val()].first);
  }

  delete[] reinterpret_cast< char* >(mNodes);
}

template < typename T >
void TSegIdMap< T >::insert(const CSegId& id, const T& value) {
  T* node = &mNodes[mBoneCount];
  uchar rawId = id.val();
  new (node) T(value);
  mIndirectionMap[rawId] = rstl::pair< char, char >(mCurPrevBone, mBoneCount);
  mCurPrevBone = rawId;
  ++mBoneCount;
}

typedef TSegIdMap< uchar > unk_TSegIdMap;
CHECK_SIZEOF(unk_TSegIdMap, 0xd8)

#endif // MP1_TSEGIDMAP_HPP
