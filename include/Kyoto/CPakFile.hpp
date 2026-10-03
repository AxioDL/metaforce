#ifndef _CPAKFILE
#define _CPAKFILE

#include "types.h"

#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/SObjectTag.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/pair.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CDvdRequest;
class CMemoryInStream;

class CPakFile {
public:
  enum EAsyncPhase { kAP_Warmup, kAP_InitialHeaderLoad, kAP_DataLoad, kAP_Loaded };
// Resource entries store a four-byte ID followed by six packed metadata bytes.
#pragma pack(push, 2)
  struct SResInfo {
    CAssetId mId;
    uchar mData[6];

    SResInfo(uint id, uint fourCC, uint offset, uint size, uint flags);

    uint GetType() const;
    uint GetOffset() const;
    uint GetSize() const;
    bool IsCompressed() const;

    CAssetId GetId() const { return mId; }
    bool operator<(const SResInfo& other) const { return mId < other.mId; }
  };
#pragma pack(pop)

  CPakFile(const rstl::string& filename, const bool buildDepList, const bool worldPak);
  ~CPakFile();

  CDvdFile& DvdFile() { return mFile; }
  const CDvdFile& GetDvdFile() const { return mFile; }

  void AsyncIdle();
  bool IsWorldPak() const { return mWorldPak; }
  bool IsCompletelyLoaded() const { return mAsyncLoadPhase == kAP_Loaded; }
  void EnsureWorldPakReady();
  void sub_8036742c();

  rstl::vector< rstl::pair< rstl::string, SObjectTag > >& NameList() { return mNameList; }
  const rstl::vector< CAssetId >* GetDepList() const;
  const SObjectTag* GetResIdByName(const char* name) const;
  const SResInfo* GetResInfo(uint id) const;
  const SResInfo* GetResInfoForLoadDirectionless(uint id);
  const SResInfo* GetResInfoForLoadPreferForward(uint id);
  uint GetFakeStaticSize() const;

  bool IsARAMPak() const { return mAramFile; }
  bool IsStashedInARAM() const { return mStashedInARAM; }

  const rstl::vector< rstl::pair< rstl::string, SObjectTag > >& GetStringToObjectList() const {
    return mNameList;
  }

private:
  void Warmup();
  void InitialHeaderLoad();
  void DataLoad();
  void LoadResourceTable(CMemoryInStream& in);

  CDvdFile mFile;
  bool mBuildDepList : 1;
  bool mAramFile : 1;
  bool mWorldPak : 1;
  bool mStashedInARAM : 1;
  EAsyncPhase mAsyncLoadPhase;
  rstl::auto_ptr< CDvdRequest > mDvdReq;
  rstl::vector< uchar, rstl::aligned_allocator > mHeaderData;
  uint mResTableOffset;
  uint mResTableCount;
  void* mAramBase;
  rstl::vector< rstl::pair< rstl::string, SObjectTag > > mNameList;
  rstl::vector< CAssetId > mDepList;
  rstl::vector< SResInfo > mResList;
  int mCurrentSeek;
};
#if VERSION < VERSION_R3IJ_00
CHECK_SIZEOF(CPakFile, 0x88)
#endif
NESTED_CHECK_SIZEOF(CPakFile, SResInfo, 0xa)

#endif // _CPAKFILE
