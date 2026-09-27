#ifndef _CFACTORYMGR
#define _CFACTORYMGR

#include "types.h"

#include "Kyoto/CFactoryFnReturn.hpp"
#include "rstl/map.hpp"

class CInputStream;
class CVParamTransfer;
class SObjectTag;

typedef const CFactoryFnReturn (*FFactoryFunc)(const SObjectTag&, CInputStream&,
                                               const CVParamTransfer&);
typedef const CFactoryFnReturn (*FMemFactoryFunc)(const SObjectTag&, const rstl::auto_ptr< uchar >&,
                                                  int, const CVParamTransfer&);

class CFactoryMgr {
public:
  CFactoryMgr();
  ~CFactoryMgr();

  void AddFactory(uint type, FFactoryFunc factory);
  void AddFactory(uint type, FMemFactoryFunc factory);
  bool CanMakeMemory(const SObjectTag& tag) const;
  rstl::auto_ptr< IObj > MakeObject(const SObjectTag& tag, CInputStream& in,
                                    const CVParamTransfer& params);
  rstl::auto_ptr< IObj > MakeObjectFromMemory(const SObjectTag& tag, void* buffer, int size,
                                              bool compressed, const CVParamTransfer& params);

  static uint FourCCToTypeIdx(uint fcc);
  static uint TypeIdxToFourCC(uint idx);

private:
  rstl::map< int, FFactoryFunc > mFactories;
  rstl::map< int, FMemFactoryFunc > mMemFactories;
};
CHECK_SIZEOF(CFactoryMgr, 0x28)

#endif // _CFACTORYMGR
