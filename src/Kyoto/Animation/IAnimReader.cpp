#include "Kyoto/Animation/IAnimReader.hpp"
#include "Kyoto/Animation/CParticlePOINode.hpp"
#include "Kyoto/Animation/CSoundPOINode.hpp"

IAnimReader::~IAnimReader() {}

rstl::optional_object< rstl::ownership_transfer< IAnimReader > > IAnimReader::VSimplified() {
  return rstl::optional_object_null();
}

CAdvancementResults IAnimReader::VGetAdvancementResults(const CCharAnimTime& aTime,
                                                        const CCharAnimTime& bTime) const {
  return CAdvancementResults::RemainderOnly(aTime);
};

uint IAnimReader::GetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
                                 uint iterator, int unk) const {
  if (time.GreaterThanZero()) {
    return VGetBoolPOIList(time, listOut, capacity, iterator, unk);
  }
  return 0;
}

uint IAnimReader::GetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
                                  uint iterator, int unk) const {
  if (time.GreaterThanZero()) {
    return VGetInt32POIList(time, listOut, capacity, iterator, unk);
  }
  return 0;
}

uint IAnimReader::GetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
                                     uint capacity, uint iterator, int unk) const {
  if (time.GreaterThanZero()) {
    return VGetParticlePOIList(time, listOut, capacity, iterator, unk);
  }
  return 0;
}

uint IAnimReader::GetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
                                  uint iterator, int unk) const {
  if (time.GreaterThanZero()) {
    return VGetSoundPOIList(time, listOut, capacity, iterator, unk);
  }
  return 0;
}

bool IAnimReader::IsCAnimTreeNode() const { return false; }
