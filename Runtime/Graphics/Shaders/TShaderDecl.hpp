    static TShader<_CLS>::IDataBindingFactory* Initialize(boo::GLDataFactory::Context& ctx,
                                                          boo::IShaderPipeline*& pipeOut);
#if _WIN32
    static TShader<_CLS>::IDataBindingFactory* Initialize(boo::ID3DDataFactory::Context& ctx,
                                                          boo::IShaderPipeline*& pipeOut,
                                                          boo::IVertexFormat*& vtxFmtOut);
#endif
#if BOO_HAS_METAL
    static TShader<_CLS>::IDataBindingFactory* Initialize(boo::MetalDataFactory::Context& ctx,
                                                          boo::IShaderPipeline*& pipeOut,
                                                          boo::IVertexFormat*& vtxFmtOut);
#endif
#if BOO_HAS_VULKAN
    static TShader<_CLS>::IDataBindingFactory* Initialize(boo::VulkanDataFactory::Context& ctx,
                                                          boo::IShaderPipeline*& pipeOut,
                                                          boo::IVertexFormat*& vtxFmtOut);
#endif
