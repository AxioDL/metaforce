add_executable(rude WIN32
               main.cpp)

target_link_libraries(rude
                      RuntimeCommonCharacter
                      RuntimeCommonInput
                      RuntimeCommon
                      DNAMP3 DNAMP2 DNAMP1
                      DNACommon Specter
                      HECLDatabase HECLBlender HECLCommon AthenaCore NOD
                      LogVisor AthenaLibYaml Boo ${PNG_LIB} squish xxhash Math
                      ${ZLIB_LIBRARIES} ${LZO_LIB}
                      ${BOO_SYS_LIBS})
