#ifndef _CSPAWNSYSTEMKEYFRAMEDATA
#define _CSPAWNSYSTEMKEYFRAMEDATA

#include "Kyoto/CToken.hpp"

#include <rstl/optional_object.hpp>
#include <rstl/pair.hpp>
#include <rstl/vector.hpp>

class CInputStream;
class CSimplePool;
class CSpawnSystemKeyframeData {
public:
  class CSpawnSystemKeyframeInfo {
  public:
    CSpawnSystemKeyframeInfo(CInputStream& in);
    ~CSpawnSystemKeyframeInfo() {
      if (x10_token) {
        CToken& token = *x10_token;
        token.Unlock();
      }
    }
    void LoadToken(CSimplePool* pool);
    rstl::optional_object< CToken > GetToken() const { return x10_token; }

  private:
    uint x0_id;
    uint x4;
    uint x8;
    uint xc;
    rstl::optional_object< CToken > x10_token;
  };

  CSpawnSystemKeyframeData(CInputStream& in);
  ~CSpawnSystemKeyframeData() {}

  void LoadAllSpawnedSystemTokens(CSimplePool* pool);
  rstl::vector< CSpawnSystemKeyframeInfo >& GetSpawnedSystemsAtFrame(uint frame);

private:
  int mUnknown1;
  int mUnknown2;
  int mEndFrame;
  int mUnknown3;
  rstl::vector< rstl::pair< uint, rstl::vector< CSpawnSystemKeyframeInfo > > > mFrames;
};

#endif // _CSPAWNSYSTEMKEYFRAMEDATA
