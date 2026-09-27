#ifndef _CRESLOADER
#define _CRESLOADER

#include "Kyoto/CPakFile.hpp"
#include "Kyoto/SObjectTag.hpp"

#include "types.h"

#include "rstl/list.hpp"
#include "rstl/string.hpp"

class CDvdRequest;

class CResLoader {
public:
  enum ECompressionType {
    kCompressionType_Uncompressed,
    kCompressionType_Compressed,
  };
  CResLoader();
  ~CResLoader();
  void MoveToCorrectLoadedList(const rstl::auto_ptr< CPakFile >& pak);
  bool CacheFromPak(const CPakFile& pak, CAssetId asset) const;
  bool CacheFromPakForLoad(CPakFile& pak, CAssetId asset);
  CPakFile* FindResourceForLoad(const SObjectTag& tag);
  CPakFile* FindResourceForLoad(const CAssetId asset);
  CPakFile* FindResource(const SObjectTag& tag);
  bool ResourceExists(CAssetId asset);
  bool ResourceExists(const SObjectTag& tag) const;
  void ClearCache();
  void AsyncIdlePakLoading();
  bool AreAllPaksLoaded() const;
  const SObjectTag* GetResourceIdByName(const char* name) const;
  FourCC GetResourceTypeById(CAssetId asset) const;
  uint ResourceSize(const SObjectTag& tag) const;
  ECompressionType GetResourceCompression(const SObjectTag& tag) const;
  void AddPakFileAsync(const rstl::string&, bool, bool);
  void RemovePakFile(const rstl::string&);
  CInputStream* LoadNewResourceSync(const SObjectTag& tag, char* extBuf);
  CInputStream* LoadResourceFromMemorySync(const SObjectTag& tag, const void* extBuf);
  void LoadMemResourceSync(const SObjectTag& tag, char** extBuf, int* len);
  CInputStream* LoadNewResourcePartSync(const SObjectTag& tag, int offset, int length,
                                        char* extBuf);
  CDvdRequest* LoadResourcePartAsync(const SObjectTag& tag, int offset, int length, char* extBuf);
  CDvdRequest* LoadResourceAsync(const SObjectTag& tag, char* extBuf);

  const rstl::vector< CAssetId >* GetTagListForFile(const rstl::string& pak) const;
  rstl::vector< rstl::pair< rstl::string, SObjectTag > > GetResourceIdToNameList() const;

  int GetPakCount() const;
  CPakFile* GetPakFile(int idx) const;

private:
  rstl::list< rstl::auto_ptr< CPakFile > > mAramList;
  rstl::list< rstl::auto_ptr< CPakFile > > mPakLoadedList;
  rstl::list< rstl::auto_ptr< CPakFile > > mPakLoadingList;
  rstl::list< rstl::auto_ptr< CPakFile > >::iterator mCurPak;
  mutable CAssetId mCachedResId;
  mutable const CPakFile::SResInfo* mCachedResInfo;
  bool mForwardSeek;
};

#endif // _CRESLOADER
