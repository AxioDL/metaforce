include_directories(${CMAKE_SOURCE_DIR}/DataSpec ${CMAKE_SOURCE_DIR}/Runtime ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB MAPPER_SRCS fixNES/mapper/*.c)
add_library(NESEmulator CNESEmulator.hpp CNESEmulator.cpp CNESShader.hpp CNESShader.cpp malloc.h
            fixNES/apu.c fixNES/audio_fds.c fixNES/audio_mmc5.c fixNES/audio_vrc6.c fixNES/audio_vrc7.c
            fixNES/audio_n163.c fixNES/audio_s5b.c fixNES/cpu.c fixNES/ppu.c fixNES/mem.c fixNES/input.c
            fixNES/mapper.c fixNES/mapperList.c fixNES/fm2play.c fixNES/vrc_irq.c ${MAPPER_SRCS})
