#pragma once

#include "Runtime/RetroTypes.hpp"

namespace metaforce {
enum class EMaterialTypes {
  NoStepLogic = 0,
  Stone = 1,
  Metal = 2,
  Grass = 3,
  Ice = 4,
  Pillar = 5,
  MetalGrating = 6,
  Phazon = 7,
  Dirt = 8,
  Lava = 9,
  LavaStone = 10,
  Snow = 11,
  MudSlow = 12,
  HalfPipe = 13,
  Mud = 14,
  Glass = 15,
  Shield = 16,
  Sand = 17,
  ProjectilePassthrough = 18,
  Solid = 19,
  NoPlatformCollision = 20,
  CameraPassthrough = 21,
  Wood = 22,
  Organic = 23,
  NoEdgeCollision = 24,
  RedundantEdgeOrFlippedTri = 25,
  SeeThrough = 26,
  ScanPassthrough = 27,
  AIPassthrough = 28,
  Ceiling = 29,
  Wall = 30,
  Floor = 31,
  Player = 32,
  Character = 33,
  Trigger = 34,
  Projectile = 35,
  Bomb = 36,
  GroundCollider = 37,
  NoStaticCollision = 38,
  Scannable = 39,
  Target = 40,
  Orbit = 41,
  Occluder = 42,
  Immovable = 43,
  Debris = 44,
  PowerBomb = 45,
  Unknown46 = 46,
  CollisionActor = 47,
  AIBlock = 48,
  Platform = 49,
  NonSolidDamageable = 50,
  RadarObject = 51,
  PlatformSlave = 52,
  AIJoint = 53,
  Unknown54 = 54,
  SolidCharacter = 55,
  ExcludeFromLineOfSightTest = 56,
  ExcludeFromRadar = 57,
  NoPlayerCollision = 58,
  SixtyThree = 63
};

class CMaterialList {
  friend class CMaterialFilter;
  u64 x0_list = 0;

public:
  constexpr CMaterialList() noexcept = default;
  constexpr CMaterialList(u64 flags) noexcept : x0_list(flags) {}
  constexpr CMaterialList(EMaterialTypes t1, EMaterialTypes t2, EMaterialTypes t3, EMaterialTypes t4, EMaterialTypes t5,
                          EMaterialTypes t6) noexcept
  : CMaterialList(t1, t2, t3, t4, t5) {
    x0_list |= u64{1} << u64(t6);
  }

  constexpr CMaterialList(EMaterialTypes t1, EMaterialTypes t2, EMaterialTypes t3, EMaterialTypes t4,
                          EMaterialTypes t5) noexcept
  : CMaterialList(t1, t2, t3, t4) {
    x0_list |= u64{1} << u64(t5);
  }

  constexpr CMaterialList(EMaterialTypes t1, EMaterialTypes t2, EMaterialTypes t3, EMaterialTypes t4) noexcept
  : CMaterialList(t1, t2, t3) {
    x0_list |= u64{1} << u64(t4);
  }

  constexpr CMaterialList(EMaterialTypes t1, EMaterialTypes t2, EMaterialTypes t3) noexcept : CMaterialList(t1, t2) {
    x0_list |= u64{1} << u64(t3);
  }

  constexpr CMaterialList(EMaterialTypes t1, EMaterialTypes t2) noexcept : CMaterialList(t1) {
    x0_list |= u64{1} << u64(t2);
  }

  constexpr CMaterialList(EMaterialTypes t1) noexcept : x0_list(u64{1} << u64(t1)) {}

  constexpr u64 GetValue() const noexcept { return x0_list; }

  static constexpr s32 BitPosition(u64 flags) noexcept {
    for (s32 ret = 0, i = 0; i < 32; ++i) {
      if ((flags & 1) != 0u) {
        return ret;
      }
      flags >>= 1;
      ++ret;
    }
    return -1;
  }

  constexpr void Add(EMaterialTypes type) noexcept { x0_list |= (u64{1} << u64(type)); }

  constexpr void Add(const CMaterialList& l) noexcept { x0_list |= l.x0_list; }

  constexpr void Remove(EMaterialTypes type) noexcept { x0_list &= ~(u64{1} << u64(type)); }

  constexpr void Remove(const CMaterialList& other) noexcept { x0_list &= ~(other.x0_list); }

  constexpr bool HasMaterial(EMaterialTypes type) const noexcept { return (x0_list & (u64{1} << u64(type))) != 0; }

  constexpr bool SharesMaterials(const CMaterialList& other) const noexcept {
    for (u32 i = 0; i < 64; i++) {
      if ((x0_list & (u64{1} << i)) != 0 && (other.x0_list & (u64{1} << i)) != 0) {
        return true;
      }
    }

    return false;
  }

  constexpr u64 Intersection(const CMaterialList& other) const noexcept { return other.x0_list & x0_list; }

  constexpr u64 XOR(const CMaterialList& other) const noexcept { return x0_list ^ other.x0_list; }
  void Union(const CMaterialList& other) noexcept { x0_list |= other.x0_list; }
};

} // namespace metaforce
