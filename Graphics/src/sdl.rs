use std::collections::{BTreeMap, HashMap};

use sdl2::{
    controller::{Axis, GameController},
    event::Event,
    GameControllerSubsystem, Sdl,
};

use crate::{cxxbridge::ffi, get_app};

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
