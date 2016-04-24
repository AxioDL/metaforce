set(COLLISION_SOURCES
    CGameCollision.hpp CGameCollision.cpp
    CCollisionInfo.hpp CCollisionInfo.cpp
    CCollidableOBBTree.hpp CCollidableOBBTree.cpp
    CCollidableOBBTreeGroup.hpp CCollidableOBBTreeGroup.cpp
    CMaterialList.hpp
    CMaterialFilter.hpp
    CRayCastResult.hpp CRayCastResult.cpp)

runtime_add_list(Collision COLLISION_SOURCES)
