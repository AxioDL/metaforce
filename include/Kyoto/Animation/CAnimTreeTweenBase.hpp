#ifndef _CANIMTREETWEENBASE
#define _CANIMTREETWEENBASE

#include "Kyoto/Animation/CAnimTreeDoubleChild.hpp"

class CAnimTreeTweenBase : public CAnimTreeDoubleChild {
  static s32 sAdvancementDepth;

public:
  static const int kBlendRoot_Offset;
  static const int kBlendRoot_Rotation;
  CAnimTreeTweenBase(const bool, const rstl::ncrc_ptr< CAnimTreeNode >& a,
                     const rstl::ncrc_ptr< CAnimTreeNode >& b, int, const rstl::string& name);
  ~CAnimTreeTweenBase() override;

  virtual void SetBlendingWeight(float w) = 0;

  float GetBlendingWeight() const;
  bool CharacterSpaceBlend() const { return x20_24_characterSpaceBlend != 0; }
  int GetBlendRoot() const { return x1c_flags; }

  void VGetWeightedReaders(
      float w, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const override;
  float VGetRightChildWeight() const override;

  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                           const CCharAnimTime& time) const override;
  bool VHasOffset(const CSegId& seg) const override;
  CVector3f VGetOffset(const CSegId& seg) const override;
  CQuaternion VGetRotation(const CSegId& seg) const override;

  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > VSimplified() override;
  virtual rstl::optional_object< rstl::ownership_transfer< IAnimReader > > VReverseSimplified();
  virtual float VGetBlendingWeight() const = 0;

  static bool ShouldCullTree();
  static void IncAdvancementDepth() { sAdvancementDepth++; }
  static void DecAdvancementDepth() { sAdvancementDepth--; }

protected:
  int x1c_flags;
  s32 x20_24_characterSpaceBlend : 1;
#if NONMATCHING
  u32 x20_25_cullSelector : 2;
#else
  s32 x20_25_cullSelector : 2;
#endif
};

CHECK_SIZEOF(CAnimTreeTweenBase, 0x24)

#endif // _CANIMTREETWEENBASE
