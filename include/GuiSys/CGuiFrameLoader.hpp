#ifndef _CGUIFRAMELOADER
#define _CGUIFRAMELOADER

#include "Kyoto/SObjectTag.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/single_ptr.hpp"

class CDvdRequest;
class CGuiFrame;
class CResFactory;
class IObjectStore;

class CGuiFrameLoader {
public:
  CGuiFrameLoader(CAssetId id, CResFactory& factory, IObjectStore& store);
  ~CGuiFrameLoader();
  bool CheckLoadComplete();
  CGuiFrame* TryBuildFrame();

private:
  SObjectTag mTag;
  IObjectStore& mStore;
  uint mSize;
  rstl::auto_ptr< char > mBuffer;
  rstl::single_ptr< CDvdRequest > mRequest;
};
CHECK_SIZEOF(CGuiFrameLoader, 0x1c)

#endif // _CGUIFRAMELOADER
