#!/usr/bin/env python3

###
# Generates build files for the project.
# This file also includes the project configuration,
# such as compiler flags and the object matching status.
#
# Usage:
#   python3 configure.py
#   ninja
#
# Append --help to see available options.
###

import argparse
import sys
from pathlib import Path
from typing import Any, Dict, List

from tools.project import (
    Object,
    ProgressCategory,
    ProjectConfig,
    calculate_progress,
    generate_build,
    is_windows,
)

# Game versions
DEFAULT_VERSION = 1
VERSIONS = [
    "GM8EAB_00",  # Multi-Game Demo Disc v7-9
    "GM8E01_00",  # mp-v1.088 NTSC-U
    "GM8E01_01",  # mp-v1.093 NTSC-U
    "GM8E01_48",  # mp-v1.097 NTSC-K
    "GM8E01_02",  # mp-v1.111 NTSC-U
    "GM8P01_00",  # mp-v1.110 PAL
    "GM8J01_00",  # mp-v1.111 NTSC-J
    "R3IJ01_00",  # mp-v3.570 New Play Controls
    "R3ME01_00",  # mp-v3.593 Trilogy NTSC
    "R3MP01_00",  # mp-v3.629 Trilogy PAL
]
RSTL_VERSIONS = {
    "GM8EAB_00": 0,
    "GM8E01_00": 1,
    "GM8E01_01": 2,
    "GM8E01_48": 3,
    "GM8E01_02": 4,
    "GM8P01_00": 5,
    "GM8J01_00": 6,
    "R3IJ01_00": 30,
    "R3ME01_00": 40,
    "R3MP01_00": 41,
}
NTSC_GC_VERSIONS = ["GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8E01_02"]

parser = argparse.ArgumentParser()
parser.add_argument(
    "mode",
    choices=["configure", "progress"],
    default="configure",
    help="script mode (default: configure)",
    nargs="?",
)
parser.add_argument(
    "-v",
    "--version",
    choices=VERSIONS,
    type=str.upper,
    default=VERSIONS[DEFAULT_VERSION],
    help="version to build",
)
parser.add_argument(
    "--build-dir",
    metavar="DIR",
    type=Path,
    default=Path("build"),
    help="base build directory (default: build)",
)
parser.add_argument(
    "--binutils",
    metavar="BINARY",
    type=Path,
    help="path to binutils (optional)",
)
parser.add_argument(
    "--compilers",
    metavar="DIR",
    type=Path,
    help="path to compilers (optional)",
)
parser.add_argument(
    "--map",
    action="store_true",
    help="generate map file(s)",
)
parser.add_argument(
    "--debug",
    action="store_true",
    help="build with debug info (non-matching)",
)
if not is_windows():
    parser.add_argument(
        "--wrapper",
        metavar="BINARY",
        type=Path,
        help="path to wibo or wine (optional)",
    )
parser.add_argument(
    "--dtk",
    metavar="BINARY | DIR",
    type=Path,
    help="path to decomp-toolkit binary or source (optional)",
)
parser.add_argument(
    "--objdiff",
    metavar="BINARY | DIR",
    type=Path,
    help="path to objdiff-cli binary or source (optional)",
)
parser.add_argument(
    "--sjiswrap",
    metavar="EXE",
    type=Path,
    help="path to sjiswrap.exe (optional)",
)
parser.add_argument(
    "--ninja",
    metavar="BINARY",
    type=Path,
    help="path to ninja binary (optional)",
)
parser.add_argument(
    "--verbose",
    action="store_true",
    help="print verbose output",
)
parser.add_argument(
    "--non-matching",
    dest="non_matching",
    action="store_true",
    help="builds equivalent (but non-matching) or modded objects",
)
parser.add_argument(
    "--warn",
    dest="warn",
    type=str,
    choices=["all", "off", "error"],
    help="how to handle warnings",
)
parser.add_argument(
    "--no-progress",
    dest="progress",
    action="store_false",
    help="disable progress calculation",
)
args = parser.parse_args()

config = ProjectConfig()
config.version = str(args.version)
version_num = VERSIONS.index(config.version)

# Apply arguments
config.build_dir = args.build_dir
config.dtk_path = args.dtk
config.objdiff_path = args.objdiff
config.binutils_path = args.binutils
config.compilers_path = args.compilers
config.generate_map = args.map
config.non_matching = args.non_matching
config.sjiswrap_path = args.sjiswrap
config.ninja_path = args.ninja
config.progress = args.progress
if not is_windows():
    config.wrapper = args.wrapper
# Don't build asm unless we're --non-matching
if not config.non_matching:
    config.asm_dir = None

# Tool versions
config.binutils_tag = "2.42-1"
config.compilers_tag = "20251118"
config.dtk_tag = "v1.8.4"
config.objdiff_tag = "v3.7.0"
config.sjiswrap_tag = "v1.2.2"
config.wibo_tag = "1.0.3"

# Project
config.config_path = Path("config") / config.version / "config.yml"
config.check_sha_path = Path("config") / config.version / "build.sha1"
config.asflags = [
    "-mgekko",
    "--strip-local-absolute",
    "-I include",
    f"-I build/{config.version}/include",
    f"--defsym version={version_num}",
]
config.ldflags = [
    "-fp hardware",
    "-nodefaults",
]
if args.debug:
    config.ldflags.append("-g")  # Or -gdwarf-2 for Wii linkers
if args.map:
    config.ldflags.append("-mapunused")
    # config.ldflags.append("-listclosure") # For Wii linkers

# Build the NES emulator module alongside the main executable.
config.build_rels = config.version in ("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02")

# Use for any additional files that should cause a re-configure when modified
config.reconfig_deps = []

# Optional numeric ID for decomp.me preset
# Can be overridden in libraries or objects
config.scratch_preset_id = None

# Base flags, common to most GC/Wii games.
# Generally leave untouched, with overrides added below.
cflags_base = [
    "-nodefaults",
    "-proc gekko",
    "-align powerpc",
    "-enum int",
    "-fp hardware",
    "-Cpp_exceptions off",
    # "-W all",
    "-O4,p",
    "-inline auto",
    '-pragma "cats off"',
    '-pragma "warn_notinlined off"',
    "-maxerrors 1",
    "-nosyspath",
    "-RTTI off",
    "-fp_contract on",
    "-str reuse",
    "-i include",
    "-i extern/sdk/include",
    "-i extern/sdk/libc",
    f"-i build/{config.version}/include",
    f"-DVERSION={version_num}",
    f"-DRSTL_VERSION={RSTL_VERSIONS[config.version]}",
    "-DPRIME1",
    "-DNONMATCHING=0",
]

# Debug flags
if args.debug:
    # Or -sym dwarf-2 for Wii compilers
    cflags_base.extend(["-sym on", "-DDEBUG=1"])
else:
    cflags_base.append("-DNDEBUG=1")

# Warning flags
if args.warn == "all":
    cflags_base.append("-W all")
elif args.warn == "off":
    cflags_base.append("-W off")
elif args.warn == "error":
    cflags_base.append("-W error")

# Dolphin flags
cflags_dolphin = [
    *cflags_base,
    "-multibyte",
    "-fp_contract off",
]

# Metrowerks library flags
cflags_runtime = [
    *cflags_base,
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
    "-gccinc",
    "-common off",
    "-char signed",
    "-inline deferred,auto",
]

cflags_retro = [
    "-nodefaults",
    "-proc gekko",
    "-align powerpc",
    "-enum int",
    "-fp hardware",
    "-Cpp_exceptions off",
    # "-W all",
    "-O4,p",
    "-maxerrors 1",
    "-nosyspath",
    "-RTTI off",
    "-str reuse",
    "-i include",
    "-i extern/sdk/include",
    "-i extern/sdk/libc",
    "-i extern/zlib-1.1.3",
    f"-i build/{config.version}/include",
    f"-DVERSION={version_num}",
    f"-DRSTL_VERSION={RSTL_VERSIONS[config.version]}",
    "-DPRIME1",
    "-DNONMATCHING=0",
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
    "-gccinc",
    "-i extern/musyx/include",
    "-i extern/rstl/include",
    # "-sym on",
    "-DMUSY_TARGET=MUSY_TARGET_DOLPHIN",
]

mw_version_retro = "GC/1.3.2"
if version_num >= VERSIONS.index("R3IJ01_00"):
    mw_version_retro = "Wii/1.0a"
    cflags_retro.extend([
        "-sdata 4",
        "-sdata2 4",
        "-func_align 4",
        "-inline noauto,nobottomup,level=8",
        "-common off",
    ])
else:
    cflags_retro.extend([
        "-fp_contract on",
        "-inline deferred",
        "-common on",
    ])

# Most Retro code uses this inline limit. Objects that still need the compiler
# default retain cflags_retro explicitly while their helper inlining is investigated.
retro_inline_max_size = 250 if version_num < VERSIONS.index("GM8P01_00") else 125
cflags_retro_inline = [*cflags_retro, f'-pragma "inline_max_size({retro_inline_max_size})"']

cflags_musyx = [
    "-proc gekko",
    "-nodefaults",
    "-nosyspath",
    "-i include",
    "-i extern/sdk/include",
    "-i extern/musyx/include",
    "-i extern/sdk/libc",
    "-inline auto,depth=4",
    "-O4,p",
    "-fp hard",
    "-enum int",
    "-sym on",
    "-Cpp_exceptions off",
    "-str reuse,pool,readonly",
    "-fp_contract off",
    "-DMUSY_TARGET=MUSY_TARGET_DOLPHIN",
]

cflags_musyx_debug = [
    "-proc gecko",
    "-fp hard",
    "-nodefaults",
    "-nosyspath",
    "-i include",
    "-i extern/sdk/include",
    "-i extern/musyx/include",
    "-i extern/sdk/libc",
    "-g",
    "-inline off",
    "-sym on",
    "-D_DEBUG=1",
    "-fp hard",
    "-enum int",
    "-Cpp_exceptions off",
    "-DMUSY_TARGET=MUSY_TARGET_DOLPHIN",
]

# REL flags
cflags_rel = [
    "-proc gecko",
    "-fp hard",
    "-nodefaults",
    "-nosyspath",
    "-i include",
    "-i extern/sdk/libc",
    "-O0",
    "-sdata 0",
    "-sdata2 0",
    "-str noreuse",
    "-Cpp_exceptions off",
]

# MetroTRK flags
cflags_trk = [
    *cflags_base,
    "-use_lmw_stmw on",
    "-pool off",
    "-sdata 0",
    "-sdata2 0",
    "-inline on,noauto",
    "-rostr",
]

config.linker_version = "GC/1.3.2"


# Helper function for Dolphin libraries
def DolphinLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "src_dir": "extern/sdk",
        "mw_version": "GC/1.2.5n",
        "cflags": cflags_base,
        "progress_category": "sdk",
        "objects": objects,
    }


def TrkLib(lib_name, objects):
    return {
        "lib": lib_name + "D" if args.debug else "",
        "mw_version": "GC/1.2.5n",
        "cflags": cflags_trk,
        "progress_category": "sdk",
        "objects": objects,
        "shift_jis": True,
    }


def RetroLib(lib_name, progress_category, objects):
    return {
        "lib": lib_name + "CW" + "D" if args.debug else "",
        "mw_version": mw_version_retro,
        "cflags": cflags_retro_inline,
        "progress_category": progress_category,
        "objects": objects,
        "shift_jis": False,
    }


def KyotoLib(lib_name, progress_category, objects):
    return {
        "lib": lib_name + "CW" + "D" if args.debug else "",
        "mw_version": mw_version_retro,
        "cflags": cflags_retro_inline,
        "host": False,
        "progress_category": progress_category,
        "objects": objects,
        "shift_jis": False,
    }


def MusyX(
        objects,
        mw_version="GC/1.3.2",
        debug=False,
        major=2,
        minor=0,
        patch=3 if config.version == "R3ME01_00" else 0,
):
    cflags = cflags_musyx if not debug else cflags_musyx_debug
    return {
        "lib": "musyx",
        "mw_version": mw_version,
        "src_dir": "extern/musyx/src",
        "cflags": [
            *cflags,
            f"-DMUSY_VERSION_MAJOR={major}",
            f"-DMUSY_VERSION_MINOR={minor}",
            f"-DMUSY_VERSION_PATCH={patch}",
        ],
        "progress_category": "third_party",
        "objects": objects,
        "shift_jis": False,
    }


# Helper function for REL script objects
def Rel(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/1.3.2",
        "cflags": cflags_rel,
        "progress_category": "third_party",
        "objects": objects,
        "shift_jis": False,
    }


Matching = True  # Object matches and should be linked
NonMatching = False  # Object does not match and should not be linked
Equivalent = config.non_matching  # Object should be linked when configured with --non-matching


# Object is only matching for specific versions
def MatchingFor(*versions):
    return config.version in versions


def EquivalentFor(*versions):
    return config.version in versions and config.non_matching


config.warn_missing_config = True
config.warn_missing_source = False
config.libs = [
    TrkLib(
        "TRK_MINNOW_DOLPHIN",
        [
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00"), "MetroTRK/nubinit.c"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroTRK/mslsupp.c"
            ),
        ],
    ),
    RetroLib(
        "MetroidPrime",
        "game",
        [
            Object(
                EquivalentFor("GM8E01_00"),
                "MetroidPrime/main.cpp",
                extra_cflags=['-pragma "inline_max_size(245)"']
                if version_num < VERSIONS.index("GM8P01_00")
                else [],
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00"),
                   "MetroidPrime/Cameras/CCameraManager.cpp"),
            Object(
                NonMatching,
                "MetroidPrime/CAimingCursor.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"), "MetroidPrime/CControlMapper.cpp"
            ),
            Object(EquivalentFor("GM8E01_00"), "MetroidPrime/Cameras/CFirstPersonCamera.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/CObjectList.cpp",
            ),
            Object(
                NonMatching,
                "MetroidPrime/Player/CPlayer.cpp",
            ),
            Object(MatchingFor("R3ME01_00"), "MetroidPrime/Player/CTrilogyOptions.cpp"),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/CAxisAngle.cpp"
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "MetroidPrime/CEulerAngles.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                   "MetroidPrime/CMatrix3f_Ext.cpp"),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS),
                "MetroidPrime/CArchMsgParmUserInput.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/CFrontEndUI.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/CInputGenerator.cpp",
                cflags=cflags_retro,
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"), "MetroidPrime/CMainFlow.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/CMFGame.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/CCredits.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/CSplashScreen.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00"),
                "MetroidPrime/CAnimData.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00"),
                "MetroidPrime/CAnimRes.cpp",
            ),
            Object(NonMatching, "MetroidPrime/Factories/CCharacterFactory.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/Factories/CAssetFactory.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00", "R3ME01_00"),
                "MetroidPrime/Tweaks/CTweakPlayer.cpp",
            ),
            Object(NonMatching, "MetroidPrime/Tweaks/CTweaks.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/Tweaks/CTweakGame.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"),
                "MetroidPrime/Weapons/CGameProjectile.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00"),
                "MetroidPrime/Player/CPlayerGun.cpp",
            ),
            Object(NonMatching, "MetroidPrime/CStateManager.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00", "R3ME01_00"),
                "MetroidPrime/CEntity.cpp",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00", "R3ME01_00"),
                "MetroidPrime/CArchMsgParmInt32.cpp",
            ),
            Object(
                MatchingFor("GM8EAB_00", "GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00", "R3ME01_00"),
                "MetroidPrime/CArchMsgParmInt32Int32VoidPtr.cpp",
            ),
            Object(
                MatchingFor("GM8EAB_00", "GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00", "R3ME01_00"),
                "MetroidPrime/CArchMsgParmNull.cpp",
            ),
            Object(
                MatchingFor("GM8EAB_00", "GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00", "R3ME01_00"),
                "MetroidPrime/CArchMsgParmReal32.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00", "R3ME01_00"),
                   "MetroidPrime/Decode.cpp"),
            Object(NonMatching, "MetroidPrime/CIOWinManager.cpp"),
            Object(MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "MetroidPrime/CIOWin.cpp"),
            Object(
                EquivalentFor("GM8E01_00"),
                "MetroidPrime/CActor.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/CWorld.cpp"),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/Tweaks/CTweakParticle.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"), "MetroidPrime/Clamp_int.cpp"),
            Object(
                MatchingFor("GM8EAB_00", "GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00", "R3ME01_00"),
                "MetroidPrime/CArchMsgParmControllerStatus.cpp",
            ),
            Object(
                MatchingFor("GM8EAB_00", "GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/CExplosion.cpp"
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00"), "MetroidPrime/CEffect.cpp"),
            Object(NonMatching, "MetroidPrime/Cameras/CGameCamera.cpp"),
            Object(NonMatching, "MetroidPrime/CGameArea.cpp"),
            Object(NonMatching, "MetroidPrime/HUD/CSamusHud.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                   "MetroidPrime/CAnimationDatabaseGame.cpp"),
            Object(
                MatchingFor("GM8P01_00", "GM8J01_00"),
                "MetroidPrime/CTransitionDatabaseGame.cpp",
                extra_cflags=['-pragma "inline_max_size(126)"']
                if version_num >= VERSIONS.index("GM8P01_00")
                else [],
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/Tweaks/CTweakPlayerControl.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/Tweaks/CTweakPlayerGun.cpp",
                cflags=cflags_retro,
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/CPauseScreen.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/Tweaks/CTweakGui.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptActor.cpp"
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/ScriptObjects/CScriptTrigger.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptWaypoint.cpp",
            ),
            Object(NonMatching, "MetroidPrime/Enemies/CPatterned.cpp",
                   extra_cflags=['-pragma "inline_max_size(260)"']
                   if version_num == VERSIONS.index("GM8E01_02")
                   else [],
                   ),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/ScriptObjects/CScriptDoor.cpp"),
            Object(EquivalentFor("GM8E01_00"), "MetroidPrime/Enemies/CStateMachine.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/CMapArea.cpp"),
            Object(NonMatching, "MetroidPrime/Cameras/CBallCamera.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "MetroidPrime/ScriptObjects/CScriptEffect.cpp",
            ),
            Object(MatchingFor("GM8EAB_00", "GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/Weapons/CBomb.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/Tweaks/CTweakBall.cpp",
            ),
            Object(
                MatchingFor(
                    *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"
                ),
                "MetroidPrime/Player/CPlayerState.cpp",
                cflags=(
                    cflags_retro_inline
                    if version_num >= VERSIONS.index("GM8P01_00")
                    else cflags_retro
                ),
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptTimer.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/Cameras/CCinematicCamera.cpp"),
            Object(NonMatching, "MetroidPrime/CAutoMapper.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptCounter.cpp",
            ),
            Object(NonMatching, "MetroidPrime/CMapWorld.cpp"),
            Object(
                EquivalentFor("GM8E01_00"),
                "MetroidPrime/Enemies/CAi.cpp",
                extra_cflags=['-pragma "inline_max_size(260)"']
                if config.version == "GM8E01_02"
                else ['-pragma "inline_max_size(126)"']
                if version_num >= VERSIONS.index("GM8E01_02")
                else [],
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"),
                "MetroidPrime/Enemies/PatternedCastTo.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"),
                "MetroidPrime/TCastTo.cpp",
            ),
            Object(
                NonMatching,
                "MetroidPrime/TypesMatch.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptSound.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptPlatform.cpp",
            ),
            Object(MatchingFor("GM8EAB_00", "GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"), "MetroidPrime/UserNames.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptGenerator.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptCameraWaypoint.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/CGameLight.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"),
                "MetroidPrime/Tweaks/CTweakTargeting.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/Tweaks/CTweakAutoMapper.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/CParticleGenInfoGeneric.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/CParticleGenInfo.cpp",
            ),
            Object(EquivalentFor("GM8E01_00"), "MetroidPrime/CParticleDatabase.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/Tweaks/CTweakGunRes.cpp",
                cflags=cflags_retro,
            ),
            Object(EquivalentFor("GM8E01_00"), "MetroidPrime/CTargetReticles.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_00", "GM8E01_48", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/CWeaponMgr.cpp",
                cflags=cflags_retro,
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS),
                "MetroidPrime/ScriptObjects/CScriptPickup.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "MetroidPrime/CDamageInfo.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/CMemoryDrawEnum.cpp",
            ),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptDock.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_48", "GM8E01_02"),
                   "MetroidPrime/ScriptObjects/CScriptCameraHint.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptLoader.cpp"),
            Object(NonMatching, "MetroidPrime/CSamusDoll.cpp"),
            Object(
                MatchingFor("GM8EAB_00", "GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/Factories/CStateMachineFactory.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/Weapons/CPlasmaBeam.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/Weapons/CPowerBeam.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8J01_00"), "MetroidPrime/Weapons/CWaveBeam.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/Weapons/CIceBeam.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "MetroidPrime/CScriptMailbox.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptRelay.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptSpawnPoint.cpp"
            ),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptRandomRelay.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CBeetle.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/HUD/CHUDMemoParms.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptHUDMemo.cpp",
            ),

            Object(
                Equivalent,
                "MetroidPrime/CMappableObject.cpp",
            ),
            Object(
                EquivalentFor("GM8E01_00"),
                "MetroidPrime/Player/CPlayerCameraBob.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptCameraFilterKeyframe.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptCameraBlurKeyframe.cpp",
            ),
            Object(NonMatching, "MetroidPrime/Cameras/CCameraFilter.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "MetroidPrime/Player/CMorphBall.cpp",
                extra_cflags=['-pragma "inline_max_size(100)"']
                if version_num >= VERSIONS.index("GM8P01_00")
                else [],
            ),
            Object(
                NonMatching, "MetroidPrime/ScriptObjects/CScriptDamageableTrigger.cpp"
            ),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptDebris.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptCameraShaker.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptActorKeyframe.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"),
                "MetroidPrime/CConsoleOutputWindow.cpp",
            ),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptWater.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"),
                "MetroidPrime/Weapons/CWeapon.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/CDamageVulnerability.cpp"),
            Object(EquivalentFor("GM8E01_00"), "MetroidPrime/CActorLights.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/Enemies/CPatternedInfo.cpp",
            ),
            Object(MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS), "MetroidPrime/CSimpleShadow.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/CActorParameters.cpp",
            ),
            Object(NonMatching, "MetroidPrime/CInGameGuiManager.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CWarWasp.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/CWorldShadow.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"), "MetroidPrime/CAudioStateWin.cpp"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "R3ME01_00"), "MetroidPrime/Player/CPlayerVisor.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"), "MetroidPrime/CModelData.cpp"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/CDecalManager.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptSpiderBallWaypoint.cpp"
            ),
            Object(NonMatching, "MetroidPrime/Enemies/CBloodFlower.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "MetroidPrime/TGameTypes.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"),
                "MetroidPrime/CPhysicsActor.cpp",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/CPhysicsState.cpp"
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "MetroidPrime/CRipple.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/CFluidUVMotion.cpp",
                extra_cflags=['-pragma "inline_max_size(250)"'],
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "MetroidPrime/CRippleManager.cpp",
                # TODO: inline ripple fill at the common limit.
                extra_cflags=['-pragma "inline_max_size(260)"'],
            ),
            Object(EquivalentFor("GM8E01_00"), "MetroidPrime/Player/CGrappleArm.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CSpacePirate.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"),
                "MetroidPrime/ScriptObjects/CScriptCoverPoint.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/Cameras/CPathCamera.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/CFluidPlane.cpp"),
            Object(NonMatching, "MetroidPrime/CFluidPlaneManager.cpp"),
            Object(
                MatchingFor("GM8EAB_00", "GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptGrapplePoint.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CHUDBillboardEffect.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/Enemies/CFlickerBat.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/BodyState/CBodyStateCmdMgr.cpp",
                cflags=[*cflags_retro, "-inline auto"],
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/BodyState/CBodyStateInfo.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "MetroidPrime/BodyState/CBSAttack.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/BodyState/CBSDie.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/BodyState/CBSFall.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/BodyState/CBSGetup.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/BodyState/CBSKnockBack.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/BodyState/CBSLieOnGround.cpp",
            ),
            Object(EquivalentFor("GM8E01_00"), "MetroidPrime/BodyState/CBSLocomotion.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/BodyState/CBSStep.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/BodyState/CBSTurn.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/BodyState/CBodyController.cpp",
                cflags=cflags_retro,
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/BodyState/CBSLoopAttack.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"),
                "MetroidPrime/Weapons/CTargetableProjectile.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/BodyState/CBSLoopReaction.cpp",
            ),
            Object(NonMatching, "MetroidPrime/CSteeringBehaviors.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/BodyState/CBSGroundHit.cpp",
            ),
            Object(
                NonMatching,
                "MetroidPrime/Enemies/CChozoGhost.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"),
                "MetroidPrime/Enemies/CFireFlea.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/BodyState/CBSSlide.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/BodyState/CBSHurled.cpp"),
            Object(EquivalentFor("GM8E01_00"), "MetroidPrime/BodyState/CBSJump.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/BodyState/CBSGenerate.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/Enemies/CPuddleSpore.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/BodyState/CBSTaunt.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "MetroidPrime/CSortedLists.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "MetroidPrime/ScriptObjects/CScriptDebugCameraWaypoint.cpp",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS),
                "MetroidPrime/ScriptObjects/CScriptSpiderBallAttractionSurface.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/BodyState/CBSScripted.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/Enemies/CPuddleToadGamma.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                   "MetroidPrime/ScriptObjects/CScriptDistanceFog.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/BodyState/CBSProjectileAttack.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"),
                "MetroidPrime/Weapons/CPowerBomb.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/Enemies/CMetaree.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptDockAreaChange.cpp",
            ),
            Object(
                NonMatching, "MetroidPrime/ScriptObjects/CScriptSpecialFunction.cpp"
            ),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptActorRotate.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/Player/CFidget.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "MetroidPrime/Enemies/CSpankWeed.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/Enemies/CParasite.cpp"
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"), "MetroidPrime/Player/CSamusFaceReflection.cpp"),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "R3ME01_00"),
                "MetroidPrime/ScriptObjects/CScriptPlayerHint.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "MetroidPrime/Enemies/CRipper.cpp",
            ),
            Object(EquivalentFor("GM8E01_00"), "MetroidPrime/Cameras/CCameraShakeData.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "MetroidPrime/ScriptObjects/CScriptPickupGenerator.cpp"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptPointOfInterest.cpp",
            ),
            Object(NonMatching, "MetroidPrime/Enemies/CDrone.cpp"),
            Object(EquivalentFor("GM8E01_00"), "MetroidPrime/CMapWorldInfo.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/Factories/CScannableObjectInfo.cpp",
                cflags=cflags_retro,
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM801_00", "GM8E01_02"), "MetroidPrime/Enemies/CMetroid.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/Player/CScanDisplay.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptSteam.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptRipple.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/CBoneTracking.cpp"),
            Object(MatchingFor("GM8EAB_00", "GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"), "MetroidPrime/Player/CFaceplateDecoration.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/BodyState/CBSCover.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "MetroidPrime/ScriptObjects/CScriptBallTrigger.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/Weapons/CPlasmaProjectile.cpp"),
            Object(
                NonMatching,
                "MetroidPrime/Player/CPlayerOrbit.cpp",
            ),
            Object(EquivalentFor("GM8E01_00"), "MetroidPrime/CGameCollision.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/CBallFilter.cpp",
                cflags=cflags_retro,
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/CAABoxFilter.cpp",
            ),
            Object(NonMatching, "MetroidPrime/CGroundMovement.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/Enemies/CNewIntroBoss.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/Weapons/CPhazonBeam.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptTargetingPoint.cpp",
            ),
            Object(EquivalentFor("GM8E01_00") or MatchingFor("GM8P01_00", "GM8J01_00"), "MetroidPrime/BodyState/CBSWallHang.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "MetroidPrime/ScriptObjects/CScriptEMPulse.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/HUD/CHudEnergyInterface.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/HUD/CHudFreeLookInterface.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"), "MetroidPrime/HUD/CHudHelmetInterface.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/HUD/CHudMissileInterface.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"), "MetroidPrime/HUD/CHudRadarInterface.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "MetroidPrime/HUD/CHudThreatInterface.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/HUD/CHudVisorBeamMenu.cpp"),
            Object(NonMatching, "MetroidPrime/HUD/CHudBeamSelect.cpp"),
            Object(NonMatching, "MetroidPrime/HUD/CHudVisorSelect.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/HUD/CHudDecoInterface.cpp"),
            Object(
                NonMatching,
                "MetroidPrime/Weapons/CFlameThrower.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/Weapons/CBeamProjectile.cpp"),
            Object(NonMatching, "MetroidPrime/CFluidPlaneCPU.cpp"),
            Object(
                MatchingFor("GM8EAB_00"),
                "MetroidPrime/CFluidPlaneDoor.cpp",
                cflags=cflags_retro,
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptRoomAcoustics.cpp",
            ),
            Object(NonMatching, "MetroidPrime/Enemies/CIceSheegoth.cpp"),
            Object(EquivalentFor("GM8E01_00"), "MetroidPrime/CCollisionActorManager.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/CCollisionActor.cpp"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptPlayerActor.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/Tweaks/CTweakPlayerRes.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/Enemies/CBurstFire.cpp",
            ),
            Object(NonMatching, "MetroidPrime/Enemies/CFlaahgra.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "MetroidPrime/Player/CPlayerEnergyDrain.cpp"),
            Object(NonMatching, "MetroidPrime/CFlameWarp.cpp"),
            Object(NonMatching, "MetroidPrime/Weapons/CIceImpact.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/GameObjectLists.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"), "MetroidPrime/Weapons/CAuxWeapon.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"),
                   "MetroidPrime/Weapons/CGunWeapon.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptAreaAttributes.cpp",
            ),
            Object(NonMatching, "MetroidPrime/Weapons/CWaveBuster.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "MetroidPrime/Player/CStaticInterference.cpp"),
            Object(EquivalentFor("GM8E01_00"), "MetroidPrime/Enemies/CMetroidBeta.cpp"),
            Object(NonMatching, "MetroidPrime/PathFinding/CPathFindSearch.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/PathFinding/CPathFindRegion.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "MetroidPrime/PathFinding/CPathFindArea.cpp",
                cflags=[*cflags_retro, "-inline auto"],
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/PathFinding/CPathFindSpline.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/Weapons/GunController/CGunController.cpp",
            ),
            Object(NonMatching, "MetroidPrime/Weapons/GunController/CGSFreeLook.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/Weapons/GunController/CGSComboFire.cpp",
            ),
            Object(NonMatching, "MetroidPrime/HUD/CHudBallInterface.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/Tweaks/CTweakGuiColors.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/ScriptObjects/CFishCloud.cpp"),
            Object(
                MatchingFor("GM8EAB_00", "GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"), "MetroidPrime/CHealthInfo.cpp"
            ),
            Object(
                NonMatching,
                "MetroidPrime/Player/CGameState.cpp",
                extra_cflags=['-pragma "inline_max_size(240)"']
                if config.version in ("GM8E01_00", "GM8E01_02")
                else [],
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptVisorFlare.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "MetroidPrime/ScriptObjects/CScriptWorldTeleporter.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "MetroidPrime/ScriptObjects/CScriptVisorGoo.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/Enemies/CJellyZap.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptControllerAction.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                   "MetroidPrime/Weapons/GunController/CGunMotion.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptSwitch.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/BodyState/CABSIdle.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/BodyState/CABSFlinch.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "MetroidPrime/BodyState/CABSAim.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptPlayerStateChange.cpp",
            ),
            Object(EquivalentFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Enemies/CThardus.cpp"),
            Object(EquivalentFor("GM8E01_00"), "MetroidPrime/CActorModelParticles.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CWallCrawlerSwarm.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "MetroidPrime/ScriptObjects/CScriptAiJumpPoint.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/CMessageScreen.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/Enemies/CFlaahgraTentacle.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/Weapons/GunController/CGSFidget.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/BodyState/CABSReaction.cpp",
            ),
            Object(EquivalentFor("GM8E01_00"), "MetroidPrime/Weapons/CIceAttackProjectile.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"),
                "MetroidPrime/Enemies/CPatternedAiFunctions.cpp",
            ),
            Object(NonMatching, "MetroidPrime/Enemies/CFlyingPirate.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"),
                "MetroidPrime/ScriptObjects/CScriptColorModulate.cpp",
            ),
            Object(NonMatching, "MetroidPrime/CMapUniverse.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS),
                   "MetroidPrime/Enemies/CThardusRockProjectile.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/CInventoryScreen.cpp"),
            Object(MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS), "MetroidPrime/CVisorFlare.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "MetroidPrime/Enemies/CFlaahgraPlants.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/CWorldTransManager.cpp",
            ),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptMidi.cpp"),
            Object(
                NonMatching,
                "MetroidPrime/ScriptObjects/CScriptStreamedMusic.cpp",
                cflags=cflags_retro,
            ),
            Object(NonMatching, "MetroidPrime/CRagDoll.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"),
                "MetroidPrime/Player/CGameOptions.cpp",
                extra_cflags=['-pragma "inline_max_size(131)"']
                if VERSIONS.index("GM8P01_00") <= version_num < VERSIONS.index("R3IJ01_00")
                else [],
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS),
                "MetroidPrime/ScriptObjects/CRepulsor.cpp",
            ),
            Object(NonMatching, "MetroidPrime/CEnvFxManager.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"),
                "MetroidPrime/Weapons/CEnergyProjectile.cpp",
                cflags=cflags_retro,
            ),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptGunTurret.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "MetroidPrime/Weapons/CProjectileInfo.cpp",
            ),
            Object(
                EquivalentFor("GM8E01_00"),
                "MetroidPrime/CInGameTweakManager.cpp",
                cflags=cflags_retro,
            ),
            Object(NonMatching, "MetroidPrime/Enemies/CBabygoth.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/Enemies/CEyeBall.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/CIkChain.cpp"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptCameraPitchVolume.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "MetroidPrime/RumbleFxTable.cpp"
            ),
            Object(NonMatching, "MetroidPrime/Enemies/CElitePirate.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "MetroidPrime/CRumbleManager.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"),
                "MetroidPrime/Enemies/CBouncyGrenade.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"),
                "MetroidPrime/Enemies/CGrenadeLauncher.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "MetroidPrime/Weapons/CShockWave.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/Enemies/CRipperControlledPlatform.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "MetroidPrime/Enemies/CKnockBackMgr.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"),
                "MetroidPrime/CScriptLayerManager.cpp",
            ),
            Object(EquivalentFor("GM8E01_00"), "MetroidPrime/Enemies/CMagdolite.cpp"),
            Object(
                MatchingFor("GM8EAB_00", "GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00"),
                "MetroidPrime/Enemies/CTeamAiMgr.cpp",
            ),
            Object(MatchingFor("GM8EAB_00", "GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/Enemies/CSnakeWeedSwarm.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                   "MetroidPrime/Cameras/CBallCameraFailsafeState.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "MetroidPrime/Enemies/CScriptContraption.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptSpindleCamera.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptMemoryRelay.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/CPauseScreenFrame.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "MetroidPrime/Enemies/CAtomicAlpha.cpp"),
            Object(EquivalentFor("GM8E01_00"), "MetroidPrime/CLogBookScreen.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/CGBASupport.cpp"
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/CMemoryCard.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptCameraHintTrigger.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/Enemies/CAmbientAI.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/CMemoryCardDriver.cpp"),
            Object(EquivalentFor("GM8E01_00"), "MetroidPrime/CSaveGameScreen.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/Enemies/CAtomicBeta.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS),
                   "MetroidPrime/Weapons/CElectricBeamProjectile.cpp"),
            Object(
                NonMatching,
                "MetroidPrime/Enemies/CRidley.cpp",
                extra_cflags=['-pragma "inline_max_total_size(10000)"'],
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/Enemies/CPuffer.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "MetroidPrime/ScriptObjects/CFire.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "MetroidPrime/CPauseScreenBlur.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/Enemies/CTryclops.cpp"),
            Object(EquivalentFor("GM8E01_00"), "MetroidPrime/Weapons/CNewFlameThrower.cpp"),
            Object(NonMatching, "MetroidPrime/Cameras/CInterpolationCamera.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "MetroidPrime/Enemies/CSeedling.cpp"),
            Object(EquivalentFor("GM8E01_00"), "MetroidPrime/CGameHintInfo.cpp"),
            Object(EquivalentFor("GM8E01_00"), "MetroidPrime/Enemies/CWallWalker.cpp"),
            Object(NonMatching, "MetroidPrime/CErrorOutputWindow.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/CRainSplashGenerator.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/CWorldSaveGameInfo.cpp",
                cflags=cflags_retro,
            ),
            Object(NonMatching, "MetroidPrime/CFluidPlaneRender.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/Enemies/CBurrower.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CMetroidPrime.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "MetroidPrime/ScriptObjects/CScriptBeam.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/Enemies/CMetroidPrimeStage2.cpp"),
            Object(MatchingFor("GM8EAB_00"), "MetroidPrime/Enemies/CMetroidPrimeRelay.cpp"),
            Object(
                NonMatching,
                "MetroidPrime/Player/CPlayerDynamics.cpp",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CScriptMazeNode.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/Weapons/WeaponTypes.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/COmegaPirate.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/Enemies/CScriptPhazonPool.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/CNESEmulator.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "MetroidPrime/Enemies/CPhazonHealingNodule.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/Player/CMorphBallShadow.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "R3ME01_00"),
                "MetroidPrime/Player/CPlayerStuckTracker.cpp",
            ),
            Object(NonMatching, "MetroidPrime/CSlideShow.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/Tweaks/CTweakSlideShow.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/CArtifactDoll.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/CProjectedShadow.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "MetroidPrime/CPreFrontEnd.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"), "MetroidPrime/CGameCubeDoll.cpp"
            ),
            Object(
                NonMatching, "MetroidPrime/ScriptObjects/CScriptProjectedShadow.cpp"
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "MetroidPrime/ScriptObjects/CEnergyBall.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/ScriptObjects/CSustainedPlayerDamage.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"),
                "MetroidPrime/Enemies/CPoisonProjectile.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "MetroidPrime/Enemies/SPositionHistory.cpp",
            ),
            Object(Equivalent, "dummy.c"),
        ],
    ),
    RetroLib(
        "WorldFormat",
        "core",
        [
            Object(MatchingFor(*NTSC_GC_VERSIONS), "WorldFormat/CAreaOctTree_Tests.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "WorldFormat/CCollisionSurface.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "WorldFormat/CMetroidModelInstance.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"), "WorldFormat/CAreaBspTree.cpp"
            ),
            Object(MatchingFor("GM8EAB_00", "GM8E01_00", "GM8E01_01", "GM8E01_02"), "WorldFormat/CAreaOctTree.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "WorldFormat/CMetroidAreaCollider.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "WorldFormat/CWorldLight.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "WorldFormat/COBBTree.cpp",
            ),
            Object(
                EquivalentFor("GM8E01_00"),
                "WorldFormat/CCollidableOBBTree.cpp",
                cflags=cflags_retro if version_num < VERSIONS.index("GM8P01_00") else None,
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "WorldFormat/CCollidableOBBTreeGroup.cpp"
            ),
            Object(NonMatching, "WorldFormat/CPVSAreaSet.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "WorldFormat/CAreaRenderOctTree.cpp"
            ),
        ],
    ),
    RetroLib(
        "Weapons",
        "core",
        [
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "Weapons/CProjectileWeapon.cpp",
                extra_cflags=(
                    ['-pragma "inline_max_size(250)"']
                    if version_num == VERSIONS.index("GM8E01_02")
                    else []
                ),
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_48", "GM8E01_02"), "Weapons/CProjectileWeaponDataFactory.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "Weapons/CCollisionResponseData.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"), "Weapons/IWeaponRenderer.cpp"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00", "R3ME01_00"),
                "Weapons/CDecalDataFactory.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "Weapons/CDecal.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Weapons/CWeaponDescription.cpp"
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Weapons/CDecalDescription.cpp"
            ),
        ],
    ),
    RetroLib(
        "MetaRender",
        "core",
        [
            Object(NonMatching, "MetaRender/CCubeRenderer.cpp"),
        ],
    ),
    KyotoLib(
        "GuiSys",
        "core",
        [
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "GuiSys/CAuiMain.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"), "GuiSys/CAuiMeter.cpp"
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "GuiSys/CGuiCamera.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "GuiSys/CGuiCompoundWidget.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "GuiSys/CGuiFactories.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "GuiSys/CGuiFeeHelper.cpp"),
            Object(NonMatching, "GuiSys/CGuiFrame.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"), "GuiSys/CGuiGroup.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"), "GuiSys/CGuiHeadWidget.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"), "GuiSys/CGuiLight.cpp"),
            Object(EquivalentFor("GM8E01_00"), "GuiSys/CGuiModel.cpp"),
            Object(NonMatching, "GuiSys/CGuiObject.cpp", cflags=[*cflags_retro, "-inline auto"]),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"), "GuiSys/CGuiPane.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"), "GuiSys/CGuiSliderGroup.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "GuiSys/CGuiSys.cpp",
                extra_cflags=['-inline deferred,auto']
                if version_num >= VERSIONS.index("GM8P01_00")
                else [],
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "GuiSys/CGuiTableGroup.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "GuiSys/CGuiTextPane.cpp"
            ),
            Object(NonMatching, "GuiSys/CGuiTextSupport.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"), "GuiSys/CGuiWidget.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"), "GuiSys/CGuiWidgetIdDB.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "GuiSys/CGuiWidgetDrawParms.cpp"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "GuiSys/CAuiEnergyBarT01.cpp",
            ),
            Object(EquivalentFor("GM8E01_00"), "GuiSys/CAuiImagePane.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "GuiSys/CRepeatState.cpp",
            ),
        ],
    ),
    RetroLib(
        "Collision",
        "core",
        [
            Object(
                MatchingFor("GM8EAB_00", "GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "Collision/CCollidableAABox.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "Collision/CCollidableCollisionSurface.cpp",
            ),
            Object(
                MatchingFor("GM8EAB_00", "GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "Collision/CCollisionInfo.cpp",
                cflags=cflags_retro,
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Collision/InternalColliders.cpp"
            ),
            Object(
                MatchingFor("GM8EAB_00", "GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00"),
                "Collision/CCollisionPrimitive.cpp",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Collision/CMaterialList.cpp"
            ),
            Object(EquivalentFor("GM8E01_00"), "Collision/CollisionUtil.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Collision/CCollidableSphere.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Collision/CMaterialFilter.cpp",
            ),
            Object(
                NonMatching,
                "Collision/COBBox.cpp",
                cflags=cflags_retro,
            ),
            Object(MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Collision/CMRay.cpp"),
        ],
    ),
    KyotoLib(
        "Kyoto1",
        "core",
        [
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/Basics/CBasics.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"), "Kyoto/Basics/CStopwatch.cpp"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/Basics/CBasicsDolphin.cpp"
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Alloc/CCallStackDolphin.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "Kyoto/Basics/COsContextDolphin.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"), "Kyoto/Basics/CSWDataDolphin.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "Kyoto/Basics/RAssertDolphin.cpp"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/Animation/CAnimation.cpp"
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CAnimationManager.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "Kyoto/Animation/CAnimationSet.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CAnimCharacterSet.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CAnimTreeLoopIn.cpp",
                extra_cflags=['-pragma "inline_max_size(260)"'] if version_num < VERSIONS.index("GM8P01_00") else [],
            ),
            Object(NonMatching, "Kyoto/Animation/CAnimTreeSequence.cpp"),
            Object(NonMatching, "Kyoto/Animation/CCharacterInfo.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                   "Kyoto/Animation/CCharacterSet.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"), "Kyoto/Animation/CMetaAnimBlend.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CMetaAnimFactory.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"), "Kyoto/Animation/CMetaAnimPhaseBlend.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CMetaAnimPlay.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                   "Kyoto/Animation/CMetaAnimRandom.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/Animation/CMetaAnimSequence.cpp"),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CMetaTransFactory.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/Animation/CMetaTransMetaAnim.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "Kyoto/Animation/CMetaTransPhaseTrans.cpp"),
            Object(
                MatchingFor("GM8EAB_00", "GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CMetaTransSnap.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                   "Kyoto/Animation/CMetaTransTrans.cpp"),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CPASAnimInfo.cpp",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CPASAnimParm.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CPASAnimState.cpp",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CPASDatabase.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CPASParmInfo.cpp",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/Animation/CPrimitive.cpp"
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/Animation/CSequenceHelper.cpp",
                   extra_cflags=(
                       ['-pragma "inline_max_size(255)"']
                       if version_num < VERSIONS.index("GM8P01_00")
                       else ['-pragma "inline_max_size(120)"']
                   ),
                   ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CTransition.cpp"
            ),
            Object(
                MatchingFor("GM8EAB_00", "GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CTransitionManager.cpp",
            ),
            Object(MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/Animation/CTreeUtils.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/Animation/IMetaAnim.cpp"),
            Object(MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                   "Kyoto/Audio/CSfxHandle.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "Kyoto/Audio/CSfxManager.cpp",
                extra_cflags=(
                    ['-pragma "inline_max_size(140)"']
                    if config.version == "GM8J01_00"
                    else []
                ),
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CAdvancementDeltas.cpp",
            ),
            Object(MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                   "Kyoto/Animation/CAnimMathUtils.cpp"),

            Object(MatchingFor("GM8EAB_00", "GM8E01_00", "GM8E01_01", "GM8E01_02"), "Kyoto/Animation/CAnimPerSegmentData.cpp"),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CAnimPOIData.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "Kyoto/Animation/CAnimSource.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                   "Kyoto/Animation/CAnimSourceReader.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CAnimSourceReaderBase.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CAnimTreeAnimReaderContainer.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                   "Kyoto/Animation/CAnimTreeBlend.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS),
                   "Kyoto/Animation/CAnimTreeContinuousPhaseBlend.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CAnimTreeDoubleChild.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CAnimTreeNode.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CAnimTreeSingleChild.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CAnimTreeTimeScale.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CAnimTreeTransition.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CAnimTreeTweenBase.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CBoolPOINode.cpp",
            ),
            Object(MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                   "Kyoto/Animation/CCharAnimMemoryMetrics.cpp"),
            Object(NonMatching, "Kyoto/Animation/CCharLayoutInfo.cpp"),
            Object(
                EquivalentFor("GM8E01_00"),
                "Kyoto/Animation/CFBStreamedAnimReader.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CFBStreamedCompression.cpp",
            ),
            Object(Matching, "Kyoto/Animation/CHierarchyPoseBuilder.cpp"),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CInt32POINode.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00"),
                "Kyoto/Animation/CMultiFormatAnimReader.CPP",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CParticlePOINode.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/Animation/CPOINode.cpp"
            ),
            Object(EquivalentFor("GM8E01_00"), "Kyoto/Animation/CSegStatementSet.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CTimeScaleFunctions.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/Animation/IAnimReader.cpp"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CAllFormatsAnimSource.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "Kyoto/CDvdRequestManager.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/CDvdRequest.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "Kyoto/Text/CColorInstruction.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "Kyoto/Text/CColorOverrideInstruction.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "Kyoto/Text/CDrawStringOptions.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Text/CFontInstruction.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS), "Kyoto/Text/CFontRenderState.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "Kyoto/Text/CLineExtraSpaceInstruction.cpp",
            ),
            Object(MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00"), "Kyoto/Text/CLineInstruction.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Text/CLineSpacingInstruction.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Text/CPopStateInstruction.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Text/CPushStateInstruction.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Text/CRasterFont.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Text/CRemoveColorOverrideInstruction.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "Kyoto/Text/CSaveableState.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"),
                "Kyoto/Text/CTextExecuteBuffer.cpp",
                extra_cflags=["-inline", "level=4"] if config.version == "GM8P01_00" else [],
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/Text/CTextInstruction.cpp"
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), "Kyoto/Text/CTextParser.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"), "Kyoto/Text/CWordBreakTables.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/Text/CWordInstruction.cpp"),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Text/CBlockInstruction.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/Text/CFont.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/Graphics/CLight.cpp"),
            Object(EquivalentFor("GM8E01_00"), "Kyoto/Graphics/CCubeModel.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00"), "Kyoto/Graphics/CGX.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Graphics/CTevCombiners.cpp",
            ),
            Object(
                EquivalentFor("GM8E01_00"),
                "Kyoto/Graphics/DolphinCGraphics.cpp",
                extra_cflags=(
                    ['-pragma "inline_max_size(100)"']
                    if config.version == "GM8J01_00"
                    else []
                ),
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"),
                "Kyoto/Graphics/DolphinCPalette.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"),
                "Kyoto/Graphics/DolphinCTexture.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Math/CloseEnough.cpp",
            ),
            Object(
                NonMatching,
                "Kyoto/Math/CMayaSpline.cpp",
            ),
            Object(MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/Math/CMatrix3f.cpp"),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Math/CMatrix4f.cpp",
            ),
            Object(MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/Math/CNUQuaternion.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Math/CQuaternion.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"), "Kyoto/CRandom16.cpp"),
            Object(NonMatching, "Kyoto/Math/CTransform4f.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Math/CUnitVector3f.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/Math/CVector2f.cpp"),
            Object(MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                   "Kyoto/Math/CVector2i.cpp"),
            Object(MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/Math/CVector3d.cpp"),
            Object(MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/Math/CVector3f.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                   "Kyoto/Math/CVector3i.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"), "Kyoto/Math/RMathUtils.cpp"),
            Object(MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/CCrc32.cpp"),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Alloc/CCircularBuffer.cpp"
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/Alloc/CMemory.cpp"),
            Object(MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/Alloc/IAllocator.cpp"),
            Object(NonMatching, "Kyoto/PVS/CPVSVisOctree.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00"), "Kyoto/PVS/CPVSVisSet.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "Kyoto/Particles/CColorElement.cpp",
            ),
            Object(NonMatching, "Kyoto/Particles/CElementGen.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/Particles/CIntElement.cpp"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Particles/CModVectorElement.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                   "Kyoto/Particles/CParticleDataFactory.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "Kyoto/Particles/CParticleGen.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Particles/CParticleGlobals.cpp",
            ),
            Object(NonMatching, "Kyoto/Particles/CParticleSwoosh.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "Kyoto/Particles/CParticleSwooshDataFactory.cpp"
                ,
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Particles/CRealElement.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/Particles/CSpawnSystemKeyframeData.cpp"),
            Object(NonMatching, "Kyoto/Particles/CUVElement.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Particles/CVectorElement.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                   "Kyoto/Particles/CWarp.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                   "Kyoto/Math/CPlane.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                   "Kyoto/Math/CSphere.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8J01_00"),
                "Kyoto/Math/CAABox.cpp",
                extra_cflags=(
                    ['-pragma "inline_max_size(140)"']
                    if version_num >= VERSIONS.index("GM8J01_00")
                    else []
                ),
            ),
            Object(EquivalentFor("GM8E01_00"), "Kyoto/CFactoryMgr.cpp"),
            Object(NonMatching, "Kyoto/CResFactory.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"), "Kyoto/CResLoader.cpp"),
            Object(
                MatchingFor("GM8EAB_00", "GM8E01_00", "GM8E01_48", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                "rstl/rstl_map.cpp",
                src_dir="extern/rstl/src",
            ),
            Object(
                MatchingFor("GM8P01_00", "GM8J01_00"),
                "rstl/rstl_allocator.cpp",
                src_dir="extern/rstl/src",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "rstl/rstl_strings.cpp",
                src_dir="extern/rstl/src",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "rstl/CStringExtras.cpp",
                src_dir="extern/rstl/src",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "rstl/RstlExtras.cpp",
                src_dir="extern/rstl/src",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "R3ME01_00"),
                "Kyoto/Streams/CInputStream.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Streams/CMemoryInStream.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Streams/CMemoryStreamOut.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Streams/COutputStream.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Streams/CZipInputStream.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "Kyoto/Streams/CZipOutputStream.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Streams/CZipSupport.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/CFactoryStore.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/CObjectReference.cpp",
            ),
            Object(NonMatching, "Kyoto/CSimplePool.cpp"),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/CToken.cpp"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/IObj.cpp"
            ),
        ],
    ),
    # TODO: Merge back into Kyoto
    {
        "lib": "zlib",
        "mw_version": "GC/1.3.2",
        "cflags": cflags_runtime,
        "progress_category": "third_party",
        "src_dir": "extern",
        "shift_jis": False,
        "objects": [
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "zlib-1.1.3/adler32.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "zlib-1.1.3/deflate.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "zlib-1.1.3/infblock.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "zlib-1.1.3/infcodes.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "zlib-1.1.3/inffast.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "zlib-1.1.3/inflate.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "zlib-1.1.3/inftrees.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "zlib-1.1.3/infutil.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "zlib-1.1.3/trees.c"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "zlib-1.1.3/zutil.c"
            ),
        ],
    },
    # TODO: Merge this with zlib and Kyoto1
    KyotoLib(
        "Kyoto2",
        "core",
        [
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"),
                "Kyoto/CARAMManager.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/Math/CFrustumPlanes.cpp"),
            Object(NonMatching, "Kyoto/Graphics/CCubeMaterial.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Graphics/CCubeSurface.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CCharAnimTime.cpp",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CSegIdList.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00", "R3ME01_00"),
                "Kyoto/Input/CFinalInput.cpp",
            ),
            Object(
                NonMatching,
                "Kyoto/Input/CInputFilter.cpp",
            ),
            Object(MatchingFor("R3ME01_00"), "Kyoto/Input/IController.cpp"),
            Object(
                NonMatching,
                "Kyoto/Input/CRevolutionController.cpp",
            ),
            Object(MatchingFor("R3ME01_00"), "Kyoto/Input/CControllerData.cpp"),
            Object(
                NonMatching,
                "Kyoto/Input/CWiiMotionProcessor.cpp",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Graphics/CColor.cpp",
            ),
            Object(NonMatching, "Kyoto/Audio/DolphinCAudioGroupSet.cpp"),
            Object(EquivalentFor("GM8E01_00"), "Kyoto/Audio/DolphinCAudioSys.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"), "Kyoto/DolphinCMemoryCardSys.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Input/DolphinIController.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Input/CDolphinController.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/DolphinCDvdFile.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Alloc/CMediumAllocPool.cpp",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Alloc/CSmallAllocPool.cpp",
            ),
            Object(
                NonMatching,
                "Kyoto/Alloc/CGameAllocator.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"),
                "Kyoto/Animation/DolphinCSkinnedModel.cpp",
                # TODO: inline optional assignment in earlier AddSkinnedRef at the common limit.
                extra_cflags=(
                    ['-pragma "inline_max_size(259)"']
                    if version_num < VERSIONS.index("GM8P01_00")
                    else []
                ),
            ),
            Object(NonMatching, "Kyoto/Animation/DolphinCSkinRules.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/Animation/DolphinCVirtualBone.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/Graphics/DolphinCModel.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "Kyoto/Text/CStringTable.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Particles/CEmitterElement.cpp",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Particles/CEffectComponent.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                   "Kyoto/Particles/CParticleData.cpp"),
            Object(MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                   "Kyoto/Animation/CVertexMorphEffect.cpp"),
            Object(MatchingFor("GM8P01_00", "GM8J01_00"), "Kyoto/Animation/CSkinnedModelWithAvgNormals.cpp"),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/CTimeProvider.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/CARAMToken.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Audio/CMidiManager.cpp",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00"),
                "Kyoto/Text/CFontImageDef.cpp",
                extra_cflags=["-inline", "level=4"] if config.version == "GM8P01_00" else [],
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"),
                "Kyoto/Text/CImageInstruction.cpp",
                extra_cflags=["-inline", "level=4"] if config.version == "GM8P01_00" else [],
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "Kyoto/Text/CTextRenderBuffer.cpp",
                extra_cflags=["-inline", "level=4"]
                if config.version in ["GM8P01_00", "GM8J01_00"]
                else [],
            ),
            Object(NonMatching, "Kyoto/Graphics/CCubeMoviePlayer.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CAdditiveAnimPlayback.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "Kyoto/Particles/CParticleElectricDataFactory.cpp",
            ),
            Object(EquivalentFor("GM8E01_00"), "Kyoto/Particles/CParticleElectric.cpp"),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS),
                "Kyoto/Graphics/DolphinCColor.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                   "Kyoto/Audio/CDSPStreamManager.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/CDependencyGroup.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"),
                   "Kyoto/Audio/CStreamAudioManager.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Animation/CHalfTransition.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Particles/CElectricDescription.cpp"
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Particles/CSwooshDescription.cpp",
            ),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                   "Kyoto/Particles/CGenDescription.cpp"),
            Object(NonMatching, "Kyoto/CPakFile.cpp"),
            Object(NonMatching, "Kyoto/Animation/CPoseAsTransformsVariableSize.cpp"),
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "Kyoto/Input/CRumbleVoice.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/Input/RumbleAdsr.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "Kyoto/Input/CRumbleGenerator.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02", "GM8P01_00", "GM8J01_00"), "Kyoto/Audio/CDSPStream.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "Kyoto/Audio/g721.cpp",
            ),
            Object(NonMatching, "Kyoto/Audio/CStaticAudioPlayer.cpp"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "Kyoto/CFrameDelayedKiller.cpp",
            ),
            Object(MatchingFor("GM8P01_00", "GM8J01_00"), "Kyoto/Animation/CTimeRemainderAndFraction.cpp"),
        ],
    ),
    DolphinLib(
        "ai",
        [
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"), "dolphin/ai.c"),
        ],
    ),
    DolphinLib(
        "ar",
        [
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "dolphin/ar/ar.c"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "dolphin/ar/arq.c"
            ),
        ],
    ),
    DolphinLib(
        "base",
        [
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/PPCArch.c"
            ),
        ],
    ),
    DolphinLib(
        "db",
        [
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                   "dolphin/db.c"),
        ],
    ),
    DolphinLib(
        "dsp",
        [
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "dolphin/dsp/dsp.c"
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/dsp/dsp_debug.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/dsp/dsp_task.c",
            ),
        ],
    ),
    DolphinLib(
        "dvd",
        [
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"),
                "dolphin/dvd/dvdlow.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/dvd/dvdfs.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"),
                "dolphin/dvd/dvd.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/dvd/dvdqueue.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/dvd/dvderror.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/dvd/dvdidutils.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/dvd/dvdfatal.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/dvd/fstload.c",
            ),
        ],
    ),
    DolphinLib(
        "gx",
        [
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/gx/GXInit.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"),
                "dolphin/gx/GXFifo.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/gx/GXAttr.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/gx/GXMisc.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/gx/GXGeometry.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/gx/GXFrameBuf.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/gx/GXLight.c",
                extra_cflags=["-fp_contract off"],
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/gx/GXTexture.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/gx/GXBump.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/gx/GXTev.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "dolphin/gx/GXPixel.c",
                extra_cflags=["-fp_contract off"],
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/gx/GXStubs.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/gx/GXDisplayList.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/gx/GXTransform.c",
                extra_cflags=["-fp_contract off"],
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/gx/GXPerf.c",
            ),
        ],
    ),
    {
        "lib": "mtx",
        "mw_version": "GC/1.2.5n",
        # "cflags": ["-nodefaults","-proc gekko","-align powerpc","-fp hardware","-g","-sym on","-maxerrors 1","-nosyspath","-i include","-i extern/sdk/libc", "-D_DEBUG=1", "-inline off", "-Cpp_exceptions off"],
        "cflags": [*cflags_base, "-fp_contract off"],
        "progress_category": "sdk",
        "src_dir": "extern/sdk",
        "shift_jis": True,
        "objects": [
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"), "dolphin/mtx/mtx.c"
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/mtx/mtxvec.c",
            ),
            Object(NonMatching, "dolphin/mtx/mtxstack.c"),
            Object(NonMatching, "dolphin/mtx/mtx44vec.c"),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/mtx/mtx44.c",
            ),
            Object(Matching, "dolphin/mtx/vec.c"),
            Object(Equivalent, "dolphin/mtx/quat.c"),
            Object(MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                   "dolphin/mtx/psmtx.c"),
        ],
    },
    DolphinLib(
        "os",
        [
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/os/__start.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/os/OS.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/os/OSAlarm.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/os/OSArena.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/os/OSAudioSystem.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/os/OSCache.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/os/OSContext.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/os/OSError.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"),
                "dolphin/os/OSFatal.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/os/OSFont.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/os/OSInterrupt.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/os/OSLink.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/os/OSMessage.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/os/OSMemory.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/os/OSMutex.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48"),
                "dolphin/os/OSReboot.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/os/OSReset.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"),
                "dolphin/os/OSResetSW.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/os/OSRtc.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/os/OSSync.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/os/OSThread.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/os/OSTime.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/os/__ppc_eabi_init.cpp",
            ),
        ],
    ),
    DolphinLib(
        "pad",
        [
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/pad/PadClamp.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"), "dolphin/pad/pad.c"
            ),
        ],
    ),
    DolphinLib(
        "vi",
        [
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"), "dolphin/vi.c"),
        ],
    ),
    {
        "lib": "MSL_C.PPCEABI.bare.H",
        "mw_version": "GC/1.3",
        "cflags": cflags_runtime,
        "progress_category": "sdk",
        "src_dir": "extern/sdk",
        "shift_jis": False,
        "objects": [
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "runtime/__mem.c"
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "runtime/__va_arg.c"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "runtime/global_destructor_chain.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "runtime/CPlusLibPPC.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"),
                "runtime/NMWException.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/ptmf.c"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/runtime.c"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "runtime/__init_cpp_exceptions.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "runtime/Gecko_ExceptionPPC.cpp",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "runtime/abort_exit.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "runtime/alloc.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "runtime/ansi_files.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/ansi_fp.c"
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/arith.c"
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "runtime/buffer_io.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/ctype.c"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/locale.c"
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "runtime/direct_io.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/file_io.c"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/errno.c"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/FILE_POS.c"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/mbstring.c"
            ),
            Object(MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/mem.c"),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "runtime/mem_funcs.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "runtime/misc_io.c"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/printf.c"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "runtime/qsort.c"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/rand.c"
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "runtime/sscanf.c"
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "runtime/string.c"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/float.c"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/strtold.c"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "runtime/uart_console_io.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "runtime/wchar_io.c"
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/e_acos.c"
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/e_asin.c"
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/e_atan2.c"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/e_exp.c"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "runtime/e_fmod.c"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/e_log.c"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/e_pow.c"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "runtime/e_rem_pio2.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/k_cos.c"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "runtime/k_rem_pio2.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/k_sin.c"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/k_tan.c"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/s_atan.c"
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "runtime/s_copysign.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/s_cos.c"
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/s_floor.c"
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/s_frexp.c"
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/s_ldexp.c"
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/s_modf.c"
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "runtime/s_nextafter.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/s_sin.c"
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/s_tan.c"
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/w_acos.c"
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/w_asin.c"
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/w_atan2.c"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/w_exp.c"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/w_fmod.c"
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/w_log.c"
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"), "runtime/w_pow.c"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "runtime/math_ppc.c"
            ),
        ],
    },
    MusyX(
        debug=False,
        # mw_version="GC/1.2.5n",
        major=2,
        minor=0,
        patch=0,
        objects=[
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "musyx/runtime/seq.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"),
                "musyx/runtime/synth.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "musyx/runtime/seq_api.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "musyx/runtime/snd_synthapi.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "musyx/runtime/stream.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "musyx/runtime/synthdata.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"),
                "musyx/runtime/synthmacros.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"),
                "musyx/runtime/synthvoice.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "musyx/runtime/synth_ac.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "musyx/runtime/synth_adsr.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "musyx/runtime/synth_vsamples.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "musyx/runtime/synth_dbtab.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "musyx/runtime/s_data.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "musyx/runtime/hw_dspctrl.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "musyx/runtime/hw_volconv.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "musyx/runtime/snd3d.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "musyx/runtime/snd_init.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "musyx/runtime/snd_math.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "musyx/runtime/snd_midictrl.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "musyx/runtime/snd_service.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"),
                "musyx/runtime/hardware.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "musyx/runtime/hw_aramdma.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "musyx/runtime/dsp_import.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "musyx/runtime/hw_dolphin.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "musyx/runtime/hw_memory.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"),
                "musyx/runtime/hw_lib_dolphin.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"),
                "musyx/runtime/profile.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "musyx/runtime/CheapReverb/creverb_fx.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "musyx/runtime/CheapReverb/creverb.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "musyx/runtime/StdReverb/reverb_fx.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "musyx/runtime/StdReverb/reverb.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "musyx/runtime/Delay/delay_fx.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "musyx/runtime/Chorus/chorus_fx.c",
            ),
        ],
    ),
    DolphinLib(
        "dtk",
        [
            Object(MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"),
                   "dolphin/dtk.c"),
        ],
    ),
    DolphinLib(
        "card",
        [
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00"),
                "dolphin/card/CARDBios.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/card/CARDUnlock.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/card/CARDRdwr.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/card/CARDBlock.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/card/CARDDir.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/card/CARDCheck.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/card/CARDMount.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/card/CARDFormat.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/card/CARDOpen.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/card/CARDCreate.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/card/CARDRead.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/card/CARDWrite.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/card/CARDDelete.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/card/CARDStat.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS),
                "dolphin/card/CARDRename.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/card/CARDNet.c",
            ),
        ],
    ),
    DolphinLib(
        "si",
        [
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/si/SIBios.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/si/SISamplingRate.c",
            ),
        ],
    ),
    DolphinLib(
        "exi",
        [
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/exi/EXIBios.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/exi/EXIUart.c",
            ),
        ],
    ),
    DolphinLib(
        "thp",
        [
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/thp/THPDec.c",
            ),
            Object(
                MatchingFor("GM8EAB_00", *NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/thp/THPAudio.c",
            ),
        ],
    ),
    DolphinLib(
        "gba",
        [
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/GBA/GBA.c"
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/GBA/GBAGetProcessStatus.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/GBA/GBAJoyBoot.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/GBA/GBARead.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/GBA/GBAWrite.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/GBA/GBAXfer.c",
            ),
            Object(
                MatchingFor(*NTSC_GC_VERSIONS, "GM8P01_00", "GM8J01_00"),
                "dolphin/GBA/GBAKey.c",
            ),
        ],
    ),
    Rel(
        "NESemuP",
        [
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"),
                "NESemu/modwrapper.cpp",
                cflags=[*cflags_base, "-O0", "-sdata 0", "-sdata2 0", "-str noreuse"],
            ),
            Object(
                NonMatching,
                "NESemu/emu.cpp",
                cflags=[*cflags_base, "-sdata 0", "-sdata2 0", "-pool on"],
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"),
                "NESemu/ksNesAudio.cpp",
                cflags=[
                    *cflags_base, "-O4,s", "-inline off", "-func_align 32",
                    "-sdata 0", "-sdata2 0", "-pool on", "-i extern/musyx/include",
                ],
            ),
            Object(
                EquivalentFor("GM8E01_00"),
                "NESemu/emusound.cpp",
                cflags=[
                    *cflags_base, "-O4,s", "-inline off", "-func_align 32", "-vector on",
                    "-sdata 0", "-sdata2 0", "-pool on", "-i extern/musyx/include",
                ],
            ),
        ],
    ),
]

# PAL ships separate 50 Hz and 60 Hz modules. Give each its own object paths and
# generated ROM include directory while compiling the shared emulator sources.
if config.version == "GM8P01_00":
    nes_lib = next(lib for lib in config.libs if lib["lib"] == "NESemuP")
    config.libs.remove(nes_lib)
    for module in ("NESPALemuP", "NESPAL60emuP"):
        objects = []
        for obj in nes_lib["objects"]:
            options = dict(obj.options)
            options["source"] = obj.name
            options["cflags"] = [
                f"-i {(config.out_path() / 'include' / module).as_posix()}",
                *options["cflags"],
            ]
            if obj.name == "NESemu/emusound.cpp":
                options["cflags"].append("-rostr")
            objects.append(
                Object(
                    obj.name in ("NESemu/modwrapper.cpp", "NESemu/ksNesAudio.cpp"),
                    obj.name.replace("NESemu/", f"{module}/", 1),
                    **options,
                )
            )
        config.libs.append(Rel(module, objects))


# Optional callback to adjust link order. This can be used to add, remove, or reorder objects.
# This is called once per module, with the module ID and the current link order.
#
# For example, this adds "dummy.c" to the end of the DOL link order if configured with --non-matching.
# "dummy.c" *must* be configured as a Matching (or Equivalent) object in order to be linked.
def link_order_callback(module_id: int, objects: List[str]) -> List[str]:
    # Don't modify the link order for matching builds
    return objects


# Uncomment to enable the link order callback.
# config.link_order_callback = link_order_callback


# Optional extra categories for progress tracking
# Adjust as desired for your project
config.progress_categories = [
    ProgressCategory("game", "Game"),
    ProgressCategory("core", "Core Engine (Kyoto)"),
    ProgressCategory("sdk", "SDK"),
    ProgressCategory("third_party", "Third Party"),
]
config.progress_each_module = args.verbose
# Optional extra arguments to `objdiff-cli report generate`
config.progress_report_args = [
    # Marks relocations as mismatching if the target value is different
    # Default is "functionRelocDiffs=none", which is most lenient
    # "--config functionRelocDiffs=data_value",
]
config.progress_modules = False
config.progress_use_fancy = True
config.progress_code_fancy_frac = 1499
config.progress_code_fancy_item = "Energy"
config.progress_data_fancy_frac = 250
config.progress_data_fancy_item = "Missiles"
config.extra_clang_flags = ["-DCLANGD"]

if args.mode == "configure":
    # Write build.ninja and objdiff.json
    generate_build(config)
elif args.mode == "progress":
    # Print progress information
    calculate_progress(config)
else:
    sys.exit("Unknown mode: " + args.mode)
