use std::collections::{BTreeMap, HashMap};

use bitflags::bitflags;
use sdl2::{
    controller::{Axis, GameController},
    event::Event,
    GameControllerSubsystem, Sdl,
};
use sdl2::keyboard::{Keycode, Mod};

use crate::{cxxbridge::ffi, get_app};
use crate::ffi::SpecialKey;

pub(crate) struct SdlState {
    context: Sdl,
    events: sdl2::EventPump,
    controller_sys: GameControllerSubsystem,
    open_controllers: HashMap<u32, GameController>,
}

pub(crate) fn initialize_sdl() -> SdlState {
    let context = sdl2::init().unwrap();
    let events = context.event_pump().unwrap();
    let controller_sys = context.game_controller().unwrap();
    SdlState { context, events, controller_sys, open_controllers: Default::default() }
}
bitflags! {
    pub struct ModifierKey : u16 {
        const None = 0;
        const LeftShift = 0x0001;
        const RightShift = 0x0002;
        const LeftControl = 0x0004;
        const RightControl = 0x0008;
        const LeftAlt = 0x0010;
        const RightAlt = 0x0020;
        const LeftGui = 0x0040;
        const RightGui = 0x0080;
        const Num = 0x0100;
        const Caps = 0x0200;
        const Mode = 0x0400;
        // SDL has a reserved value we don't need
    }
}

fn translate_modifiers(keymod: Mod) -> ModifierKey {
    let mut use_mod: ModifierKey = ModifierKey::None;
    if keymod.contains(Mod::LSHIFTMOD) {
        use_mod.insert(ModifierKey::LeftShift);
    }
    if keymod.contains(Mod::RSHIFTMOD) {
        use_mod.insert(ModifierKey::RightShift);
    }
    if keymod.contains(Mod::LCTRLMOD) {
        use_mod.insert(ModifierKey::LeftControl);
    }
    if keymod.contains(Mod::RCTRLMOD) {
        use_mod.insert(ModifierKey::RightControl);
    }
    if keymod.contains(Mod::LALTMOD) {
        use_mod.insert(ModifierKey::LeftAlt);
    }
    if keymod.contains(Mod::RALTMOD) {
        use_mod.insert(ModifierKey::RightAlt);
    }
    if keymod.contains(Mod::LGUIMOD) {
        use_mod.insert(ModifierKey::LeftGui);
    }
    if keymod.contains(Mod::RGUIMOD) {
        use_mod.insert(ModifierKey::RightGui);
    }
    if keymod.contains(Mod::NUMMOD) {
        use_mod.insert(ModifierKey::Num);
    }
    if keymod.contains(Mod::CAPSMOD) {
        use_mod.insert(ModifierKey::Caps);
    }
    if keymod.contains(Mod::MODEMOD) {
        use_mod.insert(ModifierKey::Mode);
    }

    use_mod as ModifierKey
}

fn translate_special_key(keycode: Keycode) -> ffi::SpecialKey {
    match keycode {
        Keycode::F1 => ffi::SpecialKey::F1,
        Keycode::F2 => ffi::SpecialKey::F2,
        Keycode::F3 => ffi::SpecialKey::F3,
        Keycode::F4 => ffi::SpecialKey::F4,
        Keycode::F5 => ffi::SpecialKey::F5,
        Keycode::F6 => ffi::SpecialKey::F6,
        Keycode::F7 => ffi::SpecialKey::F7,
        Keycode::F8 => ffi::SpecialKey::F8,
        Keycode::F9 => ffi::SpecialKey::F9,
        Keycode::F10 => ffi::SpecialKey::F10,
        Keycode::F11 => ffi::SpecialKey::F11,
        Keycode::F12 => ffi::SpecialKey::F12,
        Keycode::F13 => ffi::SpecialKey::F13,
        Keycode::F14 => ffi::SpecialKey::F14,
        Keycode::F15 => ffi::SpecialKey::F15,
        Keycode::F16 => ffi::SpecialKey::F16,
        Keycode::F17 => ffi::SpecialKey::F17,
        Keycode::F18 => ffi::SpecialKey::F18,
        Keycode::F19 => ffi::SpecialKey::F19,
        Keycode::F20 => ffi::SpecialKey::F20,
        Keycode::F21 => ffi::SpecialKey::F21,
        Keycode::F22 => ffi::SpecialKey::F22,
        Keycode::F23 => ffi::SpecialKey::F23,
        Keycode::F24 => ffi::SpecialKey::F23,
        Keycode::Escape => ffi::SpecialKey::Esc,
        Keycode::Return => ffi::SpecialKey::Enter,
        Keycode::Backspace => ffi::SpecialKey::Backspace,
        Keycode::Insert => ffi::SpecialKey::Insert,
        Keycode::Delete => ffi::SpecialKey::Delete,
        Keycode::Home => ffi::SpecialKey::Home,
        Keycode::End => ffi::SpecialKey::End,
        Keycode::PageUp => ffi::SpecialKey::PgUp,
        Keycode::PageDown => ffi::SpecialKey::PgDown,
        Keycode::Left => ffi::SpecialKey::Left,
        Keycode::Right => ffi::SpecialKey::Right,
        Keycode::Up => ffi::SpecialKey::Up,
        Keycode::Down => ffi::SpecialKey::Down,
        Keycode::Tab => ffi::SpecialKey::Tab,
        Keycode::PrintScreen => ffi::SpecialKey::PrintScreen,
        Keycode::ScrollLock => ffi::SpecialKey::ScrollLock,
        Keycode::Pause => ffi::SpecialKey::Pause,
        Keycode::NumLockClear => ffi::SpecialKey::NumLockClear,
        Keycode::KpDivide => ffi::SpecialKey::KpDivide,
        Keycode::KpMultiply => ffi::SpecialKey::KpMultiply,
        Keycode::KpMinus => ffi::SpecialKey::KpMinus,
        Keycode::KpPlus => ffi::SpecialKey::KpPlus,
        Keycode::KpEquals => ffi::SpecialKey::KpEquals,
        Keycode::Kp0 => ffi::SpecialKey::KpNum0,
        Keycode::Kp1 => ffi::SpecialKey::KpNum1,
        Keycode::Kp2 => ffi::SpecialKey::KpNum2,
        Keycode::Kp3 => ffi::SpecialKey::KpNum3,
        Keycode::Kp4 => ffi::SpecialKey::KpNum4,
        Keycode::Kp5 => ffi::SpecialKey::KpNum5,
        Keycode::Kp6 => ffi::SpecialKey::KpNum6,
        Keycode::Kp7 => ffi::SpecialKey::KpNum7,
        Keycode::Kp8 => ffi::SpecialKey::KpNum8,
        Keycode::Kp9 => ffi::SpecialKey::KpNum9,
        Keycode::KpPeriod => ffi::SpecialKey::KpPeriod,
        _ => ffi::SpecialKey::None,
    }
}

pub(crate) fn poll_sdl_events(
    state: &mut SdlState,
    delegate: &mut cxx::UniquePtr<ffi::AppDelegate>,
) -> bool {
    for event in state.events.poll_iter() {
        match event {
            Event::ControllerDeviceAdded { which, .. } => {
                match state.controller_sys.open(which) {
                    Ok(controller) => {
                        log::info!("Opened SDL controller \"{}\"", controller.name());
                        if let Some(new_mapping) = remap_controller_layout(controller.mapping()) {
                            state
                                .controller_sys
                                .add_mapping(new_mapping.as_str())
                                .expect("Failed to overwrite mapping");
                        }
                        let instance_id: u32 = controller.instance_id();
                        state.open_controllers.insert(instance_id, controller);
                        unsafe { ffi::App_onControllerAdded(delegate.as_mut().unwrap_unchecked(), instance_id); }
                    }
                    Err(err) => {
                        log::warn!("Failed to open SDL controller {} ({:?})", which, err);
                    }
                }
            }
            Event::ControllerDeviceRemoved { which, .. } => {
                unsafe { ffi::App_onControllerRemoved(delegate.as_mut().unwrap_unchecked(), which); }
                state.open_controllers.remove(&which);
            }
            Event::ControllerButtonDown { which, button, .. } => unsafe {
                ffi::App_onControllerButton(
                    delegate.as_mut().unwrap_unchecked(),
                    which,
                    button.into(),
                    true,
                );
            },
            Event::ControllerButtonUp { which, button, .. } => unsafe {
                ffi::App_onControllerButton(
                    delegate.as_mut().unwrap_unchecked(),
                    which,
                    button.into(),
                    false,
                );
            },
            Event::ControllerAxisMotion { which, axis, value, .. } => unsafe {
                ffi::App_onControllerAxis(
                    delegate.as_mut().unwrap_unchecked(),
                    which,
                    axis.into(),
                    value,
                );
            },
            Event::KeyDown { keycode, keymod, repeat, .. } => {
                let special_key = translate_special_key(keycode.unwrap());
                let use_mod = translate_modifiers(keymod);
                if special_key != ffi::SpecialKey::None {
                    unsafe {
                        ffi::App_onSpecialKeyDown(delegate.as_mut().unwrap_unchecked(),
                                                  special_key,
                                                  use_mod.bits,
                                                  repeat);
                    }
                } else {
                    let tmp = keycode.unwrap() as u8;
                    if tmp >= '\x20' as u8 && tmp <= 'z' as u8 {
                        unsafe {
                            ffi::App_onCharKeyDown(delegate.as_mut().unwrap_unchecked(),
                                                   tmp,
                                                   use_mod.bits,
                                                   repeat);
                        }
                    }
                }
            },
            Event::KeyUp { keycode, keymod, .. } => {
                let special_key = translate_special_key(keycode.unwrap());
                let use_mod = translate_modifiers(keymod);
                if special_key != ffi::SpecialKey::None {
                    unsafe {
                        ffi::App_onSpecialKeyUp(delegate.as_mut().unwrap_unchecked(),
                                                  special_key,
                                                  use_mod.bits);
                    }
                } else {
                    let tmp = keycode.unwrap() as u8;
                    if tmp >= '\x20' as u8 && tmp <= 'z' as u8 {
                        unsafe {
                            ffi::App_onCharKeyUp(delegate.as_mut().unwrap_unchecked(),
                                                   tmp,
                                                   use_mod.bits);
                        }
                    }
                }
            },
            // SDL overrides exit signals
            Event::Quit { .. } => {
                return false;
            }
            _ => {}
        }
    }
    return true;
}

fn remap_controller_layout(mapping: String) -> Option<String> {
    let mut new_mapping = String::with_capacity(mapping.len());
    let mut entries = BTreeMap::<&str, &str>::new();
    for (i, v) in mapping.split(',').enumerate() {
        if i < 2 {
            if i > 0 {
                new_mapping.push(',');
            }
            new_mapping.push_str(v);
            continue;
        }
        let split = v.splitn(2, ':').collect::<Vec<&str>>();
        if split.len() != 2 {
            panic!("Invalid mapping entry: {}", v);
        }
        entries.insert(split[0], split[1]);
    }
    if entries.contains_key("rightshoulder") && !entries.contains_key("leftshoulder") {
        log::debug!("Remapping GameCube controller layout");
        // TODO trigger buttons may differ per platform
        entries.insert("leftshoulder", "b11");
        let z_button = entries.insert("rightshoulder", "b10").unwrap();
        entries.insert("back", z_button);
    } else if entries.contains_key("leftshoulder")
        && entries.contains_key("rightshoulder")
        && entries.contains_key("back")
    {
        log::debug!("Controller has standard layout");
        return None;
    } else {
        log::error!("Controller has unsupported layout: {}", mapping);
        return None;
    }
    for (k, v) in entries {
        new_mapping.push(',');
        new_mapping.push_str(k);
        new_mapping.push(':');
        new_mapping.push_str(v);
    }
    return Some(new_mapping);
}

pub(crate) fn get_controller_player_index(which: u32) -> i32 {
    get_app().sdl.open_controllers.get(&which).map_or(-1, |c| c.player_index())
}

pub(crate) fn set_controller_player_index(which: u32, index: i32) {
    if let Some(c) = get_app().sdl.open_controllers.get(&which) {
        c.set_player_index(index);
    }
}

pub(crate) fn is_controller_gamecube(which: u32) -> bool {
    get_app().sdl.open_controllers.get(&which)
        .map_or(false, |c| c.name()
            .to_lowercase()
            .eq("nintendo gamecube controller"))
}

pub(crate) fn get_controller_name(which: u32) -> String {
    get_app().sdl.open_controllers.get(&which).map_or(String::from(""), |c| c.name())
}
