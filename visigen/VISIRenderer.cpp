#include "VISIRenderer.hpp"
#include "athena/FileReader.hpp"
#include "zeus/CAABox.hpp"
#include "VISIBuilder.hpp"
#include "zeus/CFrustum.hpp"
#include "logvisor/logvisor.hpp"

static logvisor::Module Log("visigen");

static const char* VS =
"#version 330\n"
"layout(location=0) in vec4 posIn;\n"
"layout(location=1) in vec4 colorIn;\n"
"\n"
"uniform UniformBlock\n"
"{\n"
"    mat4 xf;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"};\n"
"\n"
"out VertToFrag vtf;\n"
"void main()\n"
"{\n"
"    vtf.color = colorIn;\n"
"    gl_Position = xf * vec4(posIn.xyz, 1.0);\n"
"}\n";

static const char* FS =
"#version 330\n"
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"};\n"
"\n"
"in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"void main()\n"
"{\n"
"    colorOut = vtf.color;\n"
"}\n";

static const uint32_t AABBIdxs[20] =
{
    0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 1, 7, 3, 5, 5, 0, 0, 2, 6, 4
};

bool VISIRenderer::SetupShaders()
{
    m_vtxShader = glCreateShader(GL_VERTEX_SHADER);
    m_fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    m_program = glCreateProgram();

    glShaderSource(m_vtxShader, 1, &VS, nullptr);
    glCompileShader(m_vtxShader);
    GLint status;
    glGetShaderiv(m_vtxShader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        GLint logLen;
        glGetShaderiv(m_vtxShader, GL_INFO_LOG_LENGTH, &logLen);
        char* log = (char*)malloc(logLen);
        glGetShaderInfoLog(m_vtxShader, logLen, nullptr, log);
        Log.report(logvisor::Error, "unable to compile vert source\n%s\n%s\n", log, VS);
        free(log);
        return false;
    }

    glShaderSource(m_fragShader, 1, &FS, nullptr);
    glCompileShader(m_fragShader);
    glGetShaderiv(m_fragShader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        GLint logLen;
        glGetShaderiv(m_fragShader, GL_INFO_LOG_LENGTH, &logLen);
        char* log = (char*)malloc(logLen);
        glGetShaderInfoLog(m_fragShader, logLen, nullptr, log);
        Log.report(logvisor::Error, "unable to compile frag source\n%s\n%s\n", log, FS);
        free(log);
        return false;
    }

    glAttachShader(m_program, m_vtxShader);
    glAttachShader(m_program, m_fragShader);

    glLinkProgram(m_program);
    glGetProgramiv(m_program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE)
    {
        GLint logLen;
        glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &logLen);
        char* log = (char*)malloc(logLen);
        glGetProgramInfoLog(m_program, logLen, nullptr, log);
        Log.report(logvisor::Error, "unable to link shader program\n%s\n", log);
        free(log);
        return false;
    }

    glUseProgram(m_program);
    m_uniLoc = glGetUniformBlockIndex(m_program, "UniformBlock");

    glGenBuffers(1, &m_uniformBufferGL);
    glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBufferGL);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBuffer), nullptr, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &m_aabbIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_aabbIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 20 * 4, AABBIdxs, GL_STATIC_DRAW);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xffffffff);

    return true;
}

std::vector<VISIRenderer::Model::Vert> VISIRenderer::AABBToVerts(const zeus::CAABox& aabb,
                                                                 const zeus::CColor& color)
{
    std::vector<Model::Vert> verts;
    verts.resize(8);

    for (int i=0 ; i<8 ; ++i)
        verts[i].color = color;

    verts[0].pos = aabb.min;
    verts[1].pos = {aabb.max.x(), aabb.min.y(), aabb.min.z()};
    verts[2].pos = {aabb.min.x(), aabb.min.y(), aabb.max.z()};
    verts[3].pos = {aabb.max.x(), aabb.min.y(), aabb.max.z()};
    verts[4].pos = {aabb.min.x(), aabb.max.y(), aabb.max.z()};
    verts[5].pos = aabb.max;
    verts[6].pos = {aabb.min.x(), aabb.max.y(), aabb.min.z()};
    verts[7].pos = {aabb.max.x(), aabb.max.y(), aabb.min.z()};

    return verts;
}

static zeus::CColor ColorForIndex(int i)
{
    i += 1;
    return zeus::CColor((i & 0xff) / 255.f,
                        ((i >> 8) & 0xff) / 255.f,
                        ((i >> 16) & 0xff) / 255.f,
                        1.f);
}

bool VISIRenderer::SetupVertexBuffersAndFormats()
{
    for (Model& model : m_models)
    {
        glGenVertexArrays(1, &model.vao);
        glGenBuffers(1, &model.vbo);
        glGenBuffers(1, &model.ibo);

        glBindVertexArray(model.vao);

        glBindBuffer(GL_ARRAY_BUFFER, model.vbo);
        glBufferData(GL_ARRAY_BUFFER, model.verts.size() * sizeof(Model::Vert), model.verts.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Model::Vert), 0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Model::Vert), (void*)16);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.idxs.size() * 4, model.idxs.data(), GL_STATIC_DRAW);
    }

    int idx = m_models.size();
    for (Entity& ent : m_entities)
    {
        glGenVertexArrays(1, &ent.vao);
        glGenBuffers(1, &ent.vbo);

        glBindVertexArray(ent.vao);

        auto verts = AABBToVerts(ent.aabb, ColorForIndex(idx++));
        glBindBuffer(GL_ARRAY_BUFFER, ent.vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Model::Vert), verts.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Model::Vert), 0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Model::Vert), (void*)16);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_aabbIBO);
    }

    for (Light& light : m_lights)
    {
        glGenVertexArrays(1, &light.vao);
        glGenBuffers(1, &light.vbo);

        glBindVertexArray(light.vao);

        Model::Vert vert;
        vert.pos = light.point;
        vert.color = ColorForIndex(idx++);
        glBindBuffer(GL_ARRAY_BUFFER, light.vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Model::Vert), &vert, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Model::Vert), 0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Model::Vert), (void*)16);
    }

    m_queryCount = m_models.size() + m_entities.size() + m_lights.size();
    m_queries.reset(new GLuint[m_queryCount]);
    m_queryBools.reset(new bool[m_queryCount]);
    glGenQueries(GLsizei(m_queryCount), m_queries.get());

    return true;
}

static zeus::CMatrix4f g_Proj;

static void CalculateProjMatrix()
{
    float znear = 0.2f;
    float zfar = 1000.f;
    float tfov = std::tan(zeus::degToRad(90.f * 0.5f));
    float top = znear * tfov;
    float bottom = -top;
    float right = znear * tfov;
    float left = -right;

    float rml = right - left;
    float rpl = right + left;
    float tmb = top - bottom;
    float tpb = top + bottom;
    float fpn = zfar + znear;
    float fmn = zfar - znear;

    g_Proj = zeus::CMatrix4f(2.f * znear / rml, 0.f, rpl / rml, 0.f,
                             0.f, 2.f * znear / tmb, tpb / tmb, 0.f,
                             0.f, 0.f, -fpn / fmn, -2.f * zfar * znear / fmn,
                             0.f, 0.f, -1.f, 0.f);
}

static const zeus::CMatrix4f LookMATs[] =
{
    { // Forward
        1.f, 0.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, -1.f, 0.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    },
    { // Backward
        -1.f, 0.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    },
    { // Up
        1.f, 0.f, 0.f, 0.f,
        0.f, -1.f, 0.f, 0.f,
        0.f, 0.f, -1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    },
    { // Down
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    },
    { // Left
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        1.f, 0.f, 0.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    },
    { // Right
        0.f, -1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        -1.f, 0.f, 0.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    },
};

void VISIRenderer::RenderPVSOpaque(RGBA8* bufOut, const zeus::CVector3f& pos, bool& needTransparent)
{
    glViewport(0, 0, 768, 512);
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (int j=0 ; j<6 ; ++j)
    {
        GLint x = (j % 3) * 256;
        GLint y = (j / 3) * 256;
        glViewport(x, y, 256, 256);

        zeus::CMatrix4f mv = LookMATs[j] * zeus::CTransform::Translate(-pos).toMatrix4f();
        m_uniformBuffer.m_xf = g_Proj * mv;
        glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBufferGL);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBuffer), &m_uniformBuffer, GL_DYNAMIC_DRAW);

        glUniformBlockBinding(m_program, m_uniLoc, 0);
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_uniformBufferGL, 0, sizeof(UniformBuffer));

        zeus::CFrustum frustum;
        frustum.updatePlanes(mv, g_Proj);

        // Draw frontfaces
        glCullFace(GL_BACK);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        for (const Model& model : m_models)
        {
            if (!frustum.aabbFrustumTest(model.aabb))
                continue;
            glBindVertexArray(model.vao);
            for (const Model::Surface& surf : model.surfaces)
            {
                // Non-transparents first
                if (!surf.transparent)
                    glDrawElements(model.topology, surf.count, GL_UNSIGNED_INT,
                                   reinterpret_cast<void*>(uintptr_t(surf.first * 4)));
            }
        }
    }

    //m_swapFunc();
    glFinish();
    glReadPixels(0, 0, 768, 512, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)bufOut);
}

void VISIRenderer::RenderPVSTransparent(const std::function<void(int)>& passFunc, const zeus::CVector3f& pos)
{
    glDepthMask(GL_FALSE);

    for (int j=0 ; j<6 ; ++j)
    {
        GLint x = (j % 3) * 256;
        GLint y = (j / 3) * 256;
        glViewport(x, y, 256, 256);

        zeus::CMatrix4f mv = LookMATs[j] * zeus::CTransform::Translate(-pos).toMatrix4f();
        m_uniformBuffer.m_xf = g_Proj * mv;
        glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBufferGL);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBuffer), &m_uniformBuffer, GL_DYNAMIC_DRAW);

        glUniformBlockBinding(m_program, m_uniLoc, 0);
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_uniformBufferGL, 0, sizeof(UniformBuffer));

        zeus::CFrustum frustum;
        frustum.updatePlanes(mv, g_Proj);

        memset(m_queryBools.get(), 0, m_queryCount);

        int idx = 0;
        for (const Model& model : m_models)
        {
            if (!frustum.aabbFrustumTest(model.aabb))
            {
                ++idx;
                continue;
            }
            glBindVertexArray(model.vao);
            glBeginQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE, m_queries[idx]);
            m_queryBools[idx] = true;
            for (const Model::Surface& surf : model.surfaces)
            {
                // transparents
                if (surf.transparent)
                    glDrawElements(model.topology, surf.count, GL_UNSIGNED_INT,
                                   reinterpret_cast<void*>(uintptr_t(surf.first * 4)));
            }
            glEndQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE);
            ++idx;
        }

        for (int i=0 ; i<idx ; ++i)
        {
            if (m_queryBools[i])
            {
                GLint res;
                glGetQueryObjectiv(m_queries[i], GL_QUERY_RESULT, &res);
                if (res)
                    passFunc(i);
            }
        }
    }
}

void VISIRenderer::RenderPVSEntitiesAndLights(const std::function<void(int)>& passFunc,
                                              const std::function<void(int, EPVSVisSetState)>& lightPassFunc,
                                              const zeus::CVector3f& pos)
{
    glDepthMask(GL_FALSE);

    for (int j=0 ; j<6 ; ++j)
    {
        GLint x = (j % 3) * 256;
        GLint y = (j / 3) * 256;
        glViewport(x, y, 256, 256);

        zeus::CMatrix4f mv = LookMATs[j] * zeus::CTransform::Translate(-pos).toMatrix4f();
        m_uniformBuffer.m_xf = g_Proj * mv;
        glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBufferGL);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBuffer), &m_uniformBuffer, GL_DYNAMIC_DRAW);

        glUniformBlockBinding(m_program, m_uniLoc, 0);
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_uniformBufferGL, 0, sizeof(UniformBuffer));

        zeus::CFrustum frustum;
        frustum.updatePlanes(mv, g_Proj);

        memset(m_queryBools.get(), 0, m_queryCount);

        int idx = m_models.size();
        for (const Entity& ent : m_entities)
        {
            if (!frustum.aabbFrustumTest(ent.aabb))
            {
                ++idx;
                continue;
            }
            glBindVertexArray(ent.vao);
            m_queryBools[idx] = true;
            glBeginQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE, m_queries[idx]);
            glDrawElements(GL_TRIANGLE_STRIP, 20, GL_UNSIGNED_INT, 0);
            glEndQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE);
            ++idx;
        }

        for (const Light& light : m_lights)
        {
            if (!frustum.pointFrustumTest(light.point))
            {
                ++idx;
                continue;
            }
            glBindVertexArray(light.vao);
            m_queryBools[idx] = true;
            glBeginQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE, m_queries[idx]);
            glDrawArrays(GL_POINTS, 0, 1);
            glEndQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE);
            ++idx;
        }

        idx = m_models.size();
        for (const Entity& ent : m_entities)
        {
            (void)ent;
            if (m_queryBools[idx])
            {
                GLint res;
                glGetQueryObjectiv(m_queries[idx], GL_QUERY_RESULT, &res);
                if (res)
                    passFunc(idx);
            }
            ++idx;
        }

        int lightIdx = 0;
        for (const Light& light : m_lights)
        {
            if (m_queryBools[idx])
            {
                GLint res;
                glGetQueryObjectiv(m_queries[idx], GL_QUERY_RESULT, &res);
                EPVSVisSetState state = m_totalAABB.pointInside(light.point) ?
                                        EPVSVisSetState::EndOfTree : EPVSVisSetState::OutOfBounds;
                if (res && state == EPVSVisSetState::EndOfTree)
                    state = EPVSVisSetState::NodeFound;
                lightPassFunc(lightIdx, state);
            }
            ++lightIdx;
            ++idx;
        }
    }
}

void VISIRenderer::Run(FPercent updatePercent)
{
    m_updatePercent = updatePercent;
    CalculateProjMatrix();

    if (glewInit() != GLEW_OK)
    {
        Log.report(logvisor::Error, "unable to init glew");
        m_return = 1;
        return;
    }

    if (!GLEW_ARB_occlusion_query2)
    {
        Log.report(logvisor::Error, "GL_ARB_occlusion_query2 extension not present");
        m_return = 1;
        return;
    }

    if (!SetupShaders())
    {
        m_return = 1;
        return;
    }

    if (m_argc < 3)
    {
        Log.report(logvisor::Error, "Missing input/output file args");
        m_return = 1;
        return;
    }

    ProcessType parentPid = 0;
    if (m_argc > 4)
#ifdef _WIN32
        parentPid = ProcessType(wcstoull(m_argv[4], nullptr, 16));
#else
        parentPid = ProcessType(strtoull(m_argv[4], nullptr, 16));
#endif

    uint32_t layer2LightCount = 0;
    {
        athena::io::FileReader r(m_argv[1]);
        if (r.hasError())
            return;

        uint32_t modelCount = r.readUint32Big();
        m_models.resize(modelCount);
        for (uint32_t i=0 ; i<modelCount ; ++i)
        {
            zeus::CColor color = ColorForIndex(i);
            Model& model = m_models[i];
            uint32_t topology = r.readUint32Big();
            model.topology = topology ? GL_TRIANGLE_STRIP : GL_TRIANGLES;

            uint32_t vertCount = r.readUint32Big();
            model.verts.reserve(vertCount);
            for (uint32_t j=0 ; j<vertCount ; ++j)
            {
                model.verts.emplace_back();
                Model::Vert& vert = model.verts.back();
                vert.pos = r.readVec3fBig();
                vert.color = color;
                m_totalAABB.accumulateBounds(vert.pos);
                model.aabb.accumulateBounds(vert.pos);
            }

            uint32_t surfCount = r.readUint32Big();
            model.surfaces.resize(surfCount);
            uint32_t curIdx = 0;
            for (uint32_t j=0 ; j<surfCount ; ++j)
            {
                Model::Surface& surf = model.surfaces[j];
                surf.first = curIdx;
                surf.count = r.readUint32Big();
                curIdx += surf.count;

                for (uint32_t k=0 ; k<surf.count ; ++k)
                {
                    uint32_t idx = r.readUint32Big();
                    model.idxs.push_back(idx);
                }

                surf.transparent = r.readBool();
            }
        }

        uint32_t entityCount = r.readUint32Big();
        m_entities.resize(entityCount);
        for (uint32_t i=0 ; i<entityCount ; ++i)
        {
            Entity& ent = m_entities[i];
            ent.entityId = r.readUint32Big();
            ent.aabb.min = r.readVec3fBig();
            ent.aabb.max = r.readVec3fBig();
        }

        uint32_t lightCount = r.readUint32Big();
        layer2LightCount = r.readUint32Big();
        m_lights.resize(lightCount);
        for (uint32_t i=0 ; i<lightCount ; ++i)
        {
            Light& light = m_lights[i];
            light.point = r.readVec3fBig();
        }
    }

    if (!SetupVertexBuffersAndFormats())
    {
        m_return = 1;
        return;
    }

    VISIBuilder builder(*this);
    std::vector<uint8_t> dataOut = builder.build(m_totalAABB, m_models.size(),
                                                 m_entities, m_lights, layer2LightCount,
                                                 m_updatePercent, parentPid);
    if (dataOut.empty())
    {
        m_return = 1;
        return;
    }

    athena::io::FileWriter w(m_argv[2]);
    w.writeUBytes(dataOut.data(), dataOut.size());
}

void VISIRenderer::Terminate()
{
    m_terminate = true;
}
