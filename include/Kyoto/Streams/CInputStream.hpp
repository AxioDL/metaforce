#ifndef _CINPUTSTREAM
#define _CINPUTSTREAM

#include "types.h"

#include "stddef.h"

class CInputStream;
template < typename T >
struct TType {};
template < typename T >
T cinput_stream_helper(const TType< T >& type, CInputStream& in);

template < typename T >
inline TType< T > TGetType(const T&) {
  return TType< T >();
}

class CInputStream {
public:
  CInputStream(int len);
  CInputStream(const void* ptr, int len, bool owned);
  virtual ~CInputStream();
  virtual size_t Read(void* dest, size_t len) = 0;

  float ReadFloat();
  double ReadDouble();
  u64 ReadLongLong();
  uint ReadLong();
  ushort ReadShort();
  bool ReadBool();
  char ReadChar();
  uint ReadBits(uint len);
  size_t ReadBytes(void* dest, size_t len);
  void Get(void* dest, unsigned long len);

  template < typename T >
  T Get(const TType< T >& type = TType< T >()) {
    return cinput_stream_helper(TType< T >(), *this);
  }

  bool ReadPackedBool() { return ReadBits(1) != 0; }

  int ReadInt32() { return Get< int >(); }
  ushort ReadUint16() { return Get< ushort >(); }
  short ReadInt16() { return Get< short >(); }

  uint GetBlockOffset() const { return mBlockOffset; }
  const uint GetReadPosition() const { return mReadPosition; }

private:
  bool GrabAnotherBlock();
  bool InternalReadNext();

  uint mBlockOffset;
  uint mBlockLen;
  uint mLen;
  uchar* mPtr;
  bool mOwned;
  uint mReadPosition;
  uint mBitWord;
  uint mBitOffset;
};

template < typename T >
inline T cinput_stream_helper(const TType< T >& type, CInputStream& in) {
  return T(in);
}
template <>
inline bool cinput_stream_helper(const TType< bool >& type, CInputStream& in) {
  return in.ReadBool();
}
template <>
inline char cinput_stream_helper(const TType< char >& type, CInputStream& in) {
  return in.ReadChar();
}

template <>
inline unsigned char cinput_stream_helper(const TType< unsigned char >& type, CInputStream& in) {
  return in.ReadChar();
}

template <>
inline schar cinput_stream_helper(const TType< schar >& type, CInputStream& in) {
  return in.ReadChar();
}

template <>
inline int cinput_stream_helper(const TType< int >& type, CInputStream& in) {
  return in.ReadLong();
}
template <>
inline uint cinput_stream_helper(const TType< uint >& type, CInputStream& in) {
  return in.ReadLong();
}
template <>
inline unsigned long cinput_stream_helper(const TType< unsigned long >& type, CInputStream& in) {
  return in.ReadLong();
}
template <>
inline float cinput_stream_helper(const TType< float >& type, CInputStream& in) {
  return in.ReadFloat();
}
template <>
inline short cinput_stream_helper(const TType< short >& type, CInputStream& in) {
  return in.ReadShort();
}
template <>
inline ushort cinput_stream_helper(const TType< ushort >& type, CInputStream& in) {
  return in.ReadShort();
}

// rstl
#include "rstl/pair.hpp"
template < typename L, typename R >
inline rstl::pair< L, R >::pair(CInputStream& in)
: first(in.Get(TGetType(first))), second(in.Get(TGetType(second))) {}

#include "rstl/vector.hpp"
template < typename T, typename Alloc >
inline rstl::vector< T, Alloc >::vector(CInputStream& in, const Alloc& allocator)
: mCount(0), mCapacity(0), mItems(nullptr) {
  int count = in.Get(TGetType(0));
  reserve(count);
  for (int i = 0; i < count; i++) {
    push_back(in.Get< T >());
  }
}

#include "rstl/reserved_vector.hpp"
template < typename T, int N >
inline rstl::reserved_vector< T, N >::reserved_vector(CInputStream& in)
: mCount(in.Get(TGetType(0))) {
  for (int i = 0; i < mCount; i++) {
    construct(&data()[i], in.Get(TType< T >()));
  }
}

#include "rstl/set.hpp"
template < typename T, typename P, bool IsMulti, typename S, typename Cmp, typename Alloc >
inline rstl::red_black_tree< T, P, IsMulti, S, Cmp, Alloc >::red_black_tree(
    CInputStream& in, const S& selector, const Cmp& cmp, const Alloc& alloc)
: mSelector(selector), mCmp(cmp), mAllocator(alloc), mCount(0) {
  const int count = in.Get< int >();
  for (int i = 0; i < count; ++i) {
    insert(in.Get< P >());
  }
}

template < typename T, typename Cmp, typename Alloc >
inline rstl::set< T, Cmp, Alloc >::set(CInputStream& in, const Cmp& cmp, const Alloc& alloc)
: rep_type(in, identity< T >(), cmp, alloc) {}

#endif // _CINPUTSTREAM
