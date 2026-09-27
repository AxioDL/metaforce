#ifndef _CVPARAMTRANSFER
#define _CVPARAMTRANSFER

#include "Kyoto/IObj.hpp"
#include <rstl/rc_ptr.hpp>

class IVParamObj : public IObj {};

template < typename T >
class TObjOwnerParam : public IVParamObj {
public:
  ~TObjOwnerParam() {}
  TObjOwnerParam(const T& data) : mData(data) {}
  const T& GetData() const { return mData; }
  const T& operator*() const { return GetData(); }

private:
  T mData;
};

class CVParamTransfer {
public:
  CVParamTransfer() {}
  template < typename T >
  CVParamTransfer(TObjOwnerParam< T >* obj) : mObj(obj) {}
  static CVParamTransfer Null() { return CVParamTransfer(); }

  const IVParamObj& operator*() const { return *mObj; }
  rstl::rc_ptr< IVParamObj > mObj;
};

#endif // _CVPARAMTRANSFER
