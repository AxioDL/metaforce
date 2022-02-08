use std::{
    collections::hash_map::Entry::{Occupied, Vacant},
    hash::{Hash, Hasher},
    num::{NonZeroU32, NonZeroU8},
};

use twox_hash::XxHash32;
use wgpu::{util::DeviceExt, ImageDataLayout};

use crate::{
    get_app,
    shaders::{
        ffi::{TextureFormat, TextureRef},
        STATE,
    },
};

pub(crate) struct TextureWithView {
    pub(crate) texture: wgpu::Texture,
    pub(crate) view: wgpu::TextureView,
    pub(crate) format: wgpu::TextureFormat,
    pub(crate) extent: wgpu::Extent3d,
}
impl TextureWithView {
    fn new(texture: wgpu::Texture, format: wgpu::TextureFormat, extent: wgpu::Extent3d) -> Self {
        let view = texture.create_view(&wgpu::TextureViewDescriptor::default());
        Self { texture, view, format, extent }
    }
}

pub(crate) struct RenderTexture {
    pub(crate) color_texture: Option<TextureWithView>,
    pub(crate) depth_texture: Option<TextureWithView>,
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
    let extent = wgpu::Extent3d { width, height, depth_or_array_layers: 1 };
    let wgpu_format = match format {
        TextureFormat::RGBA8 => wgpu::TextureFormat::Rgba8Unorm,
        TextureFormat::R8 => wgpu::TextureFormat::R8Unorm,
        TextureFormat::R32Float => wgpu::TextureFormat::R32Float,
        TextureFormat::DXT1 => wgpu::TextureFormat::Bc1RgbaUnorm,
        TextureFormat::DXT3 => wgpu::TextureFormat::Bc3RgbaUnorm,
        TextureFormat::DXT5 => wgpu::TextureFormat::Bc5RgUnorm,
        TextureFormat::BPTC => wgpu::TextureFormat::Bc7RgbaUnorm,
        _ => todo!(),
    };
    let texture = gpu.device.create_texture_with_data(
        &gpu.queue,
        &wgpu::TextureDescriptor {
            label: Some(label),
            size: extent,
            mip_level_count: mips,
            sample_count: 1,
            dimension: wgpu::TextureDimension::D2,
            format: wgpu_format,
            usage: wgpu::TextureUsages::TEXTURE_BINDING,
        },
        data,
    );

    // Generate texture hash as ID
    let mut hasher = XxHash32::with_seed(format.into());
    width.hash(&mut hasher);
    height.hash(&mut hasher);
    mips.hash(&mut hasher);
    data.hash(&mut hasher);
    label.hash(&mut hasher);
    let id = hasher.finish() as u32;

    // Store texture and return reference
    let state = unsafe { STATE.as_mut().unwrap_unchecked() };
    match state.textures.entry(id) {
        Occupied(entry) => panic!("Hash collision ({:x}) on texture creation: {}", id, label),
        Vacant(entry) => {
            state.textures.insert(id, TextureWithView::new(texture, wgpu_format, extent));
        }
    }
    TextureRef { id, render: false }
}

pub(crate) fn create_dynamic_texture_2d(
    width: u32,
    height: u32,
    mips: u32,
    format: TextureFormat,
    label: &str,
) -> TextureRef {
    let gpu = &get_app().gpu;
    let extent = wgpu::Extent3d { width, height, depth_or_array_layers: 1 };
    let wgpu_format = match format {
        TextureFormat::RGBA8 => wgpu::TextureFormat::Rgba8Unorm,
        TextureFormat::R8 => wgpu::TextureFormat::R8Unorm,
        TextureFormat::R32Float => wgpu::TextureFormat::R32Float,
        TextureFormat::DXT1 => wgpu::TextureFormat::Bc1RgbaUnorm,
        TextureFormat::DXT3 => wgpu::TextureFormat::Bc3RgbaUnorm,
        TextureFormat::DXT5 => wgpu::TextureFormat::Bc5RgUnorm,
        TextureFormat::BPTC => wgpu::TextureFormat::Bc7RgbaUnorm,
        _ => todo!(),
    };
    let texture = gpu.device.create_texture(&wgpu::TextureDescriptor {
        label: Some(label),
        size: extent,
        mip_level_count: mips,
        sample_count: 1,
        dimension: wgpu::TextureDimension::D2,
        format: wgpu_format,
        usage: wgpu::TextureUsages::TEXTURE_BINDING | wgpu::TextureUsages::COPY_DST,
    });

    // Generate texture hash as ID
    let mut hasher = XxHash32::with_seed(format.into());
    width.hash(&mut hasher);
    height.hash(&mut hasher);
    mips.hash(&mut hasher);
    label.hash(&mut hasher);
    let id = hasher.finish() as u32;

    // Store texture and return reference
    let state = unsafe { STATE.as_mut().unwrap_unchecked() };
    match state.textures.entry(id) {
        Occupied(entry) => panic!("Hash collision ({:x}) on texture creation: {}", id, label),
        Vacant(entry) => {
            state.textures.insert(id, TextureWithView::new(texture, wgpu_format, extent));
        }
    }
    TextureRef { id, render: false }
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
        let extent = wgpu::Extent3d { width, height, depth_or_array_layers: color_bind_count };
        Some(TextureWithView::new(
            gpu.device.create_texture(&wgpu::TextureDescriptor {
                label: Some(format!("{} Color", label).as_str()),
                size: extent,
                mip_level_count: 1,
                sample_count: 1,
                dimension: wgpu::TextureDimension::D2,
                format: gpu.config.color_format,
                usage: wgpu::TextureUsages::RENDER_ATTACHMENT,
            }),
            gpu.config.color_format,
            extent,
        ))
    } else {
        None
    };
    let depth_texture = if depth_bind_count > 0 {
        let extent = wgpu::Extent3d { width, height, depth_or_array_layers: depth_bind_count };
        Some(TextureWithView::new(
            gpu.device.create_texture(&wgpu::TextureDescriptor {
                label: Some(format!("{} Depth", label).as_str()),
                size: extent,
                mip_level_count: 1,
                sample_count: 1,
                dimension: wgpu::TextureDimension::D2,
                format: gpu.config.depth_format,
                usage: wgpu::TextureUsages::RENDER_ATTACHMENT,
            }),
            gpu.config.depth_format,
            extent,
        ))
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

    // Generate texture hash as ID
    let mut hasher = XxHash32::default();
    width.hash(&mut hasher);
    height.hash(&mut hasher);
    color_bind_count.hash(&mut hasher);
    depth_bind_count.hash(&mut hasher);
    label.hash(&mut hasher);
    let id = hasher.finish() as u32;

    // Store texture and return reference
    let state = unsafe { STATE.as_mut().unwrap_unchecked() };
    match state.textures.entry(id) {
        Occupied(entry) => panic!("Hash collision ({:x}) on texture creation: {}", id, label),
        Vacant(entry) => {
            state.render_textures.insert(id, RenderTexture { color_texture, depth_texture });
        }
    }
    TextureRef { id, render: true }
}

pub(crate) fn write_texture(handle: TextureRef, data: &[u8]) {
    if handle.render {
        panic!("Can't write to render texture");
    }

    let state = unsafe { STATE.as_mut().unwrap_unchecked() };
    if let Some(TextureWithView { texture, format, extent, .. }) = state.textures.get(&handle.id) {
        state.queue.write_texture(
            texture.as_image_copy(),
            data,
            ImageDataLayout {
                offset: 0,
                bytes_per_row: match format {
                    wgpu::TextureFormat::Rgba8Unorm | wgpu::TextureFormat::R32Float => {
                        NonZeroU32::new(extent.width * 4)
                    }
                    wgpu::TextureFormat::R8Unorm => NonZeroU32::new(extent.width),
                    _ => todo!("Unimplemented format for write_texture: {:?}", format),
                },
                rows_per_image: match extent.depth_or_array_layers {
                    1 => None,
                    _ => todo!("Unimplemented write_texture for 3D/2DArray textures"),
                },
            },
            *extent,
        );
    } else {
        panic!("Failed to find texture {}", handle.id);
    }
}

pub(crate) fn drop_texture(handle: TextureRef) {
    let state = unsafe { STATE.as_mut().unwrap_unchecked() };
    if handle.render {
        state.render_textures.remove(&handle.id).expect("Render texture already dropped");
    } else {
        state.textures.remove(&handle.id).expect("Texture already dropped");
    }
}

pub(crate) fn create_sampler(
    device: &wgpu::Device,
    mut address_mode: wgpu::AddressMode,
    mut border_color: Option<wgpu::SamplerBorderColor>,
) -> wgpu::Sampler {
    if address_mode == wgpu::AddressMode::ClampToBorder
        && !device.features().contains(wgpu::Features::ADDRESS_MODE_CLAMP_TO_BORDER)
    {
        address_mode = wgpu::AddressMode::ClampToEdge;
        border_color = None;
    }
    device.create_sampler(&wgpu::SamplerDescriptor {
        label: None,
        address_mode_u: address_mode,
        address_mode_v: address_mode,
        address_mode_w: address_mode,
        mag_filter: wgpu::FilterMode::Linear,
        min_filter: wgpu::FilterMode::Linear,
        mipmap_filter: wgpu::FilterMode::Linear,
        lod_min_clamp: 0.0,
        lod_max_clamp: f32::MAX,
        compare: None,
        anisotropy_clamp: NonZeroU8::new(16),
        border_color,
    })
}
