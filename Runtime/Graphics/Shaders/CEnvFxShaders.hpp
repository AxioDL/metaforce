#pragma once

#include <array>

#include <boo/graphicsdev/IGraphicsDataFactory.hpp>

#include <zeus/CColor.hpp>
#include <zeus/CMatrix4f.hpp>
#include <zeus/CVector2f.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CEnvFxManager;
class CEnvFxManagerGrid;

class CEnvFxShaders {
public:
  struct Instance {
    std::array<zeus::CVector3f, 4> positions;
    zeus::CColor color;
    std::array<zeus::CVector2f, 4> uvs;
  };
  struct Uniform {
    zeus::CMatrix4f mv;
    zeus::CMatrix4f proj;
    zeus::CMatrix4f envMtx;
    zeus::CColor moduColor;
  };

private:
  static boo::ObjToken<boo::IShaderPipeline> m_snowPipeline;
  static boo::ObjToken<boo::IShaderPipeline> m_underwaterPipeline;

public:
  static void Initialize();
  static void Shutdown();
  static void BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CEnvFxManager& fxMgr,
                                     CEnvFxManagerGrid& grid);
};

} // namespace urde
