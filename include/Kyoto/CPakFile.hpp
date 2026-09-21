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
    CAssetId x0_id;
    uchar x4_data[6];

    SResInfo(uint id, uint fourCC, uint offset, uint size, uint flags);

    uint GetType() const;
    uint GetOffset() const;
    uint GetSize() const;
    bool IsCompressed() const;

    CAssetId GetId() const { return x0_id; }
    bool operator<(const SResInfo& other) const { return x0_id < other.x0_id; }
  };
#pragma pack(pop)

  CPakFile(const rstl::string& filename, const bool buildDepList, const bool worldPak);
  ~CPakFile();

  CDvdFile& DvdFile() { return x0_file; }
  const CDvdFile& GetDvdFile() const { return x0_file; }

  void AsyncIdle();
  bool IsWorldPak() const { return x28_26_worldPak; }
  bool IsCompletelyLoaded() const { return x2c_asyncLoadPhase == kAP_Loaded; }
  void EnsureWorldPakReady();
  void sub_8036742c();

  rstl::vector< rstl::pair< rstl::string, SObjectTag > >& NameList() { return x54_nameList; }
  const rstl::vector< CAssetId >* GetDepList() const;
  const SObjectTag* GetResIdByName(const char* name) const;
  const SResInfo* GetResInfo(uint id) const;
  const SResInfo* GetResInfoForLoadDirectionless(uint id);
  const SResInfo* GetResInfoForLoadPreferForward(uint id);
  uint GetFakeStaticSize() const;

  bool IsARAMPak() const { return x28_25_aramFile; }
  bool IsStashedInARAM() const { return x28_27_stashedInARAM; }

  const rstl::vector< rstl::pair< rstl::string, SObjectTag > >& GetStringToObjectList() const {
    return x54_nameList;
  }

private:
  void Warmup();
  void InitialHeaderLoad();
  void DataLoad();
  void LoadResourceTable(CMemoryInStream& in);

  CDvdFile x0_file;
  bool x28_24_buildDepList : 1;
  bool x28_25_aramFile : 1;
  bool x28_26_worldPak : 1;
  bool x28_27_stashedInARAM : 1;
  EAsyncPhase x2c_asyncLoadPhase;
  rstl::auto_ptr< CDvdRequest > x30_dvdReq;
  rstl::vector< uchar, rstl::aligned_allocator > x38_headerData;
  uint x48_resTableOffset;
  uint x4c_resTableCount;
#if !defined(TARGET_PC)
  void* x50_aramBase;
#endif
  rstl::vector< rstl::pair< rstl::string, SObjectTag > > x54_nameList;
  rstl::vector< CAssetId > x64_depList;
  rstl::vector< SResInfo > x74_resList;
  int x84_currentSeek;
};
CHECK_SIZEOF(CPakFile, 0x88)
NESTED_CHECK_SIZEOF(CPakFile, SResInfo, 0xa)

#endif // _CPAKFILE
