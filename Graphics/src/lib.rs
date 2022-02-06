#![allow(dead_code)]
#![allow(unused_imports)]
#![allow(unused_variables)]
#![allow(unused_unsafe)]

use std::{num::NonZeroU8, time::Instant};

use wgpu::Backend;
use winit::{
    event::{Event, WindowEvent},
    event_loop::ControlFlow,
};

use crate::{
    gpu::{create_depth_texture, create_render_texture, initialize_gpu, DeviceHolder},
    imgui::{initialize_imgui, ImGuiState},
    shaders::render_into_pass,
};

mod gpu;
mod imgui;
mod imgui_backend;
mod shaders;
mod zeus;
mod util;

#[cxx::bridge(namespace = "aurora")]
mod ffi {
    unsafe extern "C++" {
        include!("aurora.hpp");
        pub(crate) type AppDelegate;
    }

    unsafe extern "C++" {
        include!("lib.hpp");
        pub(crate) fn App_onAppLaunched(cb: Pin<&mut AppDelegate>);
        pub(crate) fn App_onAppIdle(cb: Pin<&mut AppDelegate>, dt: f32) -> bool;
        pub(crate) fn App_onAppDraw(cb: Pin<&mut AppDelegate>);
        pub(crate) fn App_onAppPostDraw(cb: Pin<&mut AppDelegate>);
        pub(crate) fn App_onAppExiting(cb: Pin<&mut AppDelegate>);
    }

    pub struct Window {
        pub(crate) inner: Box<WindowContext>,
    }
    pub struct WindowSize {
        pub width: u32,
        pub height: u32,
    }
    #[derive(Debug)]
    pub enum Backend {
        Invalid,
        Vulkan,
        Metal,
        D3D12,
        D3D11,
        OpenGL,
        WebGPU,
    }

    pub enum ElementState {
        Pressed,
        Released,
    }
    pub enum MouseButton {
        Left,
        Right,
        Middle,
        Other,
    }
    pub enum SpecialKey {
        None = 0,
        F1 = 1,
        F2 = 2,
        F3 = 3,
        F4 = 4,
        F5 = 5,
        F6 = 6,
        F7 = 7,
        F8 = 8,
        F9 = 9,
        F10 = 10,
        F11 = 11,
        F12 = 12,
        Esc = 13,
        Enter = 14,
        Backspace = 15,
        Insert = 16,
        Delete = 17,
        Home = 18,
        End = 19,
        PgUp = 20,
        PgDown = 21,
        Left = 22,
        Right = 23,
        Up = 24,
        Down = 25,
        Tab = 26,
    }
    pub struct KeyboardInput {
        pub scancode: u32,
        pub state: ElementState,
        // pub
    }

    extern "Rust" {
        type WindowContext;
        type App;
        fn app_run(mut delegate: UniquePtr<AppDelegate>);
        fn get_args() -> Vec<String>;
        fn get_window_size() -> WindowSize;
        fn set_window_title(title: &CxxString);
        fn get_dxt_compression_supported() -> bool;
        fn get_backend() -> Backend;
        fn get_backend_string() -> &'static str;
        fn set_fullscreen(v: bool);
    }
}

pub struct App {
    window: ffi::Window,
    gpu: DeviceHolder,
    imgui: ImGuiState,
}

pub struct WindowContext {
    window: winit::window::Window,
}

static mut APP: Option<App> = None;

fn app_run(mut delegate: cxx::UniquePtr<ffi::AppDelegate>) {
    if delegate.is_null() {
        return;
    }
    env_logger::init();
    log::info!("Running app");
    let event_loop = winit::event_loop::EventLoop::new();
    let window = winit::window::WindowBuilder::new().build(&event_loop).unwrap();
    let gpu = initialize_gpu(&window);
    let imgui = initialize_imgui(&window, &gpu);
    shaders::construct_state(gpu.device.clone(), gpu.queue.clone(), &gpu.config);
    let app = App { window: ffi::Window { inner: Box::new(WindowContext { window }) }, gpu, imgui };
    unsafe {
        APP.replace(app);
        ffi::App_onAppLaunched(delegate.as_mut().unwrap());
    };

    let mut last_frame: Option<Instant> = None;
    event_loop.run(move |event, _, control_flow| {
        // Have the closure take ownership of the resources.
        // `event_loop.run` never returns, therefore we must do this to ensure
        // the resources are properly cleaned up.
        let _ = &delegate;

        let app = get_app();
        let imgui = &mut app.imgui;
        let window_ctx = get_window_context();
        let gpu = &mut app.gpu;

        *control_flow = ControlFlow::Poll;
        match event {
            Event::NewEvents(_) => {}
            Event::WindowEvent { event: WindowEvent::CloseRequested, .. } => {
                *control_flow = ControlFlow::Exit;
                return;
            }
            Event::WindowEvent { event: WindowEvent::Resized(size), .. } => {
                // Reconfigure the surface with the new size
                app.gpu.surface_config.width = size.width;
                app.gpu.surface_config.height = size.height;
                app.gpu.surface.configure(&app.gpu.device, &app.gpu.surface_config);
                app.gpu.depth =
                    create_depth_texture(&app.gpu.device, &app.gpu.surface_config, &app.gpu.config);
                app.gpu.framebuffer = create_render_texture(
                    &app.gpu.device,
                    &app.gpu.surface_config,
                    &app.gpu.config,
                );
                // TODO resize callback
            }
            Event::WindowEvent { event: WindowEvent::KeyboardInput { .. }, .. } => {}
            Event::WindowEvent { .. } => {}
            Event::DeviceEvent { .. } => {}
            Event::UserEvent(_) => {}
            Event::Suspended => {}
            Event::Resumed => {}
            Event::MainEventsCleared => {
                log::trace!("Requesting redraw");
                window_ctx.window.request_redraw();
            }
            Event::RedrawRequested(_) => {
                let io = imgui.context.io_mut();
                let mut dt: f32 = 1.0 / 60.0;
                if let Some(last_frame) = last_frame {
                    let duration = last_frame.elapsed();
                    dt = duration.as_secs_f32();
                    io.update_delta_time(duration);
                }
                last_frame = Some(Instant::now());

                imgui
                    .platform
                    .prepare_frame(io, &window_ctx.window)
                    .expect("Failed to prepare frame");
                let ui = imgui.context.frame();

                #[allow(unused_unsafe)] // rust bug?
                if !unsafe { ffi::App_onAppIdle(delegate.as_mut().unwrap(), dt) } {
                    *control_flow = ControlFlow::Exit;
                    return;
                }

                log::trace!("Redrawing");
                let frame_result = gpu.surface.get_current_texture();
                if let Err(err) = frame_result {
                    log::warn!("Failed to acquire frame {}", err);
                    return;
                }
                let frame = frame_result.unwrap();

                #[allow(unused_unsafe)] // rust bug?
                unsafe {
                    ffi::App_onAppDraw(delegate.as_mut().unwrap());
                }

                let fb_view =
                    gpu.framebuffer.texture.create_view(&wgpu::TextureViewDescriptor::default());
                let view = frame.texture.create_view(&wgpu::TextureViewDescriptor::default());
                let mut encoder =
                    gpu.device.create_command_encoder(&wgpu::CommandEncoderDescriptor {
                        label: Some("Redraw Encoder"),
                    });
                {
                    let mut rpass = encoder.begin_render_pass(&wgpu::RenderPassDescriptor {
                        label: None,
                        color_attachments: &[if gpu.config.msaa_samples > 1 {
                            wgpu::RenderPassColorAttachment {
                                view: &fb_view,
                                resolve_target: Some(&view),
                                ops: wgpu::Operations {
                                    load: wgpu::LoadOp::Clear(wgpu::Color::BLACK),
                                    store: true,
                                },
                            }
                        } else {
                            wgpu::RenderPassColorAttachment {
                                view: &view,
                                resolve_target: None,
                                ops: wgpu::Operations {
                                    load: wgpu::LoadOp::Clear(wgpu::Color::BLACK),
                                    store: true,
                                },
                            }
                        }],
                        depth_stencil_attachment: Some(wgpu::RenderPassDepthStencilAttachment {
                            view: &gpu.depth.view,
                            depth_ops: Some(wgpu::Operations {
                                load: wgpu::LoadOp::Clear(1.0),
                                store: true,
                            }),
                            stencil_ops: None,
                        }),
                    });

                    render_into_pass(&mut rpass);

                    imgui.platform.prepare_render(&ui, &window_ctx.window);
                    imgui
                        .backend
                        .render(ui.render(), &gpu.queue, &gpu.device, &mut rpass)
                        .expect("Rendering failed");
                }

                gpu.queue.submit(Some(encoder.finish()));
                frame.present();

                #[allow(unused_unsafe)] // rust bug?
                unsafe {
                    ffi::App_onAppPostDraw(delegate.as_mut().unwrap());
                }
            }
            Event::RedrawEventsCleared => {}
            Event::LoopDestroyed => unsafe {
                ffi::App_onAppExiting(delegate.as_mut().unwrap());
                APP.take();
                return;
            },
        }

        imgui.platform.handle_event(imgui.context.io_mut(), &window_ctx.window, &event);
    });
}

fn get_app() -> &'static mut App {
    unsafe {
        if let Some(ref mut app) = APP {
            app
        } else {
            panic!("Application not initialized");
        }
    }
}

fn get_window_context() -> &'static mut WindowContext { &mut get_app().window.inner }

fn get_args() -> Vec<String> { std::env::args().skip(1).collect() }

fn get_window_size() -> ffi::WindowSize {
    let size = get_window_context().window.inner_size();
    ffi::WindowSize { width: size.width, height: size.height }
}

fn set_window_title(title: &cxx::CxxString) {
    get_window_context().window.set_title(&title.to_string_lossy());
}

fn get_backend() -> ffi::Backend {
    match get_app().gpu.backend {
        Backend::Vulkan => ffi::Backend::Vulkan,
        Backend::Metal => ffi::Backend::Metal,
        Backend::Dx12 => ffi::Backend::D3D12,
        Backend::Dx11 => ffi::Backend::D3D11,
        Backend::Gl => ffi::Backend::OpenGL,
        Backend::BrowserWebGpu => ffi::Backend::WebGPU,
        Backend::Empty => ffi::Backend::Invalid,
    }
}

fn get_dxt_compression_supported() -> bool {
    return get_app().gpu.adapter.features().contains(wgpu::Features::TEXTURE_COMPRESSION_BC);
}

fn get_backend_string() -> &'static str {
    match get_app().gpu.backend {
        Backend::Vulkan => "Vulkan",
        Backend::Metal => "Metal",
        Backend::Dx12 => "D3D12",
        Backend::Dx11 => "D3D11",
        Backend::Gl => "OpenGL ES",
        Backend::BrowserWebGpu => "WebGPU",
        Backend::Empty => "Invalid",
    }
}

fn set_fullscreen(v: bool) {
    get_window_context().window.set_fullscreen(if v {
        Some(winit::window::Fullscreen::Borderless(None))
    } else {
        None
    });
}
