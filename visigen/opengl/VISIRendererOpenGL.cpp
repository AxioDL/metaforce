#include "VISIRendererOpenGL.hpp"
#include "logvisor/logvisor.hpp"
#include "zeus/CFrustum.hpp"

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

static const uint32_t AABBIdxs[20] = {0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 1, 7, 3, 5, 5, 0, 0, 2, 6, 4};

static zeus::CMatrix4f g_Proj;

static void CalculateProjMatrix() {
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

  g_Proj = zeus::CMatrix4f(2.f * znear / rml, 0.f, rpl / rml, 0.f, 0.f, 2.f * znear / tmb, tpb / tmb, 0.f, 0.f, 0.f,
                           -fpn / fmn, -2.f * zfar * znear / fmn, 0.f, 0.f, -1.f, 0.f);
}

static const zeus::CMatrix4f LookMATs[] = {
    {// Forward
     1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, -1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f},
    {// Backward
     -1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f},
    {// Up
     1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, 0.f, 0.f, 1.f},
    {// Down
     1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f},
    {// Left
     0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f},
    {// Right
     0.f, -1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, -1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f},
};

bool VISIRendererOpenGL::SetupShaders() {
  CalculateProjMatrix();

  if (glewInit() != GLEW_OK) {
    Log.report(logvisor::Error, FMT_STRING("unable to init glew"));
    return false;
  }

  if (!GLEW_ARB_occlusion_query2) {
    Log.report(logvisor::Error, FMT_STRING("GL_ARB_occlusion_query2 extension not present"));
    return false;
  }

  m_vtxShader = glCreateShader(GL_VERTEX_SHADER);
  m_fragShader = glCreateShader(GL_FRAGMENT_SHADER);
  m_program = glCreateProgram();

  glShaderSource(m_vtxShader, 1, &VS, nullptr);
  glCompileShader(m_vtxShader);
  GLint status;
  glGetShaderiv(m_vtxShader, GL_COMPILE_STATUS, &status);
  if (status != GL_TRUE) {
    GLint logLen;
    glGetShaderiv(m_vtxShader, GL_INFO_LOG_LENGTH, &logLen);
    char* log = (char*)malloc(logLen);
    glGetShaderInfoLog(m_vtxShader, logLen, nullptr, log);
    Log.report(logvisor::Error, FMT_STRING("unable to compile vert source\n{}\n{}\n"), log, VS);
    free(log);
    return false;
  }

  glShaderSource(m_fragShader, 1, &FS, nullptr);
  glCompileShader(m_fragShader);
  glGetShaderiv(m_fragShader, GL_COMPILE_STATUS, &status);
  if (status != GL_TRUE) {
    GLint logLen;
    glGetShaderiv(m_fragShader, GL_INFO_LOG_LENGTH, &logLen);
    char* log = (char*)malloc(logLen);
    glGetShaderInfoLog(m_fragShader, logLen, nullptr, log);
    Log.report(logvisor::Error, FMT_STRING("unable to compile frag source\n{}\n{}\n"), log, FS);
    free(log);
    return false;
  }

  glAttachShader(m_program, m_vtxShader);
  glAttachShader(m_program, m_fragShader);

  glLinkProgram(m_program);
  glGetProgramiv(m_program, GL_LINK_STATUS, &status);
  if (status != GL_TRUE) {
    GLint logLen;
    glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &logLen);
    char* log = (char*)malloc(logLen);
    glGetProgramInfoLog(m_program, logLen, nullptr, log);
    Log.report(logvisor::Error, FMT_STRING("unable to link shader program\n{}\n"), log);
    free(log);
    return false;
  }

  glUseProgram(m_program);
  m_uniLoc = glGetUniformBlockIndex(m_program, "UniformBlock");

  glGenBuffers(1, &m_uniformBufferGL);
  glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBufferGL);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(zeus::CMatrix4f) * 6, nullptr, GL_DYNAMIC_DRAW);

  glGenBuffers(1, &m_aabbIBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_aabbIBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 20 * 4, AABBIdxs, GL_STATIC_DRAW);

  glEnable(GL_PRIMITIVE_RESTART);
  glPrimitiveRestartIndex(0xffffffff);

  return true;
}

bool VISIRendererOpenGL::SetupVertexBuffersAndFormats() {
  m_modelBindings.resize(m_models.size());
  m_entityBindings.resize(m_entities.size());
  m_lightBindings.resize(m_lights.size());

  {
    auto model = m_models.begin();
    auto modelBinding = m_modelBindings.begin();
    while (model != m_models.end()) {
      glGenVertexArrays(1, &modelBinding->vao);
      glGenBuffers(1, &modelBinding->vbo);
      glGenBuffers(1, &modelBinding->ibo);

      glBindVertexArray(modelBinding->vao);

      glBindBuffer(GL_ARRAY_BUFFER, modelBinding->vbo);
      glBufferData(GL_ARRAY_BUFFER, model->verts.size() * sizeof(Model::Vert), model->verts.data(), GL_STATIC_DRAW);
      glEnableVertexAttribArray(0);
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Model::Vert), 0);
      glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Model::Vert), (void*)16);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelBinding->ibo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->idxs.size() * 4, model->idxs.data(), GL_STATIC_DRAW);

      ++model;
      ++modelBinding;
    }
  }

  uint32_t idx = m_models.size();
  {
    auto ent = m_entities.begin();
    auto entBinding = m_entityBindings.begin();
    while (ent != m_entities.end()) {
      glGenVertexArrays(1, &entBinding->vao);
      glGenBuffers(1, &entBinding->vbo);

      glBindVertexArray(entBinding->vao);

      auto verts = AABBToVerts(ent->aabb, ColorForIndex(idx++));
      glBindBuffer(GL_ARRAY_BUFFER, entBinding->vbo);
      glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Model::Vert), verts.data(), GL_STATIC_DRAW);
      glEnableVertexAttribArray(0);
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Model::Vert), 0);
      glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Model::Vert), (void*)16);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_aabbIBO);

      ++ent;
      ++entBinding;
    }
  }

  {
    auto light = m_lights.begin();
    auto lightBinding = m_lightBindings.begin();
    while (light != m_lights.end()) {
      glGenVertexArrays(1, &lightBinding->vao);
      glGenBuffers(1, &lightBinding->vbo);

      glBindVertexArray(lightBinding->vao);

      Model::Vert vert;
      vert.pos = light->point;
      vert.color = ColorForIndex(idx++);
      glBindBuffer(GL_ARRAY_BUFFER, lightBinding->vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(Model::Vert), &vert, GL_STATIC_DRAW);
      glEnableVertexAttribArray(0);
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Model::Vert), 0);
      glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Model::Vert), (void*)16);

      ++light;
      ++lightBinding;
    }
  }

  m_queryCount = m_models.size() + m_entities.size() + m_lights.size();
  m_queries.reset(new GLuint[m_queryCount]);
  m_queryBools.reset(new bool[m_queryCount]);
  glGenQueries(GLsizei(m_queryCount), m_queries.get());

  return true;
}

void VISIRendererOpenGL::SetupRenderPass(const zeus::CVector3f& pos) {
  glViewport(0, 0, 768, 512);
  glEnable(GL_CULL_FACE);
  glDepthMask(GL_TRUE);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.f, 0.f, 0.f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  std::array<std::aligned_union_t<256, zeus::CMatrix4f>, 6> m_uniformBuffers{};
  static_assert(sizeof(m_uniformBuffers) == 256 * 6);
  glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBufferGL);
  for (int j = 0; j < 6; ++j) {
    zeus::CMatrix4f mv = LookMATs[j] * zeus::CTransform::Translate(-pos).toMatrix4f();
    *static_cast<zeus::CMatrix4f*>(static_cast<void*>(&m_uniformBuffers[j])) = g_Proj * mv;
    m_frustums[j].updatePlanes(mv, g_Proj);
  }
  glBufferData(GL_UNIFORM_BUFFER, sizeof(m_uniformBuffers), m_uniformBuffers.data(), GL_DYNAMIC_DRAW);
  glUniformBlockBinding(m_program, m_uniLoc, 0);
}

void VISIRendererOpenGL::RenderPVSOpaque(RGBA8* bufOut, bool& needTransparent) {
  for (int j = 0; j < 6; ++j) {
    GLint x = (j % 3) * 256;
    GLint y = (j / 3) * 256;
    glViewport(x, y, 256, 256);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_uniformBufferGL, static_cast<GLintptr>(256 * j),
                      sizeof(zeus::CMatrix4f));

    // Draw frontfaces
    glCullFace(GL_BACK);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    auto model = m_models.begin();
    auto modelBinding = m_modelBindings.begin();
    while (model != m_models.end()) {
      if (m_frustums[j].aabbFrustumTest(model->aabb)) {
        glBindVertexArray(modelBinding->vao);
        for (const Model::Surface& surf : model->surfaces) {
          // Non-transparents first
          if (surf.transparent) {
            needTransparent = true;
          } else {
            GLenum topology = model->topology == hecl::HMDLTopology::TriStrips ? GL_TRIANGLE_STRIP : GL_TRIANGLES;
            glDrawElements(topology, surf.count, GL_UNSIGNED_INT, reinterpret_cast<void*>(uintptr_t(surf.first * 4)));
          }
        }
      }
      ++model;
      ++modelBinding;
    }
  }

  // m_swapFunc();
  glFinish();
  glReadPixels(0, 0, 768, 512, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)bufOut);
}

void VISIRendererOpenGL::RenderPVSTransparent(const std::function<void(int)>& passFunc) {
  glDepthMask(GL_FALSE);

  for (int j = 0; j < 6; ++j) {
    GLint x = (j % 3) * 256;
    GLint y = (j / 3) * 256;
    glViewport(x, y, 256, 256);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_uniformBufferGL, static_cast<GLintptr>(256 * j),
                      sizeof(zeus::CMatrix4f));

    memset(m_queryBools.get(), 0, m_queryCount);

    int idx = 0;
    auto model = m_models.begin();
    auto modelBinding = m_modelBindings.begin();
    while (model != m_models.end()) {
      if (m_frustums[j].aabbFrustumTest(model->aabb)) {
        glBindVertexArray(modelBinding->vao);
        glBeginQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE, m_queries[idx]);
        m_queryBools[idx] = true;
        for (const Model::Surface& surf : model->surfaces) {
          // transparents
          if (surf.transparent) {
            GLenum topology = model->topology == hecl::HMDLTopology::TriStrips ? GL_TRIANGLE_STRIP : GL_TRIANGLES;
            glDrawElements(topology, surf.count, GL_UNSIGNED_INT, reinterpret_cast<void*>(uintptr_t(surf.first * 4)));
          }
        }
        glEndQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE);
      }
      ++idx;
      ++model;
      ++modelBinding;
    }

    for (int i = 0; i < idx; ++i) {
      if (m_queryBools[i]) {
        GLint res;
        glGetQueryObjectiv(m_queries[i], GL_QUERY_RESULT, &res);
        if (res)
          passFunc(i);
      }
    }
  }
}

void VISIRendererOpenGL::RenderPVSEntitiesAndLights(const std::function<void(int)>& passFunc,
                                                    const std::function<void(int, EPVSVisSetState)>& lightPassFunc) {
  glDepthMask(GL_FALSE);

  for (int j = 0; j < 6; ++j) {
    GLint x = (j % 3) * 256;
    GLint y = (j / 3) * 256;
    glViewport(x, y, 256, 256);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_uniformBufferGL, static_cast<GLintptr>(256 * j),
                      sizeof(zeus::CMatrix4f));

    memset(m_queryBools.get(), 0, m_queryCount);

    uint32_t idx = m_models.size();
    {
      auto ent = m_entities.begin();
      auto entBinding = m_entityBindings.begin();
      while (ent != m_entities.end()) {
        if (m_frustums[j].aabbFrustumTest(ent->aabb)) {
          glBindVertexArray(entBinding->vao);
          m_queryBools[idx] = true;
          glBeginQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE, m_queries[idx]);
          glDrawElements(GL_TRIANGLE_STRIP, 20, GL_UNSIGNED_INT, 0);
          glEndQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE);
        }
        ++idx;
        ++ent;
        ++entBinding;
      }
    }

    {
      auto light = m_lights.begin();
      auto lightBinding = m_lightBindings.begin();
      while (light != m_lights.end()) {
        if (m_frustums[j].pointFrustumTest(light->point)) {
          glBindVertexArray(lightBinding->vao);
          m_queryBools[idx] = true;
          glBeginQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE, m_queries[idx]);
          glDrawArrays(GL_POINTS, 0, 1);
          glEndQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE);
        }
        ++idx;
        ++light;
        ++lightBinding;
      }
    }

    idx = m_models.size();
    for (const Entity& ent : m_entities) {
      (void)ent;
      if (m_queryBools[idx]) {
        GLint res;
        glGetQueryObjectiv(m_queries[idx], GL_QUERY_RESULT, &res);
        if (res)
          passFunc(idx);
      }
      ++idx;
    }

    int lightIdx = 0;
    for (const Light& light : m_lights) {
      if (m_queryBools[idx]) {
        GLint res;
        glGetQueryObjectiv(m_queries[idx], GL_QUERY_RESULT, &res);
        EPVSVisSetState state =
            m_totalAABB.pointInside(light.point) ? EPVSVisSetState::EndOfTree : EPVSVisSetState::OutOfBounds;
        if (res && state == EPVSVisSetState::EndOfTree)
          state = EPVSVisSetState::NodeFound;
        lightPassFunc(lightIdx, state);
      }
      ++lightIdx;
      ++idx;
    }
  }
}
