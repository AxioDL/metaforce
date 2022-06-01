#pragma once

struct SDL_Renderer;
struct SDL_Window;
union SDL_Event;

namespace wgpu {
class RenderPassEncoder;
} // namespace wgpu

namespace aurora {
struct WindowSize;
} // namespace aurora

namespace aurora::imgui {
void create_context() noexcept;
void initialize(SDL_Window* window, SDL_Renderer* renderer) noexcept;
void shutdown() noexcept;

void process_event(const SDL_Event& event) noexcept;
void new_frame(const WindowSize& size) noexcept;
void render(const wgpu::RenderPassEncoder& pass) noexcept;
} // namespace aurora::imgui
