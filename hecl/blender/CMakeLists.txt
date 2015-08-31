list(APPEND PY_SOURCES
     addon/__init__.py
     addon/Nodegrid.py
     addon/hmdl/__init__.py
     addon/hmdl/HMDLMesh.py
     addon/hmdl/HMDLShader.py
     addon/hmdl/HMDLSkin.py
     addon/hmdl/HMDLTxtr.py
     addon/sact/__init__.py
     addon/sact/SACTAction.py
     addon/sact/SACTEvent.py
     addon/sact/SACTSubtype.py)

bintoc(hecl_blendershell.c hecl_blendershell.py HECL_BLENDERSHELL)

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/hecl.zip)
message("-- Generating addon package")
execute_process(COMMAND python zip_package.py
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/hecl.zip)
message(FATAL_ERROR "Unable to generate ${CMAKE_CURRENT_SOURCE_DIR}/hecl.zip; is python installed?")
endif()
endif()
bintoc(hecl_addon.c hecl.zip HECL_ADDON)

add_library(HECLBlender
            BlenderConnection.cpp
            BlenderConnection.hpp
            hecl_blendershell.py
            hecl_blendershell.c
            hecl_addon.c
            ${PY_SOURCES})
