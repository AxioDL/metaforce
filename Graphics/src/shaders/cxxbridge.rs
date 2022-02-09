use crate::shaders::{
    aabb::queue_aabb,
    fog_volume_filter::queue_fog_volume_filter,
    fog_volume_plane::queue_fog_volume_plane,
    model::{add_material_set, add_model},
    movie_player::queue_movie_player,
    resolve_color, resolve_depth, set_scissor, set_viewport,
    texture::{
        create_dynamic_texture_2d, create_render_texture, create_static_texture_2d, drop_texture,
        write_texture,
    },
    textured_quad::{queue_textured_quad, queue_textured_quad_verts},
    update_fog_state, update_model_view, update_projection,
};

#[cxx::bridge]
pub(crate) mod ffi {
    unsafe extern "C++" {
        include!("zeus/CVector2f.hpp");
        include!("zeus/CVector3f.hpp");
        include!("zeus/CVector4f.hpp");
        include!("zeus/CMatrix4f.hpp");
        include!("zeus/CColor.hpp");
        include!("zeus/CRectangle.hpp");
        include!("zeus/CAABox.hpp");
        #[namespace = "zeus"]
        type CVector2f = crate::zeus::CVector2f;
        #[namespace = "zeus"]
        type CVector3f = crate::zeus::CVector3f;
        #[namespace = "zeus"]
        type CVector4f = crate::zeus::CVector4f;
        #[namespace = "zeus"]
        type CMatrix4f = crate::zeus::CMatrix4f;
        #[namespace = "zeus"]
        type CColor = crate::zeus::CColor;
        #[namespace = "zeus"]
        type CRectangle = crate::zeus::CRectangle;
        #[namespace = "zeus"]
        type CAABox = crate::zeus::CAABox;
    }

    #[namespace = "aurora::shaders"]
    #[derive(Debug, Copy, Clone, Hash)]
    pub(crate) enum CameraFilterType {
        Passthru,
        Multiply,
        Invert,
        Add,
        Subtract,
        Blend,
        Widescreen,
        SceneAdd,
        NoColor,
        InvDstMultiply,
    }
    #[namespace = "aurora::shaders"]
    #[derive(Debug, Copy, Clone, Hash)]
    pub(crate) enum ZTest {
        Never,
        Less,
        Equal,
        LEqual,
        Greater,
        NEqual,
        GEqual,
        Always,
    }
    #[namespace = "aurora::shaders"]
    #[derive(Debug, Copy, Clone)]
    pub(crate) enum TextureFormat {
        RGBA8,
        R8,
        R32Float,
        DXT1,
        DXT3,
        DXT5,
        BPTC,
    }
    #[namespace = "aurora::shaders"]
    #[derive(Debug, Copy, Clone)]
    pub(crate) enum TextureClampMode {
        Repeat,
        ClampToEdge,
        ClampToWhite,
        ClampToBlack,
    }
    #[namespace = "aurora::shaders"]
    #[derive(Debug, Copy, Clone, Hash)]
    pub(crate) struct TextureRef {
        pub(crate) id: u32,
        pub(crate) render: bool,
    }

    #[namespace = "aurora::shaders"]
    #[derive(Debug, Copy, Clone)]
    #[repr(u32)]
    pub(crate) enum FogMode {
        None = 0x00,

        PerspLin = 0x02,
        PerspExp = 0x04,
        PerspExp2 = 0x05,
        PerspRevExp = 0x06,
        PerspRevExp2 = 0x07,

        OrthoLin = 0x0A,
        OrthoExp = 0x0C,
        OrthoExp2 = 0x0D,
        OrthoRevExp = 0x0E,
        OrthoRevExp2 = 0x0F,
    }
    #[namespace = "aurora::shaders"]
    #[derive(Debug, Copy, Clone)]
    pub(crate) struct FogState {
        #[cxx_name = "m_color"]
        pub(crate) color: CColor,
        #[cxx_name = "m_A"]
        pub(crate) a: f32,
        #[cxx_name = "m_B"]
        pub(crate) b: f32,
        #[cxx_name = "m_C"]
        pub(crate) c: f32,
        #[cxx_name = "m_mode"]
        pub(crate) mode: FogMode,
    }
    #[namespace = "aurora::shaders"]
    #[derive(Debug, Copy, Clone)]
    pub(crate) struct ClipRect {
        x: i32,
        y: i32,
        width: i32,
        height: i32,
    }

    #[namespace = "aurora::shaders"]
    #[derive(Debug, Copy, Clone)]
    pub(crate) struct MaterialPassInfo {
        tex_idx: u32,
        constant_color: CColor,
        has_constant_color: bool, // would love to use Option...
        uv_source: u8,
        uv_anim_type: u8,
        uv_anim_params: [f32; 9],
        normalize: bool, // TODO might be static based on uv type?
        sample_alpha: bool,
    }
    #[namespace = "aurora::shaders"]
    #[derive(Debug, Clone)]
    pub(crate) struct MaterialInfo {
        pass_info: [MaterialPassInfo; 8],
        texture_ids: Vec<TextureRef>,
        shader_type: u8,
        blend_mode: u8,
    }

    #[namespace = "aurora::shaders"]
    extern "Rust" {
        fn update_model_view(mv: CMatrix4f, mv_inv: CMatrix4f);
        fn update_projection(proj: CMatrix4f);
        fn update_fog_state(state: FogState);
        fn set_viewport(rect: CRectangle, znear: f32, zfar: f32);
        fn set_scissor(x: u32, y: u32, w: u32, h: u32);

        fn resolve_color(rect: ClipRect, bind: u32, clear_depth: bool);
        fn resolve_depth(rect: ClipRect, bind: u32);

        fn add_material_set(materials: Vec<MaterialInfo>) -> u32;
        fn add_model(verts: &[u8], indices: &[u8]) -> u32;

        fn queue_aabb(aabb: CAABox, color: CColor, z_only: bool);
        fn queue_fog_volume_plane(verts: &CxxVector<CVector4f>, pass: u8);
        fn queue_fog_volume_filter(color: CColor, two_way: bool);
        fn queue_textured_quad_verts(
            filter_type: CameraFilterType,
            texture: TextureRef,
            z_comparison: ZTest,
            z_test: bool,
            color: CColor,
            pos: &[CVector3f],
            uvs: &[CVector2f],
            lod: f32,
        );
        fn queue_textured_quad(
            filter_type: CameraFilterType,
            texture: TextureRef,
            z_comparison: ZTest,
            z_test: bool,
            color: CColor,
            uv_scale: f32,
            rect: CRectangle,
            z: f32,
        );
        fn queue_movie_player(
            tex_y: TextureRef,
            tex_u: TextureRef,
            tex_v: TextureRef,
            color: CColor,
            h_pad: f32,
            v_pad: f32,
        );

        fn create_static_texture_2d(
            width: u32,
            height: u32,
            mips: u32,
            format: TextureFormat,
            data: &[u8],
            label: &str,
        ) -> TextureRef;
        fn create_dynamic_texture_2d(
            width: u32,
            height: u32,
            mips: u32,
            format: TextureFormat,
            label: &str,
        ) -> TextureRef;
        fn create_render_texture(
            width: u32,
            height: u32,
            // clamp_mode: TextureClampMode,
            color_bind_count: u32,
            depth_bind_count: u32,
            label: &str,
        ) -> TextureRef;
        fn write_texture(handle: TextureRef, data: &[u8]);
        fn drop_texture(handle: TextureRef);
    }
}

unsafe impl bytemuck::Zeroable for ffi::FogMode {}
unsafe impl bytemuck::Pod for ffi::FogMode {}
unsafe impl bytemuck::Zeroable for ffi::FogState {}
unsafe impl bytemuck::Pod for ffi::FogState {}
impl Default for ffi::FogState {
    fn default() -> Self {
        Self { color: Default::default(), a: 0.0, b: 0.5, c: 0.0, mode: ffi::FogMode::None }
    }
}
impl Into<u32> for ffi::TextureFormat {
    // noinspection RsUnreachablePatterns
    fn into(self) -> u32 {
        match self {
            ffi::TextureFormat::RGBA8 => 1,
            ffi::TextureFormat::R8 => 2,
            ffi::TextureFormat::R32Float => 3,
            ffi::TextureFormat::DXT1 => 4,
            ffi::TextureFormat::DXT3 => 5,
            ffi::TextureFormat::DXT5 => 6,
            ffi::TextureFormat::BPTC => 7,
            _ => panic!("Invalid texture format {:?}", self),
        }
    }
}
