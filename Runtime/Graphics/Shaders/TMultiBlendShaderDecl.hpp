#if BOO_HAS_GL
static TMultiBlendShader<_CLS>::IDataBindingFactory* Initialize(boo::GLDataFactory::Context& ctx);
#endif
#if _WIN32
static TMultiBlendShader<_CLS>::IDataBindingFactory* Initialize(boo::ID3DDataFactory::Context& ctx);
#endif
#if BOO_HAS_METAL
static TMultiBlendShader<_CLS>::IDataBindingFactory* Initialize(boo::MetalDataFactory::Context& ctx);
#endif
#if BOO_HAS_VULKAN
static TMultiBlendShader<_CLS>::IDataBindingFactory* Initialize(boo::VulkanDataFactory::Context& ctx);
#endif
template <class F> static void Shutdown();
