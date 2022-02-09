//! This contains regular vector types for use in packed buffers (i.e. vertex).
//! zeus types are all aligned to f32x4
use binrw::BinRead;
use bytemuck::{Pod, Zeroable};

#[derive(Copy, Clone, Debug, PartialEq, BinRead, Default)]
#[repr(C)]
pub(crate) struct Vec3<T: 'static + BinRead<Args = ()> + Pod> {
    pub(crate) x: T,
    pub(crate) y: T,
    pub(crate) z: T,
}
impl<T: 'static + BinRead<Args = ()> + Pod> Vec3<T> {
    // TODO const
    pub fn new(x: T, y: T, z: T) -> Self { Self { x, y, z } }
}

unsafe impl<T: BinRead<Args = ()> + Pod> Zeroable for Vec3<T> {}

unsafe impl<T: BinRead<Args = ()> + Pod> Pod for Vec3<T> {}

impl<T: BinRead<Args = ()> + Pod> From<T> for Vec3<T> {
    fn from(v: T) -> Self { Vec3::<T> { x: v, y: v, z: v } }
}

impl<T: BinRead<Args = ()> + Pod> From<cgmath::Vector3<T>> for Vec3<T> {
    fn from(v: cgmath::Vector3<T>) -> Self { Self { x: v.x, y: v.y, z: v.z } }
}

impl<T: BinRead<Args = ()> + Pod> From<Vec3<T>> for cgmath::Vector3<T> {
    fn from(v: Vec3<T>) -> Self { Self { x: v.x, y: v.y, z: v.z } }
}

impl<T: BinRead<Args = ()> + Pod> From<cgmath::Point3<T>> for Vec3<T> {
    fn from(v: cgmath::Point3<T>) -> Self { Self { x: v.x, y: v.y, z: v.z } }
}

impl<T: BinRead<Args = ()> + Pod> From<Vec3<T>> for cgmath::Point3<T> {
    fn from(v: Vec3<T>) -> Self { Self { x: v.x, y: v.y, z: v.z } }
}

// pub(crate) const fn vec3_splat<T: BinRead<Args = ()> + Pod>(v: T) -> Vec3<T> {
//     Vec3::<T> { x: v, y: v, z: v }
// }

#[derive(Copy, Clone, Debug, PartialEq, BinRead, Default)]
#[repr(C)]
pub(crate) struct Vec2<T: 'static + BinRead<Args = ()> + Pod> {
    pub(crate) x: T,
    pub(crate) y: T,
}
impl<T: 'static + BinRead<Args = ()> + Pod> Vec2<T> {
    // TODO const
    pub fn new(x: T, y: T) -> Self { Self { x, y } }
}

unsafe impl<T: BinRead<Args = ()> + Pod> Zeroable for Vec2<T> {}

unsafe impl<T: BinRead<Args = ()> + Pod> Pod for Vec2<T> {}

impl<T: BinRead<Args = ()> + Pod> From<T> for Vec2<T> {
    fn from(v: T) -> Self { Self { x: v, y: v } }
}

impl<T: BinRead<Args = ()> + Pod> From<cgmath::Vector2<T>> for Vec2<T> {
    fn from(v: cgmath::Vector2<T>) -> Self { Self { x: v.x, y: v.y } }
}

impl<T: BinRead<Args = ()> + Pod> From<Vec2<T>> for cgmath::Vector2<T> {
    fn from(v: Vec2<T>) -> Self { Self { x: v.x, y: v.y } }
}

impl From<Vec2<i16>> for Vec2<f32> {
    fn from(v: Vec2<i16>) -> Self { Self { x: v.x as f32 / 32768.0, y: v.y as f32 / 32768.0 } }
}

// pub(crate) const fn vec2_splat<T: BinRead<Args = ()> + Pod>(v: T) -> Vec2<T> {
//     Vec2::<T> { x: v, y: v }
// }

#[inline(always)]
pub(crate) fn align<
    T: Copy
        + std::ops::Sub<Output = T>
        + std::ops::Add<Output = T>
        + std::ops::Not<Output = T>
        + std::ops::BitAnd<Output = T>
        + num_traits::One
        + num_traits::Zero
        + std::cmp::PartialEq,
>(
    n: T,
    a: T,
) -> T {
    if a == num_traits::Zero::zero() {
        return n;
    }
    (n + (a - num_traits::One::one())) & !(a - num_traits::One::one())
}
