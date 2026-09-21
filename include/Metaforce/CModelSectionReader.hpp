#pragma once

#include "Kyoto/Graphics/ModelTypes.hpp"
#include "Kyoto/Math/CAABox.hpp"

struct SModelArrays {
  TModelData positions;
  TModelData normals;
  TModelData colors;
  TModelData texCoords;
  TModelData packedTexCoords;
};

SModelArrays PrepareModelArrays(std::span< uchar > positions, std::span< uchar > normals,
                                std::span< uchar > colors, std::span< uchar > texCoords,
                                std::span< uchar > packedTexCoords, bool shortNormals);

class CModelSectionReader {
public:
  explicit CModelSectionReader(std::span< uchar > data);

  uint GetFlags() const { return mFlags; }
  uint GetMaterialSetCount() const { return mMaterialSetCount; }
  CAABox GetBounds() const;
  size_t GetRemainingSections() const { return mSectionCount - mSection; }
  std::span< uchar > Next();

private:
  std::span< uchar > mData;
  size_t mSizeTable;
  size_t mSectionCount;
  size_t mSection = 0;
  size_t mOffset;
  uint mFlags;
  uint mMaterialSetCount;
};

CAABox ReadModelBounds(TModelData data);
uint ReadModelSurfaceCount(TModelData data, size_t remainingSections);
uint ValidateModelMaterials(TModelData data);
TModelData GetModelMaterial(TModelData data, uint index);
