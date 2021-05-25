#include "ImGuiEngine.hpp"

#include "athena/Compression.hpp"
#include "hecl/Pipeline.hpp"
#include "hecl/VertexBufferPool.hpp"

#include <zeus/CMatrix4f.hpp>

extern "C" const uint8_t NOTO_MONO_FONT[];
extern "C" const size_t NOTO_MONO_FONT_SZ;
extern "C" const size_t NOTO_MONO_FONT_DECOMPRESSED_SZ;

namespace metaforce {
static logvisor::Module Log{"ImGuiEngine"};

struct ImGuiEngine::Input ImGuiEngine::Input;

static boo::IGraphicsDataFactory* m_factory;
static boo::IWindow* m_window;
static boo::ObjToken<boo::IShaderPipeline> ShaderPipeline;
static boo::ObjToken<boo::IGraphicsBufferD> VertexBuffer;
static boo::ObjToken<boo::IGraphicsBufferD> IndexBuffer;
static boo::ObjToken<boo::IGraphicsBufferD> UniformBuffer;
static boo::ObjToken<boo::IShaderDataBinding> ShaderDataBinding;
static boo::ObjToken<boo::ITextureS> ImGuiAtlas;
static boo::SWindowRect WindowRect;

struct Uniform {
  zeus::CMatrix4f xf;
};

static size_t VertexBufferSize = 5000;
static size_t IndexBufferSize = 5000;

const char* getClipboardText(void* userData) {
  static ImVector<char> ClipboardBuf;
  size_t sz = 0;
  const auto data = static_cast<boo::IWindow*>(userData)->clipboardPaste(boo::EClipboardType::String, sz);
  ClipboardBuf.resize(sz);
  strncpy(ClipboardBuf.Data, reinterpret_cast<const char*>(data.get()), sz);
  return ClipboardBuf.Data;
}

void setClipboardText(void* userData, const char* text) {
  const auto* data = reinterpret_cast<const uint8_t*>(text);
  static_cast<boo::IWindow*>(userData)->clipboardCopy(boo::EClipboardType::String, data, strlen(text));
}

void ImGuiEngine::Initialize(boo::IGraphicsDataFactory* factory, boo::IWindow* window, float scale) {
  m_factory = factory;
  m_window = window;
  WindowRect = window->getWindowFrame();

  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
  io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
  io.GetClipboardTextFn = getClipboardText;
  io.SetClipboardTextFn = setClipboardText;
  io.ClipboardUserData = window;

  io.KeyMap[ImGuiKey_Tab] = 256 + static_cast<int>(boo::ESpecialKey::Tab);
  io.KeyMap[ImGuiKey_LeftArrow] = 256 + static_cast<int>(boo::ESpecialKey::Left);
  io.KeyMap[ImGuiKey_RightArrow] = 256 + static_cast<int>(boo::ESpecialKey::Right);
  io.KeyMap[ImGuiKey_UpArrow] = 256 + static_cast<int>(boo::ESpecialKey::Up);
  io.KeyMap[ImGuiKey_DownArrow] = 256 + static_cast<int>(boo::ESpecialKey::Down);
  io.KeyMap[ImGuiKey_PageUp] = 256 + static_cast<int>(boo::ESpecialKey::PgUp);
  io.KeyMap[ImGuiKey_PageDown] = 256 + static_cast<int>(boo::ESpecialKey::PgDown);
  io.KeyMap[ImGuiKey_Home] = 256 + static_cast<int>(boo::ESpecialKey::Home);
  io.KeyMap[ImGuiKey_End] = 256 + static_cast<int>(boo::ESpecialKey::End);
  io.KeyMap[ImGuiKey_Insert] = 256 + static_cast<int>(boo::ESpecialKey::Insert);
  io.KeyMap[ImGuiKey_Delete] = 256 + static_cast<int>(boo::ESpecialKey::Delete);
  io.KeyMap[ImGuiKey_Backspace] = 256 + static_cast<int>(boo::ESpecialKey::Backspace);
  io.KeyMap[ImGuiKey_Space] = ' ';
  io.KeyMap[ImGuiKey_Enter] = 256 + static_cast<int>(boo::ESpecialKey::Enter);
  io.KeyMap[ImGuiKey_Escape] = 256 + static_cast<int>(boo::ESpecialKey::Esc);
  io.KeyMap[ImGuiKey_A] = 'a'; // for text edit CTRL+A: select all
  io.KeyMap[ImGuiKey_C] = 'c'; // for text edit CTRL+C: copy
  io.KeyMap[ImGuiKey_V] = 'v'; // for text edit CTRL+V: paste
  io.KeyMap[ImGuiKey_X] = 'x'; // for text edit CTRL+X: cut
  io.KeyMap[ImGuiKey_Y] = 'y'; // for text edit CTRL+Y: redo
  io.KeyMap[ImGuiKey_Z] = 'z'; // for text edit CTRL+Z: undo

  auto* fontData = new uint8_t[NOTO_MONO_FONT_DECOMPRESSED_SZ];
  athena::io::Compression::decompressZlib(NOTO_MONO_FONT, NOTO_MONO_FONT_SZ, fontData,
                                          NOTO_MONO_FONT_DECOMPRESSED_SZ);

  int width = 0;
  int height = 0;
  unsigned char* pixels = nullptr;
  ImFontConfig fontConfig{};
  fontConfig.FontData = fontData;
  fontConfig.FontDataSize = NOTO_MONO_FONT_DECOMPRESSED_SZ;
  fontConfig.SizePixels = std::floor(14.f * scale);
  fontConfig.OversampleH = 2;
  snprintf(fontConfig.Name, sizeof(fontConfig.Name), "Noto Mono Regular, %dpx",
           static_cast<int>(fontConfig.SizePixels));
  io.Fonts->AddFont(&fontConfig);
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
  factory->commitTransaction([&](boo::IGraphicsDataFactory::Context& ctx) {
    ShaderPipeline = hecl::conv->convert(Shader_ImGuiShader{});
    ImGuiAtlas = ctx.newStaticTexture(width, height, 1, boo::TextureFormat::RGBA8, boo::TextureClampMode::ClampToEdge,
                                      pixels, width * height * 4);
    VertexBuffer = ctx.newDynamicBuffer(boo::BufferUse::Vertex, sizeof(ImDrawVert), VertexBufferSize);
    IndexBuffer = ctx.newDynamicBuffer(boo::BufferUse::Index, sizeof(ImDrawIdx), IndexBufferSize);
    UniformBuffer = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
    BuildShaderDataBinding(ctx);
    return true;
  } BooTrace);
  io.Fonts->SetTexID(ImGuiAtlas.get());

  ImGui::GetStyle().ScaleAllSizes(scale);
}

void ImGuiEngine::Shutdown() {
  ImGui::DestroyContext();
  ShaderDataBinding.reset();
  ShaderPipeline.reset();
}

void ImGuiEngine::Begin(float dt, float scale) {
  ImGuiIO& io = ImGui::GetIO();
  io.DeltaTime = dt;
  io.DisplaySize.x = static_cast<float>(WindowRect.size[0]);
  io.DisplaySize.y = static_cast<float>(WindowRect.size[1]);
  io.DisplayFramebufferScale = ImVec2{scale, scale};
  if (Input.m_mouseIn) {
    io.MousePos = ImVec2{static_cast<float>(Input.m_mousePos.pixel[0]),
                         static_cast<float>(WindowRect.size[1] - Input.m_mousePos.pixel[1])};
  } else {
    io.MousePos = ImVec2{-FLT_MAX, -FLT_MAX};
  }
  memcpy(io.MouseDown, Input.m_mouseButtons.data(), sizeof(io.MouseDown));
  float scrollDelta = Input.m_scrollDelta.delta[1] / scale;
  float scrollDeltaH = Input.m_scrollDelta.delta[0] / scale;
  if (Input.m_scrollDelta.isAccelerated) {
    scrollDelta /= 10.f;
    scrollDeltaH /= 10.f;
  }
  io.MouseWheel = static_cast<float>(scrollDelta);
  io.MouseWheelH = static_cast<float>(scrollDeltaH);
  Input.m_scrollDelta.zeroOut();
  io.KeyCtrl = True(Input.m_modifiers & boo::EModifierKey::Ctrl);
  io.KeyShift = True(Input.m_modifiers & boo::EModifierKey::Shift);
  io.KeyAlt = True(Input.m_modifiers & boo::EModifierKey::Alt);
  io.KeySuper = True(Input.m_modifiers & boo::EModifierKey::Command);
  memcpy(io.KeysDown, Input.m_keys.data(), sizeof(io.KeysDown));

  for (const auto c : ImGuiEngine::Input.m_charCodes) {
    io.AddInputCharacter(c);
  }
  ImGuiEngine::Input.m_charCodes.clear();

  ImGuiWindowCallback::m_mouseCaptured = io.WantCaptureMouse;
  ImGuiWindowCallback::m_keyboardCaptured = io.WantCaptureKeyboard;

  switch (ImGui::GetMouseCursor()) {
  default:
    m_window->setCursor(boo::EMouseCursor::Pointer);
    break;
  case ImGuiMouseCursor_TextInput:
    m_window->setCursor(boo::EMouseCursor::IBeam);
    break;
  case ImGuiMouseCursor_ResizeNS:
    m_window->setCursor(boo::EMouseCursor::VerticalArrow);
    break;
  case ImGuiMouseCursor_ResizeEW:
    m_window->setCursor(boo::EMouseCursor::HorizontalArrow);
    break;
  case ImGuiMouseCursor_ResizeNESW:
    m_window->setCursor(boo::EMouseCursor::BottomLeftArrow);
    break;
  case ImGuiMouseCursor_ResizeNWSE:
    m_window->setCursor(boo::EMouseCursor::BottomRightArrow);
    break;
  case ImGuiMouseCursor_Hand:
    m_window->setCursor(boo::EMouseCursor::Hand);
    break;
  case ImGuiMouseCursor_NotAllowed:
    m_window->setCursor(boo::EMouseCursor::NotAllowed);
    break;
  }

  ImGui::NewFrame();
}

void ImGuiEngine::End() {
  ImGui::EndFrame();
  ImGui::Render();
  ImDrawData* drawData = ImGui::GetDrawData();

  float L = drawData->DisplayPos.x;
  float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
  float T = drawData->DisplayPos.y;
  float B = drawData->DisplayPos.y + drawData->DisplaySize.y;
  float N = 0.f;
  float F = 1.f;
  zeus::CMatrix4f projXf = zeus::CMatrix4f{
      zeus::CVector4f{2.0f / (R - L), 0.0f, 0.0f, 0.0f},
      zeus::CVector4f{0.0f, 2.0f / (T - B), 0.0f, 0.0f},
      zeus::CVector4f{0.0f, 0.0f, 1 / (F - N), 0.0f},
      zeus::CVector4f{(R + L) / (L - R), (T + B) / (B - T), N / (F - N), 1.0f},
  };

  m_factory->commitTransaction([&](boo::IGraphicsDataFactory::Context& ctx) {
    bool rebind = false;
    if (drawData->TotalIdxCount > IndexBufferSize) {
      Log.report(logvisor::Info, FMT_STRING(_SYS_STR("Resizing index buffer from {} to {}")), IndexBufferSize,
                 drawData->TotalIdxCount);
      IndexBuffer = ctx.newDynamicBuffer(boo::BufferUse::Index, sizeof(ImDrawIdx), drawData->TotalIdxCount);
      IndexBufferSize = drawData->TotalIdxCount;
      rebind = true;
    }
    if (drawData->TotalVtxCount > VertexBufferSize) {
      Log.report(logvisor::Info, FMT_STRING(_SYS_STR("Resizing vertex buffer from {} to {}")), VertexBufferSize,
                 drawData->TotalVtxCount);
      VertexBuffer = ctx.newDynamicBuffer(boo::BufferUse::Vertex, sizeof(ImDrawVert), drawData->TotalVtxCount);
      VertexBufferSize = drawData->TotalVtxCount;
      rebind = true;
    }
    if (rebind) {
      BuildShaderDataBinding(ctx);
    }

    UniformBuffer->load(&projXf, sizeof(Uniform));
    auto* vtxBuf = static_cast<ImDrawVert*>(VertexBuffer->map(drawData->TotalVtxCount * sizeof(ImDrawVert)));
    auto* idxBuf = static_cast<ImDrawIdx*>(IndexBuffer->map(drawData->TotalIdxCount * sizeof(ImDrawIdx)));

    for (int i = 0; i < drawData->CmdListsCount; ++i) {
      const auto* cmdList = drawData->CmdLists[i];
      int vtxBufferSz = cmdList->VtxBuffer.size();
      int idxBufferSz = cmdList->IdxBuffer.size();
      memcpy(vtxBuf, cmdList->VtxBuffer.begin(), vtxBufferSz * sizeof(ImDrawVert));
      memcpy(idxBuf, cmdList->IdxBuffer.begin(), idxBufferSz * sizeof(ImDrawIdx));
      vtxBuf += vtxBufferSz;
      idxBuf += idxBufferSz;
    }

    VertexBuffer->unmap();
    IndexBuffer->unmap();
    return true;
  } BooTrace);
}

void ImGuiEngine::Draw(boo::IGraphicsCommandQueue* gfxQ) {
  ImDrawData* drawData = ImGui::GetDrawData();

  gfxQ->setShaderDataBinding(ShaderDataBinding);

  boo::SWindowRect viewportRect = WindowRect;
  int viewportHeight = viewportRect.size[1];
  viewportRect.location = {0, 0};
  if (m_factory->platform() == boo::IGraphicsDataFactory::Platform::Vulkan) {
    viewportRect.location[1] = viewportHeight;
    viewportRect.size[1] = -viewportHeight;
  }
  gfxQ->setViewport(viewportRect);

  size_t idxOffset = 0;
  size_t vtxOffset = 0;
  for (int i = 0; i < drawData->CmdListsCount; ++i) {
    const auto* cmdList = drawData->CmdLists[i];
    for (const auto& drawCmd : cmdList->CmdBuffer) {
      if (drawCmd.UserCallback != nullptr) {
        drawCmd.UserCallback(cmdList, &drawCmd);
        continue;
      }
      ImVec2 pos = drawData->DisplayPos;
      int clipX = static_cast<int>(drawCmd.ClipRect.x - pos.x);
      int clipY = static_cast<int>(drawCmd.ClipRect.y - pos.y);
      int clipW = static_cast<int>(drawCmd.ClipRect.z - pos.x) - clipX;
      int clipH = static_cast<int>(drawCmd.ClipRect.w - pos.y) - clipY;
      boo::SWindowRect clipRect{clipX, clipY, clipW, clipH};
      if (m_factory->platform() == boo::IGraphicsDataFactory::Platform::Vulkan
          || m_factory->platform() == boo::IGraphicsDataFactory::Platform::Metal) {
        clipRect.location[1] = viewportHeight - clipRect.location[1] - clipRect.size[1];
      }
      gfxQ->setScissor(clipRect);
      gfxQ->drawIndexed(idxOffset + drawCmd.IdxOffset, drawCmd.ElemCount, vtxOffset + drawCmd.VtxOffset);
    }
    idxOffset += cmdList->IdxBuffer.size();
    vtxOffset += cmdList->VtxBuffer.size();
  }
}

void ImGuiEngine::BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx) {
  boo::ObjToken<boo::IGraphicsBuffer> uniforms[] = {UniformBuffer.get()};
  boo::PipelineStage unistages[] = {boo::PipelineStage::Vertex};
  size_t unioffs[] = {0};
  size_t unisizes[] = {sizeof(Uniform)};
  boo::ObjToken<boo::ITexture> texs[] = {ImGuiAtlas.get()};
  ShaderDataBinding = ctx.newShaderDataBinding(ShaderPipeline, VertexBuffer.get(), nullptr, IndexBuffer.get(), 1,
                                               uniforms, unistages, unioffs, unisizes, 1, texs, nullptr, nullptr);
}

bool ImGuiWindowCallback::m_mouseCaptured = false;
bool ImGuiWindowCallback::m_keyboardCaptured = false;

void ImGuiWindowCallback::mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mods) {
  ImGuiEngine::Input.m_mousePos = coord;
  if (button != boo::EMouseButton::None) {
    ImGuiEngine::Input.m_mouseButtons[static_cast<size_t>(button) - 1] = true;
  }
  ImGuiEngine::Input.m_modifiers = mods;
}

void ImGuiWindowCallback::mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mods) {
  ImGuiEngine::Input.m_mousePos = coord;
  if (button != boo::EMouseButton::None) {
    ImGuiEngine::Input.m_mouseButtons[static_cast<size_t>(button) - 1] = false;
  }
  ImGuiEngine::Input.m_modifiers = mods;
}

void ImGuiWindowCallback::mouseMove(const boo::SWindowCoord& coord) { ImGuiEngine::Input.m_mousePos = coord; }

void ImGuiWindowCallback::mouseEnter(const boo::SWindowCoord& coord) {
  ImGuiEngine::Input.m_mouseIn = true;
  ImGuiEngine::Input.m_mousePos = coord;
}

void ImGuiWindowCallback::mouseLeave(const boo::SWindowCoord& coord) {
  ImGuiEngine::Input.m_mouseIn = false;
  ImGuiEngine::Input.m_mousePos = coord;
}

void ImGuiWindowCallback::scroll(const boo::SWindowCoord& coord, const boo::SScrollDelta& scroll) {
  ImGuiEngine::Input.m_mousePos = coord;
  ImGuiEngine::Input.m_scrollDelta += scroll;
}

void ImGuiWindowCallback::charKeyDown(unsigned long charCode, boo::EModifierKey mods, bool isRepeat) {
  if (charCode < 256) {
    ImGuiEngine::Input.m_keys[charCode] = true;
  }
  ImGuiEngine::Input.m_charCodes.push_back(charCode);
  ImGuiEngine::Input.m_modifiers = mods;
}

void ImGuiWindowCallback::charKeyUp(unsigned long charCode, boo::EModifierKey mods) {
  if (charCode < 256) {
    ImGuiEngine::Input.m_keys[charCode] = false;
  }
  ImGuiEngine::Input.m_modifiers = mods;
}

void ImGuiWindowCallback::specialKeyDown(boo::ESpecialKey key, boo::EModifierKey mods, bool isRepeat) {
  ImGuiEngine::Input.m_keys[256 + static_cast<int>(key)] = true;
  ImGuiEngine::Input.m_modifiers = mods;
}

void ImGuiWindowCallback::specialKeyUp(boo::ESpecialKey key, boo::EModifierKey mods) {
  ImGuiEngine::Input.m_keys[256 + static_cast<int>(key)] = false;
  ImGuiEngine::Input.m_modifiers = mods;
}

void ImGuiWindowCallback::resized(const boo::SWindowRect& rect, bool sync) { WindowRect = rect; }

} // namespace metaforce
