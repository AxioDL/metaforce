list(APPEND PY_SOURCES
     hecl/__init__.py
     hecl/Nodegrid.py
     hecl/Patching.py
     hecl/hmdl/__init__.py
     hecl/hmdl/HMDLMesh.py
     hecl/hmdl/HMDLShader.py
     hecl/sact/__init__.py
     hecl/sact/SACTAction.py
     hecl/sact/SACTSubtype.py
     hecl/srea/__init__.py
     hecl/swld/__init__.py
     hecl/mapa.py
     hecl/mapu.py)

bintoc(hecl_blendershell.c hecl_blendershell.py HECL_BLENDERSHELL)

add_custom_command(OUTPUT hecl.zip DEPENDS ${PY_SOURCES}
                   COMMAND python ARGS zip_package.py ${CMAKE_CURRENT_BINARY_DIR}/hecl.zip
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                   COMMENT "Generating addon package")
bintoc(hecl_addon.c "${CMAKE_CURRENT_BINARY_DIR}/hecl.zip" HECL_ADDON)
bintoc(hecl_startup.c hecl_startup.blend HECL_STARTUP)

add_library(hecl-blender-addon
            hecl_blendershell.py
            hecl_blendershell.c
            zip_package.py
            hecl.zip
            hecl_addon.c
            hecl_startup.c
            ${PY_SOURCES})
