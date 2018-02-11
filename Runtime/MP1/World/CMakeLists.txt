set(MP1_WORLD_SOURCES
    CNewIntroBoss.hpp CNewIntroBoss.cpp
    CBeetle.hpp CBeetle.cpp
    CWarWasp.hpp CWarWasp.cpp
    CSpacePirate.hpp CSpacePirate.cpp
    CParasite.hpp CParasite.cpp
    CBabygoth.hpp CBabygoth.cpp
    CMetroidPrimeRelay.hpp CMetroidPrimeRelay.cpp
    CMetroidPrimeExo.hpp CMetroidPrimeExo.cpp
    CMetroidPrimeProjectile.hpp CMetroidPrimeProjectile.cpp
    CActorContraption.hpp CActorContraption.cpp
    CThardusRockProjectile.hpp CThardusRockProjectile.cpp
    CMetroidBeta.hpp CMetroidBeta.cpp
    CMetroid.hpp CMetroid.cpp
    CMetaree.hpp CMetaree.cpp
    CPuddleToadGamma.hpp CPuddleToadGamma.cpp
    CFlaahgraProjectile.hpp CFlaahgraProjectile.cpp)

runtime_add_list(World MP1_WORLD_SOURCES)
