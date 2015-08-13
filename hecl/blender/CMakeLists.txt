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

add_library(HECLBlender
            BlenderConnection.cpp
            BlenderConnection.hpp
            hecl_blendershell.py
            hecl_blendershell.c
            ${PY_SOURCES})
