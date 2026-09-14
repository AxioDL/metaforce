#include "NESemu/ksNes.hpp"
#include "GameVersions.h"
#include "dolphin/PPCArch.h"
#include "dolphin/gx.h"
#include "dolphin/os.h"

#include <string.h>

void ksNesLinecntIrqDefault();
void ksNesLinecntIrqQD();
void ksNesLoad4015();
void ksNesLoad4016();
void ksNesLoad4017();
void ksNesLoadBBRAM();
void ksNesLoadIO();
void ksNesLoadIgnore();
void ksNesLoadPPU();
void ksNesLoadWRAM();
void ksNesStore2000();
void ksNesStore2004();
void ksNesStore2005();
void ksNesStore2006();
void ksNesStore2007ChrRom();
void ksNesStore4000();
void ksNesStore4003();
void ksNesStore4011();
void ksNesStore4014();
void ksNesStore4015();
void ksNesStore4016();
void ksNesStore4017();
void ksNesStoreBBRAM();
void ksNesStoreIO();
void ksNesStoreInvalid();
void ksNesStorePPU();
void ksNesStorePPURam();
void ksNesStoreQD_4020();
void ksNesStoreQD_4022();
void ksNesStoreQD_4023();
void ksNesStoreQD_4024();
void ksNesStoreQD_4025();
void ksNesStoreQD_4026();
void ksNesStoreWRAM();

void ksNesActivateIntrIRQ();
void ksNesInit01();
void ksNesInst_adc_61();
void ksNesInst_and_21();
void ksNesInst_asl_06();
void ksNesInst_asl_0a();
void ksNesInst_bcc_90();
void ksNesInst_bcs_b0();
void ksNesInst_beq_f0();
void ksNesInst_bit_24();
void ksNesInst_bmi_30();
void ksNesInst_bne_d0();
void ksNesInst_bpl_10();
void ksNesInst_brk_00();
void ksNesInst_bvc_50();
void ksNesInst_bvs_70();
void ksNesInst_clc_18();
void ksNesInst_cld_d8();
void ksNesInst_cli_58();
void ksNesInst_clv_b8();
void ksNesInst_cmp_c1();
void ksNesInst_cpx_e0();
void ksNesInst_cpy_c0();
void ksNesInst_dec_c6();
void ksNesInst_dex_ca();
void ksNesInst_dey_88();
void ksNesInst_eor_41();
void ksNesInst_inc_e6();
void ksNesInst_inx_e8();
void ksNesInst_iny_c8();
void ksNesInst_jmp_4c();
void ksNesInst_jmp_6c();
void ksNesInst_jsr_20();
void ksNesInst_lda_a1();
void ksNesInst_ldx_a2();
void ksNesInst_ldx_b6();
void ksNesInst_ldy_a0();
void ksNesInst_load16_imm();
void ksNesInst_load8_abs();
void ksNesInst_load8_absx();
void ksNesInst_load8_absy();
void ksNesInst_load8_dx();
void ksNesInst_load8_dxi();
void ksNesInst_load8_dyi();
void ksNesInst_load8_zerop();
void ksNesInst_lsr_46();
void ksNesInst_lsr_4a();
void ksNesInst_ora_01();
void ksNesInst_pha_48();
void ksNesInst_php_08();
void ksNesInst_pla_68();
void ksNesInst_plp_28();
void ksNesInst_rol_26();
void ksNesInst_rol_2a();
void ksNesInst_ror_66();
void ksNesInst_ror_6a();
void ksNesInst_rti_40();
void ksNesInst_rti_40_2();
void ksNesInst_rts_60();
void ksNesInst_sbc_e1();
void ksNesInst_sec_38();
void ksNesInst_sed_f8();
void ksNesInst_sei_78();
void ksNesInst_sta_81();
void ksNesInst_sta_85();
void ksNesInst_sta_8d();
void ksNesInst_sta_91();
void ksNesInst_sta_95();
void ksNesInst_sta_99();
void ksNesInst_sta_9d();
void ksNesInst_stx_86();
void ksNesInst_stx_8e();
void ksNesInst_stx_96();
void ksNesInst_sty_84();
void ksNesInst_sty_8c();
void ksNesInst_sty_94();
void ksNesInst_tax_aa();
void ksNesInst_tay_a8();
void ksNesInst_tsx_ba();
void ksNesInst_txa_8a();
void ksNesInst_txs_9a();
void ksNesInst_tya_98();
void ksNesInst_wdm_42();
void ksNesStore01_8000();

#include "NESemu/MetroidRom.inc"

static ksNesAsmHandler ksNesStoreFuncTblDefault[] = {
    ksNesStoreWRAM,         ksNesStorePPU,          ksNesStoreIO,           ksNesLinecntIrqDefault,
    ksNesLinecntIrqDefault, ksNesLinecntIrqDefault, ksNesLinecntIrqDefault, ksNesLinecntIrqDefault,
};

static ksNesAsmHandler ksNesStorePPUFuncTblDefault[] = {
    ksNesStore2000, ksNesStorePPURam, ksNesLinecntIrqDefault, ksNesStorePPURam,
    ksNesStore2004, ksNesStore2005,   ksNesStore2006,         ksNesStore2007ChrRom,
};

static ksNesAsmHandler ksNesStoreIOFuncTblDefault[] = {
    ksNesStore4000,    ksNesStore4000,         ksNesStore4000,    ksNesStore4003,
    ksNesStore4000,    ksNesStore4000,         ksNesStore4000,    ksNesStore4003,
    ksNesStore4000,    ksNesStore4000,         ksNesStore4000,    ksNesStore4003,
    ksNesStore4000,    ksNesStore4000,         ksNesStore4000,    ksNesStore4003,
    ksNesStore4000,    ksNesStore4011,         ksNesStore4000,    ksNesStore4000,
    ksNesStore4014,    ksNesStore4015,         ksNesStore4016,    ksNesStore4017,
    ksNesStoreInvalid, ksNesStoreInvalid,      ksNesStoreInvalid, ksNesStoreInvalid,
    ksNesStoreInvalid, ksNesStoreInvalid,      ksNesStoreInvalid, ksNesStoreInvalid,
    ksNesStoreInvalid, ksNesStoreInvalid,      ksNesStoreInvalid, ksNesLinecntIrqDefault,
    ksNesStoreInvalid, ksNesLinecntIrqDefault, ksNesStoreInvalid, ksNesStoreInvalid,
};

static ksNesAsmHandler ksNesStoreQDFuncTbl[] = {
    ksNesStoreQD_4020, ksNesStoreQD_4020, ksNesStoreQD_4022, ksNesStoreQD_4023,
    ksNesStoreQD_4024, ksNesStoreQD_4025, ksNesStoreQD_4026, ksNesStoreInvalid,
};

static u8 ksNesInitQDDataTbl[11] = {
    0x00, 0x2f, 0x00, 0x00, 0x00, 0x06, 0x10, 0xc0, 0x80, 0x35, 0xac,
};

static ksNesAsmHandler ksNesStore05FuncTbl[49] = {
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
};

static ksNesAsmHandler ksNesLoadFuncTblDefault[] = {
    ksNesLoadWRAM,  ksNesLoadPPU,   ksNesLoadIO,    ksNesLoadIgnore,
    ksNesLoadBBRAM, ksNesLoadBBRAM, ksNesLoadBBRAM, ksNesLoadBBRAM,
};

static ksNesAsmHandler ksNesLoadIOFuncTblDefault[] = {
    ksNesLoadIgnore, ksNesLoadIgnore, ksNesLoadIgnore, ksNesLoadIgnore, ksNesLoadIgnore,
    ksNesLoadIgnore, ksNesLoadIgnore, ksNesLoadIgnore, ksNesLoadIgnore, ksNesLoadIgnore,
    ksNesLoadIgnore, ksNesLoadIgnore, ksNesLoadIgnore, ksNesLoadIgnore, ksNesLoadIgnore,
    ksNesLoadIgnore, ksNesLoadIgnore, ksNesLoadIgnore, ksNesLoadIgnore, ksNesLoadIgnore,
    ksNesLoadIgnore, ksNesLoad4015,   ksNesLoad4016,   ksNesLoad4017,
};

u8 ksNesVoiceIdTable_12[] = {
    0x04, 0x05, 0x06, 0x07, 0x01, 0x02, 0x03, 0x08, 0x0a, 0x0b, 0x09, 0x0c, 0x0e, 0x0d, 0x0f, 0x0f,
};

ksNesAsmHandler ksNesMapperInitFuncTbl[2][5] = {
    {ksNesInit01, ksNesStore01_8000, ksNesStore01_8000, ksNesStore01_8000, ksNesStore01_8000},
    {ksNesInit01, ksNesStore01_8000, ksNesStore01_8000, ksNesStore01_8000, ksNesStore01_8000},
};

struct ksNesInstr {
  ksNesAsmHandler instructionEvaluationFunction;
  u8 cycles;
  u8 byteSize;
  ksNesAsmHandler followUpFunction;
  u8 padding[4];
};
CHECK_SIZEOF(ksNesInstr, 0x10)

static ksNesInstr ksNesInstJumpTbl[0x100] = {
    {ksNesInst_brk_00, 7, 2, nullptr},                // 0x00
    {ksNesInst_load8_dxi, 6, 2, ksNesInst_ora_01},    // 0x01
    {ksNesLinecntIrqDefault, 8, 1, nullptr},          // 0x02
    {ksNesLinecntIrqDefault, 4, 1, nullptr},          // 0x03
    {ksNesLinecntIrqDefault, 5, 1, nullptr},          // 0x04
    {ksNesInst_load8_zerop, 3, 2, ksNesInst_ora_01},  // 0x05
    {ksNesInst_load8_zerop, 5, 2, ksNesInst_asl_06},  // 0x06
    {ksNesLinecntIrqDefault, 6, 1, nullptr},          // 0x07
    {ksNesInst_php_08, 3, 1, ksNesLinecntIrqDefault}, // 0x08
    {ksNesInst_ora_01, 2, 2, nullptr},                // 0x09
    {ksNesInst_asl_0a, 2, 1, nullptr},                // 0x0A
    {ksNesLinecntIrqDefault, 4, 1, nullptr},          // 0x0B
    {ksNesLinecntIrqDefault, 6, 1, nullptr},          // 0x0C
    {ksNesInst_load8_abs, 4, 3, ksNesInst_ora_01},    // 0x0D
    {ksNesInst_load8_abs, 6, 3, ksNesInst_asl_06},    // 0x0E
    {ksNesLinecntIrqDefault, 5, 1, nullptr},          // 0x0F
    {ksNesInst_bpl_10, 3, 2, nullptr},                // 0x10
    {ksNesInst_load8_dyi, 5, 2, ksNesInst_ora_01},    // 0x11
    {ksNesLinecntIrqDefault, 5, 1, nullptr},          // 0x12
    {ksNesLinecntIrqDefault, 7, 1, nullptr},          // 0x13
    {ksNesLinecntIrqDefault, 5, 1, nullptr},          // 0x14
    {ksNesInst_load8_dx, 4, 2, ksNesInst_ora_01},     // 0x15
    {ksNesInst_load8_dx, 6, 2, ksNesInst_asl_06},     // 0x16
    {ksNesLinecntIrqDefault, 6, 1, nullptr},          // 0x17
    {ksNesInst_clc_18, 2, 1, nullptr},                // 0x18
    {ksNesInst_load8_absy, 4, 3, ksNesInst_ora_01},   // 0x19
    {ksNesLinecntIrqDefault, 2, 1, nullptr},          // 0x1A
    {ksNesLinecntIrqDefault, 2, 1, nullptr},          // 0x1B
    {ksNesLinecntIrqDefault, 6, 1, nullptr},          // 0x1C
    {ksNesInst_load8_absx, 4, 3, ksNesInst_ora_01},   // 0x1D
    {ksNesInst_load8_absx, 7, 3, ksNesInst_asl_06},   // 0x1E
    {ksNesLinecntIrqDefault, 5, 1, nullptr},          // 0x1F
    {ksNesInst_load16_imm, 6, 3, ksNesInst_jsr_20},   // 0x20
    {ksNesInst_load8_dxi, 6, 2, ksNesInst_and_21},    // 0x21
    {ksNesLinecntIrqDefault, 8, 1, nullptr},          // 0x22
    {ksNesLinecntIrqDefault, 4, 1, nullptr},          // 0x23
    {ksNesInst_load8_zerop, 3, 2, ksNesInst_bit_24},  // 0x24
    {ksNesInst_load8_zerop, 3, 2, ksNesInst_and_21},  // 0x25
    {ksNesInst_load8_zerop, 5, 2, ksNesInst_rol_26},  // 0x26
    {ksNesLinecntIrqDefault, 6, 1, nullptr},          // 0x27
    {ksNesInst_plp_28, 4, 1, ksNesActivateIntrIRQ},   // 0x28
    {ksNesInst_and_21, 2, 2, nullptr},                // 0x29
    {ksNesInst_rol_2a, 2, 1, nullptr},                // 0x2A
    {ksNesLinecntIrqDefault, 5, 1, nullptr},          // 0x2B
    {ksNesInst_load8_abs, 4, 3, ksNesInst_bit_24},    // 0x2C
    {ksNesInst_load8_abs, 4, 3, ksNesInst_and_21},    // 0x2D
    {ksNesInst_load8_abs, 6, 3, ksNesInst_rol_26},    // 0x2E
    {ksNesLinecntIrqDefault, 5, 1, nullptr},          // 0x2F
    {ksNesInst_bmi_30, 3, 2, nullptr},                // 0x30
    {ksNesInst_load8_dyi, 5, 2, ksNesInst_and_21},    // 0x31
    {ksNesLinecntIrqDefault, 5, 1, nullptr},          // 0x32
    {ksNesLinecntIrqDefault, 7, 1, nullptr},          // 0x33
    {ksNesLinecntIrqDefault, 4, 1, nullptr},          // 0x34
    {ksNesInst_load8_dx, 4, 2, ksNesInst_and_21},     // 0x35
    {ksNesInst_load8_dx, 6, 2, ksNesInst_rol_26},     // 0x36
    {ksNesLinecntIrqDefault, 6, 1, nullptr},          // 0x37
    {ksNesInst_sec_38, 2, 1, nullptr},                // 0x38
    {ksNesInst_load8_absy, 4, 3, ksNesInst_and_21},   // 0x39
    {ksNesLinecntIrqDefault, 2, 1, nullptr},          // 0x3A
    {ksNesLinecntIrqDefault, 2, 1, nullptr},          // 0x3B
    {ksNesLinecntIrqDefault, 4, 1, nullptr},          // 0x3C
    {ksNesInst_load8_absx, 4, 3, ksNesInst_and_21},   // 0x3D
    {ksNesInst_load8_absx, 7, 3, ksNesInst_rol_26},   // 0x3E
    {ksNesLinecntIrqDefault, 5, 1, nullptr},          // 0x3F
    {ksNesInst_rti_40, 6, 1, ksNesInst_rti_40_2},     // 0x40
    {ksNesInst_load8_dxi, 6, 2, ksNesInst_eor_41},    // 0x41
    {ksNesInst_wdm_42, 2, 2, nullptr},                // 0x42
    {ksNesLinecntIrqDefault, 4, 1, nullptr},          // 0x43
    {ksNesLinecntIrqDefault, 7, 1, nullptr},          // 0x44
    {ksNesInst_load8_zerop, 3, 2, ksNesInst_eor_41},  // 0x45
    {ksNesInst_load8_zerop, 5, 2, ksNesInst_lsr_46},  // 0x46
    {ksNesLinecntIrqDefault, 6, 1, nullptr},          // 0x47
    {ksNesInst_pha_48, 3, 1, nullptr},                // 0x48
    {ksNesInst_eor_41, 2, 2, nullptr},                // 0x49
    {ksNesInst_lsr_4a, 2, 1, nullptr},                // 0x4A
    {ksNesLinecntIrqDefault, 3, 1, nullptr},          // 0x4B
    {ksNesInst_load16_imm, 3, 3, ksNesInst_jmp_4c},   // 0x4C
    {ksNesInst_load8_abs, 4, 3, ksNesInst_eor_41},    // 0x4D
    {ksNesInst_load8_abs, 6, 3, ksNesInst_lsr_46},    // 0x4E
    {ksNesLinecntIrqDefault, 5, 1, nullptr},          // 0x4F
    {ksNesInst_bvc_50, 3, 2, nullptr},                // 0x50
    {ksNesInst_load8_dyi, 5, 2, ksNesInst_eor_41},    // 0x51
    {ksNesLinecntIrqDefault, 5, 1, nullptr},          // 0x52
    {ksNesLinecntIrqDefault, 7, 1, nullptr},          // 0x53
    {ksNesLinecntIrqDefault, 7, 1, nullptr},          // 0x54
    {ksNesInst_load8_dx, 4, 2, ksNesInst_eor_41},     // 0x55
    {ksNesInst_load8_dx, 6, 2, ksNesInst_lsr_46},     // 0x56
    {ksNesLinecntIrqDefault, 6, 1, nullptr},          // 0x57
    {ksNesInst_cli_58, 2, 1, nullptr},                // 0x58
    {ksNesInst_load8_absy, 4, 3, ksNesInst_eor_41},   // 0x59
    {ksNesLinecntIrqDefault, 3, 1, nullptr},          // 0x5A
    {ksNesLinecntIrqDefault, 2, 1, nullptr},          // 0x5B
    {ksNesLinecntIrqDefault, 4, 1, nullptr},          // 0x5C
    {ksNesInst_load8_absx, 4, 3, ksNesInst_eor_41},   // 0x5D
    {ksNesInst_load8_absx, 7, 3, ksNesInst_lsr_46},   // 0x5E
    {ksNesLinecntIrqDefault, 5, 1, nullptr},          // 0x5F
    {ksNesInst_rts_60, 6, 1, nullptr},                // 0x60
    {ksNesInst_load8_dxi, 6, 2, ksNesInst_adc_61},    // 0x61
    {ksNesLinecntIrqDefault, 6, 1, nullptr},          // 0x62
    {ksNesLinecntIrqDefault, 4, 1, nullptr},          // 0x63
    {ksNesLinecntIrqDefault, 3, 1, nullptr},          // 0x64
    {ksNesInst_load8_zerop, 3, 2, ksNesInst_adc_61},  // 0x65
    {ksNesInst_load8_zerop, 5, 2, ksNesInst_ror_66},  // 0x66
    {ksNesLinecntIrqDefault, 6, 1, nullptr},          // 0x67
    {ksNesInst_pla_68, 4, 1, nullptr},                // 0x68
    {ksNesInst_adc_61, 2, 2, nullptr},                // 0x69
    {ksNesInst_ror_6a, 2, 1, nullptr},                // 0x6A
    {ksNesLinecntIrqDefault, 6, 1, nullptr},          // 0x6B
    {ksNesInst_load16_imm, 5, 3, ksNesInst_jmp_6c},   // 0x6C
    {ksNesInst_load8_abs, 4, 3, ksNesInst_adc_61},    // 0x6D
    {ksNesInst_load8_abs, 6, 3, ksNesInst_ror_66},    // 0x6E
    {ksNesLinecntIrqDefault, 5, 1, nullptr},          // 0x6F
    {ksNesInst_bvs_70, 3, 2, nullptr},                // 0x70
    {ksNesInst_load8_dyi, 5, 2, ksNesInst_adc_61},    // 0x71
    {ksNesLinecntIrqDefault, 5, 1, nullptr},          // 0x72
    {ksNesLinecntIrqDefault, 7, 1, nullptr},          // 0x73
    {ksNesLinecntIrqDefault, 4, 1, nullptr},          // 0x74
    {ksNesInst_load8_dx, 4, 2, ksNesInst_adc_61},     // 0x75
    {ksNesInst_load8_dx, 6, 2, ksNesInst_ror_66},     // 0x76
    {ksNesLinecntIrqDefault, 6, 1, nullptr},          // 0x77
    {ksNesInst_sei_78, 2, 1, nullptr},                // 0x78
    {ksNesInst_load8_absy, 4, 3, ksNesInst_adc_61},   // 0x79
    {ksNesLinecntIrqDefault, 4, 1, nullptr},          // 0x7A
    {ksNesLinecntIrqDefault, 2, 1, nullptr},          // 0x7B
    {ksNesLinecntIrqDefault, 6, 1, nullptr},          // 0x7C
    {ksNesInst_load8_absx, 4, 3, ksNesInst_adc_61},   // 0x7D
    {ksNesInst_load8_absx, 7, 3, ksNesInst_ror_66},   // 0x7E
    {ksNesLinecntIrqDefault, 5, 1, nullptr},          // 0x7F
    {ksNesLinecntIrqDefault, 2, 1, nullptr},          // 0x80
    {ksNesInst_load8_dx, 6, 2, ksNesInst_sta_81},     // 0x81
    {ksNesLinecntIrqDefault, 4, 1, nullptr},          // 0x82
    {ksNesLinecntIrqDefault, 4, 1, nullptr},          // 0x83
    {ksNesInst_sty_84, 3, 2, nullptr},                // 0x84
    {ksNesInst_sta_85, 3, 2, nullptr},                // 0x85
    {ksNesInst_stx_86, 3, 2, nullptr},                // 0x86
    {ksNesLinecntIrqDefault, 6, 1, nullptr},          // 0x87
    {ksNesInst_dey_88, 2, 1, nullptr},                // 0x88
    {ksNesLinecntIrqDefault, 2, 1, nullptr},          // 0x89
    {ksNesInst_txa_8a, 2, 1, nullptr},                // 0x8A
    {ksNesLinecntIrqDefault, 3, 1, nullptr},          // 0x8B
    {ksNesInst_load16_imm, 4, 3, ksNesInst_sty_8c},   // 0x8C
    {ksNesInst_load16_imm, 4, 3, ksNesInst_sta_8d},   // 0x8D
    {ksNesInst_load16_imm, 4, 3, ksNesInst_stx_8e},   // 0x8E
    {ksNesLinecntIrqDefault, 5, 1, nullptr},          // 0x8F
    {ksNesInst_bcc_90, 3, 2, nullptr},                // 0x90
    {ksNesInst_sta_91, 6, 2, nullptr},                // 0x91
    {ksNesLinecntIrqDefault, 5, 1, nullptr},          // 0x92
    {ksNesLinecntIrqDefault, 7, 1, nullptr},          // 0x93
    {ksNesInst_sty_94, 4, 2, nullptr},                // 0x94
    {ksNesInst_sta_95, 4, 2, nullptr},                // 0x95
    {ksNesInst_stx_96, 4, 2, nullptr},                // 0x96
    {ksNesLinecntIrqDefault, 6, 1, nullptr},          // 0x97
    {ksNesInst_tya_98, 2, 1, nullptr},                // 0x98
    {ksNesInst_load16_imm, 5, 3, ksNesInst_sta_99},   // 0x99
    {ksNesInst_txs_9a, 2, 1, nullptr},                // 0x9A
    {ksNesLinecntIrqDefault, 2, 1, nullptr},          // 0x9B
    {ksNesLinecntIrqDefault, 4, 1, nullptr},          // 0x9C
    {ksNesInst_load16_imm, 5, 3, ksNesInst_sta_9d},   // 0x9D
    {ksNesLinecntIrqDefault, 5, 1, nullptr},          // 0x9E
    {ksNesLinecntIrqDefault, 5, 1, nullptr},          // 0x9F
    {ksNesInst_ldy_a0, 2, 2, nullptr},                // 0xA0
    {ksNesInst_load8_dxi, 6, 2, ksNesInst_lda_a1},    // 0xA1
    {ksNesInst_ldx_a2, 2, 2, nullptr},                // 0xA2
    {ksNesLinecntIrqDefault, 4, 1, nullptr},          // 0xA3
    {ksNesInst_load8_zerop, 3, 2, ksNesInst_ldy_a0},  // 0xA4
    {ksNesInst_load8_zerop, 3, 2, ksNesInst_lda_a1},  // 0xA5
    {ksNesInst_load8_zerop, 3, 2, ksNesInst_ldx_a2},  // 0xA6
    {ksNesLinecntIrqDefault, 6, 1, nullptr},          // 0xA7
    {ksNesInst_tay_a8, 2, 1, nullptr},                // 0xA8
    {ksNesInst_lda_a1, 2, 2, nullptr},                // 0xA9
    {ksNesInst_tax_aa, 2, 1, nullptr},                // 0xAA
    {ksNesLinecntIrqDefault, 4, 1, nullptr},          // 0xAB
    {ksNesInst_load8_abs, 4, 3, ksNesInst_ldy_a0},    // 0xAC
    {ksNesInst_load8_abs, 4, 3, ksNesInst_lda_a1},    // 0xAD
    {ksNesInst_load8_abs, 4, 3, ksNesInst_ldx_a2},    // 0xAE
    {ksNesLinecntIrqDefault, 5, 1, nullptr},          // 0xAF
    {ksNesInst_bcs_b0, 3, 2, nullptr},                // 0xB0
    {ksNesInst_load8_dyi, 5, 2, ksNesInst_lda_a1},    // 0xB1
    {ksNesLinecntIrqDefault, 5, 1, nullptr},          // 0xB2
    {ksNesLinecntIrqDefault, 7, 1, nullptr},          // 0xB3
    {ksNesInst_load8_dx, 4, 2, ksNesInst_ldy_a0},     // 0xB4
    {ksNesInst_load8_dx, 4, 2, ksNesInst_lda_a1},     // 0xB5
    {ksNesInst_ldx_b6, 4, 2, nullptr},                // 0xB6
    {ksNesLinecntIrqDefault, 6, 1, nullptr},          // 0xB7
    {ksNesInst_clv_b8, 2, 1, nullptr},                // 0xB8
    {ksNesInst_load8_absy, 4, 3, ksNesInst_lda_a1},   // 0xB9
    {ksNesInst_tsx_ba, 2, 1, nullptr},                // 0xBA
    {ksNesLinecntIrqDefault, 2, 1, nullptr},          // 0xBB
    {ksNesInst_load8_absx, 4, 3, ksNesInst_ldy_a0},   // 0xBC
    {ksNesInst_load8_absx, 4, 3, ksNesInst_lda_a1},   // 0xBD
    {ksNesInst_load8_absy, 4, 3, ksNesInst_ldx_a2},   // 0xBE
    {ksNesLinecntIrqDefault, 5, 1, nullptr},          // 0xBF
    {ksNesInst_cpy_c0, 2, 2, nullptr},                // 0xC0
    {ksNesInst_load8_dxi, 6, 2, ksNesInst_cmp_c1},    // 0xC1
    {ksNesLinecntIrqDefault, 3, 1, nullptr},          // 0xC2
    {ksNesLinecntIrqDefault, 4, 1, nullptr},          // 0xC3
    {ksNesInst_load8_zerop, 3, 2, ksNesInst_cpy_c0},  // 0xC4
    {ksNesInst_load8_zerop, 3, 2, ksNesInst_cmp_c1},  // 0xC5
    {ksNesInst_load8_zerop, 5, 2, ksNesInst_dec_c6},  // 0xC6
    {ksNesLinecntIrqDefault, 6, 1, nullptr},          // 0xC7
    {ksNesInst_iny_c8, 2, 1, nullptr},                // 0xC8
    {ksNesInst_cmp_c1, 2, 2, nullptr},                // 0xC9
    {ksNesInst_dex_ca, 2, 1, nullptr},                // 0xCA
    {ksNesLinecntIrqDefault, 3, 1, nullptr},          // 0xCB
    {ksNesInst_load8_abs, 4, 3, ksNesInst_cpy_c0},    // 0xCC
    {ksNesInst_load8_abs, 4, 3, ksNesInst_cmp_c1},    // 0xCD
    {ksNesInst_load8_abs, 6, 3, ksNesInst_dec_c6},    // 0xCE
    {ksNesLinecntIrqDefault, 5, 1, nullptr},          // 0xCF
    {ksNesInst_bne_d0, 3, 2, nullptr},                // 0xD0
    {ksNesInst_load8_dyi, 5, 2, ksNesInst_cmp_c1},    // 0xD1
    {ksNesLinecntIrqDefault, 5, 1, nullptr},          // 0xD2
    {ksNesLinecntIrqDefault, 7, 1, nullptr},          // 0xD3
    {ksNesLinecntIrqDefault, 6, 1, nullptr},          // 0xD4
    {ksNesInst_load8_dx, 4, 2, ksNesInst_cmp_c1},     // 0xD5
    {ksNesInst_load8_dx, 6, 2, ksNesInst_dec_c6},     // 0xD6
    {ksNesLinecntIrqDefault, 6, 1, nullptr},          // 0xD7
    {ksNesInst_cld_d8, 2, 1, nullptr},                // 0xD8
    {ksNesInst_load8_absy, 4, 3, ksNesInst_cmp_c1},   // 0xD9
    {ksNesLinecntIrqDefault, 3, 1, nullptr},          // 0xDA
    {ksNesLinecntIrqDefault, 3, 1, nullptr},          // 0xDB
    {ksNesLinecntIrqDefault, 6, 1, nullptr},          // 0xDC
    {ksNesInst_load8_absx, 4, 3, ksNesInst_cmp_c1},   // 0xDD
    {ksNesInst_load8_absx, 7, 3, ksNesInst_dec_c6},   // 0xDE
    {ksNesLinecntIrqDefault, 5, 1, nullptr},          // 0xDF
    {ksNesInst_cpx_e0, 2, 2, nullptr},                // 0xE0
    {ksNesInst_load8_dxi, 6, 2, ksNesInst_sbc_e1},    // 0xE1
    {ksNesLinecntIrqDefault, 3, 1, nullptr},          // 0xE2
    {ksNesLinecntIrqDefault, 4, 1, nullptr},          // 0xE3
    {ksNesInst_load8_zerop, 3, 2, ksNesInst_cpx_e0},  // 0xE4
    {ksNesInst_load8_zerop, 3, 2, ksNesInst_sbc_e1},  // 0xE5
    {ksNesInst_load8_zerop, 5, 2, ksNesInst_inc_e6},  // 0xE6
    {ksNesLinecntIrqDefault, 6, 1, nullptr},          // 0xE7
    {ksNesInst_inx_e8, 2, 1, nullptr},                // 0xE8
    {ksNesInst_sbc_e1, 2, 2, nullptr},                // 0xE9
    {ksNesLinecntIrqDefault, 2, 1, nullptr},          // 0xEA
    {ksNesLinecntIrqDefault, 3, 1, nullptr},          // 0xEB
    {ksNesInst_load8_abs, 4, 3, ksNesInst_cpx_e0},    // 0xEC
    {ksNesInst_load8_abs, 4, 3, ksNesInst_sbc_e1},    // 0xED
    {ksNesInst_load8_abs, 6, 3, ksNesInst_inc_e6},    // 0xEE
    {ksNesLinecntIrqDefault, 5, 1, nullptr},          // 0xEF
    {ksNesInst_beq_f0, 3, 2, nullptr},                // 0xF0
    {ksNesInst_load8_dyi, 5, 2, ksNesInst_sbc_e1},    // 0xF1
    {ksNesLinecntIrqDefault, 5, 1, nullptr},          // 0xF2
    {ksNesLinecntIrqDefault, 7, 1, nullptr},          // 0xF3
    {ksNesLinecntIrqDefault, 5, 1, nullptr},          // 0xF4
    {ksNesInst_load8_dx, 4, 2, ksNesInst_sbc_e1},     // 0xF5
    {ksNesInst_load8_dx, 6, 2, ksNesInst_inc_e6},     // 0xF6
    {ksNesLinecntIrqDefault, 6, 1, nullptr},          // 0xF7
    {ksNesInst_sed_f8, 2, 1, nullptr},                // 0xF8
    {ksNesInst_load8_absy, 4, 3, ksNesInst_sbc_e1},   // 0xF9
    {ksNesLinecntIrqDefault, 4, 1, nullptr},          // 0xFA
    {ksNesLinecntIrqDefault, 2, 1, nullptr},          // 0xFB
    {ksNesLinecntIrqDefault, 6, 1, nullptr},          // 0xFC
    {ksNesInst_load8_absx, 4, 3, ksNesInst_sbc_e1},   // 0xFD
    {ksNesInst_load8_absx, 7, 3, ksNesInst_inc_e6},   // 0xFE
    {ksNesLinecntIrqDefault, 5, 1, nullptr},          // 0xFF
};

extern u8 ksNesPaletteNormal[0x80];

struct ksNesRomBitStream {
  u32 bitsLeft;
  u32 byte;
  u8* data;
};

static ksNesRomBitStream sRomBitStream;

static inline u32 ksNesReadRomBit(ksNesRomBitStream* stream, u32 value) {
  if (stream->bitsLeft == 0) {
    stream->byte = *stream->data++;
    stream->bitsLeft = 8;
  }
  value <<= 1;
  if (stream->byte & 0x80) {
    value |= 1;
  }
  stream->byte <<= 1;
  --stream->bitsLeft;
  return value;
}

static inline u32 ksNesReadRomBits(ksNesRomBitStream* stream, u32 count) {
  u32 i = 0;
  u32 value = 0;
  for (; i < count; ++i) {
    value = ksNesReadRomBit(stream, value);
  }
  return value;
}

void ksNesDecodeRom(u8* data, u8* output, u32 size, u32 key, u32 checksumSize, u32 polynomial) {
  ksNesRomBitStream* stream = &sRomBitStream;
  u8* dest = output;
  u32 i;
  for (i = 0; i < 256; ++i) {
    data[i] += key;
    key = data[i];
  }
  for (i = 0; i < 128; ++i) {
    u8 tmp = data[i];
    data[i] = data[255 - i];
    data[255 - i] = tmp;
  }

  stream->data = data + 256;
  stream->bitsLeft = 0;
  do {
    if (ksNesReadRomBits(stream, 1) == 1) {
      *output++ = data[ksNesReadRomBits(stream, 8) + 73];
    } else if (ksNesReadRomBits(stream, 1) == 1) {
      *output++ = data[ksNesReadRomBits(stream, 6) + 9];
    } else if (ksNesReadRomBits(stream, 1) == 1) {
      *output++ = data[ksNesReadRomBits(stream, 3) + 1];
    } else {
      *output++ = data[0];
    }
  } while (--size);

  u32 checksum = 0;
  for (i = 0; i < checksumSize; ++i) {
    checksum ^= dest[i];
    for (u32 j = 0; j < 8; ++j) {
      if (checksum & 1) {
        checksum >>= 1;
        checksum ^= polynomial;
      } else {
        checksum >>= 1;
      }
    }
  }
  dest[checksumSize - 1] = (checksum >> 8) & 0xff;
  dest[checksumSize - 2] = checksum & 0xff;
}

static inline void ksNesDrawMakeOBJIndTex(ksNesCommonWorkObj* wp) {
  static const u8 array[] = {
      0x00, 0x02, 0x04, 0x06, 0x07, 0x05, 0x03, 0x01,
  };
  u32 i;
  u32 j;

  for (i = 0; i < 16; i++) {
#define TO_IND1(x) (((x) / 4) * 32) + (((x) % 4) * 8)
#define TO_IND2(x) (((x) / 4) * 32) + (((x) % 4) * 2)
    for (j = 0; j < 4; j++) {
      wp->draw_ctx.sprite_indirect_lut[TO_IND1(i) + TO_IND2(j)] = (i >> 3) & 1;
      wp->draw_ctx.sprite_indirect_lut[TO_IND1(i) + TO_IND2(j) + 1] = array[i & 7] << 2;
    }
  }
#undef TO_IND1
#undef TO_IND2
  DCFlushRangeNoSync(wp->draw_ctx.sprite_indirect_lut, sizeof(wp->draw_ctx.sprite_indirect_lut));
}

void ksNesConvertChrToI8(ksNesCommonWorkObj* wp, const u8* data, u32 flags) {
  u32 idx;
  u32 bufSize;
  u32 b;
  u32 a;
  u32 c;
  u32 stride;
  u32 abc;
  u32 i;
  u32 j;
  u32 d;
  u32 mask;

  bufSize = (wp->chr_to_i8_buf_size > CHR_TO_I8_BUF_SIZE ? 0x100 : wp->chr_to_i8_buf_size >> 12);
  stride = bufSize >> 3;
  idx = (flags >> 9) & 0x1f;

  a = (flags & 0x40) << 5;
  b = (flags >> 4) & 0x18;
  c = (flags & 0x3f) << 5;
  abc = c + a + b;

  for (i = 0; i < 8; i++) {
    if (i & 1) {
      d = (data[7 - (i >> 1)]) | (data[0xf - (i >> 1)] << 8);
    } else {
      d = (data[(i >> 1)]) | (data[8 + (i >> 1)] << 8);
    }

    mask = 0x8080;
    for (j = 0; j < 8; j++) {
      wp->chr_to_u8_bufp[abc + (flags & 0x3e00) * 8 + (i * stride) * 0x1000 + j] =
          (((d & mask) & 0xff00) != 0 ? 2 : 0) | (((d & mask) & 0x00FF) != 0 ? 1 : 0);
      mask >>= 1;
    }
    DCStoreRangeNoSync(wp->chr_to_u8_bufp + (((idx) + (i * stride)) * 0x1000 + b + a + c), 0x20);
  }
}

void ksNesConvertChrToI8MMC5(ksNesCommonWorkObj* wp, const u8* data, u32 flags) {}

void ksNesQDSoundSync() {
  for (u32 i = 0; i < 13; ++i) {
    for (u32 j = 0; j < 262; ++j) {
      Sound_Write(0, 0, j * 114);
    }
    OSTick tick = OSGetTick();
    while (OSTicksToMilliseconds(OSGetTick() - tick) < 16) {
    }
  }
}

#define ksNes_READ16LE(buf, ofs) (((buf)[ofs + 1] << 8) | (buf)[ofs])
#define ksNes_READ16LE2(buf, ofs) ((buf)[ofs] | ((buf)[ofs + 1] << 8))
#define ksNes_ADDR2BANK(addr) (((addr) >> 13) & 0x7ffff)
#define QD_DISK_SIZE 0x10000
#define QD_BLOCK_FILEHEADER 3
#define QD_BLOCK_DISKINFO_SZ 0x3a
#define QD_BLOCK_AMOUNT_SZ 4
#define QD_BLOCK_FILEHEADER_SZ 0x12
#define QD_BLOCK_FILEDATA_SZ(n) ((n) + 3)
#define QD_FILE_TYPE_PRAM 0

int ksNesQDFastLoad(ksNesCommonWorkObj* wp, ksNesStateObj* sp) {
  u8* search_p;
  u8* disk_end_p;
  u32 boot_file_no;
  u32 file_count;
  u32 load_ofs;
  u32 size;
  u32 i;

  if ((sp->frame_flags & 0x400) != 0) {
    return -1;
  }

  search_p = sp->nesromp;
  disk_end_p = &search_p[QD_DISK_SIZE];

  // Reject disk images with an unsupported header flag.
  if (search_p[21] != 0) {
    return 0x46E;
  }

  memset(sp->wram + 0x200, 0, 0x600);
  boot_file_no = search_p[25];                     // boot file number
  file_count = search_p[QD_BLOCK_DISKINFO_SZ + 1]; // amount->file_amount
  search_p +=
      QD_BLOCK_DISKINFO_SZ + QD_BLOCK_AMOUNT_SZ; // skip immediately to first file info block

  while (file_count != 0 && search_p < disk_end_p) {
    if (search_p[0] != QD_BLOCK_FILEHEADER || search_p[18] != 4) {
      break;
    }

    size = ksNes_READ16LE2(search_p, 13);
    // load file ID, check it's less than the boot file
    if (search_p[2] <= boot_file_no) {
      load_ofs = ksNes_READ16LE2(search_p, 11);

      // file type
      if (search_p[15] != QD_FILE_TYPE_PRAM) {
        if (load_ofs < KS_NES_CHRRAM_SIZE && size <= KS_NES_CHRRAM_SIZE) {
          memcpy(sp->chrramp + load_ofs, &search_p[QD_BLOCK_FILEHEADER_SZ + 1], size);
        }
      } else if (((load_ofs < KS_NES_WRAM_SIZE || load_ofs > (0x6000 - 1)) &&
                  (load_ofs < 0xE000 &&
                   ((load_ofs + size) <= KS_NES_WRAM_SIZE || (load_ofs + size) > 0x6000))) &&
                 (load_ofs + size) <= 0xE000 && (size <= KS_NES_BBRAM_SIZE && size != 0)) {
        if (load_ofs < KS_NES_WRAM_SIZE) {
          memcpy(sp->wram + load_ofs, &search_p[QD_BLOCK_FILEHEADER_SZ + 1], size);
        } else {
          memcpy(sp->bbramp + (load_ofs - 0x6000), &search_p[QD_BLOCK_FILEHEADER_SZ + 1], size);
        }
      }
    }

    search_p += QD_BLOCK_FILEDATA_SZ(size) + QD_BLOCK_FILEHEADER_SZ;
    file_count--;
  }

  for (i = 0; i < KS_NES_CHRRAM_SIZE; i += 16) {
    ksNesConvertChrToI8(wp, sp->chrramp + i, i / 16);
  }

  return 0;
}

int ksNesQDFastSave(ksNesCommonWorkObj* wp, ksNesStateObj* sp) {
  u32 j;
  u8* qd_disk;
  u32 wram_14;
  u8* disk_end_p;
  u8* search_p;
  u32 save_len;
  u32 read_ofs;
  u32 i;

  if ((sp->frame_flags & 0x400) != 0) {
    return -1;
  }

  if (sp->fds_fast_io_lock != 0) {
    return -1;
  }

  for (i = 0; i < sp->fds_disk_count; i++) {
    qd_disk = &sp->nesromp[i * QD_DISK_SIZE];
    if (memcmp(&qd_disk[15], sp->_176E, 10) == 0) {
      goto search_start;
    }
  }
  return -1;

search_start:
  wram_14 = sp->wram[14];
  disk_end_p = qd_disk + QD_DISK_SIZE;
  search_p = qd_disk + 0x3E;
  while (wram_14 != 0 && search_p < disk_end_p) {
    if (search_p[0] != 3 || search_p[18] != 4) {
      return -1;
    }

    search_p += ksNes_READ16LE2(search_p, 13) + 21; // load file size plus 21?
    wram_14--;
  }

  save_len = ksNes_READ16LE(sp->_176E, 21);
  read_ofs = ksNes_READ16LE(sp->_176E, 24);

  if (search_p + (save_len + 21) >= disk_end_p) {
    return -1;
  }

  search_p[0] = 3;            // write file header block code? (3)
  search_p[1] = sp->wram[14]; // write file number?
  memcpy(search_p + 2, sp->_176E + 10, 14);

  // write file data block code (4)
  search_p[18] = 4;
  search_p += 19;

  // write data

  j = 0;
  for (j = 0; j < save_len; j++) {
    u8* read_p = (&sp->cpu_0000_1fff)[ksNes_ADDR2BANK(read_ofs + j)] + j;
    *search_p++ = read_p[read_ofs];
  }

  // increase file num?
  qd_disk[0x3B] = sp->wram[14] + 1;
  sp->qd_irq_acknowledged_flag = 1;
  return 0;
}

void ksNesPushResetButton(ksNesStateObj* sp) {
  sp->cpu_state.A = 0;
  sp->cpu_state.X = 0;
  sp->cpu_state.Y = 0;
  sp->cpu_state.S = 0xff;
  sp->cpu_state.P = 4;
  sp->cpu_state.irq_pending_flag = 0;
  sp->cpu_state.apu_frame_counter_mode = 0x40;
  sp->cpu_state.irq_status_flags = 0;
  sp->cpu_state.mapper_irq_enable = 0;

  // disable scanline IRQs
  sp->scanline_irq_target_val = 0x7fff;

  sp->sprite0_hit_scanline = 0xff;

  sp->qd_disk_head_position.full = 0;

  // fds control register initialized to 0x27, bit 5 on, transfer mode is read, motor is stopped,
  // and transfer timing state is reset
  sp->fds_control_register = KS_NES_FDS_CTRL_FLG_5 | KS_NES_FDS_CTRL_FLG_TRANSFER_MODE |
                             KS_NES_FDS_CTRL_FLG_MOTOR | KS_NES_FDS_CTRL_FLG_TRANSFER_RESET;
  sp->_16BC = 0x27;

  sp->motor_timer = 0;

  sp->fds_ready_delay_counter = 120;

  // grab the reset vector and assign it to the program counter.
  sp->PC = (sp->cpu_e000_ffff[0xfffd] << 8) | sp->cpu_e000_ffff[0xfffc];

  return;
}

int ksNesReset(ksNesCommonWorkObj* wp, ksNesStateObj* sp, u32 flags, u8* chrramp, u8* bbramp) {
  u8* ram;
  u32 i;
  static u16 sound_init_data[] = {
      0x4015, 0x0000, 0x4008, 0x0000, 0x4080, 0x0080, 0x5015, 0x0000, 0x4010, 0x000f,
      0x4011, 0x0000, 0x4012, 0x0000, 0x4013, 0x0000, 0x4015, 0x0000, 0x5015, 0x0007,
  };
  static const u8 table[] = {0, 0, 0, 0, 4, 4, 4, 4, 0, 4, 0, 4, 0, 0, 4, 4};

  if ((reinterpret_cast< uintptr_t >(wp) & 0x1f) || (reinterpret_cast< uintptr_t >(sp) & 0x1f) ||
      (reinterpret_cast< uintptr_t >(&wp->draw_ctx) & 0x1f) ||
      (reinterpret_cast< uintptr_t >(&wp->draw_ctx.ppu_scanline_regs) & 0x1f) ||
      (reinterpret_cast< uintptr_t >(sp->ppu_chr_banks) & 3) ||
      (reinterpret_cast< uintptr_t >(&wp->draw_ctx.post_process_lut) & 0x1f)) {
    return 0x52d;
  }

  if ((flags & 0x40) == 0) {
    Sound_SetC000(sp->wram);
    Sound_SetE000(sp->wram);
    for (u32 i = 0; i < 262; i++) {
      if (!(i & 7) && i >= 0x40 && i < 0x90) {
        u32 index = ((i - 0x40) >> 3) & 0x3fffffff;
        Sound_Write(static_cast< u16 >(sound_init_data[index]),
                    static_cast< u8 >(sound_init_data[index + 1]), i * 114);
      }
      Sound_Write(0, 0, i * 114);
    }
  }

  if (flags & 1) {
    memcpy(&wp->draw_ctx, sp->wram, KS_NES_WRAM_SIZE);
  }
  memset(sp, 0, sizeof(ksNesStateObj));
  if (flags & 1) {
    memcpy(sp->wram, &wp->draw_ctx, KS_NES_WRAM_SIZE);
  } else {
    for (i = 0; i < KS_NES_WRAM_SIZE; i += 4) {
      ram = &sp->wram[i];
      ram[0] = 0x0f;
      ram[1] = 0xef;
      ram[2] = 0xfe;
      ram[3] = 0x7d;
    }
  }
  memset(&wp->draw_ctx, 0, sizeof(ksNesDrawCtx));

  for (i = 0; i < 24; i++) {
    sp->ppu_chr_bank_pointers[i] = &sp->ppu_nametable_ram[table[i & 15] * 256];
  }
  sp->nesromp = wp->nesromp;
  ksNesDecodeRom(kRomData, sp->nesromp, 0x20000, 0xe9, 0x1fffc, 0xa663);
  wp->noise_bufp = sp->nesromp + 0x1bf00;

  if (sp->nesromp[0] == 1) {
    memcpy(sp->ppu_nametable_pointers, &sp->ppu_chr_bank_pointers[12], 0x10);
    sp->fds_disk_count = 2;
  } else {
    sp->mapper = 1;
    sp->prgromp = sp->nesromp;
    sp->prg_size = 0x20000;
    sp->prg_banks = 15;
    sp->chr_size = 0;
    memcpy(sp->ppu_nametable_pointers, &sp->ppu_chr_bank_pointers[8], 0x10);
  }
  sp->reset_flags = flags;
  if (sp->mapper != KS_NES_MAPPER_MMC5) {
    ksNesDrawMakeOBJIndTex(wp);
  }

  if ((flags & 0x20) == 0) {
    memcpy(sp->palette_normal, ksNesPaletteNormal, 0x80);
  }
  for (i = 0; i < 128; i++) {
    sp->ppu_palette_ram[i] = 0x0f;
  }
  for (i = 0; i < 8; i++) {
    sp->ppu_internal_state[i] = 0x20;
  }
  for (i = 0; i < 240; i++) {
    wp->draw_ctx.ppu_scanline_regs[i].nametable_ptrs[0] = sp->ppu_nametable_pointers[0];
    wp->draw_ctx.ppu_scanline_regs[i].nametable_ptrs[1] = sp->ppu_nametable_pointers[1];
  }
  for (i = 0; i < 8; i++) {
    sp->ppu_chr_banks[i] = i;
  }
  memset(wp->chr_to_u8_bufp, 0, wp->chr_to_i8_buf_size);

  if (chrramp == nullptr) {
    return 0x5f1;
  }
  sp->chr_ram_write_enable = TRUE;
  sp->chrramp = chrramp;
  sp->chr_size = 0x2000;
  if ((flags & 8) && sp->mapper == KS_NES_MAPPER_MMC5) {
    sp->chr_size = 0x20000;
  }
  if ((flags & 2) == 0) {
    memset(sp->chrramp, 0, sp->chr_size);
  }
  sp->chr_banks = (sp->chr_size >> 10) - 1;
  DCFlushRangeNoSync(wp->chr_to_u8_bufp, wp->chr_to_i8_buf_size);

  sp->cpu_cycles_per_visible_scanline = -7614;
  sp->cpu_cycles_per_vblank_scanline = -0x2a00;
  sp->cpu_cycles_for_nmi = -0x2800;
  sp->mapper_irq_handler = ksNesLinecntIrqDefault;
  memcpy(sp->store_func, ksNesStoreFuncTblDefault, sizeof(ksNesStoreFuncTblDefault));
  memcpy(sp->store_ppu_func, ksNesStorePPUFuncTblDefault, sizeof(ksNesStorePPUFuncTblDefault));
  memcpy(sp->store_io_func, ksNesStoreIOFuncTblDefault, sizeof(ksNesStoreIOFuncTblDefault));
  memcpy(sp->load_func, ksNesLoadFuncTblDefault, sizeof(ksNesLoadFuncTblDefault));
  memcpy(sp->load_io_func, ksNesLoadIOFuncTblDefault, sizeof(ksNesLoadIOFuncTblDefault));

  sp->cpu_0000_1fff = sp->wram;
  sp->cpu_2000_3fff = sp->wram - 0x2000;
  sp->cpu_4000_5fff = sp->wram - 0x4000;
  sp->cpu_6000_7fff = sp->wram - 0x6000;
  sp->cpu_8000_9fff = sp->prgromp - 0x8000;
  sp->cpu_a000_bfff = sp->prgromp - 0x8000;
  sp->cpu_c000_dfff = sp->prgromp + sp->prg_size - 0x10000;
  sp->cpu_e000_ffff = sp->prgromp + sp->prg_size - 0x10000;
  sp->bbramp = bbramp;
  if (bbramp) {
    sp->cpu_6000_7fff = bbramp - 0x6000;
    sp->store_func[3] = ksNesStoreBBRAM;
    sp->load_func[3] = ksNesLoadBBRAM;
    sp->bbram_mode_select = flags & 0x10 ? 3 : 0;
  }

  if (*sp->nesromp == 1) {
    memcpy(sp->store_io_func + 0x20, ksNesStoreQDFuncTbl, sizeof(ksNesStoreQDFuncTbl));
    sp->cpu_8000_9fff = sp->bbramp - 0x6000;
    sp->cpu_a000_bfff = sp->bbramp - 0x6000;
    sp->cpu_c000_dfff = sp->bbramp - 0x6000;
    sp->cpu_e000_ffff = wp->noise_bufp - 0xe000;
    sp->store_func[4] = ksNesStoreBBRAM;
    sp->store_func[5] = ksNesStoreBBRAM;
    sp->store_func[6] = ksNesStoreBBRAM;
    sp->mapper_irq_handler = ksNesLinecntIrqQD;
    wp->noise_bufp[0xebd] = 0x42;
    sp->fds_disk_drive_status = 0x47;
    sp->qd_drive.registers.drive_status = 0x47;
    sp->qd_drive.registers.expansion_connector = 0x80;
    wp->noise_bufp[0x651] = 0x32;
    wp->noise_bufp[0x652] = 0xea;
    wp->noise_bufp[0x653] = 0xea;
    wp->noise_bufp[0x654] = 0xea;
    if (memcmp(sp->nesromp + 0x10, "koro", 4) == 0) {
      wp->noise_bufp[0x1a0] = 0xff;
    } else {
      wp->noise_bufp[0x1a0] = 0x7f;
    }
  } else {
    u32 result = ksNesResetAsm(wp, sp);
    if (result) {
      return result;
    }
  }

  ksNesPushResetButton(sp);
  wp->pads[4] = 0;
  wp->pads[5] = 0;
  wp->pads[6] = 0;
  DCStoreRange(wp, sizeof(ksNesCommonWorkObj));
  if ((flags & 0x40) == 0) {
    Sound_SetC000(sp->cpu_c000_dfff + 0xc000);
    Sound_SetE000(sp->cpu_e000_ffff + 0xe000);
    if (sp->nesromp[0] == 1) {
      Sound_SetMMC(2);
    } else {
      Sound_SetMMC(3);
    }
  }
  sp->os_tick = OSGetTick();
  return 0;
}

void ksNesEmuFrame(ksNesCommonWorkObj* wp, ksNesStateObj* sp, u32 flags) {
  sp->frame_flags = flags;
  wp->cpu_cycle_count = 0;
  wp->frames = 1;

  do {
    if ((flags & 0x1000) == 0) {
      u8 status = Sound_Read(0x4015);
      sp->cpu_state.irq_status_flags = (sp->cpu_state.irq_status_flags & 0xc0) | status;
      if (wp->frames != 1) {
        sp->cpu_state.irq_mask_flags = 0xc0;
        sp->cpu_state.irq_status_flags &= 0xc0;
      } else {
        sp->cpu_state.irq_mask_flags = 0xff;
      }
      Sound_SetC000(sp->cpu_c000_dfff + 0xc000);
      Sound_SetE000(sp->cpu_e000_ffff + 0xe000);
    }
    ksNesEmuFrameAsm(wp, sp);
  } while ((flags & 0x400) == 0 && (sp->fds_control_register & KS_NES_FDS_CTRL_FLG_MOTOR) == 0);

  PPCSync();
}

asm u32 ksNesResetAsm(register ksNesCommonWorkObj* work, register ksNesStateObj* state) {
  // clang-format off
    register void* func_tbl;
    register ksNesStateObj* state_temp;
    nofralloc

    stwu r1, -0xf0(r1)
    mflr r0
    stw r0, 0xf4(r1)
    stw state_temp, 0xec(r1)
    stw func_tbl, 0xe8(r1)
    stw work, 0xe0(r1)
    stw state, 0xe4(r1)
    stw work, 0x08(r1)
    stw state, 0x0c(r1)

    mr state_temp, state

    li r3, 0 // return code
    lis func_tbl, ksNesMapperInitFuncTbl@h
    ori func_tbl, func_tbl, ksNesMapperInitFuncTbl@l

    // return an error if the mapper number is greater than the highest supported mapper.
    lbz r7, state_temp->mapper
    cmpwi r7, sizeof(ksNesMapperInitFuncTbl) / sizeof(ksNesMapperInitFuncTbl[0])
    blt init_mapper

mapper_error:
#if VERSION == VERSION_GM8E_48
    li r3, 0x79d
#else
    li r3, 0x788
#endif
    b exit

init_mapper:
    mulli r7, r7, 0x14
    add func_tbl, func_tbl, r7

    // void* store_8000 = ksNesMapperInitFuncTbl[(mapper + 1) * 5];
    // void* store_a000 = ksNesMapperInitFuncTbl[(mapper + 2) * 5];
    // void* store_c000 = ksNesMapperInitFuncTbl[(mapper + 3) * 5];
    // void* store_e000 = ksNesMapperInitFuncTbl[(mapper + 4) * 5];
    lwz r7, 0x04(func_tbl)
    lwz r8, 0x08(func_tbl)
    lwz r9, 0x0c(func_tbl)
    lwz r10, 0x10(func_tbl)

    // state_temp->store_func[4] = store_8000;
    // state_temp->store_func[5] = store_a000;
    // state_temp->store_func[6] = store_c000;
    // state_temp->store_func[7] = store_e000;
    stw r7, state_temp->store_func[4]
    stw r8, state_temp->store_func[5]
    stw r9, state_temp->store_func[6]
    stw r10, state_temp->store_func[7]

    // return an error if the mapper number isn't supported.
    // all supported mappers have a non-null pointer defined for state->store_func[4].
    cmpwi r7, nullptr
    beq mapper_error

    // call the mapper-specific initialization function if one exists.
    // if (ksNesMapperInitFuncTbl[mapper * 5] != nullptr) {
    //     ksNesMapperInitFuncTbl[mapper * 5]();
    // }
    lwz r0, 0x00(func_tbl)
    mtctr r0
    cmpwi r0, nullptr
    bnectrl

    b exit

entry ksNesInit01
    li r8, 0x05
    stb r8, state_temp->mmc1_regs[5] // load_counter

    li r8, 0x0f
    stb r8, state_temp->mmc1_regs[0] // ctrl reg

    lwz r9, state_temp->prg_size
    lis r8, 0x08
    cmpw r9, r8
    bnelr

    li r8, 0x01
    stb r8, state_temp->mmc1_regs[4] // 512k_prg_mode_flag

    li r8, 0x10
    stb r8, state_temp->mmc1_regs[1] // chr_bank0

    blr

exit:
    lwz state_temp, 0xec(r1)
    lwz func_tbl, 0xe8(r1)
    lwz r0, 0xf4(r1)
    mtlr r0
    addi r1, r1, 0xf0
    blr
  // clang-format on
}

extern "C" void _savegpr_14();
extern "C" void _restgpr_14();
void ksNesMainLoop2();
void ksNesPush16_a1();
void ksNesPopPC();
void ksNesActivateIntr();
void ksNesLoadInvalid();
void ksNesStoreQDSound();

#define REGISTER_A r14
#define REGISTER_X r15
#define REGISTER_Y r16
#define REGISTER_PC r17
#define REGISTER_STACK r18
#define REGISTER_FLAG_ZERO r19
#define REGISTER_FLAG_CARRY r20
#define REGISTER_FLAG_OVERFLOW r21
#define REGISTER_FLAG_NEGATIVE r22
#define REGISTER_CYCLE_COUNT r25
// r26 is reused for various purposes throughout the code
#define REGISTER_SCANLINE_STATE r26
#define REGISTER_TEMP_26 r26
#define WRAM r31

// clang-format off
asm void ksNesEmuFrameAsm(register ksNesCommonWorkObj* work_arg, register ksNesStateObj* state_arg) {
    // these might be part of a fralloc? they're stored twice
    #define STACK_WORK_ARG 0x08(r1)
    #define STACK_STATE_ARG 0x0C(r1)

    #define STACK_FRAMES 0xEC(r1)
    #define STACK_CPU_CYCLES 0xF0(r1)
    #define STACK_PPU_SCANLINE_REGS 0xF4(r1)


    register int local_25;
    register int local_26;
    register int local_27;
    register int local_28;
    register int local_29;

    register void* state_cpu_ranges;
    register void* state_store_func;
    register void* state_load_func;
    register void* instr_jump_tbl;      // r30
    register ksNesStateObj* state_temp; // r31

    nofralloc
    stwu r1, -0x140(r1)
    mflr r0
    stw r0, 0x144(r1)
    addi r11, r1, 0x140
    bl _savegpr_14
    //fralloc
    stw r3, 0xe0(r1)
    stw r4, 0xe4(r1)
    stw r3, STACK_WORK_ARG
    stw r4, STACK_STATE_ARG
    mr state_temp, state_arg
    addi r7, work_arg, work_arg->draw_ctx.ppu_scanline_regs
    stw r7, STACK_PPU_SCANLINE_REGS
    lwz r7, (ksNesCommonWorkObj.cpu_cycle_count)(r3)
    stw r7, STACK_CPU_CYCLES
    lbz r7, (ksNesCommonWorkObj.frames)(r3)
    stw r7, STACK_FRAMES

    // r8 = state_temp->frame_flags & 0x4000 ? 2 : 0
    lwz r7, state_temp->frame_flags
    andi. r8, r7, 0x4000
    beq FrameFlagsReady
    li r8, 2
FrameFlagsReady:
    stw r8, 0xe8(r1)

    // state_temp->_1A70++;
    lwz r7, state_temp->frame_counter
    addi r7, r7, 1
    stw r7, state_temp->frame_counter

    lhz REGISTER_PC, state_temp->PC
    lbz REGISTER_A, state_temp->cpu_state.A // A
    lbz REGISTER_X, state_temp->cpu_state.X // X
    lbz REGISTER_Y, state_temp->cpu_state.Y // Y
    lbz REGISTER_STACK, state_temp->cpu_state.S // S
    lbz REGISTER_FLAG_ZERO, state_temp->cpu_state.flag_z // Z
    lbz REGISTER_FLAG_CARRY, state_temp->cpu_state.flag_c // C
    lbz REGISTER_FLAG_OVERFLOW, state_temp->cpu_state.flag_v // V
    lbz REGISTER_FLAG_NEGATIVE, state_temp->cpu_state.flag_n // N

    lis instr_jump_tbl, ksNesInstJumpTbl@h
    ori instr_jump_tbl, instr_jump_tbl, ksNesInstJumpTbl@l

    // r29 = &state_temp->load_func
    addi state_load_func, state_temp, state_temp->load_func

    // r28 = &state_temp->store_func
    addi state_store_func, state_temp, state_temp->store_func

    // r27 = &state_temp->cpu_0000_1fff
    addi state_cpu_ranges, state_temp, state_temp->cpu_0000_1fff

    lwz REGISTER_CYCLE_COUNT, state_temp->cycle_count

// SUB_8003ada4
entry ksNesLinecntIrqDefault
CheckScanlineIrq:
    cmpwi REGISTER_CYCLE_COUNT, 0
    bge ScanlineAdvance

entry ksNesMainLoop2
ksNesMainLoop1:
    // r26 = (REGISTER_PC >> 11) & 0x1C;
    // address range | r26  | description
    // --------------+------+--------------------------
    // 0x0000–0x1FFF | 0x00 | internal RAM and mirrors
    // 0x2000–0x3FFF | 0x04 | PPU registers and mirrors
    // 0x4000-0x5FFF | 0x08 | APU, I/O, and usually unmapped cartridge addresses
    // 0x6000–0x7FFF | 0x0C | usually cartridge RAM
    // 0x8000–0x9FFF | 0x10 | usually cartridge ROM and mapper registers
    // 0xA000–0xBFFF | 0x14 | usually cartridge ROM and mapper registers
    // 0xC000–0xDFFF | 0x18 | usually cartridge ROM and mapper registers
    // 0xE000–0xFFFF | 0x1C | usually cartridge ROM and mapper registers
    rlwinm r26, REGISTER_PC, 21, 27, 29

    // r0 = REGISTER_PC - 0x2000;
    // subtract 0x2000 from the PC.
    // if PC points to the NES's internal 2k of RAM or one if its mirrors,
    // then r0 will be negative.
    // r0 will be positive otherwise.
    //
    // address range | size   | description
    // --------------+--------+----------------------
    // 0x0000–0x07FF | 0x0800 | 2 KB internal RAM
    // 0x0800–0x0FFF | 0x0800 | mirror of $0000–$07FF
    // 0x1000–0x17FF | 0x0800 | mirror of $0000–$07FF
    // 0x1800–0x1FFF | 0x0800 | mirror of $0000–$07FF
    subi r0, REGISTER_PC, 0x2000

    // r7 = state_cpu_ranges[r26];
    // select 1 of 8 pointers to an 8k range of the CPU address space.
    lwzx r7, state_cpu_ranges, r26

    // r0 = (r0 >> 16) & 0xF800;
    // if r0 is negative then r0 = 0x00000000
    // if r0 is positive then r0 = 0x0000F800
    // r0 will be used as a bitmask later.
    rlwinm r0, r0, 16, 16, 20

    // REGISTER_PC = REGISTER_PC & 0xFFFF;
    // restrict PC to a 16-bit value.
    andi. REGISTER_PC, REGISTER_PC, 0xFFFF

    // REGISTER_PC = REGISTER_PC & ~r0;
    // if PC points to RAM or one of its mirrors then ~r0 will be 0xFFFF07FF.
    // ~r0 will be 0xFFFFFFFF otherwise.
    // that redirects RAM mirror access to actual RAM or leaves PC unchanged otherwise.
    andc REGISTER_PC, REGISTER_PC, r0

    // r12 = (u8) r7[REGISTER_PC];
    // grab the byte pointed to by PC.
    // this should be an instruction opcode.
    lbzx r12, r7, REGISTER_PC

    // r4 = REGISTER_PC + 1;
    // get the address of the byte following PC.
    addi r4, REGISTER_PC, 1

    // r26 = (r4 >> 11) & 0x1C;
    // check which CPU range the next byte falls into.
    rlwinm r26, r4, 21, 27, 29

    // r7 = r12 << 4;
    // shift the opcode byte by 4, effectively multiplying by 16.
    // that allows r7 to be used as an offset into instr_jump_tbl
    // since every item in that array is 16 bytes.
    slwi r7, r12, 4

    // r26 = state_cpu_ranges[r26];
    // get a pointer to the range containing the byte following the one pointed to by PC.
    lwzx r26, state_cpu_ranges, r26

    // r8 = instr_jump_tbl[r7]
    // r7 = &instr_jump_tbl[r7]
    // grab a pointer to the first handler function for this instruction.
    lwzux r8, r7, instr_jump_tbl

    // r10 = r7[1]
    // grab size and cycle information about this instruction
    lhz r10, 0x04(r7)

    // r9 = r7[2]
    // grab a pointer to the second handler function for this instruction.
    // may be a null pointer.
    lwz r9, 0x08(r7)

    // lr = r8
    // put the address of the first handler in the link register
    // we'll use this to call the function shortly
    mtlr r8

    andi. r11, r10, 0x00ff // instruction bytes
    andi. r10, r10, 0xff00 // instruction cycles << 8

    // ctr = r9
    // put the address of the second handler in the counter register
    // it may be called later if it isn't null.
    mtctr r9

    // r4 = (u8) r26[r4]
    // get the byte following the one pointed to by PC.
    lbzx r4, r26, r4

    // REGISTER_PC += r11
    // advance PC to the next instruction?
    add REGISTER_PC, REGISTER_PC, r11

    // REGISTER_CYCLE_COUNT += r10
    // increase the total cycle count by the number of cycles this instruction takes?
    add REGISTER_CYCLE_COUNT, REGISTER_CYCLE_COUNT, r10

    // r4  == the byte following the instruction opcode
    // r10 == instruction cycle count shifted left by 8 bits
    // r11 == instruction length in bytes
    // REGISTER_PC == 6502 address of the next instruction
    // REGISTER_CYCLE_COUNT == something to do with cycle count
    // r26 == pointer to the range containing the byte following the instruction opcode
    // ctr == pointer to a second handler function, if any
    // lr()
    blrl

    // while (REGISTER_CYCLE_COUNT < 0)
    cmpwi REGISTER_CYCLE_COUNT, 0
    blt ksNesMainLoop1

ScanlineAdvance:
    lha r3, state_temp->ppu_scanline_counter
    clrlslwi. r7, REGISTER_CYCLE_COUNT, 30, 3 // r7 = (REGISTER_CYCLE_COUNT & 0x3) << 3
    lha r9, state_temp->cpu_cycles_per_vblank_scanline
    rlwimi REGISTER_CYCLE_COUNT, REGISTER_CYCLE_COUNT, 31, 30, 31
    beq LAB_8003b2f0

    cmpwi r3, 0
    add REGISTER_CYCLE_COUNT, REGISTER_CYCLE_COUNT, r9
    blt LAB_8003b4b8

    cmpwi r3, 0xf0
    bge LAB_8003b3b4

    // r7 holds the cycle count % 4 multiplied by 8 to get the index to write the ppu_chr_banks into the scanline state's registers
    // this is done to accomodate different mapper implementations
    lwz r9, STACK_PPU_SCANLINE_REGS
    slwi r8, r3, 5
    lwz r0, state_temp->ppu_chr_banks[0]
    lwz r10, state_temp->ppu_chr_banks[4]
    add  REGISTER_SCANLINE_STATE, r8, r9 // r26 holds a pointer to the current scanline state struct
    stwux r0, r7, REGISTER_SCANLINE_STATE // store ((u32*)ppu_chr_banks)[0] to wp->draw_ctx.ppu_scanline_regs[state_temp->ppu_scanline_counter]._00[cycle_count & 3][0]
    andi. r0, REGISTER_CYCLE_COUNT, 0x1
    stw r10, (ksNesPPUScanlineState.nametable_ptrs[1])(r7) // store ((u32*)ppu_chr_banks)[4] to wp->draw_ctx.ppu_scanline_regs[r3]._00[cycle_count & 3][1]
    bne ksNesMainLoop1

    lhz r0, state_temp->ppu_register_cache[0]
    lbz r8, state_temp->ppu_palette_bank
    lbz r7, state_temp->mapper
    rlwimi r0, r8, 0x9, 0x10, 0x11
    sth r0, (ksNesPPUScanlineState.ppu_ctrl)(REGISTER_SCANLINE_STATE) // update both ppu_ctrl and ppumask_flags (PPUCTRL and PPUMASK regs)
LAB_8003b19c:
    lhz        r7, state_temp->ppu_vram_addr_v_hi
    addi       r0, state_temp, state_temp->ppu_nametable_pointers
    xori       r9, r7, 0x100
    rlwinm     r8, r7, 0x1a, 0x1c, 0x1d
    rlwinm     r9, r9, 0x1a, 0x1c, 0x1d
    lwzx       r8, r8, r0
    lwzx       r9, r9, r0
    stw        r8, (ksNesPPUScanlineState.nametable_ptrs[0])(REGISTER_SCANLINE_STATE)
    stw        r9, (ksNesPPUScanlineState.nametable_ptrs[1])(REGISTER_SCANLINE_STATE)
    rlwinm     r8, r7, 0x1e, 0x1f, 0x1f
    rlwinm     r9, r7, 0x1, 0x0, 0x1e
    xor        r10, r7, r8
    xor        r9, r7, r9
    rlwinm     r10, r10, 0x1, 0x1e, 0x1e
    rlwimi     r8, r9, 0x0, 0x1d, 0x1d
    or         r4, r8, r10
LAB_8003b1dc:
    stb        r7, (ksNesPPUScanlineState.vram_addr_y)(REGISTER_SCANLINE_STATE)
    add        r4, state_temp, r4
    addi       r8, r7, 0x1
    andi.      r0, r7, 0x300
    lbz        r10, (ksNesStateObj.ppu_render_latches)(r4)
    andi.      r8, r8, 0xff
    cmpwi      r8, 0xf0
    lbz        r9, state_temp->ppu_fine_x_scroll
    bne        LAB_8003b208
    xori       r0, r0, 0x200
    li         r8, 0x0
LAB_8003b208:
    sth        r9, (ksNesPPUScanlineState.fine_x_and_next)(REGISTER_SCANLINE_STATE)
    or         r8, r8, r0
    sth        r8, state_temp->ppu_vram_addr_v_hi
    subic.     r10, r10, 0x80
    lbz        r9, (ksNesStateObj.ppu_render_latches + 0x10)(r4)
    beq        LAB_8003b230
    stb        r10, (ksNesStateObj.ppu_render_latches)(r4)
    stb        r3, (ksNesStateObj.ppu_render_latches + 0x08)(r4)
    stb        r3, (ksNesStateObj.ppu_render_latches + 0x10)(r4)
    b          LAB_8003b244
LAB_8003b230:
    subf       r8, r9, r3
    stb        r3, (ksNesStateObj.ppu_render_latches + 0x10)(r4)
    rlwinm     r8, r8, 0x5, 0x0, 0x1a
    subf       r8, r8, REGISTER_SCANLINE_STATE
    stb        r3, (ksNesPPUScanlineState.fine_x_and_next)(r8)
LAB_8003b244:
    lbz        r8, state_temp->primary_oam[0].y_pos
    lbz        r10, state_temp->sprite0_hit_scanline
    cmpw       r3, r8
    bne        LAB_8003b2d8
    lbz        r8, state_temp->ppu_register_cache[0]
    li         r0, 0x0
    lbz        r10, state_temp->primary_oam[0].tile_index
    andi.      r9, r8, 0x20
    beq        LAB_8003b270
    rlwinm     r0, r10, 0x3, 0x1c, 0x1c
    andi.      r10, r10, 0xfe
LAB_8003b270:
    xor        r8, r8, r0
    rlwinm     r0, r10, 0x1a, 0x1e, 0x1f
    rlwinm     r8, r8, 0x1f, 0x1d, 0x1d
    add        r0, r0, state_temp
    lwz        r7, state_temp->chrramp
    add        r8, r8, r0
    lbz        r8, (ksNesStateObj.ppu_chr_banks)(r8)
    rlwinm     r10, r10, 0x4, 0x16, 0x1b
    add        r7, r7, r10
    rlwinm     r8, r8, 0xa, 0x0, 0x15
    add        r8, r8, r7
    li         r10, 0x0
LAB_8003b2a0:
    lbz        r0, 0x0(r8)
    lbz        r7, 0x8(r8)
    addi       r10, r10, 0x1
    or.        r7, r7, r0
    bne        LAB_8003b2d0
    andi.      r7, r10, 0x7
    addi       r8, r8, 0x1
    bne        LAB_8003b2a0
    subic.     r9, r9, 0x20
    addi       r8, r8, 0x8
    bge        LAB_8003b2a0
    li         r10, 0x1
LAB_8003b2d0:
    add        r10, r10, r3
    stb        r10, state_temp->sprite0_hit_scanline
LAB_8003b2d8:
    cmpw       r3, r10
    bne        ksNesMainLoop1
    lbz        r8, state_temp->ppu_internal_state[2]
    ori        r8, r8, 0x40
    stb        r8, state_temp->ppu_internal_state[2]
    b          ksNesMainLoop1
LAB_8003b2f0:
    lwz        r8, state_temp->frame_flags
    lwz        r5, STACK_CPU_CYCLES
    lbz        r7, STACK_FRAMES
    andi.      r8, r8, 0x1000
    bne        LAB_8003b340
    divwu      r5, r5, r7 // r5 = frames / _0014
    li         r3, 0x0
    bl         Sound_Write
    li         r3, KS_NES_REG_APU_STATUS
    bl         Sound_Read
    lbz        r7, state_temp->cpu_state.irq_status_flags
    lbz        r8, state_temp->cpu_state.irq_mask_flags
    or         r7, r7, r3
    and        r7, r7, r8
    stb        r7, state_temp->cpu_state.irq_status_flags
    lha        r3, state_temp->ppu_scanline_counter
LAB_8003b340:
    lha        r9, state_temp->cpu_cycles_per_visible_scanline
    addi       r8, r3, 0x1
    lwz        r7, STACK_CPU_CYCLES
    add        REGISTER_CYCLE_COUNT, REGISTER_CYCLE_COUNT, r9
    lha        r10, state_temp->scanline_irq_target_val
    sth        r8, state_temp->ppu_scanline_counter
    addi       r7, r7, KS_NES_CPU_CYCLES_PER_SCANLINE
    stw        r7, STACK_CPU_CYCLES
    cmpw       r8, r10
    bne        ksNesMainLoop1
    lbz        r7, state_temp->cpu_state.apu_frame_counter_mode
    andi.      r7, r7, 0xc0
    bne        LAB_8003b390
    lbz        r7, state_temp->cpu_state.irq_status_flags
    ori        r7, r7, 0x40
    stb        r7, state_temp->cpu_state.irq_status_flags
    b          SUB_8003b3a0
LAB_8003b390:
    lwz        r8, state_temp->mapper_irq_handler
    mtspr      CTR, r8
    lbz        r7, state_temp->cpu_state.mapper_irq_enable
    bctr
 SUB_8003b3a0:
    lbz        r7, state_temp->cpu_state.P
    andi.      r7, r7, 0x4
    stb        r7, state_temp->cpu_state.irq_pending_flag
    beq        ksNesActivateIntrIRQ // FUN_8003bc54
    b          ksNesMainLoop1
LAB_8003b3b4:
    lbz        r7, state_temp->ppu_internal_state[2]
    li         r8, 0xff
    stb        r8, state_temp->sprite0_hit_scanline
    li         r8, 0x80
    li         r9, -0x16
    rlwimi     r7, r8, 0x0, 0x18, 0x19
    lwz        r8, state_temp->vblank_frame_counter
    stb        r7, state_temp->ppu_internal_state[2]
    sth        r9, state_temp->ppu_scanline_counter
    lwz        r9, STACK_WORK_ARG
    addi       r8, r8, 0x1
    stw        r8, state_temp->vblank_frame_counter
    lwz        r7, (ksNesCommonWorkObj._0044)(r9)
    addi       r7, r7, 0x1
    stw        r7, (ksNesCommonWorkObj._0044)(r9)
    lbz        r7, state_temp->fds_ready_delay_counter
    cmpwi      r7, 0xc3
    bgt        LAB_8003b418
    cmpwi      r7, 0x78
    blt        LAB_8003b418
    lwz        r9, STACK_WORK_ARG
    lwz        r8, (ksNesCommonWorkObj.pads[0])(r9)
    rlwinm.    r8, r8, 0x1, 0x1f, 0x1f
    beq        LAB_8003b420
    li         r7, 0xc3
LAB_8003b418:
    addi       r7, r7, 0x1
    stb        r7, state_temp->fds_ready_delay_counter
LAB_8003b420:
    lbz        r7, state_temp->fds_control_register
    lbz        r8, state_temp->motor_timer
    andi.      r0, r7, KS_NES_FDS_CTRL_FLG_MOTOR
    bne        LAB_8003b434
    li         r8, 90
LAB_8003b434:
    subic.     r8, r8, 0x1
    bge        LAB_8003b440
    li         r8, 0x0
LAB_8003b440:
    stb        r8, state_temp->motor_timer
    lwz        r9, STACK_WORK_ARG
    li         r8, 0x80
    addi       r7, state_temp, state_temp->ppu_palette_ram
    mtspr      CTR, r8
    li         r8, 0x0
    addi       r9, r9, ksNesCommonWorkObj.draw_ctx.post_process_lut
    addi       r3, state_temp, state_temp->palette_normal
LAB_8003b460:
    lbzx       r0, r7, r8
    rlwinm     r10, r8, 0x1, 0x1a, 0x1a
    rlwimi     r10, r8, 0x5, 0x17, 0x18
    rlwimi     r10, r8, 0x4, 0x15, 0x16
    rlwinm     r0, r0, 0x1, 0x0, 0x1e
    lhzx       r0, r3, r0
    rlwimi     r10, r8, 0x1, 0x1d, 0x1e
    addi       r8, r8, 0x1
    sthx       r0, r9, r10
    bdnz       LAB_8003b460
    lbz        r8, state_temp->ppu_palette_bank
    addi       r7, state_temp, state_temp->ppu_palette_ram
    add        r8, r8, r7
    lswi       r3, r8, 0x10
    addi       r8, r8, 0x10
    stswi      r3, r7, 0x10
    addi       r7, r7, 0x10
    lswi       r3, r8, 0x10
    stswi      r3, r7, 0x10
    li         r8, 0x0
    stb        r8, state_temp->ppu_palette_bank
    b          LAB_8003b570
LAB_8003b4b8:
    add        REGISTER_CYCLE_COUNT, REGISTER_CYCLE_COUNT, r9
    rlwinm     REGISTER_CYCLE_COUNT, REGISTER_CYCLE_COUNT, 0x0, 0x0, 0x1d
    cmpwi      r3, -0x15
    bne        LAB_8003b4fc
    li         r7, 0x0
    lbz        r8, state_temp->ppu_register_cache[0]
    lha        r7, state_temp->cpu_cycles_for_nmi
    andi.      r8, r8, 0x80
    add        REGISTER_CYCLE_COUNT, REGISTER_CYCLE_COUNT, r7
    beq        ksNesMainLoop1
    lis        r0, ksNesMainLoop2@h
    ori        r0, r0, ksNesMainLoop2@l // this gets interpreted as a nop by disassemblers
    lis        r7, 0x0
    ori        r7, r7, 0xfffa
    mtspr      CTR, r0
    b          ksNesActivateIntr
LAB_8003b4fc:
    cmpwi      r3, -0x1
    bne        ksNesMainLoop1
    lbz        r7, state_temp->ppu_internal_state[2]
    lhz        r8, state_temp->ppu_vram_addr_t_hi
    andi.      r7, r7, 0x3f
    sth        r8, state_temp->ppu_vram_addr_v_hi
    stb        r7, state_temp->ppu_internal_state[2]
    li         r8, 0x0
    sth        r8, state_temp->ppu_palette_write_addr
    addi       r7, state_temp, state_temp->ppu_render_latches
    li         r8, 0x0
    li         r9, 0x0
    stswi      r8, r7, 0x8
    lbz        r8, state_temp->mapper
    cmpwi      r8, KS_NES_MAPPER_MMC5
    bne        LAB_8003b54c
    li         r7, 0x40
LAB_8003b54c:
    lwz        r9, STACK_WORK_ARG
    li         r0, KS_NES_OAM_TABLE_SIZE
    addi       r7, state_temp, state_temp->primary_oam - 4
    mtctr      r0
    addi       r9, r9, (ksNesCommonWorkObj.draw_ctx.OAMTable) - 4
LAB_8003b560:
    lwzu       r0, 0x4(r7)
    stwu       r0, 0x4(r9)
    bdnz       LAB_8003b560
    b          ksNesMainLoop1
LAB_8003b570:
    sth        REGISTER_PC, state_temp->PC
    stb        REGISTER_A, state_temp->cpu_state.A
    stb        REGISTER_X, state_temp->cpu_state.X
    stb        REGISTER_Y, state_temp->cpu_state.Y
    stb        REGISTER_STACK, state_temp->cpu_state.S
    stb        REGISTER_FLAG_ZERO, state_temp->cpu_state.flag_z
    stb        REGISTER_FLAG_CARRY, state_temp->cpu_state.flag_c
    cmpwi      REGISTER_FLAG_OVERFLOW, 0x0
    beq        LAB_8003b598
    li         REGISTER_FLAG_OVERFLOW, 0x40
LAB_8003b598:
    stb        REGISTER_FLAG_OVERFLOW, state_temp->cpu_state.flag_v
    stb        REGISTER_FLAG_NEGATIVE, state_temp->cpu_state.flag_n
    stw        REGISTER_CYCLE_COUNT, state_temp->cycle_count
    lwz        r3, STACK_WORK_ARG
    lwz        r7, STACK_CPU_CYCLES
    lwz        r8, (ksNesCommonWorkObj.total_cpu_cycles)(r3)
    stw        r7, (ksNesCommonWorkObj.cpu_cycle_count)(r3)
    add        r8, r8, r7
    stw        r8, (ksNesCommonWorkObj.total_cpu_cycles)(r3)

    addi r11, r1, 0x140
    bl _restgpr_14
    lwz r0, 0x144(r1)
    mtlr r0
    addi r1, r1, 0x140
    // frfree
    blr

// load a 16-bit immediate value into r3.
// calls: CTR
// < r4 = the first byte (low byte) of a 16-bit immediate
// < REGISTER_PC = 16-bit 6502 address of the following instruction
// > r3 = 16-bit immediate value following the opcode
// > r26 = ???
entry ksNesInst_load16_imm
    // decrement REGISTER_PC by 1 to get the address of the high byte of the 16-bit immediate
    subi r7, REGISTER_PC, 0x1

    // get the high byte of the 16-bit immediate from ScanlineAdvance in memory
    // r8 = (r7 >> 11) & 0x1C;
    rlwinm r8, r7, 21, 27, 29
    // r9 = state_cpu_ranges[r8]
    lwzx r9, state_cpu_ranges, r8
    // r8 = r9[r7]
    lbzx r8, r9, r7

    // r26 = (r8 >> 3) & 0x1C;
    // why?
    rlwinm r26, r8, 29, 27, 29

    // r8 = r8 << 8
    slwi r8, r8, 8

    // combine the low and high bytes of the 16-bit immediate
    // r4 already had the low byte
    or r3, r4, r8

    // call the second handler for this instruction
    bctr

entry ksNesInst_load8_abs
    subi r7, REGISTER_PC, 0x1
    rlwinm r8, r7, 21, 27, 29
    lwzx r9, state_cpu_ranges, r8
    lbzx r8, r9, r7
    rlwinm r26, r8, 29, 27, 29
    slwi r8, r8, 8

    // get a pointer to a function for loading bytes
    // r9 = state_load_func[r26]
    lwzx r9, state_load_func, r26

    or r3, r4, r8
    mtspr LR, r9
    blr

entry ksNesInst_load8_absx
    subi       r7, REGISTER_PC, 0x1
    add        r4, r4, REGISTER_X
    rlwinm     r8, r7, 0x15, 0x1b, 0x1d
    lwzx       r9, state_cpu_ranges, r8
    lbzx       r8, r9, r7
    rlwinm     r8, r8, 0x8, 0x0, 0x17
    add        r3, r8, r4
    rlwinm     r26, r3, 0x15, 0x1b, 0x1d
    lwzx       r9, state_load_func, r26
    mtlr       r9
    blr

entry ksNesInst_load8_absy
    subi       r7, REGISTER_PC, 0x1
    add        r4, r4, REGISTER_Y
    rlwinm     r8, r7, 0x15, 0x1b, 0x1d
    lwzx       r9, state_cpu_ranges, r8
    lbzx       r8, r9, r7
    rlwinm     r8, r8, 0x8, 0x0, 0x17
    add        r3, r8, r4
    rlwinm     r26, r3, 0x15, 0x1b, 0x1d
    lwzx       r9, state_load_func, r26
    mtlr       r9
    blr

entry ksNesInst_load8_zerop
    or         r3, r4, r4
    lbzx       r4, state_temp, r4
    li         r26, 0x0
    bctr

entry ksNesInst_load8_dx
    add        r3, r4, REGISTER_X
    li         r26, 0x0
    andi.      r3, r3, 0xff
    lbzx       r4, state_temp, r3
    bctr

entry ksNesInst_load8_dxi
    add        r8, r4, REGISTER_X
    andi.      r7, r8, 0xff
    addi       r8, r8, 0x1
    lbzx       r3, state_temp, r7
    andi.      r8, r8, 0xff
    lbzx       r8, state_temp, r8
    rlwinm     r26, r8, 0x1d, 0x1b, 0x1d
    rlwimi     r3, r8, 0x8, 0x10, 0x17
    lwzx       r9, state_load_func, r26
    mtlr       r9
    blr

entry ksNesInst_load8_dyi
    lbzx       r3, state_temp, r4
    addi       r4, r4, 0x1
    andi.      r4, r4, 0xff
    lbzx       r8, state_temp, r4
    add        r3, r3, REGISTER_Y
    rlwinm     r8, r8, 0x8, 0x0, 0x17
    add        r3, r8, r3
    rlwinm     r26, r3, 0x15, 0x1b, 0x1d
    lwzx       r9, state_load_func, r26
    mtlr       r9
    blr

// https://www.masswerk.at/6502/6502_instruction_set.html

//      addressing	assembler	  opc bytes cycles
// LDA (indirect,X)	LDA (oper,X)	A1	2	6
entry ksNesInst_lda_a1
    mr REGISTER_FLAG_ZERO, r4
    andi. REGISTER_FLAG_NEGATIVE, r4, 0x80
    mr REGISTER_A, r4
    b CheckScanlineIrq

// LDX zeropage,Y	LDX oper,Y	B6	2	4
entry ksNesInst_ldx_b6
    add r7, r4, REGISTER_Y
    andi. r7, r7, 0xff
    lbzx r4, WRAM, r7
// fallthrough

// LDX immediate	LDX #oper	A2	2	2
entry ksNesInst_ldx_a2
    mr REGISTER_FLAG_ZERO, r4
    andi. REGISTER_FLAG_NEGATIVE, r4, 0x80
    mr REGISTER_X, r4
    b CheckScanlineIrq

// LDY immediate	LDY #oper	A0	2	2
entry ksNesInst_ldy_a0
    mr REGISTER_FLAG_ZERO, r4
    andi. REGISTER_FLAG_NEGATIVE, r4, 0x80
    mr REGISTER_Y, r4
    b CheckScanlineIrq

// STA zeropage	    STA oper	85	2	3
entry ksNesInst_sta_85
    stbx REGISTER_A, WRAM, r4
    b CheckScanlineIrq

// STX zeropage	    STX oper	86	2	3
entry ksNesInst_stx_86
    stbx REGISTER_X, WRAM, r4
    b CheckScanlineIrq

// STY zeropage	    STY oper	84	2	3
entry ksNesInst_sty_84
    stbx REGISTER_Y, WRAM, r4
    b CheckScanlineIrq

// STA zeropage,X	STA oper,X	95	2	4
entry ksNesInst_sta_95
    add r7, r4, REGISTER_X
    andi. r7, r7, 0xff
    stbx REGISTER_A, WRAM, r7
    b CheckScanlineIrq

// STX zeropage,Y	STX oper,Y	96	2	4
entry ksNesInst_stx_96
    add r7, r4, REGISTER_Y
    andi. r7, r7, 0xff
    stbx REGISTER_X, WRAM, r7
    b CheckScanlineIrq

// STY zeropage,X	STY oper,X	94	2	4
entry ksNesInst_sty_94
    add r7, r4, REGISTER_X
    andi. r7, r7, 0xff
    stbx REGISTER_Y, WRAM, r7
    b CheckScanlineIrq

//STA absolute	    STA oper	8D	3	4
entry ksNesInst_sta_8d
    lwzx r8, state_store_func, r26
    mr r4, REGISTER_A
    mtctr r8
    bctr

//STX absolute	    STX oper	8E	3	4
entry ksNesInst_stx_8e
    lwzx r8, state_store_func, r26
    mr r4, REGISTER_X
    mtctr r8
    bctr

// STY absolute     STY oper	8C	3	4
entry ksNesInst_sty_8c
    lwzx r8, state_store_func, r26
    mr r4, REGISTER_Y
    mtctr r8
    bctr

// STA  absolute,X	STA oper,X	9D	3	5
entry ksNesInst_sta_9d
    add r3, r3, REGISTER_X
    mr r4, REGISTER_A
    rlwinm r26, r3, 21, 27, 29 //  (r3 >> 11) & 0x1C
    lwzx r8, state_store_func, r26
    mtctr r8
    bctr

// STA  absolute,Y	STA oper,Y	99	3	5
entry ksNesInst_sta_99
    add r3, r3, REGISTER_Y
    mr r4, REGISTER_A
    rlwinm r26, r3, 21, 27, 29 //  (r3 >> 11) & 0x1C
    lwzx r8, state_store_func, r26
    mtctr r8
    bctr

// STA  (indirect,X)    STA (oper,X)	81	2	6
entry ksNesInst_sta_81
    addi r8, r3, 0x1
    andi. r8, r8, 0xff
    lbzx r3, WRAM, r8
    rlwinm r26, r3, 29, 27, 29 // (r3 >> 3) & 0x1C;
    slwi r3, r3, 8
    lwzx r8, state_store_func, r26
    or r3, r3, r4
    mtctr r8
    mr r4, REGISTER_A
    bctr

//STA   (indirect),Y	STA (oper),Y	91	2	6
entry ksNesInst_sta_91
    lbzx r3, WRAM, r4
    addi r8, r4, 0x1
    andi. r8, r8, 0xff
    lbzx r4, WRAM, r8
    add r3, r3, REGISTER_Y
    rlwinm r26, r4, 29, 27, 29
    slwi r4, r4, 8
    lwzx r8, state_store_func, r26
    add r3, r3, r4
    mtctr r8
    mr r4, REGISTER_A
    bctr

// CLC  implied	CLC	18	1	2
// Clear Carry Flag
entry ksNesInst_clc_18
    li REGISTER_FLAG_CARRY, 0x0
    b CheckScanlineIrq

// SEC  implied	SEC	38	1	2
// Set Carry Flag
entry ksNesInst_sec_38
    li REGISTER_FLAG_CARRY, 0x1
    b CheckScanlineIrq

// DEX  implied	DEX	CA	1	2
entry ksNesInst_dex_ca
    subi REGISTER_FLAG_ZERO, REGISTER_X, 0x1
    andi. REGISTER_FLAG_NEGATIVE, REGISTER_FLAG_ZERO, 0x80
    andi. REGISTER_X, REGISTER_FLAG_ZERO, 0xff
    b CheckScanlineIrq

// INX  implied	INX	E8	1	2
entry ksNesInst_inx_e8
    addi REGISTER_FLAG_ZERO, REGISTER_X, 0x1
    andi. REGISTER_FLAG_NEGATIVE, REGISTER_FLAG_ZERO, 0x80
    andi. REGISTER_X, REGISTER_FLAG_ZERO, 0xff
    b CheckScanlineIrq

// DEY  implied	DEY	88	1	2
entry ksNesInst_dey_88
    subi REGISTER_FLAG_ZERO, REGISTER_Y, 0x1
    andi. REGISTER_FLAG_NEGATIVE, REGISTER_FLAG_ZERO, 0x80
    andi. REGISTER_Y, REGISTER_FLAG_ZERO, 0xff
    b CheckScanlineIrq

// INY  implied	INY	C8	1	2
entry ksNesInst_iny_c8
    addi REGISTER_FLAG_ZERO, REGISTER_Y, 0x1
    andi. REGISTER_FLAG_NEGATIVE, REGISTER_FLAG_ZERO, 0x80
    andi. REGISTER_Y, REGISTER_FLAG_ZERO, 0xff
    b CheckScanlineIrq

// TXA  implied	TXA	8A	1	2
// Transfer Index X to Accumulator
entry ksNesInst_txa_8a
    mr REGISTER_FLAG_ZERO, REGISTER_X
    andi. REGISTER_FLAG_NEGATIVE, REGISTER_X, 0x80
    mr REGISTER_A, REGISTER_X
    b CheckScanlineIrq

// TYA  implied	TYA	98	1	2
// Transfer Index Y to Accumulator
entry ksNesInst_tya_98
    mr REGISTER_FLAG_ZERO, REGISTER_Y
    andi. REGISTER_FLAG_NEGATIVE, REGISTER_Y, 0x80
    mr REGISTER_A, REGISTER_Y
    b CheckScanlineIrq

// TAX  implied	TAX	AA	1	2
// Transfer Accumulator to Index X
entry ksNesInst_tax_aa
    mr REGISTER_FLAG_ZERO, REGISTER_A
    andi. REGISTER_FLAG_NEGATIVE, REGISTER_A, 0x80
    mr REGISTER_X, REGISTER_A
    b CheckScanlineIrq

// TAY  implied	TAY	A8	1	2
// Transfer Accumulator to Index Y
entry ksNesInst_tay_a8
    mr REGISTER_FLAG_ZERO, REGISTER_A
    andi. REGISTER_FLAG_NEGATIVE, REGISTER_A, 0x80
    mr REGISTER_Y, REGISTER_A
    b CheckScanlineIrq

// CMP  (indirect,X)	CMP (oper,X)	C1	2	6
// Compare Memory with Accumulator
entry ksNesInst_cmp_c1
    subf REGISTER_FLAG_ZERO, r4, REGISTER_A
    andi. REGISTER_FLAG_NEGATIVE, REGISTER_FLAG_ZERO, 0x80
    srwi REGISTER_FLAG_CARRY, REGISTER_FLAG_ZERO, 31
    xori REGISTER_FLAG_CARRY, REGISTER_FLAG_CARRY, 0x1
    b CheckScanlineIrq

// CPX  immediate	CPX #oper	E0	2	2
// Compare Memory with Index X
entry ksNesInst_cpx_e0
    subf REGISTER_FLAG_ZERO, r4, REGISTER_X
    andi. REGISTER_FLAG_NEGATIVE, REGISTER_FLAG_ZERO, 0x80
    srwi REGISTER_FLAG_CARRY, REGISTER_FLAG_ZERO, 31
    xori REGISTER_FLAG_CARRY, REGISTER_FLAG_CARRY, 0x1
    b CheckScanlineIrq
// CPY  immediate	CPY #oper	C0	2	2
// Compare Memory and Index Y
entry ksNesInst_cpy_c0
    subf REGISTER_FLAG_ZERO, r4, REGISTER_Y
    andi. REGISTER_FLAG_NEGATIVE, REGISTER_FLAG_ZERO, 0x80
    srwi REGISTER_FLAG_CARRY, REGISTER_FLAG_ZERO, 31
    xori REGISTER_FLAG_CARRY, REGISTER_FLAG_CARRY, 0x1
    b CheckScanlineIrq

// ADC (indirect,X)	ADC (oper,X)	61	2	6
// Add Memory to Accumulator with Carry
entry ksNesInst_adc_61
    extsb r7, REGISTER_A
    add REGISTER_FLAG_ZERO, REGISTER_A, REGISTER_FLAG_CARRY
    add r7, r7, REGISTER_FLAG_CARRY
    add REGISTER_FLAG_ZERO, REGISTER_FLAG_ZERO, r4
    extsb r8, r4
    andi. REGISTER_FLAG_NEGATIVE, REGISTER_FLAG_ZERO, 0x80
    add r7, r7, r8
    andi. REGISTER_A, REGISTER_FLAG_ZERO, 0xff
    addi r7, r7, 0x80
    extrwi REGISTER_FLAG_CARRY, REGISTER_FLAG_ZERO, 1, 23 // (REGISTER_FLAG_ZERO >> 8) & 0x1
    srwi REGISTER_FLAG_OVERFLOW, r7, 8
    b CheckScanlineIrq

// SBC  (indirect,X)	SBC (oper,X)	E1	2	6
// Subtract Memory from Accumulator with Borrow
entry ksNesInst_sbc_e1
    extsb r7, REGISTER_A
    xori REGISTER_FLAG_CARRY, REGISTER_FLAG_CARRY, 0x1
    subf REGISTER_FLAG_ZERO, r4, REGISTER_A
    subf r7, REGISTER_FLAG_CARRY, r7
    subf REGISTER_FLAG_ZERO, REGISTER_FLAG_CARRY, REGISTER_FLAG_ZERO
    extsb r8, r4
    andi. REGISTER_FLAG_NEGATIVE, REGISTER_FLAG_ZERO, 0x80
    subf r7, r8, r7
    extrwi REGISTER_FLAG_CARRY, REGISTER_FLAG_ZERO, 1, 23 // (REGISTER_FLAG_ZERO >> 8) & 0x1
    addi r7, r7, 0x80
    andi. REGISTER_A, REGISTER_FLAG_ZERO, 0xff
    srwi REGISTER_FLAG_OVERFLOW, r7, 8
    xori REGISTER_FLAG_CARRY, REGISTER_FLAG_CARRY, 0x1
    b CheckScanlineIrq

// ora  (indirect,X)	ORA (oper,X)	01	2	6
// OR Memory with Accumulator
entry ksNesInst_ora_01
    // bitwise or A with some value
    // using Z as temporary storage?
    or REGISTER_FLAG_ZERO, REGISTER_A, r4
    // set the N flag if A is negative
    andi. REGISTER_FLAG_NEGATIVE, REGISTER_FLAG_ZERO, 0x80
    // assign the result to A
    andi. REGISTER_A, REGISTER_FLAG_ZERO, 0xff
    b CheckScanlineIrq

// AND  (indirect,X)	AND (oper,X)	21	2	6
// AND Memory with Accumulator
entry ksNesInst_and_21
    and REGISTER_FLAG_ZERO, REGISTER_A, r4
    andi. REGISTER_FLAG_NEGATIVE, REGISTER_FLAG_ZERO, 0x80
    andi. REGISTER_A, REGISTER_FLAG_ZERO, 0xff
    b CheckScanlineIrq

// BIT  zeropage	BIT oper	24	2	3
// Test Bits in Memory with Accumulator
// https://www.masswerk.at/6502/6502_instruction_set.html#bitcompare
entry ksNesInst_bit_24
    and REGISTER_FLAG_ZERO, REGISTER_A, r4
    andi. REGISTER_FLAG_NEGATIVE, r4, 0x80
    andi. REGISTER_FLAG_OVERFLOW, r4, 0x40
    b CheckScanlineIrq

// EOR  (indirect,X)	EOR (oper,X)	41	2	6
// Exclusive-OR Memory with Accumulator
entry ksNesInst_eor_41
    xor REGISTER_FLAG_ZERO, REGISTER_A, r4
    andi. REGISTER_FLAG_NEGATIVE, REGISTER_FLAG_ZERO, 0x80
    andi. REGISTER_A, REGISTER_FLAG_ZERO, 0xff
    b CheckScanlineIrq

// INC  zeropage	INC oper	E6	2	5
// Increment Memory by One
entry ksNesInst_inc_e6
    lwzx r7, state_store_func, r26
    addi REGISTER_FLAG_ZERO, r4, 0x1
    mtctr r7
    andi. REGISTER_FLAG_NEGATIVE, REGISTER_FLAG_ZERO, 0x80
    andi. r4, REGISTER_FLAG_ZERO, 0xff
    bctr

// DEC  zeropage	DEC oper	C6	2	5
// Decrement Memory by One
entry ksNesInst_dec_c6
    lwzx r7, state_store_func, r26
    subi REGISTER_FLAG_ZERO, r4, 0x1
    mtctr r7
    andi. REGISTER_FLAG_NEGATIVE, REGISTER_FLAG_ZERO, 0x80
    andi. r4, REGISTER_FLAG_ZERO, 0xff
    bctr

// ASL  accumulator	ASL A	0A	1	2
// Shift Left One Bit (Memory or Accumulator)
entry ksNesInst_asl_0a
    slwi REGISTER_FLAG_ZERO, REGISTER_A, 1
    extrwi REGISTER_FLAG_CARRY, REGISTER_A, 1, 24 // (REGISTER_A >> 7) & 0x1
    andi. REGISTER_FLAG_NEGATIVE, REGISTER_FLAG_ZERO, 0x80
    andi. REGISTER_A, REGISTER_FLAG_ZERO, 0xff
    b CheckScanlineIrq

// ASL  zeropage	ASL oper	06	2	5
// Shift Left One Bit (Memory or Accumulator)
entry ksNesInst_asl_06
    lwzx r7, state_store_func, r26
    slwi REGISTER_FLAG_ZERO, r4, 1
    mtctr r7
    extrwi REGISTER_FLAG_CARRY, r4, 1, 24 // (r4 >> 7) & 0x1
    andi. REGISTER_FLAG_NEGATIVE, REGISTER_FLAG_ZERO, 0x80
    andi. r4, REGISTER_FLAG_ZERO, 0xff
    bctr

// LSR  accumulator	LSR A	4A	1	2
// Shift One Bit Right (Memory or Accumulator)
entry ksNesInst_lsr_4a
andi. REGISTER_FLAG_CARRY, REGISTER_A, 0x1
andi. REGISTER_FLAG_ZERO, REGISTER_A, 0xfe
li REGISTER_FLAG_NEGATIVE, 0x0
srwi REGISTER_A, REGISTER_A, 1
b CheckScanlineIrq

// LSR  zeropage	LSR oper	46	2	5
// Shift One Bit Right (Memory or Accumulator)
entry ksNesInst_lsr_46
    lwzx r7, state_store_func, r26
    li REGISTER_FLAG_NEGATIVE, 0x0
    mtctr r7
    andi. REGISTER_FLAG_CARRY, r4, 0x1
    andi. REGISTER_FLAG_ZERO, r4, 0xfe
    srwi r4, r4, 1
    bctr

// ROL  accumulator	ROL A	2A	1	2
// Rotate One Bit Left (Memory or Accumulator)
entry ksNesInst_rol_2a
    slwi REGISTER_FLAG_ZERO, REGISTER_A, 1
    rlwinm REGISTER_FLAG_NEGATIVE, REGISTER_A, 1, 24, 24 // (REGISTER_A << 1) & 0x80
    or REGISTER_FLAG_ZERO, REGISTER_FLAG_ZERO, REGISTER_FLAG_CARRY
    extrwi REGISTER_FLAG_CARRY, REGISTER_A, 1, 24 // (REGISTER_A >> 7) & 0x1
    andi. REGISTER_A, REGISTER_FLAG_ZERO, 0xff
    b CheckScanlineIrq

// ROL  zeropage	ROL oper	26	2	5
// Rotate One Bit Left (Memory or Accumulator)
entry ksNesInst_rol_26
    lwzx r7, state_store_func, r26
    slwi REGISTER_FLAG_ZERO, r4, 1
    mtctr r7
    rlwinm REGISTER_FLAG_NEGATIVE, r4, 1, 24, 24 // (r4 << 1) & 0x80
    or REGISTER_FLAG_ZERO, REGISTER_FLAG_ZERO, REGISTER_FLAG_CARRY
    extrwi REGISTER_FLAG_CARRY, r4, 1, 24 // (r4 >> 7) & 0x1
    andi. r4, REGISTER_FLAG_ZERO, 0xff
    bctr

// ROR  accumulator	ROR A	6A	1	2
// Rotate One Bit Right (Memory or Accumulator)
entry ksNesInst_ror_6a
    slwi REGISTER_FLAG_ZERO, REGISTER_FLAG_CARRY, 7
    andi. REGISTER_FLAG_CARRY, REGISTER_A, 0x1
    mr REGISTER_FLAG_NEGATIVE, REGISTER_FLAG_ZERO

    rlwimi REGISTER_FLAG_ZERO, REGISTER_A, 31, 25, 31
    // REGISTER_FLAG_ZERO = ((REGISTER_A>>1) & 0x7F) | (REGISTER_FLAG_ZERO & ~0x7F)

    mr REGISTER_A, REGISTER_FLAG_ZERO
    b CheckScanlineIrq

// ROR  zeropage	ROR oper	66	2	5
// Rotate One Bit Right (Memory or Accumulator)
entry ksNesInst_ror_66
    lwzx r7, state_store_func, r26
    slwi REGISTER_FLAG_ZERO, REGISTER_FLAG_CARRY, 7
    mtctr r7
    andi. REGISTER_FLAG_CARRY, r4, 0x1
    mr REGISTER_FLAG_NEGATIVE, REGISTER_FLAG_ZERO

    rlwimi REGISTER_FLAG_ZERO, r4, 31, 25, 31
    // REGISTER_FLAG_ZERO = ((r4>>1) & 0x7F) | (REGISTER_FLAG_ZERO & ~0x7F)

    mr r4, REGISTER_FLAG_ZERO
    bctr

// BPL  relative	BPL oper	10	2	2**
// Branch on Result Plus
entry ksNesInst_bpl_10
    extsb r8, r4
    andi. r7, REGISTER_FLAG_NEGATIVE, 0x80
    add REGISTER_PC, REGISTER_PC, r8
    beq CheckScanlineIrq
    subf REGISTER_PC, r8, REGISTER_PC
    subi REGISTER_CYCLE_COUNT, REGISTER_CYCLE_COUNT, 0x100
    b CheckScanlineIrq

// BMI  relative	BMI oper	30	2	2**
// Branch on Result Minus
entry ksNesInst_bmi_30
    extsb r8, r4
    andi. r7, REGISTER_FLAG_NEGATIVE, 0x80
    add REGISTER_PC, REGISTER_PC, r8
    bne CheckScanlineIrq
    subf REGISTER_PC, r8, REGISTER_PC
    subi REGISTER_CYCLE_COUNT, REGISTER_CYCLE_COUNT, 0x100
    b CheckScanlineIrq

// BVC  relative	BVC oper	50	2	2**
// Branch on Overflow Clear
entry ksNesInst_bvc_50
    extsb r8, r4
    cmpwi REGISTER_FLAG_OVERFLOW, 0x0
    add REGISTER_PC, REGISTER_PC, r8
    beq CheckScanlineIrq
    subf REGISTER_PC, r8, REGISTER_PC
    subi REGISTER_CYCLE_COUNT, REGISTER_CYCLE_COUNT, 0x100
    b CheckScanlineIrq

// BVS  relative	BVS oper	70	2	2**
// Branch on Overflow Set
entry ksNesInst_bvs_70
    extsb r8, r4
    cmpwi REGISTER_FLAG_OVERFLOW, 0x0
    add REGISTER_PC, REGISTER_PC, r8
    bne CheckScanlineIrq
    subf REGISTER_PC, r8, REGISTER_PC
    subi REGISTER_CYCLE_COUNT, REGISTER_CYCLE_COUNT, 0x100
    b CheckScanlineIrq

// BCC  relative	BCC oper	90	2	2**
// Branch on Carry Clear
entry ksNesInst_bcc_90
    extsb r8, r4
    andi. r7, REGISTER_FLAG_CARRY, 0x1
    add REGISTER_PC, REGISTER_PC, r8
    beq CheckScanlineIrq
    subf REGISTER_PC, r8, REGISTER_PC
    subi REGISTER_CYCLE_COUNT, REGISTER_CYCLE_COUNT, 0x100
    b CheckScanlineIrq

// BCS  relative	BCS oper	B0	2	2**
// Branch on Carry Set
entry ksNesInst_bcs_b0
    extsb r8, r4
    andi. r7, REGISTER_FLAG_CARRY, 0x1
    add REGISTER_PC, REGISTER_PC, r8
    bne CheckScanlineIrq
    subf REGISTER_PC, r8, REGISTER_PC
    subi REGISTER_CYCLE_COUNT, REGISTER_CYCLE_COUNT, 0x100
    b CheckScanlineIrq

// BNE  relative	BNE oper	D0	2	2**
// Branch on Result not Zero
entry ksNesInst_bne_d0
    extsb r8, r4
    andi. r7, REGISTER_FLAG_ZERO, 0xff
    add REGISTER_PC, REGISTER_PC, r8
    bne CheckScanlineIrq
    subf REGISTER_PC, r8, REGISTER_PC
    subi REGISTER_CYCLE_COUNT, REGISTER_CYCLE_COUNT, 0x100
    b CheckScanlineIrq

// BEQ  relative	BEQ oper	D0	2	2**
// Branch on Result Zero
entry ksNesInst_beq_f0
    extsb r8, r4
    andi. r7, REGISTER_FLAG_ZERO, 0xff
    add REGISTER_PC, REGISTER_PC, r8
    beq CheckScanlineIrq
    subf REGISTER_PC, r8, REGISTER_PC
    subi REGISTER_CYCLE_COUNT, REGISTER_CYCLE_COUNT, 0x100
    b CheckScanlineIrq

// JSR absolute	JSR oper	20	3	6
// Jump to New Location Saving Return Address
entry ksNesInst_jsr_20
    subi r4, REGISTER_PC, 0x1
    mr REGISTER_PC, r3
    // fallthrough

entry ksNesPush16_a1
    addi r0, WRAM, 0x100
    srwi r8, r4, 8
    stbx r8, REGISTER_STACK, r0
    subi r7, REGISTER_STACK, 0x1
    andi. r7, r7, 0xff
    subi REGISTER_STACK, REGISTER_STACK, 0x2
    stbx r4, r7, r0
    andi. REGISTER_STACK, REGISTER_STACK, 0xff
    blr

// PHA  implied	PHA	48	1	3
// Push Accumulator on Stack
entry ksNesInst_pha_48
    addi r7, REGISTER_STACK, 0x100
    subi REGISTER_STACK, REGISTER_STACK, 0x1
    stbx REGISTER_A, WRAM, r7
    andi. REGISTER_STACK, REGISTER_STACK, 0xff
    b CheckScanlineIrq

// PLA  implied	PLA	68	1	4
// Pull Accumulator from Stack
entry ksNesInst_pla_68
    addi REGISTER_STACK, REGISTER_STACK, 0x1
    addi r7, WRAM, 0x100
    andi. REGISTER_STACK, REGISTER_STACK, 0xff
    lbzx REGISTER_A, r7, REGISTER_STACK
    mr REGISTER_FLAG_ZERO, REGISTER_A
    andi. REGISTER_FLAG_NEGATIVE, REGISTER_A, 0x80
    b CheckScanlineIrq

// RTS  implied	RTS	60	1	6
// Return from Subroutine
entry ksNesInst_rts_60
    li r4, 0x1
    // fallthrough

entry ksNesPopPC
    addi r8, REGISTER_STACK, 0x1
    addi r7, REGISTER_STACK, 0x2
    ori r8, r8, 0x100
    ori r7, r7, 0x100
    lbzx r3, WRAM, r8
    andi. REGISTER_STACK, r7, 0xff
    lbzx r9, WRAM, r7
    add r3, r3, r4
    slwi r9, r9, 8
    add r3, r3, r9
    andi. REGISTER_PC, r3, 0xffff
    blr

// JMP  absolute	JMP oper	4C	3	3
// Jump to New Location
entry ksNesInst_jmp_4c
    mr REGISTER_PC, r3
    b CheckScanlineIrq

// JMP  indirect	JMP (oper)	6C	3	5
// Jump to New Location
entry ksNesInst_jmp_6c
    lwzx r9, state_cpu_ranges, r26
    subi r7, r3, 0x2000
    addi r4, r3, 0x1
    rlwinm r7, r7, 16, 16, 20
    andc r8, r3, r7
    lbzx REGISTER_PC, r9, r8
    rlwimi r3, r4, 0, 24, 31
    andc r3, r3, r7
    lbzx r4, r9, r3
    rlwimi REGISTER_PC, r4, 8, 16, 23
    b CheckScanlineIrq

// BRK  implied	BRK	00	1	7
// Force Break
entry ksNesInst_brk_00
    li r9, 0x14
    lis r7, 0x0
    ori r7, r7, 0xfffe
    b L_8003BC60

entry ksNesActivateIntrIRQ
    lis r7, 0x0
    ori r7, r7, 0xfffe
    // fallthrough

entry ksNesActivateIntr
    li r9, 0x24
L_8003BC60:
    lwz r8, state_temp->cpu_e000_ffff
    mr r4, REGISTER_PC
    lbzux REGISTER_PC, r8, r7
    lbz r7, 0x1(r8)
    rlwimi REGISTER_PC, r7, 8, 16, 23
    bl ksNesPush16_a1
    addi REGISTER_CYCLE_COUNT, REGISTER_CYCLE_COUNT, 0x700
L_8003BC7C:
    lbz r7, state_temp->cpu_state.P
    cmpwi REGISTER_FLAG_OVERFLOW, 0x0
    rlwinm r8, r9, 31, 27, 27
    or r0, REGISTER_FLAG_NEGATIVE, REGISTER_FLAG_CARRY
    andc r7, r7, r8
    beq L_8003BC98
    ori r0, r0, 0x40
L_8003BC98:
    andi. r9, r9, 0x14
    andi. r8, REGISTER_FLAG_ZERO, 0xff
    or r0, r0, r7
    subi r8, r8, 0x1
    or r9, r9, r7
    addi r7, REGISTER_STACK, 0x100
    stb r9, state_temp->cpu_state.P
    rlwimi r0, r8, 2, 30, 30
    subi REGISTER_STACK, REGISTER_STACK, 0x1
    stbx r0, WRAM, r7
    andi. REGISTER_STACK, REGISTER_STACK, 0xff
    b CheckScanlineIrq


// PHP  implied	PHP	08	1	3
// Push Processor Status on Stack
entry ksNesInst_php_08
    li r9, 0x0
    b L_8003BC7C

// PLP  implied	PLP	28	1	4
// Pull Processor Status from Stack
entry ksNesInst_plp_28
    addi REGISTER_STACK, REGISTER_STACK, 0x1
    addi r7, WRAM, 0x100
    andi. REGISTER_STACK, REGISTER_STACK, 0xff
    lbzx r8, r7, REGISTER_STACK
    lbz r9, state_temp->cpu_state.P
    andi. r7, r8, 0x3c
    andi. REGISTER_FLAG_CARRY, r8, 0x1
    stb r7, state_temp->cpu_state.P
    andi. REGISTER_FLAG_ZERO, r8, 0x2
    andi. REGISTER_FLAG_OVERFLOW, r8, 0x40
    xori REGISTER_FLAG_ZERO, REGISTER_FLAG_ZERO, 0x2
    andi. REGISTER_FLAG_NEGATIVE, r8, 0x80
    andi. r7, r7, 0x4
    lbz r8, state_temp->cpu_state.irq_pending_flag
    bnelr
    cmpwi r8, 0x0
    beqlr
    li r8, 0x0
    stb r8, state_temp->cpu_state.irq_pending_flag
    bctr

// RTI  implied	RTI	40	1	6
// Return from Interrupt
entry ksNesInst_rti_40
    bl ksNesInst_plp_28
    li r4, 0x0
    bl ksNesPopPC
    b CheckScanlineIrq

entry ksNesInst_rti_40_2
    li r4, 0x0
    bl ksNesPopPC
    b ksNesActivateIntrIRQ

// CLI  implied	CLI	58	1	2
// Clear Interrupt Disable Bit
entry ksNesInst_cli_58
    lbz r7, state_temp->cpu_state.P
    andi. r7, r7, 0xfb
    stb r7, state_temp->cpu_state.P
    lbz r8, state_temp->cpu_state.irq_pending_flag
    cmpwi r8, 0x0
    beq CheckScanlineIrq
    li r8, 0x0
    stb r8, state_temp->cpu_state.irq_pending_flag
    b ksNesActivateIntrIRQ

// SEI  implied	SEI	78	1	2
// Set Interrupt Disable Status
entry ksNesInst_sei_78
    lbz r7, state_temp->cpu_state.P
    ori r7, r7, 0x4
    stb r7, state_temp->cpu_state.P
    b CheckScanlineIrq

// TXS  implied	TXS	9A	1	2
// Transfer Index X to Stack Register
entry ksNesInst_txs_9a
    mr REGISTER_STACK, REGISTER_X
    b CheckScanlineIrq

// TSX  implied	TSX	BA	1	2
// Transfer Stack Pointer to Index X
entry ksNesInst_tsx_ba
    mr REGISTER_FLAG_ZERO, REGISTER_STACK
    andi. REGISTER_FLAG_NEGATIVE, REGISTER_STACK, 0x80
    mr REGISTER_X, REGISTER_STACK
    b CheckScanlineIrq

// CLD  Clear Decimal Mode
// Clear Decimal Mode
entry ksNesInst_cld_d8
    lbz r7, state_temp->cpu_state.P
    andi. r7, r7, ~0x8 & 0xff
    stb r7, state_temp->cpu_state.P
    b CheckScanlineIrq

// SED  implied	SED	F8	1	2
// Set Decimal Flag
entry ksNesInst_sed_f8
    lbz r7, state_temp->cpu_state.P
    ori r7, r7, 0x8
    stb r7, state_temp->cpu_state.P
    b CheckScanlineIrq

// CLV implied	CLV	B8	1	2
// Clear Overflow Flag
entry ksNesInst_clv_b8
    li REGISTER_FLAG_OVERFLOW, 0x0
    b CheckScanlineIrq

entry ksNesLoadInvalid
    bctr
entry ksNesLoadIgnore
    srwi r4, r3, 8
    bctr
// < r3 = address to load from
entry ksNesLoadWRAM
    // we're loading from wram so we'll limit the address to valid wram addresses
    andi.      r7,r3,0x7ff

    // r4 = state_temp->wram[r7]
    // read a byte from work ram
    // the state object starts with a work ram array.
    lbzx       r4,WRAM,r7
    bctr

entry ksNesLoadBBRAM
    // r7 = r27[r26]
    lwzx       r7,state_cpu_ranges,r26
    // r4 = r7[r3]
    lbzx       r4,r7,r3
    bctr

entry ksNesLoadPPU
    andi.      r7,r3,0x7
    addi       r8, state_temp, state_temp->ppu_internal_state
    cmpwi      r7,0x7
    lbzx       r4,r8,r7
    bne        LAB_8003be58
    lhz        r7,state_temp->ppu_addr_latch
    cmpwi      r7,0x3000
    bge        LAB_8003be14
    cmpwi      r7,0x2000
    blt        LAB_8003be30
    rlwinm     r9,r7,0x18,0x1c,0x1d // r9 = (r7 >> 8) & 0xC;
    addi       r0, state_temp, state_temp->ppu_nametable_pointers
    lwzx       r9,r9,r0
    andi.      r8,r7,0x3ff
    lbzx       r9,r9,r8
    stb        r9, state_temp->ppu_internal_state[7]
LAB_8003be14:
    lbz        r8, state_temp->ppu_register_cache[0]
    li         r0,0x3
    andi.      r8,r8,0x4
    rlwnm      r8,r0,r8,0x1f,0x1a // r8 = (r0 << r8) & 0x8000003F
    add        r7,r7,r8
    sth        r7,state_temp->ppu_addr_latch
    bctr
LAB_8003be30:
    rlwinm     r8, r7, 0x16, 0x1d, 0x1f
    addi       r9, state_temp, state_temp->ppu_chr_banks
    lbzx       r9, r9, r8
    andi.      r0, r7, 0x3ff
    lwz        r8, state_temp->chrramp
    rlwinm     r9, r9, 0xa, 0x0, 0x15 // r9=r9*1024
    add        r8, r8, r0
    lbzx       r9, r8, r9
    stb        r9, state_temp->ppu_internal_state[7]
    b          LAB_8003be14
LAB_8003be58:
    li         r9,0x20
    stbx       r9,r8,r7
    bctr

entry ksNesLoadIO
    addi r8, state_load_func, (8 * sizeof(void*)) // state->load_io_func immediately follows state_load_func (state->load_func), so move to the beginning of it by skipping over all entries in state_load_func.
    clrlslwi r7, r3, 26, 2
    lwzx r0, r8, r7
    cmpwi r3, 0x4018
    mtlr r0
    bltlr
    cmpwi r3, 0x4034
    bge L_8003BF0C
    subic. r7, r3, 0x4030
    addi r8, state_temp, state_temp->qd_drive.raw
    blt ksNesLoadInvalid
    lbzx r4, r8, r7
    bne L_8003BEA4
    li r7, 0x0
    stb r7, state_temp->qd_drive.raw
    bctr
L_8003BEA4:
    cmpwi r3, 0x4031
    beq L_8003BEFC
    cmpwi r3, 0x4032
    bnectr
    lbz r7, state_temp->fds_disk_drive_status
    and r4, r4, r7
    cmplwi REGISTER_PC, 0xe000
    blt L_8003BEDC
    cmplwi REGISTER_PC, 0xeee2
    beqctr
    cmplwi REGISTER_PC, 0xef36
    beqctr
    li r4, 0x40
    bctr
L_8003BEDC:
    lbz r7, state_temp->fds_ready_delay_counter
    ori r4, r4, KS_NES_FDS_CTRL_FLG_CRC_ENABLE|KS_NES_FDS_CTRL_FLG_TRANSFER_MODE|KS_NES_FDS_CTRL_FLG_MOTOR|KS_NES_FDS_CTRL_FLG_TRANSFER_RESET
    cmpwi r7, 0xc3
    bgectr
    lbz r7, state_temp->fds_control_register
    andi. r4, r4, KS_NES_FDS_CTRL_FLG_INTERRUPT_ENABLE|KS_NES_FDS_CTRL_FLG_CRC_ENABLE|KS_NES_FDS_CTRL_FLG_5|KS_NES_FDS_CTRL_FLG_CRC_TRANSFER|KS_NES_FDS_CTRL_FLG_NAMETABLE_MODE|KS_NES_FDS_CTRL_FLG_MOTOR
    rlwimi r4, r7, 0, 30, 30 // set control register while preserving disk motor state
    bctr
L_8003BEFC:
    lhz r9, state_temp->qd_disk_head_position.split.lower
    addi r9, r9, 0x1
    sth r9, state_temp->qd_disk_head_position.split.lower
    bctr
L_8003BF0C:
    mfctr r0
    stw r0, 0xd0(r1)
    bl Sound_Read
    lwz r0, 0xd0(r1)
    mr r4, r3
    mtctr r0
    bctr

entry ksNesLoad4015
	lbz r4, state_temp->cpu_state.irq_status_flags
	andi. r7, r4, 0x3f
	stb r7, state_temp->cpu_state.irq_status_flags
	bctr

entry ksNesLoad4017
    lwz r0, state_temp->io_shift_registers[6]
    li r4, 0x40
    lwz r10, state_temp->io_shift_registers[5]
    lwz r9, state_temp->io_shift_registers[4]
    rlwimi r4, r0, 5, 27, 27
    lwz r8, state_temp->io_shift_registers[3]
    rlwimi r4, r10, 4, 28, 28
    lwz r7, state_temp->io_shift_registers[2]
    rlwimi r4, r9, 3, 29, 29
    lbz r5, state_temp->cpu_state.controller_strobe
    rlwimi r4, r8, 2, 30, 30
    rlwimi r4, r7, 1, 31, 31
    andi. r5, r5, 0x1
    bnectr
    slwi r0, r0, 1
    slwi r10, r10, 1
    stw r0, state_temp->io_shift_registers[6]
    slwi r9, r9, 1
    stw r10, state_temp->io_shift_registers[5]
    slwi r8, r8, 1
    stw r9, state_temp->io_shift_registers[4]
    slwi r7, r7, 1
    stw r8, state_temp->io_shift_registers[3]
    stw r7, state_temp->io_shift_registers[2]
    bctr

entry ksNesLoad4016
    lbz r7, state_temp->cpu_state.controller_strobe
    li r4, 0x40
    lwz r8, state_temp->io_shift_registers[1]
    andi. r7, r7, 0x1
    lwz r9, state_temp->io_shift_registers[0]
    rlwimi r4, r8, 2, 30, 30
    rlwimi r4, r9, 1, 31, 31
    bnectr
    slwi r8, r8, 1
    slwi r9, r9, 1
    stw r8, state_temp->io_shift_registers[1]
    stw r9, state_temp->io_shift_registers[0]
    bctr

entry ksNesStoreWRAM
    andi. r7, r3, 0x7ff
    stbx r4, WRAM, r7
    b CheckScanlineIrq

entry ksNesStoreBBRAM
    lwzx       r7,state_cpu_ranges,r26
    stbx       r4,r7,r3
    b          CheckScanlineIrq

entry ksNesStoreInvalid
    b CheckScanlineIrq

entry ksNesStorePPU
    clrlslwi r7, r3, 29, 2
    subi r8, r28, 0xc0
    lwzx r7, r8, r7
    mtctr r7
    bctr

// PPU Ctrl Register
entry ksNesStore2000
    lbz r9, state_temp->ppu_vram_addr_v_hi
    andi. r8, r4, 0x3
    lbz r7, state_temp->ppu_register_cache[0]
    stb r4, state_temp->ppu_register_cache[0]
    rlwimi r9, r4, 0, 31, 31
    stb r8, state_temp->ppu_vram_addr_t_hi
    stb r9, state_temp->ppu_vram_addr_v_hi
    b CheckScanlineIrq

entry ksNesStorePPURam
    andi. r7, r3, 0x7
    addi r8, state_temp, state_temp->ppu_register_cache
    stbx r4, r8, r7
    b CheckScanlineIrq

// OAM Data Register
entry ksNesStore2004
    b CheckScanlineIrq

// PPU Scroll Register
entry ksNesStore2005
    lbz r7, state_temp->ppu_internal_state[2]
    andi. r9, r7, 0x1
    xori r7, r7, 0x1
    stb r7, state_temp->ppu_internal_state[2]
    bne L_8003C050
    stb r4, state_temp->ppu_fine_x_scroll
    b CheckScanlineIrq
L_8003C050:
    lbz r8, state_temp->ppu_scanline_threshold
    stb r4, state_temp->ppu_vram_addr_t_lo
    cmpwi r8, 0x0
    beq CheckScanlineIrq
    lha r9, state_temp->ppu_scanline_counter
    cmpw r9, r8
    bge CheckScanlineIrq
    stb r4, state_temp->ppu_vram_addr_v_lo
    cmpwi r9, 0x8
    blt CheckScanlineIrq
    add r4, r4, r9
L_8003C07C:
    cmpwi r4, 0xf0
    blt L_8003C08C
    subi r4, r4, 0xf0
    b L_8003C07C
L_8003C08C:
    stb r4, state_temp->ppu_vram_addr_v_lo
    b CheckScanlineIrq

// PPU Addr Register
entry ksNesStore2006
    lbz r7, state_temp->ppu_internal_state[2]
    addi r8, state_temp, state_temp->ppu_addr_latch
    andi. r9, r7, 0x1
    xori r7, r7, 0x1
    stbx r4, r8, r9 // stores to index 0 or 1, essentially writing to the u16 here
    stb r7, state_temp->ppu_internal_state[2]
    beq CheckScanlineIrq
    lhz r8, state_temp->ppu_addr_latch
    andi. r8, r8, 0x3fff
    sth r8, state_temp->ppu_addr_latch
    rlwinm r7, r8, 30, 22, 28
    rlwimi r7, r8, 20, 30, 31
    lbz r9, state_temp->mapper
    cmpwi r9, KS_NES_MAPPER_MMC5
    bne L_8003C0D4
    andi. r7, r7, 0x7ffe
L_8003C0D4:
    sth r7, state_temp->ppu_vram_addr_t_hi
    sth r7, state_temp->ppu_vram_addr_v_hi
    b CheckScanlineIrq

// PPU Data Register
entry ksNesStore2007ChrRom
    lhz r7, state_temp->ppu_addr_latch
    cmpwi r7, 0x3000
    bge L_8003C18C
    cmpwi r7, 0x2000
    blt L_8003C130
    rlwinm r9, r7, 24, 28, 29 // (r7 >> 8) & 0xC // ((r7 >> 10) & 0x2) << 2
    addi r0, state_temp, state_temp->ppu_nametable_pointers
    lwzx r9, r9, r0
    andi. r8, r7, 0x3ff
    addi r10, state_temp, state_temp->cartridge_nametable_ram
    cmpw r10, r9
    beq L_8003C114
    stbx r4, r9, r8
L_8003C114:
    lbz r8, state_temp->ppu_register_cache[0]
    li r0, 0x3
    andi. r8, r8, 0x4
    rlwnm r8, r0, r8, 31, 26
    add r7, r7, r8
    sth r7, state_temp->ppu_addr_latch
    b CheckScanlineIrq
L_8003C130:
    lbz r10, state_temp->chr_ram_write_enable
    extrwi r8, r7, 3, 19
    addi r9, state_temp, state_temp->ppu_chr_banks
    andi. r0, r7, 0x3ff
    cmpwi r10, 0x0
    lbzx r9, r9, r8
    beq CheckScanlineIrq
    lwz r8, state_temp->chrramp
    slwi r9, r9, 10
    add r9, r9, r0
    stbx r4, r8, r9
    lwz r3, 0x8(r1)
    srwi r5, r9, 4
    lbz r7, state_temp->mapper
    clrrwi r9, r9, 4
    add r4, r8, r9
    cmpwi r7, KS_NES_MAPPER_MMC5
    beq L_8003C180
    bl ksNesConvertChrToI8
    b L_8003C184
L_8003C180:
    bl ksNesConvertChrToI8MMC5
L_8003C184:
    lhz r7, state_temp->ppu_addr_latch
    b L_8003C114
L_8003C18C:
    cmpwi r7, 0x3f00
    blt L_8003C114
    cmpwi r7, 0x3f20
    bge L_8003C114
    lbz r10, state_temp->ppu_palette_bank
    andi. r8, r7, 0x1f
    andi. r0, r4, 0x3f
    andi. r9, r7, 0xf
    addi r10, r10, state_temp->ppu_palette_ram
    add r10, r10, state_temp
    beq L_8003C1CC
    andi. r9, r7, 0x3
    beq L_8003C114
    stbx r0, r10, r8
    sth r7, state_temp->ppu_palette_write_addr
    b L_8003C114
L_8003C1CC:
    stb r0, 0x0(r10)
    stb r0, 0x4(r10)
    stb r0, 0x8(r10)
    stb r0, 0xc(r10)
    stb r0, 0x10(r10)
    stb r0, 0x14(r10)
    stb r0, 0x18(r10)
    stb r0, 0x1c(r10)
    sth r7, state_temp->ppu_palette_write_addr
    b L_8003C114

entry ksNesStoreIO
    subi r8, state_store_func, (40 * sizeof(void*)) // state_store_func already exists as a register, so backtrack to the beginning of state->store_io_func which immediately preceeds it
    clrlslwi r7, r3, 26, 2
    lwzx r0, r8, r7
    cmpwi r3, 0x4027
    mtctr r0
    bge ksNesStoreQDSound
    bctr

entry ksNesStoreQDSound
    b ksNesStore4000

entry ksNesStore4017
    andi. r8, r4, 0xc0
    bne L_8003C234
    lhz r7, state_temp->ppu_scanline_counter
    li r9, 0x1
    stb r8, state_temp->cpu_state.apu_frame_counter_mode
    sth r7, state_temp->scanline_irq_target_val
    stb r9, state_temp->cpu_state.mapper_irq_enable
    b ksNesStore4000
L_8003C234:
    lbz r9, state_temp->cpu_state.apu_frame_counter_mode
    andi. r9, r9, 0xc0
    stb r8, state_temp->cpu_state.apu_frame_counter_mode
    bne ksNesStore4000
    stb r9, state_temp->cpu_state.mapper_irq_enable
    b ksNesStore4000

entry ksNesStore4011
    lbz r8, state_temp->cpu_state.dac_value
    cmpw r8, r4
    beq CheckScanlineIrq
    stb r4, state_temp->cpu_state.dac_value
    b ksNesStore4000

entry ksNesStore4015
	lbz r9, state_temp->cpu_state.irq_status_flags
	andi. r8, r4, 0x10
	or r9, r9, r8
	stb r9, state_temp->cpu_state.irq_status_flags
	b ksNesStore4003

entry ksNesStore4003
    lbz r9, state_temp->cpu_state.irq_status_flags
    srwi r7, r3, 30
    li r8, 0x1
    slw r8, r8, r7
    or r9, r9, r8
    stb r9, state_temp->cpu_state.irq_status_flags
    // fallthrough

entry ksNesStore4000
    lwz r8, state_temp->frame_flags
    lwz r5, STACK_CPU_CYCLES
    lbz r7, STACK_FRAMES
    andi. r8, r8, 0x1000
    bne CheckScanlineIrq
    divwu r5, r5, r7
    bl Sound_Write
    b CheckScanlineIrq

entry ksNesStore4014
    rlwinm r7, r4, 29, 27, 29
    slwi r8, r4, 8
    lwzx r7, state_cpu_ranges, r7
    li r0, 0x40
    addi r9, state_temp, state_temp->primary_oam - sizeof(ksNesOAMEntry)
    mtctr r0
    add r7, r7, r8
    subi r7, r7, 0x4
L_8003C2CC:
    lwzu r0, 0x4(r7)
    stwu r0, 0x4(r9)
    bdnz L_8003C2CC
    addis REGISTER_CYCLE_COUNT, REGISTER_CYCLE_COUNT, 0x2
    b CheckScanlineIrq

entry ksNesStore4016
    andi. r7, r4, 0x1
    stb r4, state_temp->cpu_state.controller_strobe
    beq CheckScanlineIrq
    lwz r9, STACK_WORK_ARG
    lwz r7, (ksNesCommonWorkObj.pads[0])(r9)
    lwz r8, (ksNesCommonWorkObj.pads[1])(r9)
    stw r7, state_temp->io_shift_registers[0]
    stw r8, state_temp->io_shift_registers[1]
    lwz r7, (ksNesCommonWorkObj.pads[2])(r9)
    lwz r8, (ksNesCommonWorkObj.pads[3])(r9)
    stw r7, state_temp->io_shift_registers[2]
    stw r8, state_temp->io_shift_registers[3]
    lwz r7, (ksNesCommonWorkObj.pads[4])(r9)
    lwz r8, (ksNesCommonWorkObj.pads[5])(r9)
    lwz r0, (ksNesCommonWorkObj.pads[6])(r9)
    stw r7, state_temp->io_shift_registers[4]
    stw r8, state_temp->io_shift_registers[5]
    stw r0, state_temp->io_shift_registers[6]
    b CheckScanlineIrq

entry ksNesInst_wdm_42
    lwz r7, state_temp->prgromp
    cmpwi r7, 0x0
    bne CheckScanlineIrq
    cmplwi REGISTER_PC, 0xe408
    beq L_8003C4D8
    cmplwi REGISTER_PC, 0xeebf
    beq L_8003C464
    cmplwi REGISTER_PC, 0xeef6
    beq L_8003C440
    cmplwi REGISTER_PC, 0xe23b
    beq L_8003C36C
    cmplwi REGISTER_PC, 0xe7a6
    bne CheckScanlineIrq
    andi. REGISTER_CYCLE_COUNT, REGISTER_CYCLE_COUNT, 0x7
    subi REGISTER_PC, REGISTER_PC, 0x2
    b CheckScanlineIrq
L_8003C36C:
    stb REGISTER_A, state_temp->wram[0xE]
    cmpwi REGISTER_A, 0xff
    beq CheckScanlineIrq
    add r7, state_temp, REGISTER_STACK
    lbzu r9, (ksNesStateObj.wram + 0x101)(r7)
    lbz r8, 0x1(r7)
    rlwimi r9, r8, 8, 16, 23
    addi r5, r9, 0x1
    addi r6, r9, 0x5
    sth r6, state_temp->wdm_return_addr
    rlwinm r26, r5, 21, 27, 29
    lwzx r7, state_cpu_ranges, r26
    lbzx r7, r7, r5
    addi r5, r5, 0x1
    rlwinm r26, r5, 21, 27, 29
    lwzx r8, state_cpu_ranges, r26
    lbzx r8, r8, r5
    addi r5, r5, 0x1
    rlwimi r7, r8, 8, 16, 23
    addi r3, state_temp, state_temp->_176E[0-1]
    addi r4, state_temp, state_temp->_176E[10-1]
L_8003C3C0:
    rlwinm r26, r7, 21, 27, 29
    lwzx r8, state_cpu_ranges, r26
    lbzx r8, r8, r7
    addi r7, r7, 0x1
    stbu r8, 0x1(r3)
    cmpw r3, r4
    bne L_8003C3C0
    rlwinm r26, r5, 21, 27, 29
    lwzx r7, state_cpu_ranges, r26
    lbzx r7, r7, r5
    addi r5, r5, 0x1
    rlwinm r26, r5, 21, 27, 29
    lwzx r8, state_cpu_ranges, r26
    lbzx r8, r8, r5
    rlwimi r7, r8, 8, 16, 23
    addi r3, r4, 0x11
L_8003C400:
    rlwinm r26, r7, 21, 27, 29
    lwzx r8, state_cpu_ranges, r26
    lbzx r8, r8, r7
    addi r7, r7, 0x1
    stbu r8, 0x1(r4)
    cmpw r4, r3
    bne L_8003C400
    lwz r3, STACK_WORK_ARG
    lwz r4, STACK_STATE_ARG
    bl ksNesQDFastSave
    andi. REGISTER_FLAG_ZERO, r3, 0xff
    andi. REGISTER_A, r3, 0xff
    bne CheckScanlineIrq
    bl ksNesInst_rts_60
    lhz REGISTER_PC, state_temp->wdm_return_addr
    b CheckScanlineIrq
L_8003C440:
    lwz r9, STACK_WORK_ARG
    lwz r8, (ksNesCommonWorkObj.pads[0])(r9)
    srwi r7, r8, 28
    cmpwi r7, 0x6
    beq L_8003C45C
    li r8, 0x46
    stb r8, state_temp->fds_disk_drive_status
L_8003C45C:
    lbz REGISTER_A, state_temp->wram[0x90]
    b CheckScanlineIrq
L_8003C464:
    andi. r7, REGISTER_A, 0xf0
    cmpwi r7, 0x60
    beq CheckScanlineIrq
    subi r7, r26, 0x2
    li r8, 0xa9
    stbx r8, r7, REGISTER_PC
    lwz r3, 0x8(r1)
    lwz r4, 0xc(r1)
    bl ksNesQDFastLoad
    andi. REGISTER_FLAG_ZERO, r3, 0xff
    bne CheckScanlineIrq
    subi r7, state_temp, 0x1
    li r8, 0x46
    stb r8, state_temp->qd_drive.registers.drive_status
    stb r8, state_temp->fds_disk_drive_status
    li r8, 0x0
    addi r0, r7, 0xfa
L_8003C4A8:
    stbu r8, 0x1(r7)
    cmpw r7, r0
    bne L_8003C4A8
    addi r0, r7, 0x104
    lis r9, ksNesInitQDDataTbl@h
    ori r9, r9, ksNesInitQDDataTbl@l
L_8003C4C0:
    lbzu r8, 0x1(r9)
    stbu r8, 0x1(r7)
    cmpw r7, r0
    bne L_8003C4C0
    subi REGISTER_PC, REGISTER_PC, 0x24
    b CheckScanlineIrq
L_8003C4D8:
    lbz r7, 0x0(state_temp)
    stb REGISTER_A, 0x1(state_temp)
    rlwinm r26, REGISTER_A, 29, 27, 29
    rlwimi r7, REGISTER_A, 8, 16, 23
    lwzx r8, state_cpu_ranges, r26
    add r7, r7, r8
    addi r7, r7, 0x1
    lswi r8, r7, 8
    cmpwi r8, -0x1
    beq CheckScanlineIrq
    lwz r5, state_temp->nesromp
    lbz r7, state_temp->fds_disk_count
    addi r10, r5, 0x10
L_8003C50C:
    lswi r3, r10, 8
    cmpw r3, r8
    bne L_8003C520
    cmpw r4, r9
    beq L_8003C530
L_8003C520:
    addis r10, r10, 0x1
    subic. r7, r7, 0x1
    bgt L_8003C50C
    b CheckScanlineIrq
L_8003C530:
    subi r10, r10, 0x10
    subf r10, r5, r10
    stw r10, state_temp->qd_disk_head_position.full
    b CheckScanlineIrq

entry ksNesLinecntIrqQD
    lbz r0, state_temp->fds_control_register
    andi. r8, r0, KS_NES_FDS_CTRL_FLG_INTERRUPT_ENABLE
    beq L_8003C588
    // check for motor start flag with all other flags enabled
    andi. r9, r0, KS_NES_FDS_CTRL_FLG_INTERRUPT_ENABLE|KS_NES_FDS_CTRL_FLG_CRC_ENABLE|KS_NES_FDS_CTRL_FLG_5|KS_NES_FDS_CTRL_FLG_MOTOR|KS_NES_FDS_CTRL_FLG_TRANSFER_RESET
    cmpwi r9, KS_NES_FDS_CTRL_FLG_INTERRUPT_ENABLE|KS_NES_FDS_CTRL_FLG_CRC_ENABLE|KS_NES_FDS_CTRL_FLG_5|KS_NES_FDS_CTRL_FLG_TRANSFER_RESET
    bne ksNesMainLoop1
    addi r10, r10, 0x1
    cmpwi r10, 0xee
    ble L_8003C568
    li r10, -0x14
L_8003C568:
    sth r10, state_temp->scanline_irq_target_val
    lwz r8, state_temp->nesromp
    lwz r9, state_temp->qd_disk_head_position.full
    andi. r10, r0, 0x4
    beq SUB_8003b3a0
    lbzx r10, r8, r9
    stb r10, state_temp->qd_drive.registers.data
    b SUB_8003b3a0
L_8003C588:
    lbz r8, state_temp->fds_timer_control_register
    // check for IRQ enabled
    andi. r0, r8, ~(KS_NES_FDS_TIMER_CTRL_FLG_IRQ_ENABLE) & 0xFF
    li r9, 0x7fff
    // check for IRQ repeat flag
    andi. r8, r8, KS_NES_FDS_TIMER_CTRL_FLG_IRQ_REPEAT
    beq L_8003C5A8
    mr r0, r8
    lhz r9, state_temp->mapper_irq_latch
    add r9, r9, r10
L_8003C5A8:
    sth r9, state_temp->scanline_irq_target_val
    stb r0, state_temp->fds_timer_control_register
    andi. r0, r0, KS_NES_FDS_TIMER_CTRL_FLG_IRQ_ENABLE
    stb r0, state_temp->cpu_state.mapper_irq_enable
    li r9, 0x1
    stb r9, state_temp->qd_drive.registers.disk_status
    cmpwi r7, 0x0
    bne SUB_8003b3a0
    b ksNesMainLoop2

entry ksNesStoreQD_4020
    subi r7, state_temp, 0x4020 - state_temp->fds_timer_latch_low // offset into 17CB, timer control 0x2855
    stbx r4, r7, r3
    b CheckScanlineIrq

entry ksNesStoreQD_4022
    stb r4, state_temp->fds_timer_control_register
    andi. r8, r4, KS_NES_FDS_TIMER_CTRL_FLG_IRQ_ENABLE
    stb r8, state_temp->cpu_state.mapper_irq_enable
    lbz r8, state_temp->fds_timer_latch_high
    lbz r7, state_temp->fds_timer_latch_low
    li r0, KS_NES_CPU_CYCLES_PER_SCANLINE
    lha r9, state_temp->ppu_scanline_counter
    rlwimi r7, r8, 8, 16, 23
    divwu r8, r7, r0
    sth r8, state_temp->mapper_irq_latch
    add r9, r9, r8
    sth r9, state_temp->scanline_irq_target_val
    b CheckScanlineIrq

entry ksNesStoreQD_4023
    stb r4, state_temp->fds_master_io_enable_register
    b ksNesStore4000

entry ksNesStoreQD_4024
    lbz r7, state_temp->fds_control_register
    lwz r8, state_temp->nesromp
    lwz r9, state_temp->qd_disk_head_position.full
    // Check that fds control state is: interrupt enabled, transfer mode = write, motor = start, and transfer reset flag is enabled
    andi. r7, r7, KS_NES_FDS_CTRL_FLG_INTERRUPT_ENABLE|KS_NES_FDS_CTRL_FLG_TRANSFER_MODE|KS_NES_FDS_CTRL_FLG_MOTOR|KS_NES_FDS_CTRL_FLG_TRANSFER_RESET
    cmpwi r7, KS_NES_FDS_CTRL_FLG_INTERRUPT_ENABLE|KS_NES_FDS_CTRL_FLG_TRANSFER_RESET
    bne CheckScanlineIrq
    add r8, r8, r9
    stb r4, -0x2(r8)
    li r7, 0x1
    stb r7, state_temp->qd_irq_acknowledged_flag
    b CheckScanlineIrq

entry ksNesStoreQD_4025
    lbz r7, state_temp->fds_control_register
    xor r8, r7, r4
    andi. r8, r8, KS_NES_FDS_CTRL_FLG_MOTOR
    stb r4, state_temp->fds_control_register
    beq L_8003C65C
    bl ksNesQDSoundSync
    lbz r4, state_temp->fds_control_register
L_8003C65C:
    andi. r7, r4, KS_NES_FDS_CTRL_FLG_INTERRUPT_ENABLE
    beq L_8003C684
    stb r7, state_temp->cpu_state.mapper_irq_enable
    lha r8, state_temp->ppu_scanline_counter
    addi r8, r8, 0x1
    cmpwi r8, 0xee
    ble L_8003C67C
    li r8, -0x14
L_8003C67C:
    sth r8, state_temp->scanline_irq_target_val
    b L_8003C69C
L_8003C684:
    lbz r7, state_temp->fds_timer_control_register
    andi. r7, r7, KS_NES_FDS_TIMER_CTRL_FLG_IRQ_ENABLE
    stb r7, state_temp->cpu_state.mapper_irq_enable
    bne L_8003C69C
    li r7, 0x7fff
    sth r7, state_temp->scanline_irq_target_val
L_8003C69C:
    andi. r7, r4, 0x3
    cmpwi r7, 0x2
    bne L_8003C6BC
    lbz r8, state_temp->qd_drive.registers.drive_status
    li r7, 0x0
    andi. r8, r8, 0xfd
    stb r8, state_temp->qd_drive.registers.drive_status
    sth r7, state_temp->qd_disk_head_position.split.lower
L_8003C6BC:
    rlwinm r7, r4, 1, 27, 27
    addi r0, state_temp, state_temp->ppu_chr_bank_pointers[8] - sizeof(u32*)
    addi r9, state_temp, state_temp->ppu_nametable_pointers[0] - sizeof(u32*)
    add r7, r7, r0
    addi r0, r7, 0x10
L_8003C6D0:
    lwzu r10, 0x4(r7)
    stwu r10, 0x4(r9)
    cmpw r7, r0
    bne L_8003C6D0
    b CheckScanlineIrq

entry ksNesStoreQD_4026
    stb r4, state_temp->qd_drive.registers.expansion_connector
    b CheckScanlineIrq

entry ksNesStore01_8000
    lbz r9, state_temp->mmc1_regs[6] // shift register
    andi. r7, r4, 0x80
    bne L_8003C718
    lbz r8, state_temp->mmc1_regs[5] // load counter
    srwi r7, r9, 1
    subic. r8, r8, 0x1
    rlwimi r7, r4, 4, 27, 27
    beq L_8003C724
    stb r8, state_temp->mmc1_regs[5] // load counter
    stb r7, state_temp->mmc1_regs[6] // shift register
    b CheckScanlineIrq
L_8003C718:
    lbz r7, state_temp->mmc1_regs[0] // ctrl reg
    li r3, 0x0
    ori r7, r7, 0xc
L_8003C724:
    li r8, 0x5
    addi r0, state_temp, state_temp->mmc1_regs
    stb r8, state_temp->mmc1_regs[5] // load counter
    extrwi r9, r3, 2, 17
    lbzx r8, r9, r0
    cmpw r8, r7
    beq CheckScanlineIrq
    stbx r7, r9, r0
    lbz r7, state_temp->mmc1_regs[0] // control reg
    lbz r10, state_temp->mmc1_regs[3] // bank select
    lbz r8, state_temp->prg_banks
    rlwinm r0, r7, 30, 30, 30
    slwi r10, r10, 1
    ori r0, r0, 0x1c
    and r10, r10, r8
    lwz r9, state_temp->prgromp
    and r10, r10, r0
    subi r10, r10, 0x4
    slwi r10, r10, 13
    add r10, r10, r9
    andi. r0, r7, 0x8
    bne L_8003C790
    stw r10, state_temp->cpu_8000_9fff
    stw r10, state_temp->cpu_a000_bfff
    stw r10, state_temp->cpu_c000_dfff
    stw r10, state_temp->cpu_e000_ffff
    b L_8003C7FC
L_8003C790:
    andi. r0, r7, 0x4
    bne L_8003C7B4
    subi r10, r10, 0x4000
    stw r10, state_temp->cpu_c000_dfff
    stw r10, state_temp->cpu_e000_ffff
    addi r0, r9, -0x8000
    stw r0, state_temp->cpu_8000_9fff
    stw r0, state_temp->cpu_a000_bfff
    b L_8003C7FC
L_8003C7B4:
    lbz r8, state_temp->mmc1_regs[4] // 512k program mode flag
    cmpwi r8, 0x0
    beq L_8003C7D0
    lbz r0, state_temp->mmc1_regs[1] // chr_bank0
    andi. r0, r0, 0x10
    slwi r0, r0, 14
    add r10, r10, r0
L_8003C7D0:
    stw r10, state_temp->cpu_8000_9fff
    stw r10, state_temp->cpu_a000_bfff
    cmpwi r8, 0x0
    beq L_8003C7E8
    xoris r0, r0, 0x4
    subf r9, r0, r9
L_8003C7E8:
    lwz r0, state_temp->prg_size
    subis r10, r9, 0x1
    add r10, r10, r0
    stw r10, state_temp->cpu_c000_dfff
    stw r10, state_temp->cpu_e000_ffff
L_8003C7FC:
    andi. r0, r7, 0x10
    lbz r8, state_temp->mmc1_regs[1] // chr_bank0
    lhz r9, state_temp->chr_banks
    bne L_8003C834
    rlwinm r8, r8, 2, 25, 28
    and r8, r8, r9
    addi r9, state_temp, state_temp->ppu_chr_banks + 0
    addi r0, state_temp, state_temp->ppu_chr_banks + 8
L_8003C81C:
    stb r8, 0x0(r9)
    addi r9, r9, 0x1
    addi r8, r8, 0x1
    cmpw r9, r0
    bne L_8003C81C
    b L_8003C86C
L_8003C834:
    lbz r0, state_temp->mmc1_regs[2] // chr_bank1
    clrlslwi r8, r8, 27, 2
    and r8, r8, r9
    clrlslwi r0, r0, 27, 2
    and r9, r0, r9
    addi r10, state_temp, state_temp->ppu_chr_banks + 0
    addi r0, state_temp, state_temp->ppu_chr_banks + 4
L_8003C850:
    stb r8, 0x0(r10)
    addi r8, r8, 0x1
    stb r9, 0x4(r10)
    addi r10, r10, 0x1
    cmpw r10, r0
    addi r9, r9, 0x1
    bne L_8003C850
L_8003C86C:
    clrlslwi r7, r7, 30, 4
    addi r0, state_temp, state_temp->ppu_chr_bank_pointers[0] - sizeof(u32*)
    addi r9, state_temp, state_temp->ppu_nametable_pointers[0] - sizeof(u32*)
    add r7, r7, r0
    addi r0, r7, 0x10
L_8003C880:
    lwzu r10, 0x4(r7)
    stwu r10, 0x4(r9)
    cmpw r7, r0
    bne L_8003C880
    b CheckScanlineIrq

}
// clang-format on

void ksNesInitAudio() { EmuSound_Start(nullptr); }

void ksNesExitAudio() { EmuSound_Exit(); }

void ksNesInitModule() {
  sNesModule->initAudio = ksNesInitAudio;
  sNesModule->exitAudio = ksNesExitAudio;
  sNesModule->getAudioStream = ksNesGetAudioStream;
  sNesModule->reset = ksNesReset;
  sNesModule->emuFrame = ksNesEmuFrame;
  sNesModule->pushResetButton = ksNesPushResetButton;
  sNesModule->draw = ksNesDraw;
  sNesModule->drawInit = ksNesDrawInit;
}

u8 ksNesPaletteNormal[] = {
    0xc2, 0x10, 0x80, 0x17, 0x98, 0x17, 0xc0, 0x14, 0xdc, 0x0d, 0xd8, 0x03, 0xd8, 0x00, 0xc8, 0x80,
    0xbc, 0xa0, 0x80, 0xe0, 0x81, 0x21, 0x80, 0xe4, 0x80, 0xac, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00,
    0xe7, 0x39, 0x81, 0x7f, 0xa0, 0xff, 0xd8, 0xd9, 0xfc, 0xd5, 0xfc, 0xcb, 0xfc, 0xc3, 0xe9, 0x20,
    0xe1, 0x80, 0x9d, 0xe0, 0x8e, 0x02, 0x82, 0x4c, 0x82, 0x18, 0x88, 0x42, 0x80, 0x00, 0x80, 0x00,
    0xff, 0xff, 0x82, 0x5f, 0xb6, 0x1f, 0xe9, 0xbf, 0xfd, 0xd9, 0xfd, 0xb3, 0xfd, 0xeb, 0xfe, 0x4b,
    0xfe, 0x86, 0xd2, 0xe0, 0xab, 0x6d, 0xa7, 0x55, 0x83, 0x7f, 0xb1, 0x8c, 0x80, 0x00, 0x80, 0x00,
    0xff, 0xff, 0xc2, 0xff, 0xde, 0xff, 0xea, 0xff, 0xfe, 0xfd, 0xfe, 0xf9, 0xff, 0x16, 0xff, 0x35,
    0xff, 0x74, 0xe7, 0x93, 0xd7, 0xb6, 0xd7, 0xdd, 0xdb, 0xbf, 0xef, 0x7b, 0x80, 0x00, 0x80, 0x00,
};

void ksNesDrawInit(ksNesCommonWorkObj* wp) {
  Mtx44 mtx;
  Vec v1 = {0.f, 0.f, 800.f};
  Vec v3 = {0.f, 0.f, -100.f};
  Vec v2 = {0.f, 1.f, 0.f};
  GXInvalidateTexAll();
  GXInvalidateVtxCache();
  GXSetClipMode(GX_CLIP_DISABLE);
  GXSetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
  GXSetCopyFilter(GX_FALSE, NULL, GX_FALSE, NULL);
  GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
  GXSetZTexture(GX_ZT_DISABLE, GX_TF_Z8, 0);
  GXSetZCompLoc(GX_FALSE);
  GXSetColorUpdate(GX_TRUE);
  GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
  GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
  GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
  GXSetTevSwapMode(GX_TEVSTAGE2, GX_TEV_SWAP0, GX_TEV_SWAP0);
  GXSetTevSwapMode(GX_TEVSTAGE3, GX_TEV_SWAP0, GX_TEV_SWAP0);
  C_MTXOrtho(mtx, 0, -480.f, 0.f, 640.f, 0.f, 2000.f);
  GXSetProjection(mtx, GX_ORTHOGRAPHIC);
  C_MTXLookAt(wp->draw_ctx.draw_mtx, &v1, &v2, &v3);
}

static inline void ksNesDrawEnd() {
  GXSetClipMode(GX_CLIP_ENABLE);
  GXSetZCompLoc(GX_TRUE);
  GXSetNumIndStages(0);
  GXSetTevDirect(GX_TEVSTAGE0);
  GXSetTevDirect(GX_TEVSTAGE1);
  GXSetTevDirect(GX_TEVSTAGE2);
  GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
  GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
  GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
  GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
  GXSetTevSwapMode(GX_TEVSTAGE2, GX_TEV_SWAP0, GX_TEV_SWAP0);
  GXSetTevSwapMode(GX_TEVSTAGE3, GX_TEV_SWAP0, GX_TEV_SWAP0);
  GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_FALSE, 0, 0);
  GXSetTexCoordScaleManually(GX_TEXCOORD1, GX_FALSE, 0, 0);
}

static inline void ksNesDrawClearEFBFirst(ksNesCommonWorkObj* wp) {
  DCFlushRange(wp->draw_ctx.post_process_lut, sizeof(wp->draw_ctx.post_process_lut));
  GXSetNumChans(1);
  GXSetNumTexGens(0);
  GXSetNumTevStages(1);
  GXSetNumIndStages(0);
  GXSetTevDirect(GX_TEVSTAGE0);
  GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
  GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
  GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
  GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
  GXSetBlendMode(GX_BM_LOGIC, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_COPY);
  GXClearVtxDesc();
  GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
  GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_S16, 0);
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
  GXSetCurrentMtx(0);
  GXLoadPosMtxImm(wp->draw_ctx.draw_mtx, 0);
  GXBegin(GX_QUADS, GX_VTXFMT0, 4);
  {
    GXPosition2s16(128, -128);
    GXColor1u32(0x00ff0000);

    GXPosition2s16(384, -128);
    GXColor1u32(0x00000000);

    GXPosition2s16(384, -384);
    GXColor1u32(0x0000ff00);

    GXPosition2s16(128, -384);
    GXColor1u32(0x00000000);
  }
  GXEnd();
}

void ksNesDrawMakeBGIndTex(ksNesCommonWorkObj* wp, u32 mmc3) {
  u32 triggerCol = mmc3 ? 9 : 0x7fff;
  u32 chrFlagXor =
      wp->chr_to_i8_buf_size <= CHR_TO_I8_BUF_SIZE ? wp->chr_to_i8_buf_size >> 13 : 0x80;
  u32 row;
  u32 col;
  for (row = 8; row < 236; ++row) {
    u32 ctrl0 = wp->draw_ctx.ppu_scanline_regs[row].vram_addr_y;
    u32 ctrl1 = wp->draw_ctx.ppu_scanline_regs[row].vram_addr_coarse_x;
    u32 mask;
    u32 palette; // The original leaves this uninitialized; each stored byte replaces both nibbles.
    u8* pattern;
    u32 tile;
    u32 bank;
    u8* nametable;

    pattern = reinterpret_cast< u8* >(wp->draw_ctx.ppu_scanline_regs[row].chr_bank_bg);
    mask = mask = (ctrl0 & 4) ? chrFlagXor : 0;
    for (col = 0; col < 34; ++col) {
      if (col == triggerCol) {
        pattern -= 8;
      }
      nametable = wp->draw_ctx.ppu_scanline_regs[row].nametable_ptrs[(ctrl1 >> 8) & 1];
      if (reinterpret_cast< s32 >(nametable) >= 0) {
        palette = (reinterpret_cast< uintptr_t >(nametable) & 3) | (palette << 4);
        tile = (reinterpret_cast< uintptr_t >(nametable) >> 8) & 0xff;
      } else {
        palette = ((nametable[0x3c0 + ((ctrl0 & 0xe0) >> 2) + ((ctrl1 & 0xe0) >> 5)] >>
                    (((ctrl1 >> 3) & 2) | ((ctrl0 & 0x10) >> 2))) &
                   3) |
                  (palette << 4);
        tile = nametable[((ctrl0 & 0xf8) << 2) + ((ctrl1 >> 3) & 0x1f)];
      }
      bank = pattern[((wp->draw_ctx.ppu_scanline_regs[row].ppu_ctrl >> 2) & 4) |
                     (static_cast< u8 >(tile) >> 6)];
      wp->draw_ctx.bg_tile_index_texture[((col * 2) & 6) + ((col * 8) & 0x1e0) + (row >> 2) * 288 +
                                         ((row * 8) & 0x18)] =
          ((bank & 1) << 6 | (tile & 0x3f)) - (col & 1);
      wp->draw_ctx.bg_tile_index_texture[((col * 2) & 6) + ((col * 8) & 0x1e0) + (row >> 2) * 288 +
                                         ((row * 8) & 0x18) + 1] = (bank >> 1) ^ mask;
      if (col & 1) {
        wp->draw_ctx.bg_palette_attr_texture[((col >> 1) & 3) + ((col * 4) & 0xe0) +
                                             (((row * 4) & 0x1c) + (row >> 3) * 160)] = palette;
      }
      ctrl1 += 8;
    }
  }
  DCFlushRangeNoSync(wp->draw_ctx.bg_tile_index_texture,
                     sizeof(wp->draw_ctx.bg_tile_index_texture));
  DCFlushRangeNoSync(wp->draw_ctx.bg_palette_attr_texture,
                     sizeof(wp->draw_ctx.bg_palette_attr_texture));
}

void ksNesDrawBG(ksNesCommonWorkObj* wp, ksNesStateObj* sp) {
  static const GXColor color_r_0xf0 = {240, 0, 0, 0};
  static f32 indtexmtx_screen[2][3] = {
      {0.5f, 0.0f, 0.0f},    // S offset = (Intensity from bg_tile_index_texture) × 0.5
      {0.0f, 0.0625f, 0.0f}, // T offset = (Alpha from bg_tile_index_texture) × 0.0625 (1/16)
  };

  GXTexObj obj0;
  GXTexObj obj1;
  GXTexObj obj2;
  u32 curline;
  u32 cnt;
  u32 color;
  u32 x0;
  u32 x1;
  s16 y;
  u32 i;
  u32 j;

  GXSetNumChans(1);
  GXSetNumTexGens(2);
  GXSetNumTevStages(3);
  GXSetNumIndStages(1);
  GXSetBlendMode(GX_BM_LOGIC, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_COPY);
  GXClearVtxDesc();
  GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
  GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
  GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_S16, 0);
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U16, 10);
  GXInitTexObj(&obj0, wp->draw_ctx.bg_palette_attr_texture, 40, 256, GX_TF_I4, GX_CLAMP, GX_REPEAT,
               0);
  GXInitTexObjLOD(&obj0, GX_NEAR, GX_NEAR, 0.0, 0.0, 0.0, 0, 0, GX_ANISO_1);
  GXLoadTexObj(&obj0, GX_TEXMAP0);
  GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
  GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_TRUE, 128, 1025);
  GXSetTexCoordBias(GX_TEXCOORD0, GX_FALSE, GX_FALSE);
  GXSetTevDirect(GX_TEVSTAGE0);
  GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  GXSetTevColor(GX_TEVREG0, color_r_0xf0);
  GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_ZERO, GX_CC_C0, GX_CC_TEXC);
  GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
  GXInitTexObj(&obj1, wp->draw_ctx.bg_tile_index_texture, 36, 256, GX_TF_IA8, GX_CLAMP, GX_CLAMP,
               0);
  GXInitTexObjLOD(&obj1, GX_NEAR, GX_NEAR, 0.0, 0.0, 0.0, 0, 0, GX_ANISO_1);
  GXLoadTexObj(&obj1, GX_TEXMAP1);
  GXSetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
  GXSetTexCoordScaleManually(GX_TEXCOORD1, GX_TRUE, 1024, 1025);
  GXSetTexCoordBias(GX_TEXCOORD1, GX_FALSE, GX_FALSE);
  GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
  GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_8, GX_ITS_1);
  GXSetIndTexMtx(GX_ITM_0, indtexmtx_screen, 36);
  GXSetTevIndirect(GX_TEVSTAGE1, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_NONE, GX_ITM_0, GX_ITW_16,
                   GX_ITW_0, GX_FALSE, GX_FALSE, GX_ITBA_OFF);
  GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR_NULL);
  GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_TEXC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
  GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
  GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
  GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
  GXSetTevDirect(GX_TEVSTAGE2);
  GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
  GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
  GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_RASC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
  GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
  GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
  GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
  GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_ALWAYS, 0);
  GXSetLineWidth(6, GX_TO_ZERO);

  for (i = 0; i < 8; i++) {
    if (sp->ppu_render_latches[i] != 0) {
      if (sp->mapper == KS_NES_MAPPER_MMC5) {
        GXInitTexObj(&obj2, wp->chr_to_u8_bufp + (wp->chr_to_i8_buf_size >> 3) * i, 1024,
                     wp->chr_to_i8_buf_size >> 13, GX_TF_I8, GX_MIRROR, GX_CLAMP, 0);
        GXInitTexObjLOD(&obj2, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
        GXLoadTexObj(&obj2, GX_TEXMAP2);
      } else if ((i & 1) == 0) {
        if (wp->chr_to_i8_buf_size > CHR_TO_I8_BUF_SIZE) {
          GXInitTexObj(&obj2, wp->chr_to_u8_bufp + 0x20000 * (i & 6), 1024, 256, GX_TF_I8,
                       GX_MIRROR, GX_CLAMP, 0);
        } else {
          GXInitTexObj(&obj2, wp->chr_to_u8_bufp + (wp->chr_to_i8_buf_size >> 3) * (i & 6), 1024,
                       wp->chr_to_i8_buf_size >> 12, GX_TF_I8, GX_MIRROR, GX_CLAMP, 0);
        }

        GXInitTexObjLOD(&obj2, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
        GXLoadTexObj(&obj2, GX_TEXMAP2);
      }

      cnt = 0;
      curline = sp->ppu_render_latches[i + 8];
      j = curline;
      do {
        if ((wp->draw_ctx.ppu_scanline_regs[j].ppumask_flags & KS_NES_PPU_MASK_SHOW_BG) != 0 &&
            j >= 8 && j < 236) {
          cnt += 2;
        }
        j = wp->draw_ctx.ppu_scanline_regs[j].fine_x_and_next;
      } while (j != 0);

      if (cnt != 0) {
        GXBegin(GX_LINES, GX_VTXFMT0, cnt);

        do {
          if ((wp->draw_ctx.ppu_scanline_regs[curline].ppumask_flags & KS_NES_PPU_MASK_SHOW_BG) !=
                  0 &&
              curline >= 8 && curline < 236) {
            if ((wp->draw_ctx.ppu_scanline_regs[curline].ppumask_flags &
                 KS_NES_PPU_MASK_SHOW_BG_LEFT) == 0) {
              x0 = 0x80 + 8;
              x1 = (wp->draw_ctx.ppu_scanline_regs[curline].vram_addr_coarse_x & 0x07) + 8;
            } else {
              x0 = 0x80 - (wp->draw_ctx.ppu_scanline_regs[curline].vram_addr_coarse_x & 0x07);
              x1 = 0;
            }

            color = (wp->draw_ctx.ppu_scanline_regs[curline].ppu_ctrl & 0xC0) << 24;

            GXPosition2s16(x0, -0x81 - curline);
            GXColor1u32(color);
            GXTexCoord2u16(x1, curline);

            GXPosition2s16(x0 + 0x100 + 8, -0x81 - curline);
            GXColor1u32(color);
            GXTexCoord2u16(x1 + 0x100 + 8, curline);
          }

          curline = wp->draw_ctx.ppu_scanline_regs[curline].fine_x_and_next;
        } while (curline != 0);

        GXEnd();
      }
    }
  }

  cnt = 0;
  for (i = 8; i < 236; i++) {
    if ((wp->draw_ctx.ppu_scanline_regs[i].ppumask_flags & KS_NES_PPU_MASK_SHOW_BG) == 0) {
      cnt += 2;
    }
  }

  if (cnt != 0) {
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetNumIndStages(0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_S16, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_RASC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);

    GXBegin(GX_LINES, GX_VTXFMT0, cnt);

    for (i = 8; i < 236; i++) {

      if ((wp->draw_ctx.ppu_scanline_regs[i].ppumask_flags & KS_NES_PPU_MASK_SHOW_BG) == 0) {
        u32 color = (wp->draw_ctx.ppu_scanline_regs[i].ppu_ctrl & 0xC0) << 24;

        GXPosition2s16(0x80, -0x81 - i);
        GXColor1u32(color);

        GXPosition2s16(0x180, -0x81 - i);
        GXColor1u32(color);
      }
    }

    GXEnd();
  }
}

static inline u32 ksNesDrawMakeOBJBlankVtxList(ksNesCommonWorkObj* wp) {
  u32 ret = 0;
  u32 comparison_mask = KS_NES_PPU_MASK_SPRITES_COMBINED;
  int i;

  for (i = 8; i < KS_NES_SCANLINE_COUNT; i++) {
    int bMask = wp->draw_ctx.ppu_scanline_regs[i].ppumask_flags & KS_NES_PPU_MASK_SPRITES_COMBINED;
    if (bMask != comparison_mask &&
        ((bMask != 0) || (comparison_mask != KS_NES_PPU_MASK_SHOW_SPRITES_LEFT)) &&
        (bMask != KS_NES_PPU_MASK_SHOW_SPRITES_LEFT || comparison_mask != 0)) {
      if ((ret & 1) != 0) {
        wp->draw_ctx.scanline_y_coords[ret] = i - wp->draw_ctx.scanline_y_coords[ret - 1];
        ret++;
      }
      if ((comparison_mask == KS_NES_PPU_MASK_SPRITES_COMBINED) ||
          (wp->draw_ctx.ppu_scanline_regs[i].ppumask_flags & KS_NES_PPU_MASK_SHOW_SPRITES) == 0) {
        wp->draw_ctx.scanline_y_coords[ret++] = i;
      }
      comparison_mask =
          wp->draw_ctx.ppu_scanline_regs[i].ppumask_flags & KS_NES_PPU_MASK_SPRITES_COMBINED;
    }
  }

  if (ret & 1) {
    wp->draw_ctx.scanline_y_coords[ret] = i - wp->draw_ctx.scanline_y_coords[ret - 1];
    ret++;
  }
  return ret;
}

void ksNesDrawOBJ(ksNesCommonWorkObj* wp, ksNesStateObj* state, u32 sprite_priority_pass) {
  u32 size =
      wp->chr_to_i8_buf_size <= CHR_TO_I8_BUF_SIZE ? wp->chr_to_i8_buf_size : CHR_TO_I8_BUF_SIZE;

  GXTexObj patternTex;
  GXTexObj bankTex;
  GXTexObj indirectTex;
  u32 i;
  u32 j;

  if (sprite_priority_pass == 0) {
    u32 i;
    u32 j;

    for (i = 0; i < KS_NES_SCANLINE_SPRITE_OVERDRAW_COUNT; i++) {
      wp->draw_ctx.sprite_scanline_limit[i] =
          i < KS_NES_SCANLINE_COUNT
              ? ((state->frame_flags & KS_NES_FLAG_NINES_OVER_MODE) ? 255
                                                                    : KS_NES_SPRITES_PER_SCANLINE)
              : 0;

      if ((wp->draw_ctx.ppu_scanline_regs[i].ppumask_flags & KS_NES_PPU_MASK_SHOW_SPRITES) == 0) {
        wp->draw_ctx.sprite_scanline_limit[i] = 0;
      }
    }

    memset(wp->draw_ctx.sprite_quad_data, 0, sizeof(wp->draw_ctx.sprite_quad_data));
    int textureY;
    int textureStep;
    int pairCount;
    int height;
    ksNesSpriteQuadData* quad_p = wp->draw_ctx.sprite_quad_data;
    int idx2;

    idx2 = 0;
    for (i = 0; i < sizeof(wp->draw_ctx.OAMTable); i += 4) {
      height = 8; // 8x8 sprite
      if (wp->draw_ctx.ppu_scanline_regs[wp->draw_ctx.OAMTable[i + 0]].ppu_ctrl &
          KS_NES_PPU_CTRL_SPRITE_SIZE) {
        height = 16; // 8x16 sprite
      }
      pairCount = 0;
      if (wp->draw_ctx.OAMTable[i + 2] & KS_NES_OAM_ATTR_FLIP_VERTICAL) {
        textureY = height << 2;
        textureStep = -4;
      } else {
        textureY = 0;
        textureStep = 4;
      }
      for (j = 0; j < height; j++) {
        if (wp->draw_ctx.sprite_scanline_limit[wp->draw_ctx.OAMTable[i + 0] + j] != 0) {
          wp->draw_ctx.sprite_scanline_limit[wp->draw_ctx.OAMTable[i + 0] + j]--;
          if ((pairCount & 2) == 0) {
            quad_p->y_and_v_pairs[pairCount] = j + wp->draw_ctx.OAMTable[i + 0];
            quad_p->y_and_v_pairs[pairCount + 1] = textureY;
            pairCount += 2;
          }
        } else if ((pairCount & 2) != 0) {
          quad_p->y_and_v_pairs[pairCount] = j + wp->draw_ctx.OAMTable[i + 0];
          quad_p->y_and_v_pairs[pairCount + 1] = textureY;
          pairCount += 2;
        }
        textureY += textureStep;
      }
      if ((pairCount & 2) != 0) {
        quad_p->y_and_v_pairs[pairCount] = j + wp->draw_ctx.OAMTable[i + 0];
        quad_p->y_and_v_pairs[pairCount + 1] = textureY;
        pairCount += 2;
      }
      wp->draw_ctx.sprite_vertex_count[i >> 2] = pairCount;
      quad_p++;
      idx2++;
    }
  }
  GXSetNumChans(1);
  GXSetNumTexGens(2);
  GXSetNumTevStages(3);
  GXSetNumIndStages(2);
  GXSetBlendMode(GX_BM_LOGIC, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_COPY);
  GXClearVtxDesc();
  GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
  GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
  GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
  GXSetVtxDesc(GX_VA_TEX1, GX_DIRECT);
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_S16, 0);
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U16, 10);
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX1, GX_TEX_ST, GX_U16, 10);
  GXInitTexObj(&patternTex, wp->chr_to_u8_bufp, 0x400, static_cast< u16 >(size >> 10), GX_TF_I8,
               GX_MIRROR, GX_CLAMP, 0);
  GXInitTexObjLOD(&patternTex, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
  GXLoadTexObj(&patternTex, GX_TEXMAP0);
  GXInitTexObj(&bankTex, wp->draw_ctx.sprite_chr_bank_lut, 8, 4, GX_TF_IA8, GX_CLAMP, GX_CLAMP, 0);

  GXInitTexObjLOD(&bankTex, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
  GXLoadTexObj(&bankTex, GX_TEXMAP1);
  GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, 0x3c);
  GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_TRUE, 0x100, 0x101);
  GXSetTexCoordBias(GX_TEXCOORD0, 0, 0);
  GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD0, GX_TEXMAP1);
  GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_8, GX_ITS_1);
  GXSetTevIndirect(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_NONE, GX_ITM_OFF, GX_ITW_OFF,
                   GX_ITW_OFF, GX_FALSE, GX_FALSE, GX_ITBA_OFF);
  GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEX_DISABLE, GX_COLOR0A0);
  GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
  GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_RASC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
  GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG2);
  GXInitTexObj(&indirectTex, wp->draw_ctx.sprite_indirect_lut, 4, 16, GX_TF_IA8, GX_CLAMP, GX_CLAMP,
               0);
  GXInitTexObjLOD(&indirectTex, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
  GXLoadTexObj(&indirectTex, GX_TEXMAP2);
  GXSetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, 0x3c);
  GXSetTexCoordScaleManually(GX_TEXCOORD1, GX_TRUE, 0x100, 0x101);
  GXSetTexCoordBias(GX_TEXCOORD1, 0, 0);
  GXSetIndTexOrder(GX_INDTEXSTAGE1, GX_TEXCOORD1, GX_TEXMAP2);
  GXSetIndTexCoordScale(GX_INDTEXSTAGE1, GX_ITS_8, GX_ITS_1);
  {

    for (i = 0; size > (0x8000 << i); i++) {
    }

    static f32 indtexmtx_obj[2][3] = {{0.5f, 0.f, 0.f}, {0.f, 0.0625f, 0.f}};
    indtexmtx_obj[0][0] = 0.5f / (i >= 4 ? (1 << (i - 3)) : 1);
    indtexmtx_obj[1][1] = 0.5f / (i <= 2 ? (1 << (3 - i)) : 1);
    GXSetIndTexMtx(GX_ITM_0, indtexmtx_obj, 36 + (i < 4 ? 0 : i - 3));
  }
  GXSetTevIndirect(GX_TEVSTAGE1, GX_INDTEXSTAGE1, GX_ITF_8, GX_ITB_NONE, GX_ITM_0, GX_ITW_OFF,
                   GX_ITW_0, GX_TRUE, GX_FALSE, GX_ITBA_OFF);
  GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP0, GX_COLOR_NULL);
  GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_C2, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
  GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
  GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
  GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
  GXSetTevDirect(GX_TEVSTAGE2);
  GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
  static const GXColor color_thres = {255, 1, 0, 0};
  GXSetTevColor(GX_TEVREG0, color_thres);
  GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_C0, GX_CC_C2, GX_CC_CPREV, GX_CC_ZERO);
  GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_COMP_GR16_GT, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
                  GX_TEVPREV);
  GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_APREV, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
  GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
  GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_ALWAYS, 0);

  u32 t1_2, y1;
  u32 s0, t1, y0, color;
  u32 t0;
  u32 x0, x1;
  u32 s1, s1_2;
  u32 oam_attrs;
  u32 tile_index, flags2;
  u8* quad_pairs;

  u32 vertexCount = 0;
  i = vertexCount;
  for (; i < sizeof(wp->draw_ctx.OAMTable); i += sizeof(ksNesOAMEntry)) {

    if (sprite_priority_pass == 0 ||
        (wp->draw_ctx.OAMTable[i + 2] & KS_NES_OAM_ATTR_PRIORITY) != 0) {
      vertexCount += wp->draw_ctx.sprite_vertex_count[i >> 2];
    }
  }

  u32 idx = sizeof(wp->draw_ctx.OAMTable) - sizeof(ksNesOAMEntry);
  u32 idx2 = KS_NES_OAM_TABLE_SIZE - 1;
  GXBegin(GX_QUADS, GX_VTXFMT0, vertexCount);
  while (TRUE) {

    quad_pairs = wp->draw_ctx.sprite_quad_data[idx2].y_and_v_pairs;
    tile_index = wp->draw_ctx.OAMTable[idx + 1];

    if (wp->draw_ctx.ppu_scanline_regs[wp->draw_ctx.OAMTable[idx + 0]].ppu_ctrl &
        KS_NES_PPU_CTRL_SPRITE_SIZE) {

      flags2 = wp->draw_ctx.ppu_scanline_regs[wp->draw_ctx.OAMTable[idx + 0]]
                   .chr_bank_sprite[(tile_index >> 6) | ((tile_index & KS_NES_OAM_TILE_BANK) << 2)];
      tile_index &= KS_NES_OAM_TILE_IDX;
    } else {

      flags2 =
          wp->draw_ctx.ppu_scanline_regs[wp->draw_ctx.OAMTable[idx + 0]].chr_bank_sprite
              [(tile_index >> 6) |
               ((wp->draw_ctx.ppu_scanline_regs[wp->draw_ctx.OAMTable[idx + 0]].ppu_ctrl >> 1) &
                4)];
    }

    oam_attrs = wp->draw_ctx.OAMTable[idx + 2];
    x0 = wp->draw_ctx.OAMTable[idx + 3] + 128;
    x1 = wp->draw_ctx.OAMTable[idx + 3] + 136;
    color = (((oam_attrs & KS_NES_OAM_ATTR_PALETTE_MASK) * 16 + 4) * 0x01000000);

    if (sprite_priority_pass != 0) {

      if ((oam_attrs & KS_NES_OAM_ATTR_PRIORITY) == 0) {
        goto loop_condition;
      }
    } else {

      if ((oam_attrs & KS_NES_OAM_ATTR_PRIORITY) != 0) {
        color |= 0xFF010000;
      }
    }

    s0 = 0;
    t0 = (flags2 & 0xFE) * 2; // select chr bank offset
    tile_index = ((tile_index & 0x3F) * 0x20) |
                 ((flags2 & 0x01) * 0x800); // flags2 bit0 determines the pattern table

    if (wp->draw_ctx.OAMTable[idx + 2] & KS_NES_OAM_ATTR_FLIP_HORIZONTAL) {
      s1 = tile_index + 32;
      s1_2 = tile_index;
    } else {
      s1 = tile_index;
      s1_2 = tile_index + 32;
    }

    for (j = 0; j < wp->draw_ctx.sprite_vertex_count[idx >> 2]; j += 4) {
      y0 = -129 - quad_pairs[0];
      t1 = quad_pairs[1];
      y1 = -129 - quad_pairs[2];
      t1_2 = quad_pairs[3];

      quad_pairs += 4;
      GXPosition2s16(x0, y0);
      GXColor1u32(color);
      GXTexCoord2u16(s0, t0);
      GXTexCoord2u16(s1, t1);

      GXPosition2s16(x1, y0);
      GXColor1u32(color);
      GXTexCoord2u16(s0, t0);
      GXTexCoord2u16(s1_2, t1);

      GXPosition2s16(x1, y1);
      GXColor1u32(color);
      GXTexCoord2u16(s0, t0);
      GXTexCoord2u16(s1_2, t1_2);

      GXPosition2s16(x0, y1);
      GXColor1u32(color);
      GXTexCoord2u16(s0, t0);
      GXTexCoord2u16(s1, t1_2);
    }

  loop_condition:
    if (idx == 0) {
      break;
    }

    idx2--;
    idx -= 4;
  }

  GXEnd();

  if (sprite_priority_pass != 0) {
    u32 n = ksNesDrawMakeOBJBlankVtxList(wp);

    if (n != 0) {
      GXSetNumChans(1);
      GXSetNumTexGens(0);
      GXSetNumTevStages(1);
      GXSetNumIndStages(0);
      GXSetBlendMode(GX_BM_LOGIC, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_COPY);
      GXClearVtxDesc();
      GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
      GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
      GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_S16, 0);
      GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
      GXSetTevDirect(GX_TEVSTAGE0);
      GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
      GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                    GX_AF_NONE);
      GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
      GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);

      GXBegin(GX_QUADS, GX_VTXFMT0, n * 2);
      for (u32 i = 0; i < n; i += 2) {
        s16 x1 = (wp->draw_ctx.ppu_scanline_regs[wp->draw_ctx.scanline_y_coords[i]].ppumask_flags &
                  KS_NES_PPU_MASK_SPRITES_COMBINED) == KS_NES_PPU_MASK_SHOW_SPRITES
                     ? 136
                     : 384;

        GXPosition2s16(128, -128 - wp->draw_ctx.scanline_y_coords[i]);
        GXColor1u32(0x00000000);
        GXPosition2s16(x1, -128 - wp->draw_ctx.scanline_y_coords[i]);
        GXColor1u32(0x00000000);
        GXPosition2s16(x1, -128 - wp->draw_ctx.scanline_y_coords[i] -
                               wp->draw_ctx.scanline_y_coords[i + 1]);
        GXColor1u32(0x00000000);
        GXPosition2s16(128, -128 - wp->draw_ctx.scanline_y_coords[i] -
                                wp->draw_ctx.scanline_y_coords[i + 1]);
        GXColor1u32(0x00000000);
      }
      GXEnd();
    }
  }
}

static inline u32 ksNesDrawMakeOBJAppearVtxList(ksNesCommonWorkObj* wp) {
  u32 ret = 0;
  u32 comparison_mask = 0;
  int i;

  for (i = 8; i < KS_NES_SCANLINE_COUNT; i++) {
    int bMask = wp->draw_ctx.ppu_scanline_regs[i].ppumask_flags & KS_NES_PPU_MASK_SPRITES_COMBINED;
    if (bMask != comparison_mask &&
        ((bMask != 0) || (comparison_mask != KS_NES_PPU_MASK_SHOW_SPRITES_LEFT)) &&
        (bMask != KS_NES_PPU_MASK_SHOW_SPRITES_LEFT || comparison_mask != 0)) {
      if ((ret & 1) != 0) {
        wp->draw_ctx.scanline_y_coords[ret] = i - wp->draw_ctx.scanline_y_coords[ret - 1];
        ret++;
      }
      if ((wp->draw_ctx.ppu_scanline_regs[i].ppumask_flags & KS_NES_PPU_MASK_SHOW_SPRITES)) {
        wp->draw_ctx.scanline_y_coords[ret++] = i;
      }
      comparison_mask =
          wp->draw_ctx.ppu_scanline_regs[i].ppumask_flags & KS_NES_PPU_MASK_SPRITES_COMBINED;
    }
  }

  if (ret & 1) {
    wp->draw_ctx.scanline_y_coords[ret] = i - wp->draw_ctx.scanline_y_coords[ret - 1];
    ret++;
  }
  return ret;
}

static inline void ksNesDrawFlushEFBToRed8(u8* buf) {
  static const GXColor black = {0, 0, 0, 0};
  GXSetTexCopySrc(128, 136, 256, 228);
  GXSetTexCopyDst(256, 228, GX_CTF_R8, GX_FALSE);
  GXSetCopyClear(black, 0xffffff);
  GXCopyTex(buf, GX_FALSE);
  GXPixModeSync();
  GXInvalidateTexAll();
}

void ksNesDrawOBJI8ToEFB(ksNesCommonWorkObj* wp, u8* buf) {
  GXTexObj obj;
  u32 i;
  u32 x;
  u32 cnt;

  GXSetNumChans(0);
  GXSetNumTexGens(1);
  GXSetNumTevStages(1);
  GXSetNumIndStages(0);
  GXSetBlendMode(GX_BM_LOGIC, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_COPY);

  GXClearVtxDesc();
  GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
  GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_S16, 0);
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U16, 8);

  GXInitTexObj(&obj, buf, 256, 228, GX_TF_I8, GX_CLAMP, GX_CLAMP, 0);
  GXInitTexObjLOD(&obj, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
  GXLoadTexObj(&obj, GX_TEXMAP0);

  GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, 60);
  GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_TRUE, 0x100, 0x100);
  GXSetTexCoordBias(GX_TEXCOORD0, 0, 0);

  GXSetTevDirect(GX_TEVSTAGE0);
  GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);

  GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_ALWAYS, 0);

  cnt = ksNesDrawMakeOBJAppearVtxList(wp);
  if (cnt == 0)
    return;

  GXBegin(GX_QUADS, GX_VTXFMT0, cnt * 2);

  for (i = 0; i < cnt; i += sizeof(ksNesScanlineYCoords)) {
    ksNesScanlineYCoords* scanline_y_coords =
        KS_NES_TYPE_FROM_DRAW_CTX_SCANLINE_BUF_OFS(ksNesScanlineYCoords, wp->draw_ctx, i);
    x = (wp->draw_ctx
             .ppu_scanline_regs[static_cast< u8 >(
                 KS_NES_TYPE_FROM_DRAW_CTX_SCANLINE_BUF_OFS(ksNesScanlineYCoords, wp->draw_ctx, i)
                     ->top)]
             .ppumask_flags &
         KS_NES_PPU_MASK_SPRITES_COMBINED) == KS_NES_PPU_MASK_SHOW_SPRITES
            ? 8
            : 0;

    GXPosition2s16(x + 0x80, -128 - scanline_y_coords->top);
    GXTexCoord2u16(x, scanline_y_coords->top - 8);

    GXPosition2s16(0x180, -128 - scanline_y_coords->top);
    GXTexCoord2u16(0x100, scanline_y_coords->top - 8);

    GXPosition2s16(0x180, -128 - scanline_y_coords->top - scanline_y_coords->height);
    GXTexCoord2u16(0x100, scanline_y_coords->top + scanline_y_coords->height - 8);

    GXPosition2s16(x + 0x80, -128 - scanline_y_coords->top - scanline_y_coords->height);
    GXTexCoord2u16(x, scanline_y_coords->top + scanline_y_coords->height - 8);
  }

  GXEnd();
}

void ksNesDrawEmuResult(ksNesCommonWorkObj* wp) {
  static f32 indtexmtx[2][3] = {{0.5f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
  static const GXColor black = {0, 0, 0, 0};
  static const GXColor color0 = {0x3A, 0x3A, 0x3A, 0x00};
  static const GXColor color1 = {0x71, 0x71, 0x71, 0x00};
  static const GXColor color2 = {0x15, 0x15, 0x15, 0x00};

  u32 cnt;
  u32 i;
  u32 color_effects_state;
  u8 y;
  u32 val;
  u32 clr;
  ksNesScanlineYCoords* scanline_y_coords;
  GXTexObj obj;
  GXTexObj obj2;

  cnt = 0;
  for (i = 8, color_effects_state = 0xFF; i < KS_NES_SCANLINE_COUNT - 4; i++) {
    val = wp->draw_ctx.ppu_scanline_regs[i].ppumask_flags & KS_NES_PPU_MASK_COLOR_EFFECTS;
    if (val != color_effects_state) {
      color_effects_state = val;
      if ((cnt & 1) != 0) {
        wp->draw_ctx.scanline_y_coords[cnt++] = i - 8;
      }
      wp->draw_ctx.scanline_y_coords[cnt++] = i - 8;
    }
  }

  if ((cnt & 1) != 0) {
    wp->draw_ctx.scanline_y_coords[cnt++] = i - 8;
  }
  wp->draw_ctx.scanline_y_coords[cnt] = 0xFF;

  GXInitTexObj(&obj2, wp->result_bufp, KS_NES_WIDTH, KS_NES_HEIGHT, GX_TF_I8, GX_CLAMP, GX_CLAMP,
               0);
  GXInitTexObjLOD(&obj2, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
  GXLoadTexObj(&obj2, GX_TEXMAP1);

  // Sampling LUT for applying color effects like color emphasis (I think)
  GXInitTexObj(&obj, wp->draw_ctx.post_process_lut, 256, 4, GX_TF_RGB5A3, GX_CLAMP, GX_CLAMP, 0);
  GXInitTexObjLOD(&obj, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
  GXLoadTexObj(&obj, GX_TEXMAP0);

  GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, 60, GX_FALSE, 125);
  GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_TRUE, 256, 256);
  GXSetTexCoordBias(GX_TEXCOORD0, 0, 0);

  GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD0, GX_TEXMAP1);
  GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
  GXSetIndTexMtx(GX_ITM_0, indtexmtx, 1);

  GXSetTevIndirect(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_NONE, GX_ITM_0, GX_ITW_0,
                   GX_ITW_0, GX_FALSE, GX_FALSE, GX_ITBA_OFF);
  GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);

  GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);

  // Count non-grayscale scanlines
  cnt = 0;
  for (i = 0; wp->draw_ctx.scanline_y_coords[i] != 0xFF; i += sizeof(ksNesScanlineYCoords)) {
    val = wp->draw_ctx.ppu_scanline_regs[wp->draw_ctx.scanline_y_coords[i]].ppumask_flags;
    if ((val & KS_NES_PPU_MASK_COLOR_EFFECTS) == 0) {
      cnt += 4; // has color and no color emphasis
    }
  }

  // Draw non-grayscale scanlines
  if (cnt != 0) {
    GXSetNumChans(0);
    GXSetNumTexGens(1);
    GXSetNumTevStages(1);
    GXSetNumIndStages(1);

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_S16, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U16, 8);
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);

    GXBegin(GX_QUADS, GX_VTXFMT0, cnt);

    do {
      i -= sizeof(ksNesScanlineYCoords);
      scanline_y_coords =
          KS_NES_TYPE_FROM_DRAW_CTX_SCANLINE_BUF_OFS(ksNesScanlineYCoords, wp->draw_ctx, i);
      val = wp->draw_ctx.ppu_scanline_regs[scanline_y_coords->top].ppumask_flags &
            KS_NES_PPU_MASK_COLOR_EFFECTS;
      if (val == 0) {
        GXPosition2s16(KS_NES_WIDTH + KS_NES_CENTER_X, -KS_NES_CENTER_Y - scanline_y_coords->top);
        GXTexCoord2u16(256, scanline_y_coords->top);

        GXPosition2s16(KS_NES_WIDTH + KS_NES_CENTER_X,
                       -KS_NES_CENTER_Y - scanline_y_coords->height);
        GXTexCoord2u16(256, scanline_y_coords->height);

        GXPosition2s16(KS_NES_CENTER_X, -KS_NES_CENTER_Y - scanline_y_coords->height);
        GXTexCoord2u16(0, scanline_y_coords->height);

        GXPosition2s16(KS_NES_CENTER_X, -KS_NES_CENTER_Y - scanline_y_coords->top);
        GXTexCoord2u16(0, scanline_y_coords->top);
      }
    } while (i != 0);
  }

  // Count grayscale scanlines
  cnt = 0;
  for (i = 0; wp->draw_ctx.scanline_y_coords[i] != 0xFF; i += 2) {
    val = wp->draw_ctx.ppu_scanline_regs[wp->draw_ctx.scanline_y_coords[i]].ppumask_flags;
    if ((val & KS_NES_PPU_MASK_COLOR_EFFECTS) != 0) {
      cnt += 4;
    }
  }

  // Draw grayscale scanlines
  if (cnt != 0) {
    GXSetNumChans(1);
    GXSetNumTexGens(1);
    GXSetNumTevStages(4);
    GXSetNumIndStages(1);

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_S16, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U16, 8);

    GXSetTevIndirect(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_NONE, GX_ITM_0, GX_ITW_0,
                     GX_ITW_0, GX_FALSE, GX_FALSE, GX_ITBA_OFF);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevIndirect(GX_TEVSTAGE1, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_NONE, GX_ITM_0, GX_ITW_0,
                     GX_ITW_0, GX_FALSE, GX_FALSE, GX_ITBA_OFF);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevIndirect(GX_TEVSTAGE2, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_NONE, GX_ITM_0, GX_ITW_0,
                     GX_ITW_0, GX_FALSE, GX_FALSE, GX_ITBA_OFF);
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);

    GXSetTevDirect(GX_TEVSTAGE3);
    GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);

    GXSetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_RED, GX_CH_RED, GX_CH_RED, GX_CH_ALPHA);
    GXSetTevSwapModeTable(GX_TEV_SWAP2, GX_CH_GREEN, GX_CH_GREEN, GX_CH_GREEN, GX_CH_ALPHA);
    GXSetTevSwapModeTable(GX_TEV_SWAP3, GX_CH_BLUE, GX_CH_BLUE, GX_CH_BLUE, GX_CH_ALPHA);

    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP1);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP2);
    GXSetTevSwapMode(GX_TEVSTAGE2, GX_TEV_SWAP0, GX_TEV_SWAP3);
    GXSetTevSwapMode(GX_TEVSTAGE3, GX_TEV_SWAP0, GX_TEV_SWAP0);

    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C0, GX_CC_ZERO);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C1, GX_CC_CPREV);
    GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C2, GX_CC_CPREV);
    GXSetTevColorIn(GX_TEVSTAGE3, GX_CC_RASC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);

    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE3, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);

    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

    GXSetTevColor(GX_TEVREG0, color0);
    GXSetTevColor(GX_TEVREG1, color1);
    GXSetTevColor(GX_TEVREG2, color2);

    GXBegin(GX_QUADS, GX_VTXFMT0, cnt);

    do {
      i -= sizeof(ksNesScanlineYCoords);
      scanline_y_coords =
          KS_NES_TYPE_FROM_DRAW_CTX_SCANLINE_BUF_OFS(ksNesScanlineYCoords, wp->draw_ctx, i);
      if ((wp->draw_ctx.ppu_scanline_regs[scanline_y_coords->top].ppumask_flags &
           KS_NES_PPU_MASK_COLOR_EFFECTS) != 0) {
        clr = 0x2F2F2F00; // If any of the grayscale color bits are set, force the color to RGB8
                          // (47, 47, 47) (chroma removed, only luma left)

        // For each emphasis bit set, add one to the luma value to get 48 (0x30).
        if ((wp->draw_ctx.ppu_scanline_regs[scanline_y_coords->top].ppumask_flags &
             KS_NES_PPU_MASK_EMPHASIZE_RED) != 0) {
          clr += 0x10000000;
        }
        if ((wp->draw_ctx.ppu_scanline_regs[scanline_y_coords->top].ppumask_flags &
             KS_NES_PPU_MASK_EMPHASIZE_GREEN) != 0) {
          clr += 0x00100000;
        }
        if ((wp->draw_ctx.ppu_scanline_regs[scanline_y_coords->top].ppumask_flags &
             KS_NES_PPU_MASK_EMPHASIZE_BLUE) != 0) {
          clr += 0x00001000;
        }

        GXPosition2s16(KS_NES_WIDTH + KS_NES_CENTER_X, -KS_NES_CENTER_Y - scanline_y_coords->top);
        GXColor1u32(clr);
        GXTexCoord2u16(256, scanline_y_coords->top);

        GXPosition2s16(KS_NES_WIDTH + KS_NES_CENTER_X,
                       -KS_NES_CENTER_Y - scanline_y_coords->height);
        GXColor1u32(clr);
        GXTexCoord2u16(256, scanline_y_coords->height);

        GXPosition2s16(KS_NES_CENTER_X, -KS_NES_CENTER_Y - scanline_y_coords->height);
        GXColor1u32(clr);
        GXTexCoord2u16(0, scanline_y_coords->height);

        GXPosition2s16(KS_NES_CENTER_X, -KS_NES_CENTER_Y - scanline_y_coords->top);
        GXColor1u32(clr);
        GXTexCoord2u16(0, scanline_y_coords->top);
      }
    } while (i != 0);
  }

  GXSetTexCopySrc(KS_NES_CENTER_X, KS_NES_CENTER_Y, KS_NES_WIDTH, KS_NES_HEIGHT);
  GXSetTexCopyDst(KS_NES_WIDTH, KS_NES_HEIGHT, GX_TF_RGB565, GX_FALSE);
  GXSetCopyClear(black, 0xFFFFFF);
  GXCopyTex(wp->result_bufp, GX_FALSE);
  GXPixModeSync();
}

void ksNesDraw(ksNesCommonWorkObj* wp, ksNesStateObj* state) {
  ksNesDrawInit(wp);
  ksNesDrawClearEFBFirst(wp);
  ksNesDrawMakeBGIndTex(wp, 0);
  PPCSync();
  ksNesDrawOBJ(wp, state, 0);
  ksNesDrawFlushEFBToRed8(wp->result_bufp);
  ksNesDrawOBJ(wp, state, 1);
  ksNesDrawBG(wp, state);
  ksNesDrawOBJI8ToEFB(wp, wp->result_bufp);
  ksNesDrawFlushEFBToRed8(wp->result_bufp);
  ksNesDrawEmuResult(wp);
  ksNesDrawEnd();
}
