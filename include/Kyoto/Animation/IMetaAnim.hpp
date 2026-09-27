#ifndef _IMETAANIM
#define _IMETAANIM

#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "Kyoto/Animation/CCharAnimTime.hpp"

#include <rstl/optional_object.hpp>
#include <rstl/rc_ptr.hpp>
#include <rstl/set.hpp>
#include <rstl/string.hpp>

enum EMetaAnimType {
  kMAT_Play,
  kMAT_Blend,
  kMAT_PhaseBlend,
  kMAT_Random,
  kMAT_Sequence,
};

class CPrimitive;
class CCharAnimTime;
class IAnimReader;
class CAnimSysContext;

class CPreAdvanceIndicator {
  bool mIsTime;
  CCharAnimTime mTime;
  char mString[50];

public:
  explicit CPreAdvanceIndicator(const CCharAnimTime& time) : mIsTime(true), mTime(time) {}
  explicit CPreAdvanceIndicator(const char* string);
  bool IsTime() const;
  const CCharAnimTime& GetTime() const;

  bool IsString() const;
  const char* GetString() const;
};
CHECK_SIZEOF(CPreAdvanceIndicator, 0x40)

class CMetaAnimTreeBuildOrders {
public:
  static CMetaAnimTreeBuildOrders NoSpecialOrders();
  static CMetaAnimTreeBuildOrders PreAdvanceForAll(const CPreAdvanceIndicator& ind);

  rstl::optional_object< CPreAdvanceIndicator > mRecursiveAdvance;
  rstl::optional_object< CPreAdvanceIndicator > mSingleAdvance;
};

class IMetaAnim {
public:
  virtual ~IMetaAnim() = 0;

  virtual rstl::ncrc_ptr< CAnimTreeNode >
  GetAnimationTree(const CAnimSysContext& animSys, const CMetaAnimTreeBuildOrders& orders) const;
  virtual void GetUniquePrimitives(rstl::set< CPrimitive >& primsOut) const = 0;
  virtual EMetaAnimType GetType() const = 0;
  virtual void WriteAnimData(COutputStream& out) const = 0;
  virtual rstl::ncrc_ptr< CAnimTreeNode >
  VGetAnimationTree(const CAnimSysContext& animSys,
                    const CMetaAnimTreeBuildOrders& orders) const = 0;

  void PutTo(COutputStream&) const;

  static void AdvanceAnim(IAnimReader& anim, const CCharAnimTime& dt);
  static CCharAnimTime GetTime(const CPreAdvanceIndicator& ind, const IAnimReader& anim);
};

inline IMetaAnim::~IMetaAnim() {}

#endif // _IMETAANIM
