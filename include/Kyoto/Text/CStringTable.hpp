#ifndef _CSTRINGTABLE
#define _CSTRINGTABLE

#include <stdint.h>

#include "types.h"

#include <rstl/auto_ptr.hpp>
#include <rstl/single_ptr.hpp>
#if TARGET_LITTLE_ENDIAN || WCHAR_MAX > 0xffff
#include "rstl/vector.hpp"
#endif

#include <Kyoto/CFactoryFnReturn.hpp>

class CInputStream;
class CResFactory;
class CDvdRequest;
class CStringTable {
public:
  struct SReloadData {
    uint mSize;
    rstl::single_ptr< CDvdRequest > mRequest;
    rstl::auto_ptr< uchar > mBuffer;

    SReloadData(CAssetId id, CResFactory& factory);
    ~SReloadData();
  };

  CStringTable(CInputStream& in);
#if VERSION >= VERSION_GM8P_00
  ~CStringTable();
#endif

  void Load(CInputStream& in);

  static void SetLanguage(int language);
  void Reload(CAssetId id, CResFactory& factory);
  void TryFinishReload();
  bool IsReloading() const;

  const wchar_t* GetString(int idx) const;
  int GetStringCount() const { return mStringCount; }

private:
  int mStringCount;
  rstl::single_ptr< uchar > mData;
#if VERSION >= VERSION_GM8P_00
  rstl::single_ptr< SReloadData > mReloadData;
#endif
#if TARGET_LITTLE_ENDIAN || WCHAR_MAX > 0xffff
  rstl::vector< rstl::vector< wchar_t > > mNativeStrings;
#endif
};

#if VERSION >= VERSION_GM8P_00
CHECK_SIZEOF(CStringTable, 0xc)
#else
CHECK_SIZEOF(CStringTable, 0x8)
#endif
NESTED_CHECK_SIZEOF(CStringTable, SReloadData, 0x10)

extern CStringTable* gpStringTable;

const CFactoryFnReturn FStringTableFactory(const SObjectTag& tag, CInputStream& in,
                                     const CVParamTransfer& xfer);

#endif // _CSTRINGTABLE
