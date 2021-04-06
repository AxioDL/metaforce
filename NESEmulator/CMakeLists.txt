file(GLOB MAPPER_SRCS ../extern/fixNES/mapper/*.c)
add_library(NESEmulator CNESEmulator.hpp CNESEmulator.cpp CNESShader.hpp CNESShader.cpp malloc.h
        apu.c ../extern/fixNES/audio_fds.c ../extern/fixNES/audio_mmc5.c ../extern/fixNES/audio_vrc6.c
        ../extern/fixNES/audio_vrc7.c ../extern/fixNES/audio_n163.c ../extern/fixNES/audio_s5b.c
        ../extern/fixNES/cpu.c ppu.c ../extern/fixNES/mem.c ../extern/fixNES/input.c ../extern/fixNES/mapper.c
        ../extern/fixNES/mapperList.c ../extern/fixNES/fm2play.c ../extern/fixNES/vrc_irq.c ${MAPPER_SRCS})
target_include_directories(NESEmulator PRIVATE
        ${CMAKE_SOURCE_DIR}/DataSpec
        ${CMAKE_SOURCE_DIR}/Runtime
        ${CMAKE_SOURCE_DIR}/extern
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(NESEmulator PRIVATE COL_32BIT=1 COL_TEX_BSWAP=1)
target_link_libraries(NESEmulator boo hecl-full RuntimeCommon ${HECL_APPLICATION_REPS_TARGETS_LIST})
if (NOT MSVC)
    target_compile_options(NESEmulator PRIVATE -Wno-implicit-fallthrough -Wno-format -Wno-pointer-compare
            -Wno-memset-elt-size)
endif ()