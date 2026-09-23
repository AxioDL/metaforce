#ifndef _IANIMSOURCEINFO
#define _IANIMSOURCEINFO

#include "Kyoto/Animation/CCharAnimTime.hpp"

#include "rstl/vector.hpp"

class CBoolPOINode;
class CInt32POINode;
class CParticlePOINode;
class CSoundPOINode;

class IAnimSourceInfo {
public:
  virtual bool HasPOIData() const = 0;
  virtual const rstl::vector< CBoolPOINode >& GetBoolPOIStream() const = 0;
  virtual const rstl::vector< CInt32POINode >& GetInt32POIStream() const = 0;
  virtual const rstl::vector< CParticlePOINode >& GetParticlePOIStream() const = 0;
  virtual const rstl::vector< CSoundPOINode >& GetSoundPOIStream() const = 0;
  virtual CCharAnimTime GetAnimationDuration() const = 0;
  virtual ~IAnimSourceInfo() {}
};

CHECK_SIZEOF(IAnimSourceInfo, 0x4)

#endif // _IANIMSOURCEINFO
