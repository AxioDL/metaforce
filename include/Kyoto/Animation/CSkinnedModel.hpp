#ifndef _CSKINNEDMODEL
#define _CSKINNEDMODEL

#include "types.h"

#include "Kyoto/Animation/CSkinRules.hpp"
#include "Kyoto/Animation/CVertexMorphEffect.hpp"
#include "Kyoto/Graphics/ModelTypes.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/optional_object.hpp"

class CModel;
class CModelFlags;
class CCharLayoutInfo;
class CPoseAsTransforms;
class CVector3f;

typedef void (*TDrawFunc)(TModelPositions, TModelNormals, const void*);

class CSkinnedModel {
public:
  enum EDataOwnership {
    kDO_Unowned,
    kDO_Owned,
  };

  CSkinnedModel(const CSkinnedModel&);
  CSkinnedModel(const TLockedToken< CModel >&, const TLockedToken< CSkinRules >&,
                const TLockedToken< CCharLayoutInfo >&, EDataOwnership);
  virtual ~CSkinnedModel();

  TLockedToken< CModel >& Model() { return x4_model; }
  const TLockedToken< CModel >& GetModel() const { return x4_model; }
  const TLockedToken< CCharLayoutInfo >& GetLayoutInfo() const { return x1c_layoutInfo; }
  void SetLayoutInfo(const TLockedToken< CCharLayoutInfo >& layout) { x1c_layoutInfo = layout; }

  void CalculateDefault();
  int GetNumPoints() const { return x10_skinRules->GetNumPoints(); }
  const CVector3f* GetPositions() const;
  void Calculate(const CPoseAsTransforms&, const rstl::optional_object< CVertexMorphEffect >&,
                 const float*, float*);
  void Draw(const CModelFlags&) const;
  void Draw(const float* positions, const float* normals, const CModelFlags& flags) const;
#if defined(TARGET_PC)
  TModelPositions GetPositionView(const float* positions) const {
    return positions ? TModelPositions(positions, static_cast< size_t >(GetNumPoints()) * 3)
                     : TModelPositions{};
  }
  TModelNormals GetNormalView(const float* normals) const {
    return normals
               ? TModelNormals(normals, static_cast< size_t >(x10_skinRules->GetNumNormals()) * 3)
               : TModelNormals{};
  }
#endif
  void Draw(TDrawFunc func, void* data);
  template < typename T >
  void Draw(void (*func)(TModelPositions, TModelNormals, T*), void* data) {
#if defined(TARGET_PC)
    struct SDrawContext {
      void (*func)(TModelPositions, TModelNormals, T*);
      T* data;
    } context{
        .func = func,
        .data = static_cast< T* >(data),
    };
    Draw(
        [](TModelPositions positions, TModelNormals normals, const void* data) {
          const auto& context = *static_cast< const SDrawContext* >(data);
          context.func(positions, normals, context.data);
        },
        &context);
#else
    Draw(reinterpret_cast< TDrawFunc >(func), data);
#endif
  }
  void PostDrawFunc() const;

  float* AllocateNewWorkspace(float** nrmOut);

  static void SetPointGeneratorFunc(void*,
                                    void (*)(void*, const CVector3f*, const CVector3f*, int));
  static void ClearPointGeneratorFunc();
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
  static void SetSkinningBuffer(void* buffer, int size);
#endif
  static void AddDummySkinnedModelRef();
  static void RemoveDummySkinnedModelRef();

  void Construct();
  void AllocateStorage();
  static void TickAllocations();
  static void* EnsureAllocation(int size);

  typedef void (*TPointGenFunc)(void*, const CVector3f*, const CVector3f*, int);
  static TPointGenFunc sPointGen;
  static void* sPointGenData;

  static void Draw(const TDrawFunc func, TModelPositions positions, TModelNormals normals,
                   void* data) {
    func(positions, normals, data);
  }

private:
  TLockedToken< CModel > x4_model;
  TLockedToken< CSkinRules > x10_skinRules;
  TLockedToken< CCharLayoutInfo > x1c_layoutInfo;
  mutable rstl::auto_ptr< float > x28_vertWorkspace;
  mutable rstl::auto_ptr< float > x30_normalWorkspace;
  bool x38_owned;
  bool x39_disableWorkspaces;
};

class CSkinnedModelWithAvgNormals {
  CSkinnedModel x0_skinnedModel;
  rstl::auto_ptr< float > x3c_avgNormals;

public:
  CSkinnedModelWithAvgNormals(const CSkinnedModel& model);
  const CSkinnedModel& GetSkinnedModel() const { return x0_skinnedModel; }
  float* GetAvgNormals() const { return x3c_avgNormals.get(); }
};

#endif // _CSKINNEDMODEL
