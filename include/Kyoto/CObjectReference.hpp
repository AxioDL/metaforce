#ifndef _COBJECTREFERENCE
#define _COBJECTREFERENCE

#include <Kyoto/CVParamTransfer.hpp>
#include <Kyoto/SObjectTag.hpp>
#include <rstl/auto_ptr.hpp>

class IObj;
class IObjectStore;
class CObjectReference {
public:
  CObjectReference(const rstl::auto_ptr< IObj >& obj);
  ~CObjectReference();
  CObjectReference(IObjectStore& store, const rstl::auto_ptr< IObj >& obj, const SObjectTag& tag,
                   CVParamTransfer xfer);

  bool IsLoaded() const { return mObject != nullptr; }

  void AddReference() { mRefCount++; }
  int RemoveReference();
  void Lock();
  void Unlock();
  IObj* GetObject();
  void Unload();
  void CancelLoad();
  bool IsLoading() const;
  const SObjectTag& GetTag() const { return mObjTag; }

private:
  short mRefCount : 16;
  short mLockCount : 15;
  short mLoading : 1;
  SObjectTag mObjTag;
  IObjectStore* mObjectStore;
  IObj* mObject;
  CVParamTransfer mParams;
};
#endif // _COBJECTREFERENCE
