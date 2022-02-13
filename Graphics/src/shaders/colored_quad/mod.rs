use std::{collections::HashMap, hash::Hash, ops::Range};

use bytemuck_derive::{Pod, Zeroable};
use wgpu::{include_wgsl, vertex_attr_array};

use crate::{
    get_app,
    gpu::GraphicsConfig,
    shaders::{
        bind_pipeline, cxxbridge::ffi, get_combined_matrix, pipeline_ref, push_draw_command,
        push_uniform, push_verts, texture::create_sampler, BuiltBuffers, PipelineCreateCommand,
        PipelineHolder, PipelineRef, ShaderDrawCommand, STATE,
    },
    util::{align, Vec2, Vec3},
    zeus::{CColor, CMatrix4f, CRectangle, CVector2f, CVector3f, CVector4f},
};

#[derive(Debug, Clone)]
pub(crate) struct DrawData {
    pipeline: PipelineRef,
    vert_range: Range<u64>,
    uniform_range: Range<u64>,
}

#[derive(Hash)]
pub(crate) struct PipelineConfig {
    filter_type: ffi::CameraFilterType,
    z_comparison: ffi::ZTest,
    z_test: bool,
}
pub(crate) const INITIAL_PIPELINES: &[PipelineCreateCommand] = &[
    // PipelineCreateCommand::ColoredQuad(PipelineConfig { z_only: false }),
    // PipelineCreateCommand::ColoredQuad(PipelineConfig { z_only: true }),
];

pub(crate) struct State {
    shader: wgpu::ShaderModule,
    uniform_layout: wgpu::BindGroupLayout,
    uniform_bind_group: wgpu::BindGroup,
    sampler: wgpu::Sampler,
    pipeline_layout: wgpu::PipelineLayout,
}

pub(crate) fn construct_state(
    device: &wgpu::Device,
    _queue: &wgpu::Queue,
    buffers: &BuiltBuffers,
    graphics_config: &GraphicsConfig,
) -> State {
    let shader = device.create_shader_module(&include_wgsl!("shader.wgsl"));
    let uniform_alignment = device.limits().min_uniform_buffer_offset_alignment;
    let uniform_size = wgpu::BufferSize::new(align(
        std::mem::size_of::<Uniform>() as u64,
        uniform_alignment as u64,
    ));
    let uniform_layout = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
        label: Some("Colored Quad Uniform Bind Group Layout"),
        entries: &[
            wgpu::BindGroupLayoutEntry {
                binding: 0,
                visibility: wgpu::ShaderStages::VERTEX_FRAGMENT,
                ty: wgpu::BindingType::Buffer {
                    ty: wgpu::BufferBindingType::Uniform,
                    has_dynamic_offset: true,
                    min_binding_size: uniform_size,
                },
                count: None,
            },
            wgpu::BindGroupLayoutEntry {
                binding: 1,
                visibility: wgpu::ShaderStages::FRAGMENT,
                ty: wgpu::BindingType::Sampler(wgpu::SamplerBindingType::Filtering),
                count: None,
            },
        ],
    });
    let sampler = create_sampler(device, wgpu::AddressMode::Repeat, None);
    let uniform_bind_group = device.create_bind_group(&wgpu::BindGroupDescriptor {
        label: Some("Colored Quad Uniform Bind Group"),
        layout: &uniform_layout,
        entries: &[
            wgpu::BindGroupEntry {
                binding: 0,
                resource: wgpu::BindingResource::Buffer(wgpu::BufferBinding {
                    buffer: &buffers.uniform_buffer,
                    offset: 0, // dynamic
                    size: uniform_size,
                }),
            },
            wgpu::BindGroupEntry { binding: 1, resource: wgpu::BindingResource::Sampler(&sampler) },
        ],
    });
    let pipeline_layout = device.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
        label: Some("Colored Quad Pipeline Layout"),
        bind_group_layouts: &[&uniform_layout],
        push_constant_ranges: &[],
    });
    State {
        shader,
        uniform_layout,
        uniform_bind_group,
        sampler,
        pipeline_layout,
    }
}

pub(crate) fn construct_pipeline(
    device: &wgpu::Device,
    graphics: &GraphicsConfig,
    state: &State,
    config: &PipelineConfig,
) -> PipelineHolder {
    let (blend_component, alpha_write) = match config.filter_type {
        ffi::CameraFilterType::Multiply => (
            wgpu::BlendComponent {
                src_factor: wgpu::BlendFactor::Zero,
                dst_factor: wgpu::BlendFactor::Src,
                operation: wgpu::BlendOperation::Add,
            },
            true,
        ),
        ffi::CameraFilterType::Add => (
            wgpu::BlendComponent {
                src_factor: wgpu::BlendFactor::SrcAlpha,
                dst_factor: wgpu::BlendFactor::One,
                operation: wgpu::BlendOperation::Add,
            },
            false,
        ),
        ffi::CameraFilterType::Subtract => (
            wgpu::BlendComponent {
                src_factor: wgpu::BlendFactor::SrcAlpha,
                dst_factor: wgpu::BlendFactor::One,
                operation: wgpu::BlendOperation::Subtract,
            },
            false,
        ),
        ffi::CameraFilterType::Blend => (
            wgpu::BlendComponent {
                src_factor: wgpu::BlendFactor::SrcAlpha,
                dst_factor: wgpu::BlendFactor::OneMinusSrcAlpha,
                operation: wgpu::BlendOperation::Add,
            },
            false,
        ),
        ffi::CameraFilterType::InvDstMultiply => (
            wgpu::BlendComponent {
                src_factor: wgpu::BlendFactor::Zero,
                dst_factor: wgpu::BlendFactor::OneMinusSrc,
                operation: wgpu::BlendOperation::Add,
            },
            true,
        ),
        _ => todo!(),
    };
    PipelineHolder {
        pipeline: device.create_render_pipeline(&wgpu::RenderPipelineDescriptor {
            label: Some("Colored Quad Pipeline"),
            layout: Some(&state.pipeline_layout),
            vertex: wgpu::VertexState {
                module: &state.shader,
                entry_point: "vs_main",
                buffers: &[wgpu::VertexBufferLayout {
                    array_stride: std::mem::size_of::<Vert>() as u64,
                    step_mode: wgpu::VertexStepMode::Vertex,
                    attributes: &vertex_attr_array![0 => Float32x3],
                }],
            },
            primitive: wgpu::PrimitiveState {
                topology: wgpu::PrimitiveTopology::TriangleStrip,
                ..Default::default()
            },
            depth_stencil: Some(wgpu::DepthStencilState {
                format: graphics.depth_format,
                depth_write_enabled: config.z_test,
                depth_compare: match config.z_comparison {
                    ffi::ZTest::Never => wgpu::CompareFunction::Never,
                    ffi::ZTest::Less => wgpu::CompareFunction::Less,
                    ffi::ZTest::Equal => wgpu::CompareFunction::Equal,
                    ffi::ZTest::LEqual => wgpu::CompareFunction::LessEqual,
                    ffi::ZTest::Greater => wgpu::CompareFunction::Greater,
                    ffi::ZTest::NEqual => wgpu::CompareFunction::NotEqual,
                    ffi::ZTest::GEqual => wgpu::CompareFunction::GreaterEqual,
                    ffi::ZTest::Always => wgpu::CompareFunction::Always,
                    _ => todo!(),
                },
                stencil: Default::default(),
                bias: Default::default(),
            }),
            multisample: wgpu::MultisampleState {
                count: graphics.msaa_samples,
                ..Default::default()
            },
            fragment: Some(wgpu::FragmentState {
                module: &state.shader,
                entry_point: "fs_main",
                targets: &[wgpu::ColorTargetState {
                    format: graphics.color_format,
                    blend: Some(wgpu::BlendState {
                        color: blend_component,
                        alpha: blend_component,
                    }),
                    write_mask: if alpha_write {
                        wgpu::ColorWrites::ALL
                    } else {
                        wgpu::ColorWrites::COLOR
                    },
                }],
            }),
            multiview: None,
        }),
    }
}

#[derive(Pod, Zeroable, Copy, Clone, Default, Debug)]
#[repr(C)]
struct Uniform {
    xf: CMatrix4f,
    color: CColor,
}

#[derive(Pod, Zeroable, Copy, Clone, Default, Debug)]
#[repr(C)]
struct Vert {
    pos: Vec3<f32>,
    //norm: Vec3<f32>,
}

pub(crate) fn queue_colored_quad(
    filter_type: ffi::CameraFilterType,
    z_comparison: ffi::ZTest,
    z_test: bool,
    color: CColor,
    rect: CRectangle,
    z: f32,
) {
    let pipeline = pipeline_ref(&PipelineCreateCommand::ColoredQuad(PipelineConfig {
        filter_type,
        z_comparison,
        z_test,
    }));
    let vert_range = push_verts(&[
        Vert { pos: Vec3::new(0.0, 0.0, z) },
        Vert { pos: Vec3::new(0.0, 1.0, z) },
        Vert { pos: Vec3::new(1.0, 0.0, z) },
        Vert { pos: Vec3::new(1.0, 1.0, z) },
    ]);
    let uniform_range = push_uniform(&Uniform {
        xf: CMatrix4f::new(
            CVector4f::new(rect.size.x * 2.0, 0.0, 0.0, 0.0),
            CVector4f::new(0.0, rect.size.y * 2.0, 0.0, 0.0),
            CVector4f::new(0.0, 0.0, 1.0, 0.0),
            CVector4f::new(rect.position.x * 2.0 - 1.0, rect.position.y * 2.0 - 1.0, 0.0, 1.0),
        ),
        color,
    });

    push_colored_quad(pipeline, vert_range, uniform_range);
}

pub(crate) fn queue_colored_quad_verts(
    filter_type: ffi::CameraFilterType,
    z_comparison: ffi::ZTest,
    z_test: bool,
    color: CColor,
    pos: &[CVector3f],
) {
    if pos.len() != 4 {
        panic!("Invalid pos: {}", pos.len());
    }

    let pipeline = pipeline_ref(&PipelineCreateCommand::ColoredQuad(PipelineConfig {
        filter_type,
        z_comparison,
        z_test,
    }));
    let vert_range = push_verts(
        &pos.iter().map(|pos| Vert { pos: pos.into() })
            .collect::<Vec<Vert>>(),
    );
    let uniform_range = push_uniform(&Uniform { xf: get_combined_matrix(), color});

    push_colored_quad(pipeline, vert_range, uniform_range);
}

fn push_colored_quad(
    pipeline: PipelineRef,
    vert_range: Range<u64>,
    uniform_range: Range<u64>,
) {
    // TODO defer bind group creation to draw time or another thread?
    let state = unsafe { STATE.as_mut().unwrap_unchecked() };

    push_draw_command(ShaderDrawCommand::ColoredQuad(DrawData {
        pipeline,
        vert_range,
        uniform_range,
    }));
}

pub(crate) fn draw_colored_quad<'a>(
    data: &DrawData,
    state: &'a State,
    pass: &mut wgpu::RenderPass<'a>,
    buffers: &'a BuiltBuffers,
) {
    if !bind_pipeline(data.pipeline, pass) {
        return;
    }

    // Uniform bind group
    pass.set_bind_group(0, &state.uniform_bind_group, &[
        data.uniform_range.start as wgpu::DynamicOffset
    ]);

    // Vertex buffer
    pass.set_vertex_buffer(0, buffers.vertex_buffer.slice(data.vert_range.clone()));
    pass.draw(0..4, 0..1);
}
