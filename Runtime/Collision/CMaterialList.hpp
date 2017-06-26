#ifndef __URDE_CMATERIALLIST_HPP__
#define __URDE_CMATERIALLIST_HPP__

#include "RetroTypes.hpp"

namespace urde
{
enum class EMaterialTypes
{
    Unknown = 0,
    Stone = 1,
    Metal = 2,
    Grass = 3,
    Ice = 4,
    Pillar = 5,
    MetalGrating = 6,
    Phazon = 7,
    Dirt = 8,
    Lava = 9,
    Ten = 10,
    Snow = 11,
    MudSlow = 12,
    HalfPipe = 13,
    Mud = 14,
    Glass = 15,
    Shield = 16,
    Sand = 17,
    ProjectilePassthrough = 18,
    Solid = 19,
    Twenty = 20,
    CameraPassthrough = 21,
    Wood = 22,
    Organic = 23,
    TwentyFour = 24,
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
    StaticCollision = 38,
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
    Unknown54 = 54,
    SolidCharacter = 55,
    ExcludeFromLineOfSightTest = 56,
    ExcludeFromRadar = 57,
    NoPlayerCollision = 58,
    SixtyThree = 63
};

class CMaterialList
{
    friend class CMaterialFilter;
    u64 x0_list = 0;
public:
    CMaterialList() = default;
    CMaterialList(u64 flags) : x0_list(flags) {}
    CMaterialList(EMaterialTypes t1, EMaterialTypes t2, EMaterialTypes t3, EMaterialTypes t4, EMaterialTypes t5, EMaterialTypes t6)
        : CMaterialList(t1, t2, t3, t4, t5)
    { x0_list = 1ull << u64(t6); }

    CMaterialList(EMaterialTypes t1, EMaterialTypes t2, EMaterialTypes t3, EMaterialTypes t4, EMaterialTypes t5)
        : CMaterialList(t1, t2, t3, t4)
    { x0_list = 1ull << u64(t5); }

    CMaterialList(EMaterialTypes t1, EMaterialTypes t2, EMaterialTypes t3, EMaterialTypes t4)
        : CMaterialList(t1, t2, t3)
    { x0_list = 1ull << u64(t4); }

    CMaterialList(EMaterialTypes t1, EMaterialTypes t2, EMaterialTypes t3)
        : CMaterialList(t1, t2)
    { x0_list = 1ull << u64(t3); }

    CMaterialList(EMaterialTypes t1, EMaterialTypes t2)
        : CMaterialList(t1)
    { x0_list = 1ull << u64(t2); }

    CMaterialList(EMaterialTypes t1)
        : x0_list(1ull << u64(t1))
    {
    }

    u64 GetValue() const { return x0_list; }

    static s32 BitPosition(u64 flag)
    {
        for (u32 i = 0; i < 63; ++i)
             if ((flag & (1ull << i)) != 0)
                 return i;
        return -1;
    }

    void Add(EMaterialTypes type)
    {
        x0_list |= (1ull << u32(type));
    }

    void Add(const CMaterialList& l)
    {
        x0_list |= l.x0_list;
    }

    void Remove(EMaterialTypes type)
    {
        x0_list &= ~(1ull << u32(type));
    }

    void Remove(const CMaterialList& other)
    {
        x0_list &= ~(other.x0_list);
    }

    bool HasMaterial(EMaterialTypes type) const
    {
        return (x0_list & (1ull << u32(type))) != 0;
    }

    bool SharesMaterials(const CMaterialList& other)
    {
        for (u32 i = 0; i < 64; i++)
        {
            if ((x0_list & (1ull << i)) != 0 && (other.x0_list & (1ull << i)) != 0)
                return true;
        }

        return false;
    }

    u64 Intersection(const CMaterialList& other) const
    {
        return other.x0_list & x0_list;
    }

    u64 XOR(const CMaterialList& other) const
    {
        return x0_list ^ other.x0_list;
    }
};

}

#endif // __URDE_CMATERIALLIST_HPP__
