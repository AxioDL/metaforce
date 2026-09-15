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
DEFAULT_VERSION = 0
VERSIONS = [
    "GM8E01_00",  # mp-v1.088 NTSC-U
    "GM8E01_01",  # mp-v1.093 NTSC-U
    "GM8E01_48",  # mp-v1.097 NTSC-K
    "GM8P01_00",  # mp-v1.110 PAL
    "GM8J01_00",  # mp-v1.111 NTSC-J
    "GM8E01_02",  # mp-v1.111 NTSC-U
    "R3IJ01_00",  # mp-v3.570 New Play Controls
    "R3ME01_00",  # mp-v3.593 Trilogy NTSC
    "R3MP01_00",  # mp-v3.629 Trilogy PAL
]

DISABLED_VERSIONS = [
    # 1,
    # 2,
    # 3,
    # 4,
    # 5,
    6,
    7,
    8,
]

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

if version_num in DISABLED_VERSIONS:
    print(f"Version {VERSIONS[version_num]} is disabled using default")
    version_num = DEFAULT_VERSION
    config.version = VERSIONS[DEFAULT_VERSION]

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
    "-fp_contract on",
    "-str reuse",
    "-i include",
    "-i extern/sdk/include",
    "-i extern/sdk/libc",
    f"-i build/{config.version}/include",
    f"-DVERSION={version_num}",
    "-DPRIME1",
    "-DNONMATCHING=0",
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
    "-gccinc",
    "-inline deferred",
    "-common on",
    "-i extern/musyx/include",
    # "-sym on",
    "-DMUSY_TARGET=MUSY_TARGET_DOLPHIN",
]

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
        "mw_version": "GC/1.3.2",
        "cflags": cflags_retro_inline,
        "progress_category": progress_category,
        "objects": objects,
        "shift_jis": False,
    }


def KyotoLib(lib_name, progress_category, objects):
    return {
        "lib": lib_name + "CW" + "D" if args.debug else "",
        "mw_version": "GC/1.3.2",
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
        patch=3 if version_num == 7 else 0,
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
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00", "GM8E01_02"), "MetroTRK/nubinit.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8J01_00", "GM8E01_02"), "MetroTRK/mslsupp.c"
            ),
        ],
    ),
    RetroLib(
        "MetroidPrime",
        "game",
        [
            Object(NonMatching, "MetroidPrime/main.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "MetroidPrime/Cameras/CCameraManager.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8J01_00"), "MetroidPrime/CControlMapper.cpp"
            ),
            Object(NonMatching, "MetroidPrime/Cameras/CFirstPersonCamera.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CObjectList.cpp"
            ),
            Object(NonMatching, "MetroidPrime/Player/CPlayer.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"), "MetroidPrime/CAxisAngle.cpp"
            ),
            Object(MatchingFor("GM8E01_00"), "MetroidPrime/CEulerAngles.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "MetroidPrime/CMatrix3f_Ext.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/CArchMsgParmUserInput.cpp",
            ),
            Object(NonMatching, "MetroidPrime/CFrontEndUI.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "MetroidPrime/CInputGenerator.cpp",
                cflags=cflags_retro,
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CMainFlow.cpp"),
            Object(MatchingFor("GM8E01_00"), "MetroidPrime/CMFGame.cpp"),
            Object(NonMatching, "MetroidPrime/CCredits.cpp"),
            Object(MatchingFor("GM8E01_00"), "MetroidPrime/CSplashScreen.cpp"),
            Object(NonMatching, "MetroidPrime/CAnimData.cpp"),
            Object(NonMatching, "MetroidPrime/Factories/CCharacterFactory.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Factories/CAssetFactory.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "MetroidPrime/Tweaks/CTweakPlayer.cpp",
            ),
            Object(NonMatching, "MetroidPrime/Tweaks/CTweaks.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Tweaks/CTweakGame.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Weapons/CGameProjectile.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "MetroidPrime/Player/CPlayerGun.cpp"),
            Object(NonMatching, "MetroidPrime/CStateManager.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "MetroidPrime/CEntity.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/CArchMsgParmInt32.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "MetroidPrime/CArchMsgParmInt32Int32VoidPtr.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "MetroidPrime/CArchMsgParmNull.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "MetroidPrime/CArchMsgParmReal32.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "MetroidPrime/Decode.cpp"),
            Object(NonMatching, "MetroidPrime/CIOWinManager.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CIOWin.cpp"),
            Object(NonMatching, "MetroidPrime/CActor.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CWorld.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "MetroidPrime/Tweaks/CTweakParticle.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00", "GM8E01_02"), "MetroidPrime/Clamp_int.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/CArchMsgParmControllerStatus.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CExplosion.cpp"
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CEffect.cpp"),
            Object(NonMatching, "MetroidPrime/Cameras/CGameCamera.cpp"),
            Object(NonMatching, "MetroidPrime/CGameArea.cpp"),
            Object(NonMatching, "MetroidPrime/HUD/CSamusHud.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CAnimationDatabaseGame.cpp"),
            Object(NonMatching, "MetroidPrime/CTransitionDatabaseGame.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Tweaks/CTweakPlayerControl.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Tweaks/CTweakPlayerGun.cpp",
                cflags=cflags_retro,
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CPauseScreen.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Tweaks/CTweakGui.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptActor.cpp"
            ),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptTrigger.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptWaypoint.cpp",
            ),
            Object(NonMatching, "MetroidPrime/Enemies/CPatterned.cpp"),
            Object(MatchingFor("GM8E01_00"), "MetroidPrime/ScriptObjects/CScriptDoor.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CStateMachine.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CMapArea.cpp"),
            Object(NonMatching, "MetroidPrime/Cameras/CBallCamera.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptEffect.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Weapons/CBomb.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Tweaks/CTweakBall.cpp",
            ),
            Object(
                NonMatching,
                "MetroidPrime/Player/CPlayerState.cpp",
                cflags=cflags_retro,
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptTimer.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Cameras/CCinematicCamera.cpp"),
            Object(NonMatching, "MetroidPrime/CAutoMapper.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptCounter.cpp",
            ),
            Object(NonMatching, "MetroidPrime/CMapWorld.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CAi.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Enemies/PatternedCastTo.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), 
                "MetroidPrime/TCastTo.cpp",
            ),
            Object(
                NonMatching,
                "MetroidPrime/TypesMatch.cpp",
            ),
            Object(MatchingFor("GM8E01_00"), "MetroidPrime/ScriptObjects/CScriptSound.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptPlatform.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00", "GM8E01_02"), "MetroidPrime/UserNames.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptGenerator.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptCameraWaypoint.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CGameLight.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Tweaks/CTweakTargeting.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Tweaks/CTweakAutoMapper.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/CParticleGenInfoGeneric.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/CParticleGenInfo.cpp",
            ),
            Object(NonMatching, "MetroidPrime/CParticleDatabase.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Tweaks/CTweakGunRes.cpp",
                cflags=cflags_retro,
            ),
            Object(NonMatching, "MetroidPrime/CTargetReticles.cpp"),
            Object(
                NonMatching,
                "MetroidPrime/CWeaponMgr.cpp",
                cflags=cflags_retro,
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptPickup.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CDamageInfo.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "MetroidPrime/CMemoryDrawEnum.cpp",
            ),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptDock.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptCameraHint.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptLoader.cpp"),
            Object(NonMatching, "MetroidPrime/CSamusDoll.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Factories/CStateMachineFactory.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Weapons/CPlasmaBeam.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Weapons/CPowerBeam.cpp",
            ),
            Object(MatchingFor("GM8E01_00"), "MetroidPrime/Weapons/CWaveBeam.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Weapons/CIceBeam.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CScriptMailbox.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptRelay.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptSpawnPoint.cpp"
            ),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptRandomRelay.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CBeetle.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00", "GM8E01_02"),
                "MetroidPrime/HUD/CHUDMemoParms.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptHUDMemo.cpp",
            ),
            
            Object(
                Equivalent, 
                "MetroidPrime/CMappableObject.cpp",
            ),
            Object(NonMatching, "MetroidPrime/Player/CPlayerCameraBob.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptCameraFilterKeyframe.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptCameraBlurKeyframe.cpp",
            ),
            Object(NonMatching, "MetroidPrime/Cameras/CCameraFilter.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CMorphBall.cpp"),
            Object(
                NonMatching, "MetroidPrime/ScriptObjects/CScriptDamageableTrigger.cpp"
            ),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptDebris.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptCameraShaker.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptActorKeyframe.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/CConsoleOutputWindow.cpp",
            ),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptWater.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Weapons/CWeapon.cpp",
            ),
            Object(MatchingFor("GM8E01_00"), "MetroidPrime/CDamageVulnerability.cpp"),
            Object(NonMatching, "MetroidPrime/CActorLights.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "MetroidPrime/Enemies/CPatternedInfo.cpp",
            ),
            Object(MatchingFor("GM8E01_00"), "MetroidPrime/CSimpleShadow.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/CActorParameters.cpp",
            ),
            Object(NonMatching, "MetroidPrime/CInGameGuiManager.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CWarWasp.cpp"),
            Object(MatchingFor("GM8E01_00"), "MetroidPrime/CWorldShadow.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CAudioStateWin.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Player/CPlayerVisor.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CModelData.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/CDecalManager.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00"), "MetroidPrime/ScriptObjects/CScriptSpiderBallWaypoint.cpp"
            ),
            Object(NonMatching, "MetroidPrime/Enemies/CBloodFlower.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "MetroidPrime/TGameTypes.cpp"
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CPhysicsActor.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00", "GM8E01_02"), "MetroidPrime/CPhysicsState.cpp"
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "MetroidPrime/CRipple.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), 
                "MetroidPrime/CFluidUVMotion.cpp",
                extra_cflags=['-pragma "inline_max_size(250)"'],
            ),
            Object(
                MatchingFor("GM8E01_00"),
                "MetroidPrime/CRippleManager.cpp",
                # TODO: inline ripple fill at the common limit.
                extra_cflags=['-pragma "inline_max_size(260)"'],
            ),
            Object(NonMatching, "MetroidPrime/Player/CGrappleArm.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CSpacePirate.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptCoverPoint.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Cameras/CPathCamera.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CFluidPlane.cpp"),
            Object(NonMatching, "MetroidPrime/CFluidPlaneManager.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptGrapplePoint.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CHUDBillboardEffect.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Enemies/CFlickerBat.cpp"),
            Object(
                MatchingFor("GM8E01_00"),
                "MetroidPrime/BodyState/CBodyStateCmdMgr.cpp",
                cflags=[*cflags_retro, "-inline auto"],
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/BodyState/CBodyStateInfo.cpp",
            ),
            Object(NonMatching, "MetroidPrime/BodyState/CBSAttack.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/BodyState/CBSDie.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/BodyState/CBSFall.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/BodyState/CBSGetup.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/BodyState/CBSKnockBack.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/BodyState/CBSLieOnGround.cpp",
            ),
            Object(NonMatching, "MetroidPrime/BodyState/CBSLocomotion.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/BodyState/CBSStep.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/BodyState/CBSTurn.cpp"),
            Object(
                MatchingFor("GM8E01_00"),
                "MetroidPrime/BodyState/CBodyController.cpp",
                cflags=cflags_retro,
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/BodyState/CBSLoopAttack.cpp",
            ),
            Object(MatchingFor("GM8E01_02"), "MetroidPrime/Weapons/CTargetableProjectile.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/BodyState/CBSLoopReaction.cpp",
            ),
            Object(NonMatching, "MetroidPrime/CSteeringBehaviors.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/BodyState/CBSGroundHit.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CChozoGhost.cpp"),
            Object(MatchingFor("GM8E01_00"), "MetroidPrime/Enemies/CFireFlea.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/BodyState/CBSSlide.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/BodyState/CBSHurled.cpp"),
            Object(NonMatching, "MetroidPrime/BodyState/CBSJump.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/BodyState/CBSGenerate.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Enemies/CPuddleSpore.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/BodyState/CBSTaunt.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "MetroidPrime/CSortedLists.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptDebugCameraWaypoint.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptSpiderBallAttractionSurface.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/BodyState/CBSScripted.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Enemies/CPuddleToadGamma.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/ScriptObjects/CScriptDistanceFog.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/BodyState/CBSProjectileAttack.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Weapons/CPowerBomb.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Enemies/CMetaree.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/ScriptObjects/CScriptDockAreaChange.cpp"),
            Object(
                NonMatching, "MetroidPrime/ScriptObjects/CScriptSpecialFunction.cpp"
            ),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptActorRotate.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Player/CFidget.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Enemies/CSpankWeed.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Enemies/CParasite.cpp"
            ),
            Object(NonMatching, "MetroidPrime/Player/CSamusFaceReflection.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptPlayerHint.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Enemies/CRipper.cpp",
            ),
            Object(NonMatching, "MetroidPrime/Cameras/CCameraShakeData.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/ScriptObjects/CScriptPickupGenerator.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptPointOfInterest.cpp",
            ),
            Object(NonMatching, "MetroidPrime/Enemies/CDrone.cpp"),
            Object(NonMatching, "MetroidPrime/CMapWorldInfo.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Factories/CScannableObjectInfo.cpp",
                cflags=cflags_retro,
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Enemies/CMetroid.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Player/CScanDisplay.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptSteam.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptRipple.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CBoneTracking.cpp"),
            Object(MatchingFor("GM8P01_00", "GM8E01_02"), "MetroidPrime/Player/CFaceplateDecoration.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/BodyState/CBSCover.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptBallTrigger.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Weapons/CPlasmaProjectile.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CPlayerOrbit.cpp"),
            Object(NonMatching, "MetroidPrime/CGameCollision.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/CBallFilter.cpp",
                cflags=cflags_retro,
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CAABoxFilter.cpp"
            ),
            Object(NonMatching, "MetroidPrime/CGroundMovement.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CNewIntroBoss.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Weapons/CPhazonBeam.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "MetroidPrime/ScriptObjects/CScriptTargetingPoint.cpp",
            ),
            Object(NonMatching, "MetroidPrime/BodyState/CBSWallHang.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptEMPulse.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/HUD/CHudEnergyInterface.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/HUD/CHudFreeLookInterface.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/HUD/CHudHelmetInterface.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/HUD/CHudMissileInterface.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/HUD/CHudRadarInterface.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/HUD/CHudThreatInterface.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/HUD/CHudVisorBeamMenu.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/HUD/CHudDecoInterface.cpp"),
            Object(NonMatching, "MetroidPrime/Weapons/CFlameThrower.cpp"),
            Object(MatchingFor("GM8E01_00"), "MetroidPrime/Weapons/CBeamProjectile.cpp"),
            Object(NonMatching, "MetroidPrime/CFluidPlaneCPU.cpp"),
            Object(
                NonMatching,
                "MetroidPrime/CFluidPlaneDoor.cpp",
                cflags=cflags_retro,
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptRoomAcoustics.cpp",
            ),
            Object(NonMatching, "MetroidPrime/Enemies/CIceSheegoth.cpp"),
            Object(NonMatching, "MetroidPrime/CCollisionActorManager.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CCollisionActor.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptPlayerActor.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Tweaks/CTweakPlayerRes.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "MetroidPrime/Enemies/CBurstFire.cpp",
            ),
            Object(NonMatching, "MetroidPrime/Enemies/CFlaahgra.cpp"),
            Object(MatchingFor("GM8E01_00"), "MetroidPrime/Player/CPlayerEnergyDrain.cpp"),
            Object(NonMatching, "MetroidPrime/CFlameWarp.cpp"),
            Object(NonMatching, "MetroidPrime/Weapons/CIceImpact.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/GameObjectLists.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Weapons/CAuxWeapon.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Weapons/CGunWeapon.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/ScriptObjects/CScriptAreaAttributes.cpp"),
            Object(NonMatching, "MetroidPrime/Weapons/CWaveBuster.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Player/CStaticInterference.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CMetroidBeta.cpp"),
            Object(NonMatching, "MetroidPrime/PathFinding/CPathFindSearch.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/PathFinding/CPathFindRegion.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/PathFinding/CPathFindArea.cpp",
                cflags=[*cflags_retro, "-inline auto"],
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/PathFinding/CPathFindSpline.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Weapons/GunController/CGunController.cpp"
            ),
            Object(NonMatching, "MetroidPrime/Weapons/GunController/CGSFreeLook.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Weapons/GunController/CGSComboFire.cpp",
            ),
            Object(NonMatching, "MetroidPrime/HUD/CHudBallInterface.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Tweaks/CTweakGuiColors.cpp"),
            Object(MatchingFor("GM8E01_00"), "MetroidPrime/ScriptObjects/CFishCloud.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00", "GM8E01_02"), "MetroidPrime/CHealthInfo.cpp"
            ),
            Object(NonMatching, "MetroidPrime/Player/CGameState.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptVisorFlare.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptWorldTeleporter.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptVisorGoo.cpp",
            ),
            Object(MatchingFor("GM8E01_00"), "MetroidPrime/Enemies/CJellyZap.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptControllerAction.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8P01_00"), "MetroidPrime/Weapons/GunController/CGunMotion.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptSwitch.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/BodyState/CABSIdle.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/BodyState/CABSFlinch.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/BodyState/CABSAim.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptPlayerStateChange.cpp",
            ),
            Object(NonMatching, "MetroidPrime/Enemies/CThardus.cpp"),
            Object(NonMatching, "MetroidPrime/CActorModelParticles.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CWallCrawlerSwarm.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptAiJumpPoint.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CMessageScreen.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Enemies/CFlaahgraTentacle.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "MetroidPrime/Weapons/GunController/CGSFidget.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/BodyState/CABSReaction.cpp",
            ),
            Object(NonMatching, "MetroidPrime/Weapons/CIceProjectile.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Enemies/CPatternedAiFunctions.cpp",
            ),
            Object(NonMatching, "MetroidPrime/Enemies/CFlyingPirate.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptColorModulate.cpp",
            ),
            Object(NonMatching, "MetroidPrime/CMapUniverse.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Enemies/CThardusRockProjectile.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CInventoryScreen.cpp"),
            Object(MatchingFor("GM8E01_00"), "MetroidPrime/CVisorFlare.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Enemies/CFlaahgraPlants.cpp",
            ),
            Object(MatchingFor("GM8E01_00"), "MetroidPrime/CWorldTransManager.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptMidi.cpp"),
            Object(
                NonMatching,
                "MetroidPrime/ScriptObjects/CScriptStreamedMusic.cpp",
                cflags=cflags_retro,
            ),
            Object(NonMatching, "MetroidPrime/CRagDoll.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Player/CGameOptions.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CRepulsor.cpp",
            ),
            Object(NonMatching, "MetroidPrime/CEnvFxManager.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Weapons/CEnergyProjectile.cpp",
                cflags=cflags_retro,
            ),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptGunTurret.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Weapons/CProjectileInfo.cpp",
            ),
            Object(
                NonMatching,
                "MetroidPrime/CInGameTweakManager.cpp",
                cflags=cflags_retro,
            ),
            Object(NonMatching, "MetroidPrime/Enemies/CBabygoth.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CEyeBall.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CIkChain.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptCameraPitchVolume.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/RumbleFxTable.cpp"
            ),
            Object(NonMatching, "MetroidPrime/Enemies/CElitePirate.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CRumbleManager.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Enemies/CBouncyGrenade.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Enemies/CGrenadeLauncher.cpp",
            ),
            Object(MatchingFor("GM8E01_00"), "MetroidPrime/Weapons/CShockWave.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Enemies/CRipperControlledPlatform.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Enemies/CKnockBackMgr.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/CScriptLayerManager.cpp",
            ),
            Object(NonMatching, "MetroidPrime/Enemies/CMagdolite.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CTeamAiMgr.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Enemies/CSnakeWeedSwarm.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "MetroidPrime/Cameras/CBallCameraFailsafeState.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Enemies/CScriptContraption.cpp"),
            Object(NonMatching, "MetroidPrime/ScriptObjects/CScriptSpindleCamera.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptMemoryRelay.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CPauseScreenFrame.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Enemies/CAtomicAlpha.cpp"),
            Object(NonMatching, "MetroidPrime/CLogBookScreen.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CGBASupport.cpp"
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CMemoryCard.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptCameraHintTrigger.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Enemies/CAmbientAI.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CMemoryCardDriver.cpp"),
            Object(NonMatching, "MetroidPrime/CSaveGameScreen.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Enemies/CAtomicBeta.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Weapons/CElectricBeamProjectile.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CRidley.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CPuffer.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CFire.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/CPauseScreenBlur.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Enemies/CTryclops.cpp"),
            Object(NonMatching, "MetroidPrime/Weapons/CNewFlameThrower.cpp"),
            Object(NonMatching, "MetroidPrime/Cameras/CInterpolationCamera.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Enemies/CSeedling.cpp"),
            Object(NonMatching, "MetroidPrime/CGameHintInfo.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CWallWalker.cpp"),
            Object(NonMatching, "MetroidPrime/CErrorOutputWindow.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CRainSplashGenerator.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/CWorldSaveGameInfo.cpp",
                cflags=cflags_retro,
            ),
            Object(NonMatching, "MetroidPrime/CFluidPlaneRender.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Enemies/CBurrower.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CMetroidPrime.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptBeam.cpp",
            ),
            Object(NonMatching, "MetroidPrime/Enemies/CMetroidPrimeStage2.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/CMetroidPrimeRelay.cpp"),
            Object(NonMatching, "MetroidPrime/Player/CPlayerDynamics.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/ScriptObjects/CScriptMazeNode.cpp",
            ),
            Object(MatchingFor("GM8E01_00"), "MetroidPrime/Weapons/WeaponTypes.cpp"),
            Object(NonMatching, "MetroidPrime/Enemies/COmegaPirate.cpp"),
            Object(MatchingFor("GM8E01_00"), "MetroidPrime/Enemies/CScriptPhazonPool.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CNESEmulator.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Enemies/CPhazonHealingNodule.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Player/CMorphBallShadow.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/Player/CPlayerStuckTracker.cpp"),
            Object(NonMatching, "MetroidPrime/CSlideShow.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Tweaks/CTweakSlideShow.cpp",
            ),
            Object(MatchingFor("GM8E01_00"), "MetroidPrime/CArtifactDoll.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CProjectedShadow.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CPreFrontEnd.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/CGameCubeDoll.cpp"
            ),
            Object(
                NonMatching, "MetroidPrime/ScriptObjects/CScriptProjectedShadow.cpp"
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/ScriptObjects/CEnergyBall.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "MetroidPrime/ScriptObjects/CSustainedPlayerDamage.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "MetroidPrime/Enemies/CPoisonProjectile.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "MetroidPrime/Enemies/SPositionHistory.cpp",
            ),
            Object(Equivalent, "dummy.c"),
        ],
    ),
    RetroLib(
        "WorldFormat",
        "core",
        [
            Object(MatchingFor("GM8E01_00"), "WorldFormat/CAreaOctTree_Tests.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "WorldFormat/CCollisionSurface.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "WorldFormat/CMetroidModelInstance.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00", "GM8E01_02"), "WorldFormat/CAreaBspTree.cpp"
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "WorldFormat/CAreaOctTree.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "WorldFormat/CMetroidAreaCollider.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "WorldFormat/CWorldLight.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "WorldFormat/COBBTree.cpp"),
            Object(
                NonMatching,
                "WorldFormat/CCollidableOBBTree.cpp",
                cflags=cflags_retro if version_num < VERSIONS.index("GM8P01_00") else None,
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "WorldFormat/CCollidableOBBTreeGroup.cpp"
            ),
            Object(NonMatching, "WorldFormat/CPVSAreaSet.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "WorldFormat/CAreaRenderOctTree.cpp"),
        ],
    ),
    RetroLib(
        "Weapons",
        "core",
        [
            Object(NonMatching, "Weapons/CProjectileWeapon.cpp"),
            Object(NonMatching, "Weapons/CProjectileWeaponDataFactory.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Weapons/CCollisionResponseData.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "Weapons/IWeaponRenderer.cpp"
            ),
            Object(
                EquivalentFor("GM8E01_00", "GM8E01_01"), 
                "Weapons/CDecalDataFactory.cpp"
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Weapons/CDecal.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Weapons/CWeaponDescription.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00", "GM8E01_02"), "Weapons/CDecalDescription.cpp"
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
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "GuiSys/CAuiMain.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "GuiSys/CAuiMeter.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "GuiSys/CGuiCamera.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "GuiSys/CGuiCompoundWidget.cpp"
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "GuiSys/CGuiFactories.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "GuiSys/CGuiFeeHelper.cpp"),
            Object(NonMatching, "GuiSys/CGuiFrame.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "GuiSys/CGuiGroup.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "GuiSys/CGuiHeadWidget.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "GuiSys/CGuiLight.cpp"),
            Object(NonMatching, "GuiSys/CGuiModel.cpp"),
            Object(NonMatching, "GuiSys/CGuiObject.cpp", cflags=[*cflags_retro, "-inline auto"]),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "GuiSys/CGuiPane.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "GuiSys/CGuiSliderGroup.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "GuiSys/CGuiSys.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "GuiSys/CGuiTableGroup.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "GuiSys/CGuiTextPane.cpp"
            ),
            Object(NonMatching, "GuiSys/CGuiTextSupport.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "GuiSys/CGuiWidget.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "GuiSys/CGuiWidgetIdDB.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "GuiSys/CGuiWidgetDrawParms.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "GuiSys/CAuiEnergyBarT01.cpp"
            ),
            Object(NonMatching, "GuiSys/CAuiImagePane.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "GuiSys/CRepeatState.cpp"),
        ],
    ),
    RetroLib(
        "Collision",
        "core",
        [
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Collision/CCollidableAABox.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "Collision/CCollidableCollisionSurface.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "Collision/CCollisionInfo.cpp",
                cflags=cflags_retro,
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Collision/InternalColliders.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "Collision/CCollisionPrimitive.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "Collision/CMaterialList.cpp"
            ),
            Object(NonMatching, "Collision/CollisionUtil.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Collision/CCollidableSphere.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48"), "Collision/CMaterialFilter.cpp"
            ),
            Object(
                NonMatching,
                "Collision/COBBox.cpp",
                cflags=cflags_retro,
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Collision/CMRay.cpp"),
        ],
    ),
    KyotoLib(
        "Kyoto1",
        "core",
        [
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Kyoto/Basics/CBasics.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Kyoto/Basics/CStopwatch.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Kyoto/Basics/CBasicsDolphin.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "Kyoto/Alloc/CCallStackDolphin.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "Kyoto/Basics/COsContextDolphin.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Kyoto/Basics/CSWDataDolphin.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Basics/RAssertDolphin.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00", "GM8E01_02"), "Kyoto/Animation/CAnimation.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "Kyoto/Animation/CAnimationManager.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Animation/CAnimationSet.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "Kyoto/Animation/CAnimCharacterSet.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00"), 
                "Kyoto/Animation/CAnimTreeLoopIn.cpp",
                extra_cflags = ['-pragma "inline_max_size(260)"'] if version_num == 0 else [],
            ),
            Object(NonMatching, "Kyoto/Animation/CAnimTreeSequence.cpp"),
            Object(NonMatching, "Kyoto/Animation/CCharacterInfo.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00", "GM8E01_02"), "Kyoto/Animation/CCharacterSet.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Animation/CMetaAnimBlend.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "Kyoto/Animation/CMetaAnimFactory.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Animation/CMetaAnimPhaseBlend.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "Kyoto/Animation/CMetaAnimPlay.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Kyoto/Animation/CMetaAnimRandom.cpp"),
            Object(NonMatching, "Kyoto/Animation/CMetaAnimSequence.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "Kyoto/Animation/CMetaTransFactory.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Animation/CMetaTransMetaAnim.cpp"),
            Object(NonMatching, "Kyoto/Animation/CMetaTransPhaseTrans.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "Kyoto/Animation/CMetaTransSnap.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Kyoto/Animation/CMetaTransTrans.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "Kyoto/Animation/CPASAnimInfo.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "Kyoto/Animation/CPASAnimParm.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Animation/CPASAnimState.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Animation/CPASDatabase.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "Kyoto/Animation/CPASParmInfo.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Kyoto/Animation/CPrimitive.cpp"
            ),
            Object(NonMatching, "Kyoto/Animation/CSequenceHelper.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00", "GM8E01_02"), "Kyoto/Animation/CTransition.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00", "GM8E01_02"),
                "Kyoto/Animation/CTransitionManager.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Kyoto/Animation/CTreeUtils.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Animation/IMetaAnim.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Kyoto/Audio/CSfxHandle.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "Kyoto/Audio/CSfxManager.cpp",
                extra_cflags=(
                    ['-pragma "inline_max_size(140)"']
                    if version_num == 4
                    else []
                ),
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "Kyoto/Animation/CAdvancementDeltas.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Kyoto/Animation/CAnimMathUtils.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Animation/CAnimPerSegmentData.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Animation/CAnimPOIData.cpp"),
            Object(MatchingFor("GM8E01_00"), "Kyoto/Animation/CAnimSource.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8P01_00"), "Kyoto/Animation/CAnimSourceReader.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "Kyoto/Animation/CAnimSourceReaderBase.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "Kyoto/Animation/CAnimTreeAnimReaderContainer.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8P01_00"), "Kyoto/Animation/CAnimTreeBlend.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Animation/CAnimTreeContinuousPhaseBlend.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "Kyoto/Animation/CAnimTreeDoubleChild.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "Kyoto/Animation/CAnimTreeNode.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "Kyoto/Animation/CAnimTreeSingleChild.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "Kyoto/Animation/CAnimTreeTimeScale.cpp",
            ),
            Object(NonMatching, "Kyoto/Animation/CAnimTreeTransition.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "Kyoto/Animation/CAnimTreeTweenBase.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "Kyoto/Animation/CBoolPOINode.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Kyoto/Animation/CCharAnimMemoryMetrics.cpp"),
            Object(NonMatching, "Kyoto/Animation/CCharLayoutInfo.cpp"),
            Object(NonMatching, "Kyoto/Animation/CFBStreamedAnimReader.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Animation/CFBStreamedCompression.cpp"),
            Object(Matching, "Kyoto/Animation/CHierarchyPoseBuilder.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "Kyoto/Animation/CInt32POINode.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "Kyoto/Animation/CParticlePOINode.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Kyoto/Animation/CPOINode.cpp"
            ),
            Object(NonMatching, "Kyoto/Animation/CSegStatementSet.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "Kyoto/Animation/CTimeScaleFunctions.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Animation/IAnimReader.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "Kyoto/Animation/CAllFormatsAnimSource.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/CDvdRequestManager.cpp"
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Kyoto/CDvdRequest.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "Kyoto/Text/CColorInstruction.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "Kyoto/Text/CColorOverrideInstruction.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "Kyoto/Text/CDrawStringOptions.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Text/CFontInstruction.cpp"
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Text/CFontRenderState.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "Kyoto/Text/CLineExtraSpaceInstruction.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Kyoto/Text/CLineInstruction.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "Kyoto/Text/CLineSpacingInstruction.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "Kyoto/Text/CPopStateInstruction.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "Kyoto/Text/CPushStateInstruction.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Text/CRasterFont.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "Kyoto/Text/CRemoveColorOverrideInstruction.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Text/CSaveableState.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Text/CTextExecuteBuffer.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Text/CTextInstruction.cpp"
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Text/CTextParser.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Kyoto/Text/CWordBreakTables.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Text/CWordInstruction.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "Kyoto/Text/CBlockInstruction.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Kyoto/Text/CFont.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Kyoto/Graphics/CLight.cpp"),
            Object(NonMatching, "Kyoto/Graphics/CCubeModel.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Graphics/CGX.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "Kyoto/Graphics/CTevCombiners.cpp",
            ),
            Object(
                NonMatching,
                "Kyoto/Graphics/DolphinCGraphics.cpp",
                extra_cflags=(
                    ['-pragma "inline_max_size(100)"']
                    if version_num == 4
                    else []
                ),
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "Kyoto/Graphics/DolphinCPalette.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "Kyoto/Graphics/DolphinCTexture.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Math/CloseEnough.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Kyoto/Math/CMatrix3f.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Math/CMatrix4f.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Math/CNUQuaternion.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Math/CQuaternion.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Kyoto/CRandom16.cpp"),
            Object(NonMatching, "Kyoto/Math/CTransform4f.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Math/CUnitVector3f.cpp"
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Kyoto/Math/CVector2f.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"), "Kyoto/Math/CVector2i.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Kyoto/Math/CVector3d.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Math/CVector3f.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "Kyoto/Math/CVector3i.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Kyoto/Math/RMathUtils.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Kyoto/CCrc32.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"), "Kyoto/Alloc/CCircularBuffer.cpp"
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Alloc/CMemory.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Kyoto/Alloc/IAllocator.cpp"),
            Object(NonMatching, "Kyoto/PVS/CPVSVisOctree.cpp"),
            Object(NonMatching, "Kyoto/PVS/CPVSVisSet.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "Kyoto/Particles/CColorElement.cpp",
            ),
            Object(NonMatching, "Kyoto/Particles/CElementGen.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Kyoto/Particles/CIntElement.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "Kyoto/Particles/CModVectorElement.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Kyoto/Particles/CParticleDataFactory.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48"),
                "Kyoto/Particles/CParticleGen.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "Kyoto/Particles/CParticleGlobals.cpp",
            ),
            Object(NonMatching, "Kyoto/Particles/CParticleSwoosh.cpp"),
            Object(
                EquivalentFor("GM8E01_00", "GM8E01_01"), 
                "Kyoto/Particles/CParticleSwooshDataFactory.cpp"
                ,
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "Kyoto/Particles/CRealElement.cpp",
            ),
            Object(MatchingFor("GM8E01_00"), "Kyoto/Particles/CSpawnSystemKeyframeData.cpp"),
            Object(NonMatching, "Kyoto/Particles/CUVElement.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48"),
                "Kyoto/Particles/CVectorElement.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"), "Kyoto/Particles/CWarp.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"), "Kyoto/Math/CPlane.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "Kyoto/Math/CSphere.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48"),
                "Kyoto/Math/CAABox.cpp",
                extra_cflags=(
                    ['-pragma "inline_max_size(140)"']
                    if version_num >= 4
                    else []
                ),
            ),
            Object(NonMatching, "Kyoto/CFactoryMgr.cpp"),
            Object(NonMatching, "Kyoto/CResFactory.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/CResLoader.cpp"),
            Object(NonMatching, "Kyoto/rstl/rstl_map.cpp"),
            Object(MatchingFor("GM8P01_00", "GM8J01_00"), "Kyoto/rstl/rstl_allocator.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/rstl/rstl_strings.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/rstl/RstlExtras.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "Kyoto/Streams/CInputStream.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48"),
                "Kyoto/Streams/CMemoryInStream.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "Kyoto/Streams/CMemoryStreamOut.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "Kyoto/Streams/COutputStream.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48"),
                "Kyoto/Streams/CZipInputStream.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "Kyoto/Streams/CZipOutputStream.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "Kyoto/Streams/CZipSupport.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "Kyoto/CFactoryStore.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "Kyoto/CObjectReference.cpp",
            ),
            Object(NonMatching, "Kyoto/CSimplePool.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"), "Kyoto/CToken.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"), "Kyoto/IObj.cpp"
            ),
        ],
    ),
    # TODO: Merge back into Kyoto
    {
        "lib": "zlib",
        "mw_version": "GC/1.3.2",
        "cflags": cflags_runtime,
        "progress_category": "third_party",
        "shift_jis": False,
        "objects": [
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "Kyoto/zlib/adler32.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "Kyoto/zlib/deflate.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48"),
                "Kyoto/zlib/infblock.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48"),
                "Kyoto/zlib/infcodes.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "Kyoto/zlib/inffast.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48"),
                "Kyoto/zlib/inflate.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48"),
                "Kyoto/zlib/inftrees.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "Kyoto/zlib/infutil.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/zlib/trees.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "Kyoto/zlib/zutil.c"
            ),
        ],
    },
    # TODO: Merge this with zlib and Kyoto1
    KyotoLib(
        "Kyoto2",
        "core",
        [
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48"),
                "Kyoto/CARAMManager.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8P01_00"), "Kyoto/Math/CFrustumPlanes.cpp"),
            Object(NonMatching, "Kyoto/Graphics/CCubeMaterial.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8J01_00"),
                "Kyoto/Graphics/CCubeSurface.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "Kyoto/Animation/CCharAnimTime.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "Kyoto/Animation/CSegIdList.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48"),
                "Kyoto/Input/CFinalInput.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "Kyoto/Graphics/CColor.cpp",
            ),
            Object(NonMatching, "Kyoto/Audio/DolphinCAudioGroupSet.cpp"),
            Object(NonMatching, "Kyoto/Audio/DolphinCAudioSys.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/DolphinCMemoryCardSys.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "Kyoto/Input/DolphinIController.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "Kyoto/Input/CDolphinController.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48"),
                "Kyoto/DolphinCDvdFile.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "Kyoto/Alloc/CMediumAllocPool.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "Kyoto/Alloc/CSmallAllocPool.cpp",
            ),
            Object(
                NonMatching,
                "Kyoto/Alloc/CGameAllocator.cpp",
            ),
            Object(
                NonMatching,
                "Kyoto/Animation/DolphinCSkinnedModel.cpp",
                # TODO: inline optional assignment in AddSkinnedRef at the common limit.
                extra_cflags=['-pragma "inline_max_size(259)"'],
            ),
            Object(NonMatching, "Kyoto/Animation/DolphinCSkinRules.cpp"),
            Object(NonMatching, "Kyoto/Animation/DolphinCVirtualBone.cpp"),
            Object(NonMatching, "Kyoto/Graphics/DolphinCModel.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "Kyoto/Text/CStringTable.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "Kyoto/Particles/CEmitterElement.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "Kyoto/Particles/CEffectComponent.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Kyoto/Particles/CParticleData.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8P01_00"), "Kyoto/Animation/CVertexMorphEffect.cpp"),
            Object(NonMatching, "Kyoto/Animation/CSkinnedModelWithAvgNormals.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "Kyoto/CTimeProvider.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "Kyoto/CARAMToken.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48"),
                "Kyoto/Audio/CMidiManager.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "Kyoto/Text/CFontImageDef.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48"),
                "Kyoto/Text/CImageInstruction.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48"),
                "Kyoto/Text/CTextRenderBuffer.cpp",
            ),
            Object(NonMatching, "Kyoto/Graphics/CCubeMoviePlayer.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"),
                "Kyoto/Animation/CAdditiveAnimPlayback.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01"),
                "Kyoto/Particles/CParticleElectricDataFactory.cpp",
            ),
            Object(NonMatching, "Kyoto/Particles/CParticleElectric.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48"),
                "Kyoto/Graphics/DolphinCColor.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Audio/CDSPStreamManager.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48"),
                "Kyoto/CDependencyGroup.cpp",
            ),
            Object(NonMatching, "Kyoto/Audio/CStreamAudioManager.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "Kyoto/Animation/CHalfTransition.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "Kyoto/Particles/CElectricDescription.cpp"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "Kyoto/Particles/CSwooshDescription.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48"), "Kyoto/Particles/CGenDescription.cpp"),
            Object(NonMatching, "Kyoto/CPakFile.cpp"),
            Object(NonMatching, "Kyoto/Animation/CPoseAsTransformsVariableSize.cpp"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00"), "Kyoto/Input/CRumbleVoice.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "Kyoto/Input/RumbleAdsr.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48"),
                "Kyoto/Input/CRumbleGenerator.cpp",
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01"), "Kyoto/Audio/CDSPStream.cpp"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48"),
                "Kyoto/Audio/g721.cpp",
            ),
            Object(NonMatching, "Kyoto/Audio/CStaticAudioPlayer.cpp"),
            Object(MatchingFor("GM8E01_00"), "Kyoto/CFrameDelayedKiller.cpp"),
        ],
    ),
    DolphinLib(
        "ai",
        [
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "dolphin/ai.c"),
        ],
    ),
    DolphinLib(
        "ar",
        [
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "dolphin/ar/ar.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "dolphin/ar/arq.c"
            ),
        ],
    ),
    DolphinLib(
        "base",
        [
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "dolphin/PPCArch.c"
            ),
        ],
    ),
    DolphinLib(
        "db",
        [
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "dolphin/db.c"),
        ],
    ),
    DolphinLib(
        "dsp",
        [
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "dolphin/dsp/dsp.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/dsp/dsp_debug.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/dsp/dsp_task.c",
            ),
        ],
    ),
    DolphinLib(
        "dvd",
        [
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/dvd/dvdlow.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/dvd/dvdfs.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8E01_02"), 
                "dolphin/dvd/dvd.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/dvd/dvdqueue.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/dvd/dvderror.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/dvd/dvdidutils.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/dvd/dvdfatal.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/dvd/fstload.c",
            ),
        ],
    ),
    DolphinLib(
        "gx",
        [
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "dolphin/gx/GXInit.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "dolphin/gx/GXFifo.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "dolphin/gx/GXAttr.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "dolphin/gx/GXMisc.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "dolphin/gx/GXGeometry.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "dolphin/gx/GXFrameBuf.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "dolphin/gx/GXLight.c",
                extra_cflags=["-fp_contract off"],
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "dolphin/gx/GXTexture.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "dolphin/gx/GXBump.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "dolphin/gx/GXTev.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48"),
                "dolphin/gx/GXPixel.c",
                extra_cflags=["-fp_contract off"],
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/gx/GXStubs.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "dolphin/gx/GXDisplayList.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
                "dolphin/gx/GXTransform.c",
                extra_cflags=["-fp_contract off"],
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00"),
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
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "dolphin/mtx/mtx.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/mtx/mtxvec.c",
            ),
            Object(NonMatching, "dolphin/mtx/mtxstack.c"),
            Object(NonMatching, "dolphin/mtx/mtx44vec.c"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8J01_00", "GM8E01_02"),
                "dolphin/mtx/mtx44.c",
            ),
            Object(Matching, "dolphin/mtx/vec.c"),
            Object(Equivalent, "dolphin/mtx/quat.c"),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8J01_00", "GM8E01_02"), "dolphin/mtx/psmtx.c"),
        ],
    },
    DolphinLib(
        "os",
        [
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8J01_00", "GM8E01_02"),
                "dolphin/os/__start.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), 
                "dolphin/os/OS.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/os/OSAlarm.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/os/OSArena.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/os/OSAudioSystem.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/os/OSCache.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/os/OSContext.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/os/OSError.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00", "GM8E01_02"),
                "dolphin/os/OSFatal.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/os/OSFont.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/os/OSInterrupt.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/os/OSLink.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/os/OSMessage.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/os/OSMemory.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/os/OSMutex.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48"), 
                "dolphin/os/OSReboot.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/os/OSReset.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/os/OSResetSW.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_02"), 
                "dolphin/os/OSRtc.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8P01_00", "GM8E01_02"), 
                "dolphin/os/OSSync.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/os/OSThread.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/os/OSTime.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/os/__ppc_eabi_init.cpp",
            ),
        ],
    ),
    DolphinLib(
        "pad",
        [
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/pad/PadClamp.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "dolphin/pad/pad.c"
            ),
        ],
    ),
    DolphinLib(
        "vi",
        [
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "dolphin/vi.c"),
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
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02", "GM8J01_01"),
                "runtime/__mem.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/__va_arg.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "runtime/global_destructor_chain.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "runtime/CPlusLibPPC.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "runtime/NMWException.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/ptmf.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/runtime.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "runtime/__init_cpp_exceptions.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "runtime/Gecko_ExceptionPPC.cpp",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "runtime/abort_exit.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "runtime/alloc.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "runtime/ansi_files.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/ansi_fp.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/arith.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "runtime/buffer_io.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/ctype.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/locale.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "runtime/direct_io.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/file_io.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/errno.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/FILE_POS.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/mbstring.c"
            ),
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/mem.c"),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "runtime/mem_funcs.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/misc_io.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/printf.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/qsort.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/rand.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/sscanf.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/string.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/float.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/strtold.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "runtime/uart_console_io.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/wchar_io.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/e_acos.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/e_asin.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/e_atan2.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/e_exp.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/e_fmod.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/e_log.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/e_pow.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "runtime/e_rem_pio2.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/k_cos.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "runtime/k_rem_pio2.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/k_sin.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/k_tan.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/s_atan.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "runtime/s_copysign.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/s_cos.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/s_floor.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/s_frexp.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/s_ldexp.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/s_modf.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "runtime/s_nextafter.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/s_sin.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/s_tan.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/w_acos.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/w_asin.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/w_atan2.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/w_exp.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/w_fmod.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/w_log.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"), "runtime/w_pow.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
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
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/seq.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/synth.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/seq_api.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/snd_synthapi.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/stream.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/synthdata.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/synthmacros.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/synthvoice.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/synth_ac.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/synth_adsr.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/synth_vsamples.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/synth_dbtab.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/s_data.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/hw_dspctrl.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/hw_volconv.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/snd3d.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/snd_init.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/snd_math.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/snd_midictrl.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/snd_service.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/hardware.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/hw_aramdma.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/dsp_import.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/hw_dolphin.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/hw_memory.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/hw_lib_dolphin.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/profile.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/CheapReverb/creverb_fx.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/CheapReverb/creverb.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/StdReverb/reverb_fx.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/StdReverb/reverb.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/Delay/delay_fx.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "musyx/runtime/Chorus/chorus_fx.c",
            ),
        ],
    ),
    DolphinLib(
        "dtk",
        [
            Object(MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                   "dolphin/dtk.c"),
        ],
    ),
    DolphinLib(
        "card",
        [
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/card/CARDBios.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/card/CARDUnlock.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/card/CARDRdwr.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/card/CARDBlock.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/card/CARDDir.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/card/CARDCheck.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/card/CARDMount.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/card/CARDFormat.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/card/CARDOpen.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/card/CARDCreate.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/card/CARDRead.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/card/CARDWrite.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/card/CARDDelete.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/card/CARDStat.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8E01_02"),
                "dolphin/card/CARDRename.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/card/CARDNet.c",
            ),
        ],
    ),
    DolphinLib(
        "si",
        [
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/si/SIBios.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/si/SISamplingRate.c",
            ),
        ],
    ),
    DolphinLib(
        "exi",
        [
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/exi/EXIBios.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/exi/EXIUart.c",
            ),
        ],
    ),
    DolphinLib(
        "thp",
        [
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/thp/THPDec.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/thp/THPAudio.c",
            ),
        ],
    ),
    DolphinLib(
        "gba",
        [
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/GBA/GBA.c"
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/GBA/GBAGetProcessStatus.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/GBA/GBAJoyBoot.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/GBA/GBARead.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/GBA/GBAWrite.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
                "dolphin/GBA/GBAXfer.c",
            ),
            Object(
                MatchingFor("GM8E01_00", "GM8E01_01", "GM8E01_48", "GM8P01_00", "GM8E01_02"),
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
                NonMatching,
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
