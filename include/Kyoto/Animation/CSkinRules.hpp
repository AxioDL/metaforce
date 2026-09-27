#ifndef _CSKINRULES
#define _CSKINRULES

#include "Kyoto/Animation/CVirtualBone.hpp"
#include "Kyoto/CFactoryFnReturn.hpp"

class CCharLayoutInfo;
class CInputStream;
class CModel;
class CPoseAsTransforms;

class CSkinRules {
public:
  CSkinRules(CInputStream& in);
  ~CSkinRules();

  void InitLockedCacheState(const CModel& model);
  static void StartNextTransaction();

  void BuildAccumulatedTransforms(const CPoseAsTransforms& pose,
                                  const CCharLayoutInfo& layoutInfo) const;
  void BuildPoints(volatile void* pipe) const;
  void BuildNormals(volatile void* pipe) const;
  void BuildNormalsFrom(const CVector3f* averageNormals, CVector3f* out) const;
  int GetNumPoints() const { return mVertexCount; }
  int GetNumNormals() const { return mNormalCount; }
  int GetNumVirtualBones() const { return mVirtualBones.size(); }
  const rstl::vector< CVirtualBone >& GetVirtualBones() const { return mVirtualBones; }

  
  static int ProcessingPoints(int count, ushort** buf);
  static int ProcessingNormals(int count, ushort** buf);
private:
  rstl::vector< CVirtualBone > mVirtualBones;
  int mVertexCount;
  int mNormalCount;
};

const CFactoryFnReturn FSkinRulesFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer&);
#endif // _CSKINRULES
