#ifndef VISIRENDERER_HPP
#define VISIRENDERER_HPP

#include "boo/graphicsdev/glew.h"
#include "hecl/SystemChar.hpp"
#include "zeus/CColor.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CAABox.hpp"

typedef void(*FPercent)(float);

enum class EPVSVisSetState
{
    EndOfTree,
    NodeFound,
    OutOfBounds
};

class VISIRenderer
{
    friend struct VISIBuilder;

    int m_argc;
    const hecl::SystemChar** m_argv;
    int m_return = 0;

    zeus::CAABox m_totalAABB;

    struct UniformBuffer
    {
        zeus::CMatrix4f m_xf;
    } m_uniformBuffer;

    struct Model
    {
        GLenum topology;
        zeus::CAABox aabb;

        struct Vert
        {
            zeus::CVector3f pos;
            zeus::CColor color;
        };
        std::vector<Vert> verts;

        std::vector<uint32_t> idxs;
        GLuint vbo, ibo, vao;

        struct Surface
        {
            uint32_t first;
            uint32_t count;
            bool transparent;
        };
        std::vector<Surface> surfaces;
    };

    struct Entity
    {
        uint32_t entityId;
        zeus::CAABox aabb;
        GLuint vbo, vao;
    };

    struct Light
    {
        zeus::CVector3f point;
        GLuint vbo, vao;
    };

    GLuint m_vtxShader, m_fragShader, m_program, m_uniLoc;
    GLuint m_uniformBufferGL;
    GLuint m_aabbIBO;
    bool SetupShaders();

    std::vector<Model> m_models;
    std::vector<Entity> m_entities;
    std::vector<Light> m_lights;
    bool SetupVertexBuffersAndFormats();

    GLuint m_query;

    FPercent m_updatePercent;

    static std::vector<Model::Vert> AABBToVerts(const zeus::CAABox& aabb,
                                                const zeus::CColor& color);

public:
    bool m_terminate = false;
    struct RGBA8
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };

    VISIRenderer(int argc, const hecl::SystemChar** argv) : m_argc(argc), m_argv(argv) {}
    void Run(FPercent updatePercent);
    void Terminate();
    void RenderPVSOpaque(RGBA8* bufOut, const zeus::CVector3f& pos, bool& needTransparent);
    void RenderPVSTransparent(const std::function<void(int)>& passFunc, const zeus::CVector3f& pos);
    void RenderPVSEntitiesAndLights(const std::function<void(int)>& passFunc,
                                    const std::function<void(int, EPVSVisSetState)>& lightPassFunc,
                                    const zeus::CVector3f& pos);
    int ReturnVal() const { return m_return; }
};

#endif // VISIRENDERER_HPP
