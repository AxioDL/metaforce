#include "MetroidPrime/Tweaks/CTweakPlayerControl.hpp"

#if VERSION >= VERSION_R3IJ_00

#include "float.h"

namespace {
const CTweakPlayerControl::SPhysicalControl skNullPhysicalControl;
const CTweakPlayerControl::SCommandDescription skNullCommandDescription(
    CControlMapper::kC_Forward, CTweakPlayerControl::kCT_Physical,
    CTweakPlayerControl::SPhysicalControl(CFinalInput::kPC_None, CMayaSpline()));

const CMayaSplineKnot skControlResponseKnotsPreset0[128] = {
    // Curve 0
    CMayaSplineKnot(0.85f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(0.95f, 1.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 1
    CMayaSplineKnot(0.85f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(0.95f, 1.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 2
    CMayaSplineKnot(10.f, 1.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(55.f, 0.6f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 3
    CMayaSplineKnot(10.f, 1.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(60.f, 0.6f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 4
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(0.9f, 0.6f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 5
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(0.9f, 0.6f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 6
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(1.f, 0.9f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 7
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(1.f, 0.9f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 8
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(0.9f, 0.59f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 9
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(0.9f, 0.6f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 10
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(1.f, 1.f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 11
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(1.f, 0.6f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 12
    CMayaSplineKnot(0.45f, 0.f, CMayaSplineKnot::kTT_Fixed, CMayaSplineKnot::kTT_Fixed,
                    CAbsAngle::FromRadians(0.0069440017f), CAbsAngle::FromRadians(0.0069440017f)),
    CMayaSplineKnot(0.587169f, 0.153224f, CMayaSplineKnot::kTT_Smooth, CMayaSplineKnot::kTT_Smooth),
    CMayaSplineKnot(0.815204f, 0.917458f, CMayaSplineKnot::kTT_Smooth, CMayaSplineKnot::kTT_Smooth),
    CMayaSplineKnot(0.9f, 1.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 13
    CMayaSplineKnot(0.45f, 0.f, CMayaSplineKnot::kTT_Fixed, CMayaSplineKnot::kTT_Fixed,
                    CAbsAngle::FromRadians(0.0069440017f), CAbsAngle::FromRadians(0.0069440017f)),
    CMayaSplineKnot(0.587169f, 0.153224f, CMayaSplineKnot::kTT_Smooth, CMayaSplineKnot::kTT_Smooth),
    CMayaSplineKnot(0.815204f, 0.917458f, CMayaSplineKnot::kTT_Smooth, CMayaSplineKnot::kTT_Smooth),
    CMayaSplineKnot(0.9f, 1.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 14
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(0.42f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(0.596421f, 7.137164f, CMayaSplineKnot::kTT_Smooth, CMayaSplineKnot::kTT_Smooth),
    CMayaSplineKnot(0.827725f, 30.983269f, CMayaSplineKnot::kTT_Smooth,
                    CMayaSplineKnot::kTT_Smooth),
    CMayaSplineKnot(0.95f, 35.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 15
    CMayaSplineKnot(0.7f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(0.75211f, -4.847367f, CMayaSplineKnot::kTT_Smooth, CMayaSplineKnot::kTT_Smooth),
    CMayaSplineKnot(0.906051f, -31.923761f, CMayaSplineKnot::kTT_Smooth,
                    CMayaSplineKnot::kTT_Smooth),
    CMayaSplineKnot(0.95f, -35.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
};

const CMayaSplineKnot skControlResponseKnotsPreset1[128] = {
    // Curve 0
    CMayaSplineKnot(0.1f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(0.708401f, 0.331429f, CMayaSplineKnot::kTT_Smooth, CMayaSplineKnot::kTT_Smooth),
    CMayaSplineKnot(0.9f, 1.f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 1
    CMayaSplineKnot(0.1f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(0.708401f, 0.331429f, CMayaSplineKnot::kTT_Smooth, CMayaSplineKnot::kTT_Smooth),
    CMayaSplineKnot(0.9f, 1.f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 2
    CMayaSplineKnot(10.f, 1.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(55.f, 0.6f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 3
    CMayaSplineKnot(10.f, 1.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(60.f, 0.6f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 4
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(0.9f, 0.6f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 5
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(0.9f, 0.6f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 6
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(1.f, 0.9f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 7
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(1.f, 0.9f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 8
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(0.9f, 0.6f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 9
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(0.9f, 0.6f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 10
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(1.f, 1.f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 11
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(1.f, 0.6f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 12
    CMayaSplineKnot(0.45f, 0.f, CMayaSplineKnot::kTT_Fixed, CMayaSplineKnot::kTT_Fixed,
                    CAbsAngle::FromRadians(0.0069440017f), CAbsAngle::FromRadians(0.0069440017f)),
    CMayaSplineKnot(0.587169f, 0.153224f, CMayaSplineKnot::kTT_Smooth, CMayaSplineKnot::kTT_Smooth),
    CMayaSplineKnot(0.815204f, 0.917458f, CMayaSplineKnot::kTT_Smooth, CMayaSplineKnot::kTT_Smooth),
    CMayaSplineKnot(0.9f, 1.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 13
    CMayaSplineKnot(0.45f, 0.f, CMayaSplineKnot::kTT_Fixed, CMayaSplineKnot::kTT_Fixed,
                    CAbsAngle::FromRadians(0.0069440017f), CAbsAngle::FromRadians(0.0069440017f)),
    CMayaSplineKnot(0.587169f, 0.153224f, CMayaSplineKnot::kTT_Smooth, CMayaSplineKnot::kTT_Smooth),
    CMayaSplineKnot(0.815204f, 0.917458f, CMayaSplineKnot::kTT_Smooth, CMayaSplineKnot::kTT_Smooth),
    CMayaSplineKnot(0.9f, 1.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 14
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(0.368691f, 8.536812f, CMayaSplineKnot::kTT_Smooth, CMayaSplineKnot::kTT_Smooth),
    CMayaSplineKnot(0.560058f, 30.828072f, CMayaSplineKnot::kTT_Smooth,
                    CMayaSplineKnot::kTT_Smooth),
    CMayaSplineKnot(0.699948f, 35.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(1.f, 35.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 15
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(0.412723f, -7.823048f, CMayaSplineKnot::kTT_Smooth,
                    CMayaSplineKnot::kTT_Smooth),
    CMayaSplineKnot(0.801283f, -46.70173f, CMayaSplineKnot::kTT_Smooth,
                    CMayaSplineKnot::kTT_Smooth),
    CMayaSplineKnot(1.f, -55.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
};

const CMayaSplineKnot skControlResponseKnotsPreset2[128] = {
    // Curve 0
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(0.95f, 1.f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 1
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(0.95f, 1.f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 2
    CMayaSplineKnot(10.f, 1.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(55.f, 0.6f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 3
    CMayaSplineKnot(10.f, 1.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(60.f, 0.6f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 4
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(0.9f, 0.6f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 5
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(0.9f, 0.6f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 6
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(1.f, 0.9f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 7
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(1.f, 0.9f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 8
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(0.9f, 0.6f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 9
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(0.9f, 0.6f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 10
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(1.f, 1.f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 11
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Linear),
    CMayaSplineKnot(1.f, 0.6f, CMayaSplineKnot::kTT_Linear, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 12
    CMayaSplineKnot(0.45f, 0.f, CMayaSplineKnot::kTT_Fixed, CMayaSplineKnot::kTT_Fixed,
                    CAbsAngle::FromRadians(0.0069440017f), CAbsAngle::FromRadians(0.0069440017f)),
    CMayaSplineKnot(0.587169f, 0.153224f, CMayaSplineKnot::kTT_Smooth, CMayaSplineKnot::kTT_Smooth),
    CMayaSplineKnot(0.815204f, 0.917458f, CMayaSplineKnot::kTT_Smooth, CMayaSplineKnot::kTT_Smooth),
    CMayaSplineKnot(0.9f, 1.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 13
    CMayaSplineKnot(0.45f, 0.f, CMayaSplineKnot::kTT_Fixed, CMayaSplineKnot::kTT_Fixed,
                    CAbsAngle::FromRadians(0.0069440017f), CAbsAngle::FromRadians(0.0069440017f)),
    CMayaSplineKnot(0.587169f, 0.153224f, CMayaSplineKnot::kTT_Smooth, CMayaSplineKnot::kTT_Smooth),
    CMayaSplineKnot(0.815204f, 0.917458f, CMayaSplineKnot::kTT_Smooth, CMayaSplineKnot::kTT_Smooth),
    CMayaSplineKnot(0.9f, 1.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 14
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(0.061539f, 2.205602f, CMayaSplineKnot::kTT_Smooth, CMayaSplineKnot::kTT_Smooth),
    CMayaSplineKnot(0.66508f, 31.725725f, CMayaSplineKnot::kTT_Smooth, CMayaSplineKnot::kTT_Smooth),
    CMayaSplineKnot(0.8f, 35.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(1.f, 35.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    // Curve 15
    CMayaSplineKnot(0.f, 0.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(0.054426f, -3.078016f, CMayaSplineKnot::kTT_Smooth,
                    CMayaSplineKnot::kTT_Smooth),
    CMayaSplineKnot(0.91519f, -52.69516f, CMayaSplineKnot::kTT_Smooth, CMayaSplineKnot::kTT_Smooth),
    CMayaSplineKnot(1.f, -55.f, CMayaSplineKnot::kTT_Flat, CMayaSplineKnot::kTT_Flat),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
    CMayaSplineKnot(100000.f, 0.f, CMayaSplineKnot::kTT_Invalid, CMayaSplineKnot::kTT_Invalid),
};
} // namespace

CTweakPlayerControl::SControlAnnulus::SControlAnnulus(float centerX, float centerY,
                                                      float innerRadius, float outerRadius)
: mCenterX(centerX)
, mCenterY(centerY)
, mInnerRadius(innerRadius)
, mOuterRadius(outerRadius) {}

CTweakPlayerControl::SControlAnnulus::SControlAnnulus() {}

CTweakPlayerControl::SControlAnnulus::~SControlAnnulus() {}

CTweakPlayerControl::SControlRectangle::SControlRectangle() {}

CTweakPlayerControl::SControlRectangle::~SControlRectangle() {}

CTweakPlayerControl::SControlSector::SControlSector(float centerX, float centerY, float innerRadius,
                                                    float outerRadius, float centerAngleDegrees,
                                                    float sweepDegrees)
: mCenterX(centerX)
, mCenterY(centerY)
, mInnerRadius(innerRadius)
, mOuterRadius(outerRadius)
, mCenterAngleDegrees(centerAngleDegrees)
, mSweepDegrees(sweepDegrees) {}

CTweakPlayerControl::SControlSector::SControlSector() {}

CTweakPlayerControl::SControlSector::~SControlSector() {}

CTweakPlayerControl::SPhysicalControl::SPhysicalControl(CFinalInput::EPhysicalControl control,
                                                        const CMayaSpline& response)
: mControl(control), mResponse(response) {}

CTweakPlayerControl::SPhysicalControl::SPhysicalControl() {}

CTweakPlayerControl::SPhysicalControl::~SPhysicalControl() {}

CTweakPlayerControl::SVirtualMenu::SVirtualMenu(EVirtualMenuShape shape,
                                                const SControlAnnulus& annulus,
                                                const SControlRectangle& rectangle,
                                                const SControlSector& sector)
: mShape(shape), mAnnulus(annulus), mRectangle(rectangle), mSector(sector) {}

CTweakPlayerControl::SVirtualMenu::SVirtualMenu() {}

CTweakPlayerControl::SVirtualMenu::~SVirtualMenu() {}

CTweakPlayerControl::SCommandDescription::SCommandDescription(CControlMapper::ECommands command,
                                                              EControlType type,
                                                              const SPhysicalControl& physical)
: mCommand(command), mType(type), mPrimary(physical) {}

CTweakPlayerControl::SCommandDescription::SCommandDescription(CControlMapper::ECommands command,
                                                              EControlType type,
                                                              CFinalInput::EMotionControl motion)
: mCommand(command), mType(type), mPrimaryMotion(motion) {}

CTweakPlayerControl::SCommandDescription::SCommandDescription(CControlMapper::ECommands command,
                                                              EControlType type,
                                                              const SPhysicalControl& primary,
                                                              EControlBoolean operation,
                                                              const SPhysicalControl& secondary)
: mCommand(command)
, mType(type)
, mPrimary(primary)
, mPhysicalBoolean(operation)
, mSecondary(secondary) {}

CTweakPlayerControl::SCommandDescription::SCommandDescription(CControlMapper::ECommands command,
                                                              EControlType type,
                                                              const SVirtualMenu& menu)
: mCommand(command), mType(type), mVirtualMenu(menu) {}

CTweakPlayerControl::SCommandDescription::~SCommandDescription() {}

CControlMapper::SCommandMapping
CTweakPlayerControl::GetMappingFromDescription(const SCommandDescription& description) const {
  switch (description.mType) {
  case kCT_Physical: {
    return CControlMapper::SCommandMapping(description.mType, description.mPrimary.mControl,
                                           0);
  }
  case kCT_Virtual: {
    return CControlMapper::SCommandMapping(description.mType, description.mPrimaryMotion, 0);
  }
  case kCT_PhysicalCombination: {
    return CControlMapper::SCommandMapping(description.mType, description.mPrimary.mControl,
                                           description.mSecondary.mControl);
  }
  case kCT_VirtualCombination: {
    return CControlMapper::SCommandMapping(description.mType, description.mPrimaryMotion,
                                           description.mSecondaryMotion);
  }
  case kCT_Virtual2: {
    return CControlMapper::SCommandMapping(description.mType, description.mSwing,
                                           description.mSwing);
  }
  case kCT_VirtualMenu: {
    return CControlMapper::SCommandMapping(description.mType,
                                           description.mVirtualMenu.mShape,
                                           description.mVirtualMenu.mShape);
  }
  default:
    return CControlMapper::SCommandMapping(description.mType, 0, 0);
  }
}

const CTweakPlayerControl::SCommandDescription&
CTweakPlayerControl::GetCommandDescription(CControlMapper::ECommands command) const {
  return mCommands[command];
}

CControlMapper::SCommandMapping
CTweakPlayerControl::GetCommandMapping(CControlMapper::ECommands command) const {
  return GetMappingFromDescription(GetCommandDescription(command));
}

CTweakPlayerControl::CTweakPlayerControl(uint controlPreset)
: mResponseCurves(16), mControlPreset(controlPreset) {
  InitializeControls();
}

void CTweakPlayerControl::InitializeControls() {
  static const CMayaSpline skLinearResponse = CMayaSpline::BuildLinearSpline(0.f, 0.f, 1.f, 1.f);
  static const SCommandDescription skDefaultCommands[91] = {
      skNullCommandDescription, // 0
      SCommandDescription(
          CControlMapper::kC_Forward, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_StickUp, CMayaSpline(skLinearResponse))), // 1
      SCommandDescription(
          CControlMapper::kC_Backward, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_StickDown, CMayaSpline(skLinearResponse))), // 2
      SCommandDescription(
          CControlMapper::kC_TurnLeft, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_PointerLeft, CMayaSpline(skLinearResponse))), // 3
      SCommandDescription(
          CControlMapper::kC_TurnRight, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_PointerRight, CMayaSpline(skLinearResponse))), // 4
      SCommandDescription(
          CControlMapper::kC_StrafeLeft, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_StickLeft, CMayaSpline(skLinearResponse))), // 5
      SCommandDescription(
          CControlMapper::kC_StrafeRight, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_StickRight, CMayaSpline(skLinearResponse))), // 6
      skNullCommandDescription,                                                          // 7
      skNullCommandDescription,                                                          // 8
      SCommandDescription(
          CControlMapper::kC_LookUp, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_PointerUp, CMayaSpline(skLinearResponse))), // 9
      SCommandDescription(
          CControlMapper::kC_LookDown, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_PointerDown, CMayaSpline(skLinearResponse))), // 10
      SCommandDescription(
          CControlMapper::kC_JumpOrBoost, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_B, CMayaSpline(skLinearResponse))), // 11
      skNullCommandDescription,                                                 // 12
      SCommandDescription(
          CControlMapper::kC_FireOrBomb, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_A, CMayaSpline(skLinearResponse))), // 13
      SCommandDescription(
          CControlMapper::kC_Command14, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_A, CMayaSpline(skLinearResponse))), // 14
      skNullCommandDescription,                                                 // 15
      SCommandDescription(
          CControlMapper::kC_Command16, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_A, CMayaSpline(skLinearResponse))), // 16
      SCommandDescription(
          CControlMapper::kC_Command17, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_A, CMayaSpline(skLinearResponse))), // 17
      SCommandDescription(
          CControlMapper::kC_MissileOrPowerBomb, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_DPadDown, CMayaSpline(skLinearResponse))), // 18
      skNullCommandDescription,                                                        // 19
      skNullCommandDescription,                                                        // 20
      skNullCommandDescription,                                                        // 21
      skNullCommandDescription,                                                        // 22
      skNullCommandDescription,                                                        // 23
      SCommandDescription(CControlMapper::kC_PowerBeam, kCT_VirtualMenu,
                          SVirtualMenu(kVMS_Annulus, SControlAnnulus(320.f, 224.f, 0.f, 45.f),
                                       SControlRectangle(), SControlSector())), // 24
      SCommandDescription(
          CControlMapper::kC_IceBeam, kCT_VirtualMenu,
          SVirtualMenu(kVMS_Sector, SControlAnnulus(), SControlRectangle(),
                       SControlSector(320.f, 224.f, 45.f, 400.f, 240.f, 120.f))), // 25
      SCommandDescription(
          CControlMapper::kC_WaveBeam, kCT_VirtualMenu,
          SVirtualMenu(kVMS_Sector, SControlAnnulus(), SControlRectangle(),
                       SControlSector(320.f, 224.f, 45.f, 400.f, 0.f, 120.f))), // 26
      SCommandDescription(
          CControlMapper::kC_PlasmaBeam, kCT_VirtualMenu,
          SVirtualMenu(kVMS_Sector, SControlAnnulus(), SControlRectangle(),
                       SControlSector(320.f, 224.f, 45.f, 400.f, 120.f, 120.f))), // 27
      skNullCommandDescription,                                                   // 28
      skNullCommandDescription,                                                   // 29
      skNullCommandDescription,                                                   // 30
      SCommandDescription(
          CControlMapper::kC_OrbitObject, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_NunchukZ, CMayaSpline(skLinearResponse))), // 31
      skNullCommandDescription,                                                        // 32
      skNullCommandDescription,                                                        // 33
      skNullCommandDescription,                                                        // 34
      skNullCommandDescription,                                                        // 35
      skNullCommandDescription,                                                        // 36
      skNullCommandDescription,                                                        // 37
      skNullCommandDescription,                                                        // 38
      skNullCommandDescription,                                                        // 39
      skNullCommandDescription,                                                        // 40
      skNullCommandDescription,                                                        // 41
      skNullCommandDescription,                                                        // 42
      SCommandDescription(
          CControlMapper::kC_MapCircleUp, kCT_PhysicalCombination,
          SPhysicalControl(CFinalInput::kPC_StickUp, CMayaSpline(skLinearResponse)), kCB_AndNot,
          SPhysicalControl(CFinalInput::kPC_NunchukZ, CMayaSpline(skLinearResponse))), // 43
      SCommandDescription(
          CControlMapper::kC_MapCircleDown, kCT_PhysicalCombination,
          SPhysicalControl(CFinalInput::kPC_StickDown, CMayaSpline(skLinearResponse)), kCB_AndNot,
          SPhysicalControl(CFinalInput::kPC_NunchukZ, CMayaSpline(skLinearResponse))), // 44
      SCommandDescription(
          CControlMapper::kC_MapCircleLeft, kCT_PhysicalCombination,
          SPhysicalControl(CFinalInput::kPC_StickLeft, CMayaSpline(skLinearResponse)), kCB_AndNot,
          SPhysicalControl(CFinalInput::kPC_NunchukZ, CMayaSpline(skLinearResponse))), // 45
      SCommandDescription(
          CControlMapper::kC_MapCircleRight, kCT_PhysicalCombination,
          SPhysicalControl(CFinalInput::kPC_StickRight, CMayaSpline(skLinearResponse)), kCB_AndNot,
          SPhysicalControl(CFinalInput::kPC_NunchukZ, CMayaSpline(skLinearResponse))), // 46
      SCommandDescription(
          CControlMapper::kC_MapMoveForward, kCT_PhysicalCombination,
          SPhysicalControl(CFinalInput::kPC_StickUp, CMayaSpline(skLinearResponse)), kCB_And,
          SPhysicalControl(CFinalInput::kPC_NunchukZ, CMayaSpline(skLinearResponse))), // 47
      SCommandDescription(
          CControlMapper::kC_MapMoveBack, kCT_PhysicalCombination,
          SPhysicalControl(CFinalInput::kPC_StickDown, CMayaSpline(skLinearResponse)), kCB_And,
          SPhysicalControl(CFinalInput::kPC_NunchukZ, CMayaSpline(skLinearResponse))), // 48
      SCommandDescription(
          CControlMapper::kC_MapMoveLeft, kCT_PhysicalCombination,
          SPhysicalControl(CFinalInput::kPC_StickLeft, CMayaSpline(skLinearResponse)), kCB_And,
          SPhysicalControl(CFinalInput::kPC_NunchukZ, CMayaSpline(skLinearResponse))), // 49
      SCommandDescription(
          CControlMapper::kC_MapMoveRight, kCT_PhysicalCombination,
          SPhysicalControl(CFinalInput::kPC_StickRight, CMayaSpline(skLinearResponse)), kCB_And,
          SPhysicalControl(CFinalInput::kPC_NunchukZ, CMayaSpline(skLinearResponse))), // 50
      SCommandDescription(
          CControlMapper::kC_MapZoomIn, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_Plus, CMayaSpline(skLinearResponse))), // 51
      SCommandDescription(
          CControlMapper::kC_MapZoomOut, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_Minus, CMayaSpline(skLinearResponse))), // 52
      SCommandDescription(
          CControlMapper::kC_SpiderBall, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_NunchukZ, CMayaSpline(skLinearResponse))), // 53
      SCommandDescription(
          CControlMapper::kC_SpiderBall, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_NunchukZ, CMayaSpline(skLinearResponse))), // 54
      SCommandDescription(
          CControlMapper::kC_XRayVisor, kCT_VirtualMenu,
          SVirtualMenu(kVMS_Sector, SControlAnnulus(), SControlRectangle(),
                       SControlSector(320.f, 224.f, 45.f, 400.f, 120.f, 120.f))), // 55
      SCommandDescription(
          CControlMapper::kC_ThermalVisor, kCT_VirtualMenu,
          SVirtualMenu(kVMS_Sector, SControlAnnulus(), SControlRectangle(),
                       SControlSector(320.f, 224.f, 45.f, 400.f, 240.f, 120.f))), // 56
      SCommandDescription(
          CControlMapper::kC_ScanVisor, kCT_VirtualMenu,
          SVirtualMenu(kVMS_Sector, SControlAnnulus(), SControlRectangle(),
                       SControlSector(320.f, 224.f, 45.f, 400.f, 0.f, 120.f))), // 57
      SCommandDescription(CControlMapper::kC_CombatVisor, kCT_VirtualMenu,
                          SVirtualMenu(kVMS_Annulus, SControlAnnulus(320.f, 224.f, 0.f, 45.f),
                                       SControlRectangle(), SControlSector())), // 58
      skNullCommandDescription,                                                 // 59
      skNullCommandDescription,                                                 // 60
      skNullCommandDescription,                                                 // 61
      skNullCommandDescription,                                                 // 62
      skNullCommandDescription,                                                 // 63
      skNullCommandDescription,                                                 // 64
      skNullCommandDescription,                                                 // 65
      SCommandDescription(
          CControlMapper::kC_ScanItem, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_NunchukZ, CMayaSpline(skLinearResponse))), // 66
      SCommandDescription(
          CControlMapper::kC_PauseScreen, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_Two, CMayaSpline(skLinearResponse))), // 67
      SCommandDescription(
          CControlMapper::kC_Command68, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_One, CMayaSpline(skLinearResponse))), // 68
      skNullCommandDescription,                                                   // 69
      skNullCommandDescription,                                                   // 70
      SCommandDescription(
          CControlMapper::kC_Command70, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_Minus, CMayaSpline(skLinearResponse))), // 71
      SCommandDescription(
          CControlMapper::kC_Command70, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_Plus, CMayaSpline(skLinearResponse))), // 72
      SCommandDescription(
          CControlMapper::kC_Command73, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_B, CMayaSpline(skLinearResponse))), // 73
      SCommandDescription(
          CControlMapper::kC_Morph, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_NunchukC, CMayaSpline(skLinearResponse))), // 74
      SCommandDescription(
          CControlMapper::kC_Command75, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_NunchukC, CMayaSpline(skLinearResponse))), // 75
      SCommandDescription(
          CControlMapper::kC_VisorMenu, kCT_PhysicalCombination,
          SPhysicalControl(CFinalInput::kPC_Minus, CMayaSpline(skLinearResponse)), kCB_AndNot,
          SPhysicalControl(CFinalInput::kPC_Plus, CMayaSpline(skLinearResponse))), // 76
      skNullCommandDescription,                                                    // 77
      SCommandDescription(
          CControlMapper::kC_BeamMenu, kCT_PhysicalCombination,
          SPhysicalControl(CFinalInput::kPC_Plus, CMayaSpline(skLinearResponse)), kCB_AndNot,
          SPhysicalControl(CFinalInput::kPC_Minus, CMayaSpline(skLinearResponse))), // 78
      skNullCommandDescription,                                                     // 79
      SCommandDescription(
          CControlMapper::kC_StrafeLeft, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_StickLeft, CMayaSpline(skLinearResponse))), // 80
      SCommandDescription(
          CControlMapper::kC_StrafeRight, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_StickRight, CMayaSpline(skLinearResponse))), // 81
      SCommandDescription(
          CControlMapper::kC_Command82, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_NunchukZ, CMayaSpline(skLinearResponse))), // 82
      SCommandDescription(
          CControlMapper::kC_Command83, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_B, CMayaSpline(skLinearResponse))), // 83
      SCommandDescription(
          CControlMapper::kC_Command84, kCT_PhysicalCombination,
          SPhysicalControl(CFinalInput::kPC_Plus, CMayaSpline(skLinearResponse)), kCB_And,
          SPhysicalControl(CFinalInput::kPC_Home, CMayaSpline(skLinearResponse))), // 84
      SCommandDescription(
          CControlMapper::kC_Command85, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_Home, CMayaSpline(skLinearResponse))), // 85
      SCommandDescription(CControlMapper::kC_PowerBeamAlternative, kCT_VirtualMenu,
                          SVirtualMenu(kVMS_Annulus, SControlAnnulus(320.f, 224.f, 0.f, 45.f),
                                       SControlRectangle(), SControlSector())), // 86
      SCommandDescription(CControlMapper::kC_CombatVisorAlternative, kCT_VirtualMenu,
                          SVirtualMenu(kVMS_Annulus, SControlAnnulus(320.f, 224.f, 0.f, 45.f),
                                       SControlRectangle(), SControlSector())), // 87
      SCommandDescription(
          CControlMapper::kC_OrbitObject, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_NunchukZ, CMayaSpline(skLinearResponse))), // 88
      SCommandDescription(CControlMapper::kC_SpringBall, kCT_Virtual,
                          CFinalInput::kMC_WiimoteShakeZ), // 89
      SCommandDescription(
          CControlMapper::kC_Screenshot, kCT_Physical,
          SPhysicalControl(CFinalInput::kPC_DPadUp, CMayaSpline(skLinearResponse))), // 90
  };

  const CMayaSplineKnot* knots = skControlResponseKnotsPreset1;
  switch (mControlPreset) {
  case 0:
    knots = skControlResponseKnotsPreset0;
    break;
  case 1:
    break;
  case 2:
    knots = skControlResponseKnotsPreset2;
    break;
  }

  for (int i = 0; i < 16; ++i) {
    uint count = 0;
    for (uint j = 0; j < 8; ++j) {
      if (knots[i * 8 + j].GetInTangentType() == CMayaSplineKnot::kTT_Invalid) {
        break;
      }
      ++count;
    }
    mResponseCurves[i] = CMayaSpline::BuildSpline(&knots[i * 8], count, CMayaSpline::kCM_None,
                                                    CMayaSpline::kIT_Constant,
                                                    CMayaSpline::kIT_Constant, -FLT_MAX, FLT_MAX);
  }

  for (int i = 0; i < 91; ++i) {
    mCommands.push_back(skDefaultCommands[i]);
  }
  mCommands[CControlMapper::kC_TurnLeft].mPrimary.mResponse = GetTurnLeftResponse();
  mCommands[CControlMapper::kC_TurnRight].mPrimary.mResponse = GetTurnRightResponse();
}

const CMayaSpline& CTweakPlayerControl::GetTurnLeftResponse() const { return mResponseCurves[0]; }

const CMayaSpline& CTweakPlayerControl::GetTurnRightResponse() const {
  return mResponseCurves[1];
}

CTweakPlayerControl::SCommandDescription::SCommandDescription(const SCommandDescription& other)
: mCommand(other.mCommand)
, mType(other.mType)
, mPrimary(other.mPrimary)
, mPhysicalBoolean(other.mPhysicalBoolean)
, mSecondary(other.mSecondary)
, mPrimaryMotion(other.mPrimaryMotion)
, mVirtualBoolean(other.mVirtualBoolean)
, mSecondaryMotion(other.mSecondaryMotion)
, mSwing(other.mSwing)
, mVirtualMenu(other.mVirtualMenu) {}

CTweakPlayerControl::~CTweakPlayerControl() {}

#else

#include "Kyoto/Streams/CInputStream.hpp"

CTweakPlayerControl::~CTweakPlayerControl() {}

rstl::reserved_vector< ControlMapper::EFunctionList, 67 > LoadMappings(CInputStream& in) {
  rstl::reserved_vector< ControlMapper::EFunctionList, 67 > result;
  for (int i = 0; i < result.capacity(); ++i) {
    result.push_back(static_cast< ControlMapper::EFunctionList >(in.ReadLong()));
  }
  return result;
}

CTweakPlayerControl::CTweakPlayerControl(CInputStream& in) : m_mappings(LoadMappings(in)) {}

ControlMapper::EFunctionList
CTweakPlayerControl::GetMapping(ControlMapper::ECommands command) const {
  if (command < ControlMapper::kC_Forward || command > ControlMapper::kC_UNKNOWN - 1)
    return m_mappings[0];

  return m_mappings[command];
}

#endif
