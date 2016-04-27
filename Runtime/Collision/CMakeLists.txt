set(COLLISION_SOURCES
    CGameCollision.hpp CGameCollision.cpp
    CCollisionInfo.hpp CCollisionInfo.cpp
    CCollisionEdge.hpp CCollisionEdge.cpp
    COBBTree.hpp COBBTree.cpp
    CCollidableOBBTree.hpp CCollidableOBBTree.cpp
    CCollidableOBBTreeGroup.hpp CCollidableOBBTreeGroup.cpp
    CCollisionPrimitive.hpp CCollisionPrimitive.cpp
    CMaterialList.hpp
    CMaterialFilter.hpp CMaterialFilter.cpp
    CInternalRayCastStructure.hpp CInternalRayCastStructure.cpp
    CRayCastResult.hpp CRayCastResult.cpp)

runtime_add_list(Collision COLLISION_SOURCES)
