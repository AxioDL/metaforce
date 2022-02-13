use sdl2::controller::{Axis, Button};

use crate::{
    App, app_run, get_args, get_backend, get_backend_string,
    get_dxt_compression_supported,
    get_window_size,
    sdl::{
        get_controller_name,
        get_controller_player_index,
        is_controller_gamecube,
        set_controller_player_index,
    }, set_fullscreen, set_window_title, WindowContext,
};

#[cxx::bridge(namespace = "aurora")]
pub(crate) mod ffi {
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
        pub(crate) fn App_onCharKeyDown(cb: Pin<&mut AppDelegate>,
                                        code: u8,
                                        mods: u16,
                                        is_repeat: bool,
        );
        pub(crate) fn App_onCharKeyUp(
            cb: Pin<&mut AppDelegate>,
            code: u8,
            mods: u16,
        );
        pub(crate) fn App_onSpecialKeyDown(
            cb: Pin<&mut AppDelegate>,
            key: SpecialKey,
            keymod: u16,
            is_repeat: bool,
        );
        pub(crate) fn App_onSpecialKeyUp(
            cb: Pin<&mut AppDelegate>,
            key: SpecialKey,
            mods: u16,
        );
        // Controller
        pub(crate) fn App_onControllerAdded(cb: Pin<&mut AppDelegate>, which: u32);
        pub(crate) fn App_onControllerRemoved(cb: Pin<&mut AppDelegate>, which: u32);
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
        F13 = 13,
        F14 = 14,
        F15 = 15,
        F16 = 16,
        F17 = 17,
        F18 = 18,
        F19 = 19,
        F20 = 20,
        F21 = 21,
        F22 = 22,
        F23 = 23,
        F24 = 24,
        Esc = 25,
        Enter = 26,
        Backspace = 27,
        Insert = 28,
        Delete = 29,
        Home = 30,
        End = 31,
        PgUp = 32,
        PgDown = 33,
        Left = 34,
        Right = 35,
        Up = 36,
        Down = 37,
        Tab = 38,
        PrintScreen = 39,
        ScrollLock = 40,
        Pause = 41,
        NumLockClear = 42,
        KpDivide = 43,
        KpMultiply = 44,
        KpMinus = 45,
        KpPlus = 46,
        KpEnter = 47,
        KpNum0 = 48,
        KpNum1 = 49,
        KpNum2 = 51,
        KpNum3 = 52,
        KpNum4 = 53,
        KpNum5 = 54,
        KpNum6 = 55,
        KpNum7 = 56,
        KpNum8 = 57,
        KpNum9 = 58,
        KpPercent = 59,
        KpPeriod = 60,
        KpComma = 61,
        KpEquals = 62,
        Application = 63,
        Power = 64,
        Execute = 65,
        Help = 66,
        Menu = 67,
        Select = 68,
        Stop = 69,
        Again = 70,
        Undo = 71,
        Cut = 72,
        Paste = 73,
        Find = 74,
        VolumeUp = 75,
        VolumeDown = 76,
        MAX,
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
        fn is_controller_gamecube(which: u32) -> bool;
        fn get_controller_name(which: u32) -> String;
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
