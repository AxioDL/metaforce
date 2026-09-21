#ifndef _CMAPAREA
#define _CMAPAREA

#include "MetroidPrime/CMappableObject.hpp"

#include "Kyoto/CFactoryFnReturn.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

#if defined(TARGET_PC)
#include <span>

class CResourceReader;
typedef std::span< const CVector3f > TMapVertices;
#else
typedef const CVector3f* TMapVertices;
#endif

class IWorld;

class CMapArea {
public:
  class CMapAreaSurface {
    friend class CMapArea;
    CVector3f x0_normal;
    CVector3f xc_centroid;
    const int* x18_surfOffset;
    const int* x1c_outlineOffset;

  public:
#if defined(TARGET_PC)
    CMapAreaSurface(CResourceReader& in, const CMapArea& area, size_t commandStart);
#else
    void PostConstruct(const void* buf);
#endif
    void Draw(TMapVertices verts, const CColor& surfColor, const CColor& lineColor,
              float lineWidth) const;

    static void SetupGXMaterial();

    CVector3f GetNormal() const { return x0_normal; }
    CVector3f GetCenterPosition() const { return xc_centroid; }
  };
  enum EVisMode { kVM_Always, kVM_MapStationOrVisit, kVM_Visit, kVM_Never };

  CMapArea(CInputStream& in, uint size);
#if defined(TARGET_PC)
  CMapArea(const CMapArea&) = delete;
  CMapArea& operator=(const CMapArea&) = delete;
#endif
  ~CMapArea();

  int GetNumMappableObjects() const { return x28_mappableObjCount; }
  const CMappableObject& GetMappableObject(int idx) const { return x38_moStart[idx]; }
  int GetNumSurfaces() const { return x30_surfaceCount; }
  const CMapAreaSurface& GetSurface(int idx) const { return x40_surfaceStart[idx]; }
  TMapVertices GetVertices() const {
#if defined(TARGET_PC)
    return {x3c_vertexStart, static_cast< size_t >(x2c_vertexCount)};
#else
    return x3c_vertexStart;
#endif
  }

#if !defined(TARGET_PC)
  void PostConstruct();
#endif
  bool GetIsVisibleToAutoMapper(bool worldVis, bool areaVis) const;
  CVector3f GetAreaCenterPoint() const;
  const CAABox& GetBoundingBox() const { return x10_box; }
  CTransform4f GetAreaPostTransform(const IWorld&, int);
  static const CVector3f& GetAreaPostTranslate(const IWorld&, int);

private:
  uint x0_magic;
  uint x4_version;
  uint x8_;
  EVisMode xc_visibilityMode;
  CAABox x10_box;
  int x28_mappableObjCount;
  int x2c_vertexCount;
  int x30_surfaceCount;
  uint x34_size;
  CMappableObject* x38_moStart;
  CVector3f* x3c_vertexStart;
  CMapAreaSurface* x40_surfaceStart;
  rstl::single_ptr< uchar > x44_buf;

#if defined(TARGET_PC)
  rstl::vector< CMappableObject > mObjects;
  rstl::vector< CMapAreaSurface > mSurfaces;
#endif

  static int gUsedMemory;
};

const CFactoryFnReturn FMapAreaFactory(const SObjectTag& objTag, CInputStream& in,
                                       const CVParamTransfer&);

#endif // _CMAPAREA
