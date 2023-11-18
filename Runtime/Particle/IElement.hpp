#pragma once

#include <memory>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/Streams/IOStreams.hpp"

#include <zeus/CColor.hpp>
#include <zeus/CVector3f.hpp>

namespace metaforce {

class IElement {
public:
  virtual ~IElement() = default;
};

class CRealElement : public IElement {
public:
  virtual bool GetValue(int frame, float& valOut) const = 0;
  virtual bool IsConstant() const { return false; }
};

class CIntElement : public IElement {
public:
  virtual bool GetValue(int frame, int& valOut) const = 0;
  virtual int GetMaxValue() const = 0;
};

class CVectorElement : public IElement {
public:
  virtual bool GetValue(int frame, zeus::CVector3f& valOut) const = 0;
  virtual bool IsFastConstant() const { return false; }
};

class CModVectorElement : public IElement {
public:
  virtual bool GetValue(int frame, zeus::CVector3f& pVel, zeus::CVector3f& pPos) const = 0;
};

class CColorElement : public IElement {
public:
  virtual bool GetValue(int frame, zeus::CColor& colorOut) const = 0;
};

class CEmitterElement : public IElement {
public:
  virtual bool GetValue(int frame, zeus::CVector3f& pPos, zeus::CVector3f& pVel) const = 0;
};

} // namespace metaforce
