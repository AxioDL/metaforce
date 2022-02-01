use std::{hash::Hash, ops::Range};

use bytemuck_derive::{Pod, Zeroable};
use wgpu::{include_wgsl, vertex_attr_array};

use crate::{
    gpu::GraphicsConfig,
    shaders::{
        bind_pipeline, get_combined_matrix, pipeline_ref, push_draw_command, push_uniform,
        push_verts, BuiltBuffers, PipelineCreateCommand, PipelineHolder, PipelineRef,
        ShaderDrawCommand,
    },
    zeus::{CAABox, CColor, CMatrix4f, CVector3f},
};

#[derive(Debug, Clone)]
pub(crate) struct DrawData {
    pipeline: PipelineRef,
    vert_range: Range<u64>,
    uniform_range: Range<u64>,
}

#[derive(Hash)]
pub(crate) struct PipelineConfig {
    z_only: bool,
}
pub(crate) const INITIAL_PIPELINES: &[PipelineConfig] =
    &[PipelineConfig { z_only: false }, PipelineConfig { z_only: true }];

pub(crate) struct State {
    shader: wgpu::ShaderModule,
    uniform_layout: wgpu::BindGroupLayout,
    uniform_bind_group: wgpu::BindGroup,
    pipeline_layout: wgpu::PipelineLayout,
}

pub(crate) fn construct_state(
    device: &wgpu::Device,
    _queue: &wgpu::Queue,
    buffers: &BuiltBuffers,
) -> State {
    let shader = device.create_shader_module(&include_wgsl!("aabb.wgsl"));
    let uniform_size = wgpu::BufferSize::new(std::mem::size_of::<Uniform>() as u64);
    let uniform_layout = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
        label: Some("AABB Bind Group Layout"),
        entries: &[wgpu::BindGroupLayoutEntry {
            binding: 0,
            visibility: wgpu::ShaderStages::VERTEX_FRAGMENT,
            ty: wgpu::BindingType::Buffer {
                ty: wgpu::BufferBindingType::Uniform,
                has_dynamic_offset: true,
                min_binding_size: uniform_size,
            },
            count: None,
        }],
    });
    let uniform_bind_group = device.create_bind_group(&wgpu::BindGroupDescriptor {
        label: Some("AABB Bind Group"),
        layout: &uniform_layout,
        entries: &[wgpu::BindGroupEntry {
            binding: 0,
            resource: wgpu::BindingResource::Buffer(wgpu::BufferBinding {
                buffer: &buffers.uniform_buffer,
                offset: 0, // dynamic
                size: uniform_size,
            }),
        }],
    });
    let pipeline_layout = device.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
        label: Some("AABB Pipeline Layout"),
        bind_group_layouts: &[&uniform_layout],
        push_constant_ranges: &[],
    });
    State { shader, uniform_layout, uniform_bind_group, pipeline_layout }
}

pub(crate) fn construct_pipeline(
    device: &wgpu::Device,
    graphics: &GraphicsConfig,
    state: &State,
    config: &PipelineConfig,
) -> PipelineHolder {
    PipelineHolder {
        pipeline: device.create_render_pipeline(&wgpu::RenderPipelineDescriptor {
            label: Some("AABB Pipeline"),
            layout: Some(&state.pipeline_layout),
            vertex: wgpu::VertexState {
                module: &state.shader,
                entry_point: "vs_main",
                buffers: &[wgpu::VertexBufferLayout {
                    array_stride: std::mem::size_of::<CVector3f>() as u64,
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
                depth_write_enabled: true,
                depth_compare: wgpu::CompareFunction::LessEqual,
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
                        color: wgpu::BlendComponent {
                            src_factor: wgpu::BlendFactor::SrcAlpha,
                            dst_factor: wgpu::BlendFactor::OneMinusSrcAlpha,
                            operation: wgpu::BlendOperation::Add,
                        },
                        alpha: wgpu::BlendComponent {
                            src_factor: wgpu::BlendFactor::SrcAlpha,
                            dst_factor: wgpu::BlendFactor::OneMinusSrcAlpha,
                            operation: wgpu::BlendOperation::Add,
                        },
                    }),
                    write_mask: if config.z_only {
                        wgpu::ColorWrites::empty()
                    } else {
                        wgpu::ColorWrites::ALL
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

pub(crate) fn queue_aabb(aabb: CAABox, color: CColor, z_only: bool) {
    let pipeline = pipeline_ref(PipelineCreateCommand::Aabb(PipelineConfig { z_only }));
    let vert_range = push_verts(&[
        CVector3f::new(aabb.max.x, aabb.max.y, aabb.min.z),
        CVector3f::new(aabb.max.x, aabb.min.y, aabb.min.z),
        CVector3f::new(aabb.max.x, aabb.max.y, aabb.max.z),
        CVector3f::new(aabb.max.x, aabb.min.y, aabb.max.z),
        CVector3f::new(aabb.max.x, aabb.min.y, aabb.max.z),
        CVector3f::new(aabb.min.x, aabb.max.y, aabb.min.z),
        CVector3f::new(aabb.min.x, aabb.max.y, aabb.min.z),
        CVector3f::new(aabb.max.x, aabb.max.y, aabb.min.z),
        CVector3f::new(aabb.min.x, aabb.max.y, aabb.max.z),
        CVector3f::new(aabb.max.x, aabb.max.y, aabb.max.z),
        CVector3f::new(aabb.max.x, aabb.max.y, aabb.max.z),
        CVector3f::new(aabb.min.x, aabb.max.y, aabb.min.z),
        CVector3f::new(aabb.min.x, aabb.max.y, aabb.min.z),
        CVector3f::new(aabb.min.x, aabb.min.y, aabb.min.z),
        CVector3f::new(aabb.min.x, aabb.max.y, aabb.max.z),
        CVector3f::new(aabb.min.x, aabb.min.y, aabb.max.z),
        CVector3f::new(aabb.min.x, aabb.min.y, aabb.max.z),
        CVector3f::new(aabb.min.x, aabb.min.y, aabb.min.z),
        CVector3f::new(aabb.min.x, aabb.min.y, aabb.min.z),
        CVector3f::new(aabb.max.x, aabb.min.y, aabb.min.z),
        CVector3f::new(aabb.min.x, aabb.min.y, aabb.max.z),
        CVector3f::new(aabb.max.x, aabb.min.y, aabb.max.z),
        CVector3f::new(aabb.max.x, aabb.min.y, aabb.max.z),
        CVector3f::new(aabb.min.x, aabb.min.y, aabb.max.z),
        CVector3f::new(aabb.min.x, aabb.min.y, aabb.max.z),
        CVector3f::new(aabb.max.x, aabb.min.y, aabb.max.z),
        CVector3f::new(aabb.min.x, aabb.max.y, aabb.max.z),
        CVector3f::new(aabb.max.x, aabb.max.y, aabb.max.z),
        CVector3f::new(aabb.max.x, aabb.max.y, aabb.max.z),
        CVector3f::new(aabb.min.x, aabb.min.y, aabb.min.z),
        CVector3f::new(aabb.min.x, aabb.min.y, aabb.min.z),
        CVector3f::new(aabb.max.x, aabb.min.y, aabb.min.z),
        CVector3f::new(aabb.min.x, aabb.max.y, aabb.min.z),
        CVector3f::new(aabb.max.x, aabb.max.y, aabb.min.z),
    ]);
    let uniform_range = push_uniform(&Uniform { xf: get_combined_matrix(), color });
    push_draw_command(ShaderDrawCommand::Aabb(DrawData { pipeline, vert_range, uniform_range }));
}

pub(crate) fn draw_aabb<'a>(
    data: &DrawData,
    state: &'a State,
    pass: &mut wgpu::RenderPass<'a>,
    buffers: &'a BuiltBuffers,
) {
    if !bind_pipeline(data.pipeline, pass) {
        return;
    }
    pass.set_bind_group(0, &state.uniform_bind_group, &[
        data.uniform_range.start as wgpu::DynamicOffset
    ]);
    pass.set_vertex_buffer(0, buffers.vertex_buffer.slice(data.vert_range.clone()));
    pass.draw(0..34, 0..1);
}
