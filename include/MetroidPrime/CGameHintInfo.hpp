#ifndef _CGAMEHINTINFO
#define _CGAMEHINTINFO

#include "types.h"

#include "Kyoto/SObjectTag.hpp"

#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CInputStream;

class CGameHintInfo {
public:
  static const float skHintTextTime;

  struct SHintLocation {
    CAssetId mMlvlId;
    CAssetId mMreaId;
    TAreaId mAreaId;
    CAssetId mStringId;

    SHintLocation(CInputStream& in);
  };

  class CGameHint {
  public:
    CGameHint(CInputStream& in, int version);

    const rstl::string& GetName() const { return mName; }
    float GetImmediateTime() const { return mImmediateTime; }
    float GetNormalTime() const { return mNormalTime; }
    CAssetId GetStringId() const { return mStringId; }
    float GetTextTime() const { return mTextTime; }
    const rstl::vector< SHintLocation >& GetLocations() const { return mLocations; }

    rstl::string mName;
    float mImmediateTime;
    float mNormalTime;
    CAssetId mStringId;
    float mTextTime;
    rstl::vector< SHintLocation > mLocations;

  private:
    void ReadLocations(CInputStream& in);
  };

  CGameHintInfo(CInputStream& in, int version);

  const rstl::vector< CGameHint >& GetHints() const { return mHints; }

  static int FindHintIndex(const rstl::string& name);

private:
  rstl::vector< CGameHint > mHints;
};

CHECK_SIZEOF(CGameHintInfo, 0x10)
NESTED_CHECK_SIZEOF(CGameHintInfo, SHintLocation, 0x10)
NESTED_CHECK_SIZEOF(CGameHintInfo, CGameHint, 0x30)

#endif // _CGAMEHINTINFO
