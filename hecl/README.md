### HECL (high-level, extensible combiner language)

HECL is a toolkit for building custom asset pipelines, assisting the development of conversion tools and runtime loaders.

The most significant feature is the intermediate HECL language, using an expressive command syntax to represent cross-platform shaders. This includes a common source representation and intermediate binary representation. Complete vertex and fragment shader programs are generated for supported platforms and may be built on-demand as part of a 3D application runtime.

```py
# Combiner 1: Opaque *Diffuse* and *Emissive*
HECLOpaque(Texture(0, UV(0)) * Lighting() + Texture(1, UV(0)))

# Combiner 2: Alpha-blended single-texture
# (both texture-accesses folded to a single sample operation)
HECLAlpha(Texture(0, UV(0)), Texture(0, UV(0)).a)

# Combiner 3: Additive-blended single-texture
# (modern graphics APIs require blending configuration along with all shader configs)
HECLAdditive(Texture(0, UV(0)), Texture(0, UV(0)).a)
```

Beyond shaders, HECL also defines a rigged mesh format called HMDL. Meshes using this encoding interact with HECL, with pose transforms applied via the vertex shader.

For asset pipelines, HECL provides a project system with dependency-resolution much like an IDE or `make`. Assets in their editable representation are *cooked* in-bulk and whenever the source file is updated. Currently, blender is the only-supported input format for rigged meshes with node-materials. 

#### Supported Backends

* GLSL 330 *(with optional SPIR-V conversion)*
* HLSL (Shader Model 4)
* Metal 1.1
