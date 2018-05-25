#if BOO_HAS_GL
static TShader<_CLS>::IDataBindingFactory* Initialize(boo::GLDataFactory::Context& ctx);
#endif
#if _WIN32
static TShader<_CLS>::IDataBindingFactory* Initialize(boo::D3DDataFactory::Context& ctx);
#endif
#if BOO_HAS_METAL
static TShader<_CLS>::IDataBindingFactory* Initialize(boo::MetalDataFactory::Context& ctx);
#endif
#if BOO_HAS_VULKAN
static TShader<_CLS>::IDataBindingFactory* Initialize(boo::VulkanDataFactory::Context& ctx);
#endif
template <class F> static void Shutdown();
