#include "common.hpp"

#include <magic_enum.hpp>

#include "movie_player/shader.hpp"

namespace aurora::gfx {
struct ShaderDrawCommand {
  ShaderType type;
  union {
    movie_player::DrawData moviePlayer;
  };
};
struct PipelineCreateCommand {
  ShaderType type;
  union {
    movie_player::PipelineConfig moviePlayer;
  };
};
enum class CommandType {
  SetViewport,
  SetScissor,
  Draw,
};
struct Command {
  CommandType type;
  union Data {
    struct SetViewportCommand {
      zeus::CRectangle rect;
      float znear;
      float zfar;
    } setViewport;
    struct SetScissorCommand {
      uint32_t x;
      uint32_t y;
      uint32_t w;
      uint32_t h;
    } setScissor;
    ShaderDrawCommand draw;
  } data;
};

zeus::CMatrix4f g_mv;
zeus::CMatrix4f g_mvInv;
zeus::CMatrix4f g_proj;
metaforce::CFogState g_fogState;

std::vector<Command> g_commands;

bool get_dxt_compression_supported() noexcept { return g_Device.HasFeature(wgpu::FeatureName::TextureCompressionBC); }

void update_model_view(const zeus::CMatrix4f& mv, const zeus::CMatrix4f& mv_inv) noexcept {
  g_mv = mv;
  g_mvInv = mv_inv;
}
void update_projection(const zeus::CMatrix4f& proj) noexcept { g_proj = proj; }
void update_fog_state(const metaforce::CFogState& state) noexcept { g_fogState = state; }
void set_viewport(const zeus::CRectangle& rect, float znear, float zfar) noexcept {
  g_commands.push_back({CommandType::SetViewport, {.setViewport = {rect, znear, zfar}}});
}
void set_scissor(uint32_t x, uint32_t y, uint32_t w, uint32_t h) noexcept {
  g_commands.push_back({CommandType::SetScissor, {.setScissor = {x, y, w, h}}});
}

void resolve_color(const ClipRect& rect, uint32_t bind, bool clear_depth) noexcept {
  // TODO
}
void resolve_depth(const ClipRect& rect, uint32_t bind) noexcept {
  // TODO
}

void add_material_set(/* TODO */) noexcept {}
void add_model(/* TODO */) noexcept {}

void queue_aabb(const zeus::CAABox& aabb, const zeus::CColor& color, bool z_only) noexcept {
  // TODO
}
void queue_fog_volume_plane(const ArrayRef<zeus::CVector4f>& verts, uint8_t pass) {
  // TODO
}
void queue_fog_volume_filter(const zeus::CColor& color, bool two_way) noexcept {
  // TODO
}
void queue_textured_quad_verts(CameraFilterType filter_type, const TextureHandle& texture, ZTest z_comparison,
                               bool z_test, const zeus::CColor& color, const ArrayRef<zeus::CVector3f>& pos,
                               const ArrayRef<zeus::CVector2f>& uvs, float lod) noexcept {
  // TODO
}
void queue_textured_quad(CameraFilterType filter_type, const TextureHandle& texture, ZTest z_comparison, bool z_test,
                         const zeus::CColor& color, float uv_scale, const zeus::CRectangle& rect, float z) noexcept {
  // TODO
}
void queue_colored_quad_verts(CameraFilterType filter_type, ZTest z_comparison, bool z_test, const zeus::CColor& color,
                              const ArrayRef<zeus::CVector3f>& pos) noexcept {
  // TODO
}
void queue_colored_quad(CameraFilterType filter_type, ZTest z_comparison, bool z_test, const zeus::CColor& color,
                        const zeus::CRectangle& rect, float z) noexcept {
  // TODO
}
void queue_movie_player(const TextureHandle& tex_y, const TextureHandle& tex_u, const TextureHandle& tex_v,
                        const zeus::CColor& color, float h_pad, float v_pad) noexcept {
  // TODO
}
} // namespace aurora::gfx
