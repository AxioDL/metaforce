#![allow(dead_code)]
#![allow(unused_imports)]
#![allow(unused_variables)]
#![allow(unused_unsafe)]

use std::{
    collections::{BTreeMap, HashMap},
    time::Instant,
};

use cxxbridge::ffi;
use sdl2::{
    controller::{Axis, Button, GameController},
    event::Event as SDLEvent,
    GameControllerSubsystem, Sdl,
};
use wgpu::Backend;
use winit::{
    event::{ElementState, Event, KeyboardInput, VirtualKeyCode, WindowEvent},
    event_loop::ControlFlow,
};

use crate::{
    gpu::{create_depth_texture, create_render_texture, initialize_gpu, DeviceHolder},
    imgui::{initialize_imgui, ImGuiState},
    sdl::{
        get_controller_player_index, initialize_sdl, poll_sdl_events, remap_controller_layout,
        set_controller_player_index, SdlState,
    },
    shaders::render_into_pass,
};

mod cxxbridge;
mod gpu;
mod imgui;
mod imgui_backend;
mod sdl;
mod shaders;
mod util;
mod zeus;

pub struct App {
    window: ffi::Window,
    gpu: DeviceHolder,
    imgui: ImGuiState,
    sdl: SdlState,
}

pub struct WindowContext {
    window: winit::window::Window,
}

static mut APP: Option<App> = None;

fn app_run(mut delegate: cxx::UniquePtr<ffi::AppDelegate>, icon: ffi::Icon) {
    if delegate.is_null() {
        return;
    }
    env_logger::init();
    log::info!("Running app");
    let event_loop = winit::event_loop::EventLoop::new();
    let window_icon = winit::window::Icon::from_rgba(icon.data, icon.width, icon.height)
        .expect("Failed to load icon");
    let window = winit::window::WindowBuilder::new()
        .with_inner_size(winit::dpi::LogicalSize::new(1280, 720))
        .with_window_icon(Some(window_icon))
        .build(&event_loop)
        .unwrap();
    let sdl = initialize_sdl();
    let gpu = initialize_gpu(&window);
    let imgui = initialize_imgui(&window, &gpu);
    let mut special_keys_pressed: [bool; 512] = [false; 512];
    shaders::construct_state(gpu.device.clone(), gpu.queue.clone(), &gpu.config);
    let app =
        App { window: ffi::Window { inner: Box::new(WindowContext { window }) }, gpu, imgui, sdl };
    let window_size = ffi::WindowSize {
        width: app.gpu.surface_config.width,
        height: app.gpu.surface_config.height,
    };
    unsafe {
        APP.replace(app);
        ffi::App_onAppLaunched(delegate.as_mut().unwrap());
        ffi::App_onAppWindowResized(delegate.as_mut().unwrap_unchecked(), &window_size);
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

        // SDL event loop
        if !poll_sdl_events(&mut app.sdl, &mut delegate) {
            *control_flow = ControlFlow::Exit;
            return;
        }

        // winit event loop
        *control_flow = ControlFlow::Poll;
        match event {
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
                let window_size = ffi::WindowSize {
                    width: app.gpu.surface_config.width,
                    height: app.gpu.surface_config.height,
                };
                unsafe {
                    ffi::App_onAppWindowResized(delegate.as_mut().unwrap_unchecked(), &window_size);
                }
            }
            Event::WindowEvent { event: WindowEvent::Moved(loc), .. } => unsafe {
                ffi::App_onAppWindowMoved(delegate.as_mut().unwrap_unchecked(), loc.x, loc.y);
            },
            Event::WindowEvent {
                event:
                    WindowEvent::KeyboardInput {
                        input: KeyboardInput { scancode, virtual_keycode: Some(key), state, .. },
                        ..
                    },
                ..
            } => {
                // TODO: Handle normal keys, this will require a refactor in game runtime code
                let special_key = match key {
                    VirtualKeyCode::F1 => ffi::SpecialKey::F1,
                    VirtualKeyCode::F2 => ffi::SpecialKey::F2,
                    VirtualKeyCode::F3 => ffi::SpecialKey::F3,
                    VirtualKeyCode::F4 => ffi::SpecialKey::F4,
                    VirtualKeyCode::F5 => ffi::SpecialKey::F5,
                    VirtualKeyCode::F6 => ffi::SpecialKey::F6,
                    VirtualKeyCode::F7 => ffi::SpecialKey::F7,
                    VirtualKeyCode::F8 => ffi::SpecialKey::F8,
                    VirtualKeyCode::F9 => ffi::SpecialKey::F9,
                    VirtualKeyCode::F10 => ffi::SpecialKey::F10,
                    VirtualKeyCode::F11 => ffi::SpecialKey::F11,
                    VirtualKeyCode::F12 => ffi::SpecialKey::F12,
                    VirtualKeyCode::Escape => ffi::SpecialKey::Esc,
                    VirtualKeyCode::Return => ffi::SpecialKey::Enter,
                    VirtualKeyCode::Back => ffi::SpecialKey::Backspace,
                    VirtualKeyCode::Insert => ffi::SpecialKey::Insert,
                    VirtualKeyCode::Delete => ffi::SpecialKey::Delete,
                    VirtualKeyCode::Home => ffi::SpecialKey::Home,
                    VirtualKeyCode::PageUp => ffi::SpecialKey::PgUp,
                    VirtualKeyCode::PageDown => ffi::SpecialKey::PgDown,
                    VirtualKeyCode::Left => ffi::SpecialKey::Left,
                    VirtualKeyCode::Right => ffi::SpecialKey::Right,
                    VirtualKeyCode::Up => ffi::SpecialKey::Up,
                    VirtualKeyCode::Down => ffi::SpecialKey::Down,
                    VirtualKeyCode::Tab => ffi::SpecialKey::Tab,
                    _ => ffi::SpecialKey::None,
                };

                if special_key != ffi::SpecialKey::None {
                    let pressed = state == ElementState::Pressed;
                    let repeat = special_keys_pressed[key as usize] == pressed;
                    special_keys_pressed[key as usize] = pressed;

                    unsafe {
                        if pressed {
                            ffi::App_onSpecialKeyDown(
                                delegate.as_mut().unwrap_unchecked(),
                                special_key,
                                repeat,
                            );
                        } else {
                            ffi::App_onSpecialKeyUp(
                                delegate.as_mut().unwrap_unchecked(),
                                special_key,
                            );
                        }
                    }
                }
            }
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
                    ffi::App_onAppDraw(delegate.as_mut().unwrap_unchecked());
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
                    ffi::App_onAppPostDraw(delegate.as_mut().unwrap_unchecked());
                }
            }
            Event::LoopDestroyed => unsafe {
                ffi::App_onAppExiting(delegate.as_mut().unwrap_unchecked());
                APP.take();
                return;
            },
            _ => {}
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
