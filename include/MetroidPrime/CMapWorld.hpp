#ifndef _CMAPWORLD
#define _CMAPWORLD

#include "MetroidPrime/CMapArea.hpp"

#include "Kyoto/Graphics/CColor.hpp"

#include "rstl/reserved_vector.hpp"

class CStateManager;

class CMapWorld {
public:
  /* skDrawProfileItemNames; */
  enum EMapAreaList { kMAL_Loaded, kMAL_Loading, kMAL_Unloaded };

  class CMapAreaBFSInfo {
    int mAreaIdx;
    int mDepth;
    float mSurfDrawDepth;
    float mOutlineDrawDepth;

  public:
    CMapAreaBFSInfo(int areaIdx, int depth, float surfDepth, float outlineDepth);
    int GetAreaIndex() const { return mAreaIdx; }
    int GetDepth() const { return mDepth; }
    float GetOutlineDrawDepth() const { return mOutlineDrawDepth; }
    float GetSurfaceDrawDepth() const { return mSurfDrawDepth; }
  };

  class CMapObjectSortInfo {
    float mZDist;
    int mAreaIdx;
    int mTypeAndIdx;
    CColor mSurfColor;
    CColor mOutlineColor;

  public:
    enum EObjectCode {
      kOC_Invalid = -1,
      kOC_Object = 1 << 16,
      kOC_DoorSurface = 2 << 16,
      kOC_Door = 3 << 16,
      kOC_Surface = 4 << 16
    };

    CMapObjectSortInfo(float zDist, int areaIdx, EObjectCode type, int idx, CColor surfColor,
                       CColor outlineColor);
    const CColor& GetOutlineColor() const { return mOutlineColor; }
    const CColor& GetSurfaceColor() const { return mSurfColor; }
    int GetLocalObjectIndex() const { return mTypeAndIdx & 0xffff; }
    EObjectCode GetObjectCode() const { return EObjectCode(mTypeAndIdx & 0xffff0000); }
    int GetAreaIndex() const { return mAreaIdx; }
    float GetZDistance() const { return mZDist; }
  };

  class CMapAreaData {
    CAssetId mAreaRes;
    mutable TCachedToken< CMapArea > mArea;
    mutable EMapAreaList mList;
    mutable CMapAreaData* mNext;

  public:
    CMapAreaData(CAssetId areaRes, EMapAreaList list, CMapAreaData* next);
    void Lock();
    void Unlock();
    bool IsLoaded() const;
    CMapArea* MapArea() { return mArea.GetT(); }
    CMapArea* GetMapArea() const;
    CMapAreaData* NextMapAreaData() { return mNext; }
    CMapAreaData* GetNextMapAreaData() const { return mNext; }
    EMapAreaList GetContainingList() const { return mList; }
    void SetContainingList(EMapAreaList list) const { mList = list; }
    void SetNextMapArea(CMapAreaData* next) const { mNext = next; }
  };

  class CMapWorldDrawParms {
    float mAlphaSurfVisited;
    float mAlphaOlVisited;
    float mAlphaSurfUnvisited;
    float mAlphaOlUnvisited;
    float mAlpha;
    float mOutlineWidthScale;
    const CStateManager& mMgr;
    const CTransform4f& mModelXf;
    const CTransform4f& mViewXf;
    const IWorld& mWld;
    const CMapWorldInfo& mMwInfo;
    float mPlayerFlashIntensity;
    float mHintFlashIntensity;
    float mObjectScale;
    bool mSortDoorSurfs;

  public:
    CMapWorldDrawParms(float alphaSurfVisited, float alphaOlVisited, float alphaSurfUnvisited,
                       float alphaOlUnvisited, float alpha, const CStateManager& mgr,
                       const CTransform4f& modelXf, const CTransform4f& viewXf, const IWorld& wld,
                       const CMapWorldInfo& mwInfo, float outlineWidthScale, bool sortDoorSurfs,
                       float playerFlash, float hintFlash, float objectScale);
    const IWorld& GetWorld() const { return mWld; }
    float GetOutlineWidthScale() const { return mOutlineWidthScale; }
    const CTransform4f& GetPlaneProjectionTransform() const { return mModelXf; }
    float GetHintAreaFlashIntensity() const { return mHintFlashIntensity; }
    float GetPlayerAreaFlashIntensity() const { return mPlayerFlashIntensity; }
    const CTransform4f& GetCameraTransform() const { return mViewXf; }
    float GetAlphaOutlineUnvisited() const { return mAlphaOlUnvisited; }
    float GetAlphaSurfaceUnvisited() const { return mAlphaSurfUnvisited; }
    float GetAlphaOutlineVisited() const { return mAlphaOlVisited; }
    float GetAlphaSurfaceVisited() const { return mAlphaSurfVisited; }
    float GetAlpha() const { return mAlpha; }
    const CMapWorldInfo& GetMapWorldInfo() const { return mMwInfo; }
    const CStateManager& GetStateManager() const { return mMgr; }
    bool GetIsSortDoorSurfaces() const { return mSortDoorSurfs; }
    float GetObjectScale() const { return mObjectScale; }
  };

private:
  rstl::vector< CMapAreaData > mAreas;
  rstl::reserved_vector< CMapAreaData*, 3 > mListHeads;
  mutable rstl::vector< bool > mTraversed;
  mutable CVector3f mWorldSpherePoint;
  mutable float mWorldSphereRadius;
  mutable float mWorldSphereHalfDepth;

public:
  explicit CMapWorld(CInputStream& in);
  ~CMapWorld();
  uint GetNumAreas() const { return mAreas.size(); }
  CMapArea* GetMapArea(int aid) { return mAreas[aid].MapArea(); }
  CMapArea* GetMapArea(int aid) const;
  bool IsMapAreaInBFSInfoVector(const CMapAreaData* area,
                                const rstl::vector< CMapAreaBFSInfo >& vec) const;
  void SetWhichMapAreasLoaded(const IWorld& wld, int start, int count);
  void MoveMapAreaToList(CMapAreaData* data, EMapAreaList list);
  int GetCurrentMapAreaDepth(const IWorld& wld, int aid) const;
  rstl::vector< int > GetVisibleAreas(const IWorld& wld, const CMapWorldInfo& mwInfo) const;
  void Draw(const CMapWorldDrawParms& parms, int curArea, int otherArea, float depth1, float depth2,
            bool inMapScreen) const;
  void DoBFS(const IWorld& wld, int startArea, int areaCount, float surfDepth, float outlineDepth,
             bool checkLoad, rstl::vector< CMapAreaBFSInfo >& bfsInfos) const;
  bool IsMapAreaValid(const IWorld& wld, int areaIdx, bool checkLoad) const;
  void DrawAreas(const CMapWorldDrawParms& parms, int selArea,
                 const rstl::vector< CMapAreaBFSInfo >& bfsInfos, bool inMapScreen) const;
  void RecalculateWorldSphere(const CMapWorldInfo& mwInfo, const IWorld& wld) const;
  CVector3f ConstrainToWorldVolume(const CVector3f& point, const CVector3f& lookVec) const;
  void ClearTraversedFlags() const;
  bool IsMapAreasStreaming() const;
};
NESTED_CHECK_SIZEOF(CMapWorld, CMapAreaBFSInfo, 0x10)
NESTED_CHECK_SIZEOF(CMapWorld, CMapObjectSortInfo, 0x14)
NESTED_CHECK_SIZEOF(CMapWorld, CMapAreaData, 0x18)
NESTED_CHECK_SIZEOF(CMapWorld, CMapWorldDrawParms, 0x3c)
CHECK_SIZEOF(CMapWorld, 0x44)

#endif // _CMAPWORLD
