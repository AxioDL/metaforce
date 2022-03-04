#include "common.hpp"

namespace aurora::gfx {
constexpr u32 maxTevStages = 2;
//static std::array<STevStage, maxTevStages> sTevStages;

void update_tev_stage(metaforce::ERglTevStage stage, const metaforce::CTevCombiners::ColorPass& colPass,
                      const metaforce::CTevCombiners::AlphaPass& alphaPass,
                      const metaforce::CTevCombiners::CTevOp& colorOp,
                      const metaforce::CTevCombiners::CTevOp& alphaOp) noexcept {}
void stream_begin(GX::Primitive primitive) noexcept {}
void stream_vertex(metaforce::EStreamFlags flags, const zeus::CVector3f& pos, const zeus::CVector3f& nrm,
                   const zeus::CColor& color, const zeus::CVector2f& uv) noexcept {}
void stream_end() noexcept {}
} // namespace aurora::gfx
