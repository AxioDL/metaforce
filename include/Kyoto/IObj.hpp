#ifndef _IOBJ
#define _IOBJ

#include "types.h"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "rstl/auto_ptr.hpp"

extern const SObjectTag gkInvalidObjectTag;

class IObj {
public:
  virtual ~IObj() = 0;
};

inline IObj::~IObj() {}

class CObjOwnerDerivedFromIObjUntyped : public IObj {
public:
  ~CObjOwnerDerivedFromIObjUntyped() = 0;
  template < typename T >
  CObjOwnerDerivedFromIObjUntyped(T* obj) : m_objPtr(obj) {}
  template < typename T >
  CObjOwnerDerivedFromIObjUntyped(const rstl::auto_ptr< T >& obj) : m_objPtr(obj.release()) {}

  void* GetContents() const { return m_objPtr; }

protected:
  void* m_objPtr;
};

inline CObjOwnerDerivedFromIObjUntyped::~CObjOwnerDerivedFromIObjUntyped() {}

template < typename T >
class TObjOwnerDerivedFromIObj : public CObjOwnerDerivedFromIObjUntyped {
public:
  TObjOwnerDerivedFromIObj(T* obj) : CObjOwnerDerivedFromIObjUntyped(obj) {}
  ~TObjOwnerDerivedFromIObj() {
    if (Owned()) {
      delete Owned();
    }
  }

  T* Owned() { return static_cast< T* >(m_objPtr); }

#if VERSION >= VERSION_R3IJ_00
  typedef rstl::auto_ptr< IObj > TOwnerPtr;
#else
  typedef rstl::auto_ptr< TObjOwnerDerivedFromIObj< T > > TOwnerPtr;
#endif

  static TOwnerPtr GetNewDerivedObject(T* obj) {
#if VERSION >= VERSION_R3IJ_00
    return rstl::auto_ptr< TObjOwnerDerivedFromIObj< T > >(
        rs_new TObjOwnerDerivedFromIObj< T >(obj));
#else
    return rs_new TObjOwnerDerivedFromIObj< T >(obj);
#endif
  }

  static TOwnerPtr GetNewDerivedObject(const rstl::auto_ptr< T >& obj) {
#if VERSION >= VERSION_R3IJ_00
    return rstl::auto_ptr< TObjOwnerDerivedFromIObj< T > >(
        rs_new TObjOwnerDerivedFromIObj< T >(obj));
#else
    return rs_new TObjOwnerDerivedFromIObj< T >(obj);
#endif
  }
  
private:
  TObjOwnerDerivedFromIObj(const rstl::auto_ptr< T >& obj) : CObjOwnerDerivedFromIObjUntyped(obj) {}
};

#endif // _IOBJ
