#pragma once

struct SDL_Window;
union SDL_Event;

namespace wgpu {
class RenderPassEncoder;
} // namespace wgpu

namespace aurora::imgui {
void create_context() noexcept;
void initialize(SDL_Window* window) noexcept;
void shutdown() noexcept;

void process_event(const SDL_Event& event) noexcept;
void new_frame() noexcept;
void render(const wgpu::RenderPassEncoder& pass) noexcept;
} // namespace aurora::imgui
