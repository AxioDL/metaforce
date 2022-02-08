#![allow(dead_code)]
#![allow(unused_imports)]
#![allow(unused_variables)]
#![allow(unused_unsafe)]

use std::{collections::HashMap, time::Instant};

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
    ffi::{SpecialKey, WindowSize},
    gpu::{create_depth_texture, create_render_texture, initialize_gpu, DeviceHolder},
    imgui::{initialize_imgui, ImGuiState},
    shaders::render_into_pass,
};

mod gpu;
mod imgui;
mod imgui_backend;
mod shaders;
mod util;
mod zeus;

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
        pub(crate) fn App_onAppWindowResized(cb: Pin<&mut AppDelegate>, size: &WindowSize);
        pub(crate) fn App_onAppWindowMoved(cb: Pin<&mut AppDelegate>, x: i32, y: i32);
        pub(crate) fn App_onAppExiting(cb: Pin<&mut AppDelegate>);
        // Input
        pub(crate) fn App_onCharKeyDown(cb: Pin<&mut AppDelegate>, code: u8, is_repeat: bool);
        pub(crate) fn App_onCharKeyUp(cb: Pin<&mut AppDelegate>, code: u8);
        pub(crate) fn App_onSpecialKeyDown(
            cb: Pin<&mut AppDelegate>,
            key: SpecialKey,
            is_repeat: bool,
        );
        pub(crate) fn App_onSpecialKeyUp(cb: Pin<&mut AppDelegate>, key: SpecialKey);
        // Controller
        pub(crate) fn App_onControllerButton(
            cb: Pin<&mut AppDelegate>,
            idx: u32,
            button: ControllerButton,
            pressed: bool,
        );
        pub(crate) fn App_onControllerAxis(
            cb: Pin<&mut AppDelegate>,
            idx: u32,
            axis: ControllerAxis,
            value: i16,
        );
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

    pub enum ControllerButton {
        A,
        B,
        X,
        Y,
        Back,
        Guide,
        Start,
        LeftStick,
        RightStick,
        LeftShoulder,
        RightShoulder,
        DPadUp,
        DPadDown,
        DPadLeft,
        DPadRight,
        Other,
        MAX,
    }
    pub enum ControllerAxis {
        LeftX,
        LeftY,
        RightX,
        RightY,
        TriggerLeft,
        TriggerRight,
        MAX,
    }
    pub struct Icon {
        pub data: Vec<u8>,
        pub width: u32,
        pub height: u32,
    }

    extern "Rust" {
        type WindowContext;
        type App;
        fn app_run(mut delegate: UniquePtr<AppDelegate>, icon: Icon);
        fn get_args() -> Vec<String>;
        fn get_window_size() -> WindowSize;
        fn set_window_title(title: &CxxString);
        fn get_dxt_compression_supported() -> bool;
        fn get_backend() -> Backend;
        fn get_backend_string() -> &'static str;
        fn set_fullscreen(v: bool);
        fn get_controller_player_index(which: u32) -> i32;
        fn set_controller_player_index(which: u32, index: i32);
    }
}
impl From<Button> for ffi::ControllerButton {
    fn from(button: Button) -> Self {
        match button {
            Button::A => ffi::ControllerButton::A,
            Button::B => ffi::ControllerButton::B,
            Button::X => ffi::ControllerButton::X,
            Button::Y => ffi::ControllerButton::Y,
            Button::Back => ffi::ControllerButton::Back,
            Button::Guide => ffi::ControllerButton::Guide,
            Button::Start => ffi::ControllerButton::Start,
            Button::LeftStick => ffi::ControllerButton::LeftStick,
            Button::RightStick => ffi::ControllerButton::RightStick,
            Button::LeftShoulder => ffi::ControllerButton::LeftShoulder,
            Button::RightShoulder => ffi::ControllerButton::RightShoulder,
            Button::DPadUp => ffi::ControllerButton::DPadUp,
            Button::DPadDown => ffi::ControllerButton::DPadDown,
            Button::DPadLeft => ffi::ControllerButton::DPadLeft,
            Button::DPadRight => ffi::ControllerButton::DPadRight,
            _ => ffi::ControllerButton::Other,
        }
    }
}
impl From<Axis> for ffi::ControllerAxis {
    fn from(axis: Axis) -> Self {
        match axis {
            Axis::LeftX => ffi::ControllerAxis::LeftX,
            Axis::LeftY => ffi::ControllerAxis::LeftY,
            Axis::RightX => ffi::ControllerAxis::RightX,
            Axis::RightY => ffi::ControllerAxis::RightY,
            Axis::TriggerLeft => ffi::ControllerAxis::TriggerLeft,
            Axis::TriggerRight => ffi::ControllerAxis::TriggerRight,
        }
    }
}

pub struct App {
    window: ffi::Window,
    gpu: DeviceHolder,
    imgui: ImGuiState,
    // SDL
    sdl: Sdl,
    sdl_events: sdl2::EventPump,
    sdl_controller_sys: GameControllerSubsystem,
    sdl_open_controllers: HashMap<u32, GameController>,
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
    let window_icon = winit::window::Icon::from_rgba(icon.data, icon.width, icon.height).expect("Failed to load icon");
    let window = winit::window::WindowBuilder::new()
        .with_inner_size(winit::dpi::LogicalSize::new(1280, 720))
        .with_window_icon(Some(window_icon))
        .build(&event_loop)
        .unwrap();
    let sdl = sdl2::init().unwrap();
    let sdl_events = sdl.event_pump().unwrap();
    let controller = sdl.game_controller().unwrap();
    let gpu = initialize_gpu(&window);
    let imgui = initialize_imgui(&window, &gpu);
    let mut special_keys_pressed: [bool; 512] = [false; 512];
    shaders::construct_state(gpu.device.clone(), gpu.queue.clone(), &gpu.config);
    let app = App {
        window: ffi::Window { inner: Box::new(WindowContext { window }) },
        gpu,
        imgui,
        sdl,
        sdl_events,
        sdl_controller_sys: controller,
        sdl_open_controllers: Default::default(),
    };
    let window_size = WindowSize {
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
        for event in app.sdl_events.poll_iter() {
            // log::info!("SDL event: {:?}", event);
            match event {
                SDLEvent::ControllerDeviceAdded { which, .. } => {
                    match app.sdl_controller_sys.open(which) {
                        Ok(controller) => {
                            log::info!("Opened SDL controller \"{}\"", controller.name());
                            app.sdl_open_controllers.insert(which, controller);
                        }
                        Err(err) => {
                            log::warn!("Failed to open SDL controller {} ({:?})", which, err);
                        }
                    }
                    // TODO app connected event
                }
                SDLEvent::ControllerDeviceRemoved { which, .. } => {
                    app.sdl_open_controllers.remove(&which);
                    // TODO app disconnected event
                }
                SDLEvent::ControllerButtonDown { which, button, .. } => unsafe {
                    ffi::App_onControllerButton(
                        delegate.as_mut().unwrap_unchecked(),
                        which,
                        button.into(),
                        true,
                    );
                },
                SDLEvent::ControllerButtonUp { which, button, .. } => unsafe {
                    ffi::App_onControllerButton(
                        delegate.as_mut().unwrap_unchecked(),
                        which,
                        button.into(),
                        false,
                    );
                },
                SDLEvent::ControllerAxisMotion { which, axis, value, .. } => unsafe {
                    ffi::App_onControllerAxis(
                        delegate.as_mut().unwrap_unchecked(),
                        which,
                        axis.into(),
                        value,
                    );
                },
                // SDL overrides exit signals
                SDLEvent::Quit { .. } => {
                    *control_flow = ControlFlow::Exit;
                    return;
                }
                _ => {}
            }
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
                let window_size = WindowSize {
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
                    VirtualKeyCode::F1 => SpecialKey::F1,
                    VirtualKeyCode::F2 => SpecialKey::F2,
                    VirtualKeyCode::F3 => SpecialKey::F3,
                    VirtualKeyCode::F4 => SpecialKey::F4,
                    VirtualKeyCode::F5 => SpecialKey::F5,
                    VirtualKeyCode::F6 => SpecialKey::F6,
                    VirtualKeyCode::F7 => SpecialKey::F7,
                    VirtualKeyCode::F8 => SpecialKey::F8,
                    VirtualKeyCode::F9 => SpecialKey::F9,
                    VirtualKeyCode::F10 => SpecialKey::F10,
                    VirtualKeyCode::F11 => SpecialKey::F11,
                    VirtualKeyCode::F12 => SpecialKey::F12,
                    VirtualKeyCode::Escape => SpecialKey::Esc,
                    VirtualKeyCode::Return => SpecialKey::Enter,
                    VirtualKeyCode::Back => SpecialKey::Backspace,
                    VirtualKeyCode::Insert => SpecialKey::Insert,
                    VirtualKeyCode::Delete => SpecialKey::Delete,
                    VirtualKeyCode::Home => SpecialKey::Home,
                    VirtualKeyCode::PageUp => SpecialKey::PgUp,
                    VirtualKeyCode::PageDown => SpecialKey::PgDown,
                    VirtualKeyCode::Left => SpecialKey::Left,
                    VirtualKeyCode::Right => SpecialKey::Right,
                    VirtualKeyCode::Up => SpecialKey::Up,
                    VirtualKeyCode::Down => SpecialKey::Down,
                    VirtualKeyCode::Tab => SpecialKey::Tab,
                    _ => SpecialKey::None,
                };

                if special_key != SpecialKey::None {
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

fn get_controller_player_index(which: u32) -> i32 {
    let mut result: i32 = -1;
    for (key, value) in &get_app().sdl_open_controllers {
        if value.instance_id() == which {
            result = value.player_index();
            break;
        }
    }

    result as i32
}

fn set_controller_player_index(which: u32, index: i32) {

    for (key, value) in &get_app().sdl_open_controllers {
        if value.instance_id() == which {
            value.set_player_index(index);
            break;
        }
    }
}
