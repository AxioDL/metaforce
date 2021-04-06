#define SPECTER_GLSL_VIEW_VERT_BLOCK\
    UBINDING0 uniform SpecterViewBlock\
    {\
        mat4 mv;\
        vec4 mulColor;\
    };
#define SPECTER_HLSL_VIEW_VERT_BLOCK\
    cbuffer SpecterViewBlock : register(b0)\
    {\
        float4x4 mv;\
        float4 mulColor;\
    };
#define SPECTER_METAL_VIEW_VERT_BLOCK\
    struct SpecterViewBlock\
    {\
        float4x4 mv;\
        float4 mulColor;\
    };