use cxxbridge::ffi;

use crate::{gpu::DeviceHolder, imgui_backend};

mod cxxbridge;

pub struct ImGuiState {
    pub(crate) context: imgui::Context,
    pub(crate) platform: imgui_winit_support::WinitPlatform,
    pub(crate) backend: imgui_backend::Renderer,
}

pub(crate) fn initialize_imgui(window: &winit::window::Window, gpu: &DeviceHolder) -> ImGuiState {
    let hidpi_factor = window.scale_factor();
    let mut imgui = imgui::Context::create();
    let mut platform = imgui_winit_support::WinitPlatform::init(&mut imgui);
    platform.attach_window(imgui.io_mut(), &window, imgui_winit_support::HiDpiMode::Default);
    imgui.set_ini_filename(None);
    imgui.io_mut().font_global_scale = (1.0 / hidpi_factor) as f32;

    unsafe {
        ffi::ImGuiEngine_Initialize(hidpi_factor as f32);
    }

    let renderer_config = imgui_backend::RendererConfig::new_srgb();

    let imgui_backend = imgui_backend::Renderer::new(
        &mut imgui,
        &gpu.device,
        &gpu.queue,
        imgui_backend::RendererConfig {
            texture_format: gpu.config.color_format,
            depth_format: Some(gpu.config.depth_format),
            sample_count: gpu.config.msaa_samples,
            ..renderer_config
        },
    );
    let mut state = ImGuiState { context: imgui, platform, backend: imgui_backend };
    unsafe {
        ffi::ImGuiEngine_AddTextures(&mut state, &gpu);
    }
    state
}

#[allow(non_snake_case)]
fn ImGuiEngine_AddTexture(
    state: &mut ImGuiState,
    gpu: &DeviceHolder,
    width: u32,
    height: u32,
    data: &[u8],
) -> usize {
    let texture =
        imgui_backend::Texture::new(&gpu.device, &state.backend, imgui_backend::TextureConfig {
            size: wgpu::Extent3d { width, height, depth_or_array_layers: 1 },
            format: Some(wgpu::TextureFormat::Rgba8Unorm),
            ..Default::default()
        });
    texture.write(&gpu.queue, data, width, height);
    state.backend.textures.insert(texture).id()
}
