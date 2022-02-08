use bytemuck_derive::{Pod, Zeroable};
use cxx::{type_id, ExternType};
use crate::util::{Vec2, Vec3};

#[derive(Debug, Copy, Clone, Pod, Zeroable, Default)]
#[repr(C)]
pub struct CVector2f {
    pub x: f32,
    pub y: f32,
    _p1: f32,
    _p2: f32,
}
impl CVector2f {
    pub const fn new(x: f32, y: f32) -> Self { Self { x, y, _p1: 0.0, _p2: 0.0 } }
}
unsafe impl ExternType for CVector2f {
    type Id = type_id!("zeus::CVector2f");
    type Kind = cxx::kind::Trivial;
}

#[derive(Debug, Copy, Clone, Pod, Zeroable, Default)]
#[repr(C)]
pub struct CVector3f {
    pub x: f32,
    pub y: f32,
    pub z: f32,
    _p: f32,
}
impl CVector3f {
    pub const fn new(x: f32, y: f32, z: f32) -> Self { Self { x, y, z, _p: 0.0 } }
}
unsafe impl ExternType for CVector3f {
    type Id = type_id!("zeus::CVector3f");
    type Kind = cxx::kind::Trivial;
}

#[derive(Debug, Copy, Clone, Pod, Zeroable, Default)]
#[repr(C)]
pub struct CVector4f {
    pub x: f32,
    pub y: f32,
    pub z: f32,
    pub w: f32,
}
impl CVector4f {
    pub const fn new(x: f32, y: f32, z: f32, w: f32) -> Self { Self { x, y, z, w } }
}
unsafe impl ExternType for CVector4f {
    type Id = type_id!("zeus::CVector4f");
    type Kind = cxx::kind::Trivial;
}

#[derive(Debug, Copy, Clone, Pod, Zeroable, Default)]
#[repr(C)]
pub struct CColor {
    pub r: f32,
    pub g: f32,
    pub b: f32,
    pub a: f32,
}
impl CColor {
    pub const fn new(r: f32, g: f32, b: f32, a: f32) -> Self { Self { r, g, b, a } }
}
unsafe impl ExternType for CColor {
    type Id = type_id!("zeus::CColor");
    type Kind = cxx::kind::Trivial;
}

#[derive(Debug, Copy, Clone, Pod, Zeroable, Default)]
#[repr(C)]
pub struct CRectangle {
    pub position: CVector2f,
    pub size: CVector2f,
}
impl CRectangle {
    pub const fn new(x: f32, y: f32, w: f32, h: f32) -> Self {
        Self { position: CVector2f::new(x, y), size: CVector2f::new(w, h) }
    }
}
unsafe impl ExternType for CRectangle {
    type Id = type_id!("zeus::CRectangle");
    type Kind = cxx::kind::Trivial;
}

#[derive(Debug, Copy, Clone, Pod, Zeroable, Default)]
#[repr(C)]
pub struct CAABox {
    pub min: CVector3f,
    pub max: CVector3f,
}
impl CAABox {
    pub const fn new(min: CVector3f, max: CVector3f) -> Self { Self { min, max } }
}
unsafe impl ExternType for CAABox {
    type Id = type_id!("zeus::CAABox");
    type Kind = cxx::kind::Trivial;
}

pub const IDENTITY_MATRIX4F: CMatrix4f = CMatrix4f {
    x: CVector4f::new(1.0, 0.0, 0.0, 0.0),
    y: CVector4f::new(0.0, 1.0, 0.0, 0.0),
    z: CVector4f::new(0.0, 0.0, 1.0, 0.0),
    w: CVector4f::new(0.0, 0.0, 0.0, 1.0),
};
#[derive(Debug, Copy, Clone, Pod, Zeroable)]
#[repr(C)]
pub struct CMatrix4f {
    pub x: CVector4f,
    pub y: CVector4f,
    pub z: CVector4f,
    pub w: CVector4f,
}
impl CMatrix4f {
    pub const fn new(x: CVector4f, y: CVector4f, z: CVector4f, w: CVector4f) -> Self {
        Self { x, y, z, w }
    }
}
impl Default for CMatrix4f {
    fn default() -> Self { IDENTITY_MATRIX4F }
}
unsafe impl ExternType for CMatrix4f {
    type Id = type_id!("zeus::CMatrix4f");
    type Kind = cxx::kind::Trivial;
}

#[cxx::bridge(namespace = "zeus")]
mod ffi {
    unsafe extern "C++" {
        include!("zeus/CVector2f.hpp");
        include!("zeus/CVector3f.hpp");
        include!("zeus/CVector4f.hpp");
        include!("zeus/CMatrix4f.hpp");
        include!("zeus/CColor.hpp");
        include!("zeus/CRectangle.hpp");
        include!("zeus/CAABox.hpp");
        type CVector2f = crate::zeus::CVector2f;
        type CVector3f = crate::zeus::CVector3f;
        type CVector4f = crate::zeus::CVector4f;
        type CMatrix4f = crate::zeus::CMatrix4f;
        type CColor = crate::zeus::CColor;
        type CRectangle = crate::zeus::CRectangle;
        type CAABox = crate::zeus::CAABox;
    }
}

impl From<cgmath::Vector4<f32>> for CVector4f {
    fn from(v: cgmath::Vector4<f32>) -> Self { Self { x: v.x, y: v.y, z: v.z, w: v.w } }
}
impl From<CVector4f> for cgmath::Vector4<f32> {
    fn from(v: CVector4f) -> Self { Self { x: v.x, y: v.y, z: v.z, w: v.w } }
}

impl From<cgmath::Matrix4<f32>> for CMatrix4f {
    fn from(m: cgmath::Matrix4<f32>) -> Self {
        Self {
            x: CVector4f::from(m.x),
            y: CVector4f::from(m.y),
            z: CVector4f::from(m.z),
            w: CVector4f::from(m.w),
        }
    }
}
impl From<CMatrix4f> for cgmath::Matrix4<f32> {
    fn from(m: CMatrix4f) -> Self {
        Self {
            x: cgmath::Vector4::from(m.x),
            y: cgmath::Vector4::from(m.y),
            z: cgmath::Vector4::from(m.z),
            w: cgmath::Vector4::from(m.w),
        }
    }
}

impl From<Vec2<f32>> for CVector2f {
    fn from(v: Vec2<f32>) -> Self {
        Self {
            x: v.x,
            y: v.y,
            _p1: 0.0,
            _p2: 0.0
        }
    }
}
impl From<CVector2f> for Vec2<f32> {
    fn from(v: CVector2f) -> Self {
        Self {
            x: v.x,
            y: v.y,
        }
    }
}
impl From<&CVector2f> for Vec2<f32> {
    fn from(v: &CVector2f) -> Self {
        Self {
            x: v.x,
            y: v.y,
        }
    }
}

impl From<Vec3<f32>> for CVector3f {
    fn from(v: Vec3<f32>) -> Self {
        Self {
            x: v.x,
            y: v.y,
            z: v.z,
            _p: 0.0,
        }
    }
}
impl From<CVector3f> for Vec3<f32> {
    fn from(v: CVector3f) -> Self {
        Self {
            x: v.x,
            y: v.y,
            z: v.z,
        }
    }
}
impl From<&CVector3f> for Vec3<f32> {
    fn from(v: &CVector3f) -> Self {
        Self {
            x: v.x,
            y: v.y,
            z: v.z,
        }
    }
}
