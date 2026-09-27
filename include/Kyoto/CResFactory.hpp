#ifndef _CRESFACTORY
#define _CRESFACTORY

#include "types.h"

#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/CResLoader.hpp"
#include "Kyoto/CVParamTransfer.hpp"
#include "Kyoto/IFactory.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/list.hpp"
#include "rstl/map.hpp"

class CDvdRequest;
struct z_stream_s;

class CResFactory : public IFactory {
public:
  ~CResFactory() override;
  rstl::auto_ptr< IObj > Build(const SObjectTag&, const CVParamTransfer&) override;
  void BuildAsync(const SObjectTag&, const CVParamTransfer&, IObj**) override;
  void CancelBuild(const SObjectTag&) override;

  bool CanBuild(const SObjectTag& tag) override { return mResLoader.ResourceExists(tag); }

  const SObjectTag* GetResourceIdByName(const char* name) const override {
    return mResLoader.GetResourceIdByName(name);
  }

  struct SLoadingData {
    SObjectTag mTag;
    rstl::auto_ptr< CDvdRequest > mDvdReq;
    IObj** mTarget;
    rstl::auto_ptr< uchar > mBuffer;
    rstl::auto_ptr< uchar > mDecompBuffer;
    rstl::auto_ptr< z_stream_s > mZip;
    int mSize;
    CResLoader::ECompressionType mCompression;
    CVParamTransfer mParams;

    SLoadingData(const SObjectTag& tag, CDvdRequest* request, IObj** target, void* buffer, int size,
                 CResLoader::ECompressionType compression, const CVParamTransfer& params);
    ~SLoadingData();
    bool PumpDecompression(uint time);
  };

  CResFactory();

  FourCC GetResourceTypeById(CAssetId id) { return GetResLoader().GetResourceTypeById(id); }

  rstl::vector< rstl::pair< rstl::string, SObjectTag > > GetResourceIdToNameList() const;

  uint ResourceSize(const SObjectTag& tag) { return mResLoader.ResourceSize(tag); }

  CDvdRequest* LoadResourceAsync(const SObjectTag& tag, void* buffer) {
    return mResLoader.LoadResourceAsync(tag, static_cast< char* >(buffer));
  }

  void AsyncIdle(uint time);

  CResLoader& GetResLoader() { return mResLoader; }
  CFactoryMgr& GetFactoryMgr() { return mFactoryMgr; }

  const rstl::vector< CAssetId >* GetTagListForFile(const rstl::string& pak) const {
    return mResLoader.GetTagListForFile(pak);
  }

  CInputStream* LoadResourceFromMemorySync(const SObjectTag& tag, const void* extBuf) {
    return mResLoader.LoadResourceFromMemorySync(tag, extBuf);
  }

private:
  typedef rstl::list< SLoadingData > LoadList;

  void AddToLoadList(const SLoadingData& data);
  void EraseFromLoadList(const LoadList::iterator& it);
  LoadList::iterator FindInLoadList(const SObjectTag& tag);
  rstl::auto_ptr< IObj > BuildSync(const SObjectTag& tag, const CVParamTransfer& params);
  bool PumpResource(const LoadList::iterator& it, uint time);

  CResLoader mResLoader;
  CFactoryMgr mFactoryMgr;
  LoadList mLoadList;
  rstl::map< SObjectTag, LoadList::iterator > mLoadMap;
  LoadList mCancelledList;
};
NESTED_CHECK_SIZEOF(CResFactory, SLoadingData, 0x38)
CHECK_SIZEOF(CResFactory, 0xc8)

extern CResFactory* gpResourceFactory;

#endif // _CRESFACTORY
