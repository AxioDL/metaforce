#ifndef _CAUDIOGROUPSET
#define _CAUDIOGROUPSET

#include "Kyoto/TToken.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/string.hpp"

#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"

class CInputStream;
class CAudioGrpSetLoc {
public:
  CAudioGrpSetLoc(const rstl::auto_ptr< uchar >& data, int length);
  const rstl::string& GetBaseDirName() const { return mBaseDirName; }
  const rstl::string& GetGroupSetName() const { return mGroupSetName; }
  void FreeSampleBuffer();
  int AramUsage() const { return mAramSize; }
  void* GetPool() const { return mPool; }
  void* GetProject() const { return mProject; }
  void* GetSample() const { return mSamples; }
  void* GetSampleDir() const { return mSampleDir; }

private:
  uint ReadHeader(void* ptr, uint len, uint& readPosition) {
    CMemoryInStream in(ptr, len);
    mBaseDirName = rstl::string(in);
    mGroupSetName = rstl::string(in);
    const uint ret = in.ReadLong();
    readPosition = in.GetReadPosition();
    return ret;
  }
  rstl::auto_ptr< uchar > mData;
  rstl::auto_ptr< uchar > mGroupData;
  rstl::string mBaseDirName;
  rstl::string mGroupSetName;
  int mAramSize;
  uchar* mPool;
  uchar* mProject;
  uchar* mSampleDir;
  uchar* mSamples;
};

class CAudioGroupSet {
public:
  CAudioGroupSet(const TLockedToken< CAudioGrpSetLoc >& group);
  ~CAudioGroupSet();
  void Reload();
  void FreeSampleBuffer();
  void FreeAllBuffers();
  const rstl::string& GetName() const { return mGroupSetName; }
  int AramUsage() { return mGroupSetTok.data()->AramUsage(); }
  void* GetProjBuffer() const { return mGroupSetTok.data()->GetProject(); }
  void* GetSampleBuffer() const { return mGroupSetTok.data()->GetSample(); }
  void* GetSDirBuffer() const { return mGroupSetTok.data()->GetSampleDir(); }
  void* GetPoolBuffer() const { return mGroupSetTok.data()->GetPool(); }

private:
  rstl::string mBaseDir;
  rstl::string mGroupSetName;
  rstl::optional_object< TLockedToken< CAudioGrpSetLoc > > mGroupSetTok;
};

CHECK_SIZEOF(CAudioGrpSetLoc, 0x44)
CHECK_SIZEOF(CAudioGroupSet, 0x30)

#endif // _CAUDIOGROUPSET
