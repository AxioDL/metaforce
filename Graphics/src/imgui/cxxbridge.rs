use crate::{
    gpu::DeviceHolder,
    imgui::{ImGuiEngine_AddTexture, ImGuiState},
};

#[cxx::bridge(namespace = "metaforce")]
pub(crate) mod ffi {
    unsafe extern "C++" {
        include!("ImGuiEngine.hpp");
        pub(crate) fn ImGuiEngine_Initialize(scale: f32);
        pub(crate) fn ImGuiEngine_AddTextures(state: &mut ImGuiState, device: &DeviceHolder);
    }

    extern "Rust" {
        type ImGuiState;
        type DeviceHolder;
        fn ImGuiEngine_AddTexture(
            state: &mut ImGuiState,
            device: &DeviceHolder,
            width: u32,
            height: u32,
            data: &[u8],
        ) -> usize;
    }
}
