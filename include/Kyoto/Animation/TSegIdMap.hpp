#ifndef MP1_TSEGIDMAP_HPP
#define MP1_TSEGIDMAP_HPP

#include <rstl/reserved_vector.hpp>

template < typename T >
class TSegIdMap {
public:
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
  TSegIdMap(uchar count);
#else
  inline TSegIdMap(uchar count);
#endif

  TSegIdMap(CInputStream& in)
  : x0_boneCount(0)
  , x1_capacity(in.Get< uint >())
  , x8_indirectionMap(100, rstl::pair< char, char >(-1, -1))
  , xd0_nodes(reinterpret_cast< T* >(rs_new char[x1_capacity * sizeof(T)]))
  , xd4_curPrevBone(0) {
    for (int i = 0; i < x1_capacity; i++) {
      CSegId seg(in);
      T node(in);
      insert(seg, node);
    }
  }

  T& operator[](const CSegId& id) {
    int index = x8_indirectionMap[id.val()].second;
    return AccessElement(static_cast< uchar >(index));
  }
  const T& operator[](const CSegId& id) const {
    return AccessElement(static_cast< uchar >(x8_indirectionMap[id.val()].second));
  }

  bool ContainsDataFor(const CSegId& id) const {
    return x8_indirectionMap[id.val()] != rstl::pair< char, char >(-1, -1);
  }

  T& AccessElement(int index) {
    T* ptr = xd0_nodes + index;
    return *ptr;
  }
  const T& AccessElement(int index) const { return xd0_nodes[index]; }

  void insert(const CSegId& id, const T& value);

  ~TSegIdMap();

private:
  char x0_boneCount;
  char x1_capacity;
  rstl::reserved_vector< rstl::pair< char, char >, 100 > x8_indirectionMap;
  T* xd0_nodes;
  char xd4_curPrevBone;
};

template < typename T >
TSegIdMap< T >::TSegIdMap(uchar count)
: x0_boneCount(0)
, x1_capacity(count)
, x8_indirectionMap(100, rstl::pair< char, char >(-1, -1))
, xd0_nodes(nullptr)
, xd4_curPrevBone(0) {
  xd0_nodes = reinterpret_cast< T* >(rs_new uchar[count * sizeof(T)]);
}

template < typename T >
TSegIdMap< T >::~TSegIdMap() {
  CSegId id(xd4_curPrevBone);
  while (id != CSegId::Null()) {
    (*this)[id].~T();
    id = CSegId(x8_indirectionMap[id.val()].first);
  }

  delete[] reinterpret_cast< char* >(xd0_nodes);
}

template < typename T >
void TSegIdMap< T >::insert(const CSegId& id, const T& value) {
  T* node = &xd0_nodes[x0_boneCount];
  uchar rawId = id.val();
  new (node) T(value);
  x8_indirectionMap[rawId] = rstl::pair< char, char >(xd4_curPrevBone, x0_boneCount);
  xd4_curPrevBone = rawId;
  ++x0_boneCount;
}

typedef TSegIdMap< uchar > unk_TSegIdMap;
CHECK_SIZEOF(unk_TSegIdMap, 0xd8)

#endif // MP1_TSEGIDMAP_HPP
