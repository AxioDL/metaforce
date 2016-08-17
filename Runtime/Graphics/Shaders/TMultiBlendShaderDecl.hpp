static TMultiBlendShader<_CLS>::IDataBindingFactory* Initialize(boo::GLDataFactory::Context& ctx,
                                                                boo::IShaderPipeline*& alphaPipeOut,
                                                                boo::IShaderPipeline*& additivePipeOut,
                                                                boo::IShaderPipeline*& colorMultiplyPipeOut);
#if _WIN32
static TMultiBlendShader<_CLS>::IDataBindingFactory* Initialize(boo::ID3DDataFactory::Context& ctx,
                                                                boo::IShaderPipeline*& alphaPipeOut,
                                                                boo::IShaderPipeline*& additivePipeOut,
                                                                boo::IShaderPipeline*& colorMultiplyPipeOut,
                                                                boo::IVertexFormat*& vtxFmtOut);
#endif
#if BOO_HAS_METAL
static TMultiBlendShader<_CLS>::IDataBindingFactory* Initialize(boo::MetalDataFactory::Context& ctx,
                                                                boo::IShaderPipeline*& alphaPipeOut,
                                                                boo::IShaderPipeline*& additivePipeOut,
                                                                boo::IShaderPipeline*& colorMultiplyPipeOut,
                                                                boo::IVertexFormat*& vtxFmtOut);
#endif
#if BOO_HAS_VULKAN
static TMultiBlendShader<_CLS>::IDataBindingFactory* Initialize(boo::VulkanDataFactory::Context& ctx,
                                                                boo::IShaderPipeline*& alphaPipeOut,
                                                                boo::IShaderPipeline*& additivePipeOut,
                                                                boo::IShaderPipeline*& colorMultiplyPipeOut,
                                                                boo::IVertexFormat*& vtxFmtOut);
#endif
