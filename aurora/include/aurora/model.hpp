#pragma once

#include "common.hpp"

namespace aurora::gfx::model {
void set_vertex_buffer(const std::vector<zeus::CVector3f>* data) noexcept;
void set_normal_buffer(const std::vector<zeus::CVector3f>* norm) noexcept;
void set_tex0_tc_buffer(const std::vector<Vec2<float>>* tcs) noexcept; // Tex coords for TEX0
void set_tc_buffer(const std::vector<Vec2<float>>* tcs) noexcept; // Tex coords for the TEX1-7

void set_vtx_desc_compressed(u32 vtxDesc) noexcept;
void queue_surface(const u8* dlStart, u32 dlSize) noexcept;
} // namespace aurora::gfx::model
