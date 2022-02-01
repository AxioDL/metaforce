use std::{num::NonZeroU8, sync::Arc};

use pollster::block_on;

#[derive(Clone)]
pub(crate) struct GraphicsConfig {
    pub(crate) color_format: wgpu::TextureFormat,
    pub(crate) depth_format: wgpu::TextureFormat,
    pub(crate) texture_anistropy: Option<NonZeroU8>,
    pub(crate) msaa_samples: u32,
}

pub struct DeviceHolder {
    pub(crate) instance: wgpu::Instance,
    pub(crate) surface: wgpu::Surface,
    pub(crate) adapter: wgpu::Adapter,
    pub(crate) backend: wgpu::Backend,
    pub(crate) device: Arc<wgpu::Device>,
    pub(crate) queue: Arc<wgpu::Queue>,
    pub(crate) config: GraphicsConfig,
    pub(crate) surface_config: wgpu::SurfaceConfiguration,
    pub(crate) framebuffer: TextureWithSampler,
    pub(crate) depth: TextureWithSampler,
}

pub(crate) struct TextureWithSampler {
    pub(crate) texture: wgpu::Texture,
    pub(crate) view: wgpu::TextureView,
    pub(crate) sampler: wgpu::Sampler,
}

pub(crate) fn create_render_texture(
    device: &wgpu::Device,
    config: &wgpu::SurfaceConfiguration,
    graphics_config: &GraphicsConfig,
) -> TextureWithSampler {
    let texture = device.create_texture(&wgpu::TextureDescriptor {
        label: Some("Render texture"),
        size: wgpu::Extent3d {
            width: config.width,
            height: config.height,
            depth_or_array_layers: 1,
        },
        mip_level_count: 1,
        sample_count: graphics_config.msaa_samples,
        dimension: wgpu::TextureDimension::D2,
        format: graphics_config.color_format,
        usage: wgpu::TextureUsages::RENDER_ATTACHMENT,
    });
    let view = texture.create_view(&wgpu::TextureViewDescriptor::default());
    let sampler = device.create_sampler(&wgpu::SamplerDescriptor {
        label: Some("Render sampler"),
        address_mode_u: wgpu::AddressMode::ClampToEdge,
        address_mode_v: wgpu::AddressMode::ClampToEdge,
        address_mode_w: wgpu::AddressMode::ClampToEdge,
        mag_filter: wgpu::FilterMode::Linear,
        min_filter: wgpu::FilterMode::Linear,
        mipmap_filter: wgpu::FilterMode::Linear,
        lod_min_clamp: 0.0,
        lod_max_clamp: f32::MAX,
        compare: None,
        anisotropy_clamp: None,
        border_color: None,
    });
    TextureWithSampler { texture, view, sampler }
}

pub(crate) fn create_depth_texture(
    device: &wgpu::Device,
    config: &wgpu::SurfaceConfiguration,
    graphics_config: &GraphicsConfig,
) -> TextureWithSampler {
    let texture = device.create_texture(&wgpu::TextureDescriptor {
        label: Some("Depth texture"),
        size: wgpu::Extent3d {
            width: config.width,
            height: config.height,
            depth_or_array_layers: 1,
        },
        mip_level_count: 1,
        sample_count: graphics_config.msaa_samples,
        dimension: wgpu::TextureDimension::D2,
        format: graphics_config.depth_format,
        usage: wgpu::TextureUsages::RENDER_ATTACHMENT,
    });
    let view = texture.create_view(&wgpu::TextureViewDescriptor::default());
    let sampler = device.create_sampler(&wgpu::SamplerDescriptor {
        label: Some("Depth sampler"),
        address_mode_u: wgpu::AddressMode::ClampToEdge,
        address_mode_v: wgpu::AddressMode::ClampToEdge,
        address_mode_w: wgpu::AddressMode::ClampToEdge,
        mag_filter: wgpu::FilterMode::Linear,
        min_filter: wgpu::FilterMode::Linear,
        mipmap_filter: wgpu::FilterMode::Linear,
        lod_min_clamp: 0.0,
        lod_max_clamp: f32::MAX,
        compare: None,
        anisotropy_clamp: None,
        border_color: None,
    });
    TextureWithSampler { texture, view, sampler }
}

pub(crate) fn initialize_gpu(window: &winit::window::Window) -> DeviceHolder {
    log::info!("Creating wgpu instance");
    let instance = wgpu::Instance::new(wgpu::Backends::all());
    let surface = unsafe { instance.create_surface(window) };
    let adapter = block_on(instance.request_adapter(&wgpu::RequestAdapterOptions {
        power_preference: wgpu::PowerPreference::HighPerformance,
        compatible_surface: Some(&surface),
        force_fallback_adapter: false,
    }))
    .expect("Failed to find an appropriate adapter");
    let best_limits = adapter.limits();
    let backend = adapter.get_info().backend;
    #[cfg(any(target_os = "android", target_os = "ios"))]
    let needs_denorm = true;
    #[cfg(not(any(target_os = "android", target_os = "ios")))]
    let _needs_denorm = backend == wgpu::Backend::Gl;

    let required_features = if backend == wgpu::Backend::Vulkan {
        wgpu::Features::SPIRV_SHADER_PASSTHROUGH
    } else {
        wgpu::Features::empty()
    };
    let optional_features = wgpu::Features::TEXTURE_COMPRESSION_BC
        | wgpu::Features::ADDRESS_MODE_CLAMP_TO_BORDER
        | required_features;
    log::info!("Requesting device with features: {:?}", optional_features);
    let limits = wgpu::Limits::downlevel_defaults()
        .using_resolution(best_limits.clone())
        .using_alignment(best_limits);
    let (device, queue) = block_on(adapter.request_device(
        &wgpu::DeviceDescriptor {
            label: Some("Device with optional features"),
            features: optional_features,
            limits: limits.clone(),
        },
        None,
    ))
    .unwrap_or_else(|_| {
        log::warn!("Failed... requesting device with no features");
        block_on(adapter.request_device(
            &wgpu::DeviceDescriptor {
                label: Some("Device without optional features"),
                features: required_features,
                limits,
            },
            None,
        ))
        .expect("Failed to create device")
    });
    log::info!("Successfully created device");

    let mut swapchain_format = surface.get_preferred_format(&adapter).unwrap();
    if backend != wgpu::Backend::Gl {
        swapchain_format = match swapchain_format {
            wgpu::TextureFormat::Rgba8UnormSrgb => wgpu::TextureFormat::Rgba8Unorm,
            wgpu::TextureFormat::Bgra8UnormSrgb => wgpu::TextureFormat::Bgra8Unorm,
            _ => swapchain_format,
        };
    }
    let graphics_config = GraphicsConfig {
        color_format: swapchain_format,
        depth_format: wgpu::TextureFormat::Depth32Float,
        texture_anistropy: NonZeroU8::new(16),
        msaa_samples: 4,
    };

    let size = window.inner_size();
    let surface_config = wgpu::SurfaceConfiguration {
        usage: wgpu::TextureUsages::RENDER_ATTACHMENT,
        format: swapchain_format,
        width: size.width,
        height: size.height,
        present_mode: wgpu::PresentMode::Fifo,
    };
    surface.configure(&device, &surface_config);
    let depth = create_depth_texture(&device, &surface_config, &graphics_config);
    let framebuffer = create_render_texture(&device, &surface_config, &graphics_config);

    DeviceHolder {
        instance,
        surface,
        adapter,
        backend,
        device: Arc::new(device),
        queue: Arc::new(queue),
        config: graphics_config,
        surface_config,
        framebuffer,
        depth,
    }
}
