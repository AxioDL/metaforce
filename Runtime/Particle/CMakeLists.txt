if(WIN32)
  set(PLAT_SRCS CElementGenShadersHLSL.cpp)
elseif(APPLE)
  set(PLAT_SRCS CElementGenShadersMetal.cpp)
endif()

add_library(RuntimeCommonParticle
            IElement.hpp
            CGenDescription.hpp
            CRealElement.hpp CRealElement.cpp
            CIntElement.hpp CIntElement.cpp
            CVectorElement.hpp CVectorElement.cpp
            CModVectorElement.hpp CModVectorElement.cpp
            CColorElement.hpp CColorElement.cpp
            CUVElement.hpp CUVElement.cpp
            CEmitterElement.hpp CEmitterElement.cpp
            CParticleDataFactory.hpp CParticleDataFactory.cpp
            CSwooshDescription.hpp CSwooshDescription.cpp
            CElectricDescription.hpp CElectricDescription.cpp
            CDecalDescription.hpp CDecalDescription.cpp
            CWeaponDescription.hpp CWeaponDescription.cpp
            CDecalDataFactory.hpp CDecalDataFactory.cpp
            CElementGen.hpp CElementGen.cpp
            CParticleSwooshDataFactory.hpp CParticleSwooshDataFactory.cpp
            CParticleSwoosh.hpp CParticleSwoosh.cpp
            CParticleElectricDataFactory.hpp CParticleElectricDataFactory.cpp
            CParticleElectric.hpp CParticleElectric.cpp
            CParticleGen.hpp CParticleGen.cpp
            CProjectileWeaponDataFactory.hpp CProjectileWeaponDataFactory.cpp
            CCollisionResponseData.hpp CCollisionResponseData.cpp
            CDecalManager.hpp CDecalManager.cpp
            CSpawnSystemKeyframeData.hpp CSpawnSystemKeyframeData.cpp
            CWarp.hpp CWarp.cpp
            CFlameWarp.hpp CFlameWarp.cpp
            CParticleGlobals.hpp CParticleGlobals.cpp
            CElementGenShaders.hpp
            CElementGenShadersGLSL.cpp
            ${PLAT_SRCS})
