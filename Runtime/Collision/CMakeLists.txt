set(COLLISION_SOURCES
    CollisionUtil.hpp CollisionUtil.cpp
    CGameCollision.hpp CGameCollision.cpp
    CCollisionInfo.hpp CCollisionInfo.cpp
    CCollisionInfoList.hpp CCollisionInfoList.cpp
    CCollisionEdge.hpp CCollisionEdge.cpp
    CCollisionSurface.hpp CCollisionSurface.cpp
    InternalColliders.hpp InternalColliders.cpp
    COBBTree.hpp COBBTree.cpp
    CCollidableAABox.hpp CCollidableAABox.cpp
    CCollidableCollisionSurface.hpp CCollidableCollisionSurface.cpp
    CCollidableSphere.hpp CCollidableSphere.cpp
    CCollidableOBBTree.hpp CCollidableOBBTree.cpp
    CCollidableOBBTreeGroup.hpp CCollidableOBBTreeGroup.cpp
    CCollisionPrimitive.hpp CCollisionPrimitive.cpp
    CMaterialList.hpp
    CMaterialFilter.hpp CMaterialFilter.cpp
    CInternalRayCastStructure.hpp CInternalRayCastStructure.cpp
    CRayCastResult.hpp CRayCastResult.cpp)

runtime_add_list(Collision COLLISION_SOURCES)
