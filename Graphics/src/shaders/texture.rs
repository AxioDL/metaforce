use std::ptr::null;

use cxx::SharedPtr;
use wgpu::util::DeviceExt;

use crate::{
    get_app,
    gpu::TextureWithSampler,
    shaders::ffi::{TextureClampMode, TextureFormat, TextureRef},
};

pub(crate) struct TextureWithView {
    texture: wgpu::Texture,
    view: wgpu::TextureView,
}
impl TextureWithView {
    fn new(texture: wgpu::Texture) -> Self {
        let view = texture.create_view(&wgpu::TextureViewDescriptor::default());
        Self { texture, view }
    }
}

pub(crate) struct RenderTexture {
    color_texture: Option<TextureWithView>,
    depth_texture: Option<TextureWithView>,
}

pub(crate) struct Texture {
    texture: wgpu::Texture,
}

pub(crate) fn create_static_texture_2d(
    width: u32,
    height: u32,
    mips: u32,
    format: TextureFormat,
    data: &[u8],
    label: &str,
) -> TextureRef {
    let gpu = &get_app().gpu;
    let texture = gpu.device.create_texture_with_data(
        &gpu.queue,
        &wgpu::TextureDescriptor {
            label: Some(label),
            size: wgpu::Extent3d { width, height, depth_or_array_layers: 1 },
            mip_level_count: mips,
            sample_count: 1,
            dimension: wgpu::TextureDimension::D2,
            format: match format {
                TextureFormat::RGBA8 => wgpu::TextureFormat::Rgba8Unorm,
                TextureFormat::R8 => wgpu::TextureFormat::R8Unorm,
                TextureFormat::R32Float => wgpu::TextureFormat::R32Float,
                TextureFormat::DXT1 => wgpu::TextureFormat::Bc1RgbaUnorm,
                TextureFormat::DXT3 => wgpu::TextureFormat::Bc3RgbaUnorm,
                TextureFormat::DXT5 => wgpu::TextureFormat::Bc5RgUnorm,
                TextureFormat::BPTC => wgpu::TextureFormat::Bc7RgbaUnorm,
                _ => todo!(),
            },
            usage: wgpu::TextureUsages::TEXTURE_BINDING,
        },
        data,
    );
    TextureRef { id: u32::MAX }
}

pub(crate) fn create_render_texture(
    width: u32,
    height: u32,
    // clamp_mode: TextureClampMode,
    color_bind_count: u32,
    depth_bind_count: u32,
    label: &str,
) -> TextureRef {
    let gpu = &get_app().gpu;
    let color_texture = if color_bind_count > 0 {
        Some(TextureWithView::new(gpu.device.create_texture(&wgpu::TextureDescriptor {
            label: Some(format!("{} Color", label).as_str()),
            size: wgpu::Extent3d { width, height, depth_or_array_layers: color_bind_count },
            mip_level_count: 1,
            sample_count: 1,
            dimension: wgpu::TextureDimension::D2,
            format: gpu.config.color_format,
            usage: wgpu::TextureUsages::RENDER_ATTACHMENT,
        })))
    } else {
        None
    };
    let depth_texture = if depth_bind_count > 0 {
        Some(TextureWithView::new(gpu.device.create_texture(&wgpu::TextureDescriptor {
            label: Some(format!("{} Depth", label).as_str()),
            size: wgpu::Extent3d { width, height, depth_or_array_layers: depth_bind_count },
            mip_level_count: 1,
            sample_count: 1,
            dimension: wgpu::TextureDimension::D2,
            format: gpu.config.depth_format,
            usage: wgpu::TextureUsages::RENDER_ATTACHMENT,
        })))
    } else {
        None
    };
    // let (clamp_mode, border_color) = match clamp_mode {
    //     TextureClampMode::Repeat => (wgpu::AddressMode::Repeat, None),
    //     TextureClampMode::ClampToWhite => {
    //         (wgpu::AddressMode::ClampToBorder, Some(wgpu::SamplerBorderColor::OpaqueWhite))
    //     }
    //     TextureClampMode::ClampToBlack => {
    //         (wgpu::AddressMode::ClampToBorder, Some(wgpu::SamplerBorderColor::OpaqueBlack))
    //     }
    //     TextureClampMode::ClampToEdge => (wgpu::AddressMode::ClampToEdge, None),
    // };
    // let sampler = gpu.device.create_sampler(&wgpu::SamplerDescriptor {
    //     label: Some(format!("{} Color Sampler", label).as_str()),
    //     address_mode_u: clamp_mode,
    //     address_mode_v: clamp_mode,
    //     address_mode_w: clamp_mode,
    //     mag_filter: wgpu::FilterMode::Linear,
    //     min_filter: wgpu::FilterMode::Linear,
    //     mipmap_filter: wgpu::FilterMode::Linear,
    //     lod_min_clamp: 0.0,
    //     lod_max_clamp: f32::MAX,
    //     compare: None,
    //     anisotropy_clamp: None,
    //     border_color,
    // });
    RenderTexture { color_texture, depth_texture };
    TextureRef { id: u32::MAX }
}

pub(crate) fn drop_texture(handle: TextureRef) {}
