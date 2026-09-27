#ifndef _CPVSAREASET
#define _CPVSAREASET

#include "Kyoto/PVS/CPVSVisOctree.hpp"
class CPVSVisSet;
class CInputStream;

class CPVSAreaSet {
public:
  CPVSAreaSet(const int, const int, const int, const int, const int, const int, const char*,
              const char*, const char*);

  const CPVSVisOctree& GetOctree() const { return mOctree; }

  static rstl::auto_ptr< CPVSAreaSet > MakeAreaSet(const char* data, int len);
  CPVSVisSet GetLightSet(int) const;
  int GetEntityIdByIndex(uint idx) const;

  CPVSVisOctree& GetVisOctree() const;
  int GetNumFeatures() const { return x0_; }
  int GetNumActors() const { return xc_; }
  int GetNum2ndLights() const { return x8_; }
  bool Has2ndLayerLights() const { return x8_ != 0; }

private:
  int x0_;
  int x4_;
  int x8_;
  int xc_;
  int mLeafSize;
  int x14_;
  const char* x18_;
  const char* mLightLeaves;
  mutable CPVSVisOctree mOctree;
};

#endif // _CPVSAREASET
