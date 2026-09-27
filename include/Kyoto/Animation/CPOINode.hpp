#ifndef _CPOINODE
#define _CPOINODE

#include "types.h"

#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

enum EPOIType {
  kPT_Loop = 0,
  kPT_EmptyBool = 1,
  kPT_EmptyInt32 = 2,
  kPT_SoundInt32 = 4,
  kPT_Particle = 5,
  kPT_UserEvent = 6,
  kPT_RandRate = 7,
  kPT_Sound = 8,
};

class CInputStream;
class CPOINode {
public:
  CPOINode(const rstl::string& name, ushort type, const CCharAnimTime& time, int index, bool unique,
           float weight, int charIdx, int flags);
  CPOINode(CInputStream& in);
  virtual ~CPOINode() {}

  const rstl::string& GetString() const { return mName; }
  const EPOIType GetPoiType() const { return static_cast< EPOIType >(mType); }
  const CCharAnimTime& GetTime() const { return mTime; }
  void SetTime(const CCharAnimTime& time) { mTime = time; }
  const int GetIndex() const { return mIndex; }
  const bool GetSaveState() const { return mUnique; }
  const float GetWeight() const { return mWeight; }
  const int GetCharacterIndex() const { return mCharIdx; }
  const int GetFlags() const { return mFlags; }

  bool operator>(const CPOINode& other) const;
  bool operator<(const CPOINode& other) const;
  static int compare(const void* a, const void* b);

protected:
  ushort x4_;
  rstl::string mName;
  ushort mType;
  CCharAnimTime mTime;
  int mIndex;
  bool mUnique;
  float mWeight;
  int mCharIdx;
  int mFlags;
};
CHECK_SIZEOF(CPOINode, 0x38)

#endif // _CPOINODE
