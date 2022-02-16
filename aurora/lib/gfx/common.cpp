#include <aurora/gfx.hpp>

namespace aurora::gfx {
bool get_dxt_compression_supported() noexcept {
  return true; // TODO
}

void update_model_view(const zeus::CMatrix4f& mv, const zeus::CMatrix4f& mv_inv) noexcept {
  // TODO
}
void update_projection(const zeus::CMatrix4f& proj) noexcept {
  // TODO
}
void update_fog_state(const metaforce::CFogState& state) noexcept {
  // TODO
}
void set_viewport(const zeus::CRectangle& rect, float znear, float zfar) noexcept {
  // TODO
}
void set_scissor(uint32_t x, uint32_t y, uint32_t w, uint32_t h) noexcept {
  // TODO
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
void queue_textured_quad_verts(CameraFilterType filter_type, TextureRef texture, ZTest z_comparison, bool z_test,
                               const zeus::CColor& color, const ArrayRef<zeus::CVector3f>& pos,
                               const ArrayRef<zeus::CVector2f>& uvs, float lod) noexcept {
  // TODO
}
void queue_textured_quad(CameraFilterType filter_type, TextureRef texture, ZTest z_comparison, bool z_test,
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
void queue_movie_player(TextureRef tex_y, TextureRef tex_u, TextureRef tex_v, const zeus::CColor& color, float h_pad,
                        float v_pad) noexcept {
  // TODO
}

std::shared_ptr<TextureHandle> new_static_texture_2d(uint32_t width, uint32_t height, uint32_t mips,
                                                     TextureFormat format, ArrayRef<uint8_t> data,
                                                     std::string_view label) noexcept {
  return {}; // TODO
}
std::shared_ptr<TextureHandle> new_dynamic_texture_2d(uint32_t width, uint32_t height, uint32_t mips,
                                                      TextureFormat format, std::string_view label) noexcept {
  return {}; // TODO
}
std::shared_ptr<TextureHandle> new_render_texture(uint32_t width, uint32_t height, uint32_t color_bind_count,
                                                  uint32_t depth_bind_count, std::string_view label) noexcept {
  return {}; // TODO
}
void write_texture(TextureRef ref, ArrayRef<uint8_t> data) noexcept {
  // TODO
}
} // namespace aurora::gfx
