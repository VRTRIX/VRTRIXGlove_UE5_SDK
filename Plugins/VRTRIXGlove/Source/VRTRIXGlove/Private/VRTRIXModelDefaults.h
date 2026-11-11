//============= Copyright (c) VRTRIX INC, All rights reserved. ================
//
// Purpose: Per-model tuning presets (FingerSpacing, FinalFingerSpacing and the
//          thumb offsets) and the rules that decide which glove model is in
//          effect and which tuning values are pushed to the server.
//
//          Header-only C++14, no includes. Declare VRTRIX::GLOVEVERSION before
//          including this file; in the SDK it comes from
//          IVRTRIXIMUEventHandler.h.
//
//          This copy in the C++ SDK is the canonical one. The UE plugin reuses
//          the file byte-for-byte and the Unity SDK ports it to C#, so keep the
//          names free of engine macros and keep the preset table between the
//          PRESET-TABLE markers as plain literals, in the same order.
//
//=============================================================================

#pragma once

namespace VRTRIX {
namespace ModelDefaults {

    //! Thumb offset or any other 3-component tuning value.
    struct Vec3 { double x; double y; double z; };

    //! Number of thumb joints with an offset: 0 proximal, 1 intermediate, 2 distal.
    constexpr int kThumbJointCount = 3;

    //! Index of the thumb proximal joint in Tuning::thumbOffset.
    constexpr int kThumbProximalJoint = 0;

    //! Tuning values pushed to the server for one hand.
    struct Tuning {
        double fingerSpacing;
        double finalFingerSpacing;
        Vec3 thumbOffset[kThumbJointCount];  //!< 0 proximal, 1 intermediate, 2 distal
    };

    //! A value within this distance (per component) of a factory value counts as that factory value.
    constexpr double kTolerance = 1e-3;

    //! One row of the preset table.
    struct PresetRow {
        GLOVEVERSION version;
        double fingerSpacing;
        double finalFingerSpacing;
        Vec3 rightThumbProximal;  //!< right hand, thumb proximal joint
        Vec3 otherThumbOffset;    //!< every other thumb joint, both hands
    };

    //! Vec3 under a name without digits, so that the only numbers between the
    //! PRESET-TABLE markers are the table values themselves.
    using Offset = Vec3;

    // Preset rows, column order: model, FingerSpacing, FinalFingerSpacing,
    // right thumb proximal (x, y, z), every other thumb offset (x, y, z).
    // Models without a row (DK1, DK2, PRO7, PRO12) have no preset.
    // Then the factory-value sets, in this order: FingerSpacing,
    // FinalFingerSpacing, right thumb proximal, every other thumb offset.
    // PRESET-TABLE-BEGIN
    constexpr PresetRow kPresetRows[] = {
        { PRO,   5.0, 2.0, { 0.0, 0.0, -20.0 }, { 0.0, 0.0, 0.0 } },
        { PRO11, 2.0, 2.0, { 0.0, 0.0, 0.0 },   { 0.0, 0.0, 0.0 } },
    };
    constexpr double kFactoryFingerSpacing[] = { 2.0, 5.0 };
    constexpr double kFactoryFinalFingerSpacing[] = { 2.0, 5.0 };
    constexpr Offset kFactoryRightThumbProximal[] = { { 0.0, 0.0, 0.0 }, { 0.0, 0.0, -20.0 } };
    constexpr Offset kFactoryOtherThumbOffset[] = { { 0.0, 0.0, 0.0 } };
    // PRESET-TABLE-END

    namespace Detail {

        inline bool NearValue(double a, double b)
        {
            const double difference = a - b;
            return difference <= kTolerance && -difference <= kTolerance;
        }

        inline bool NearOffset(const Vec3& a, const Vec3& b)
        {
            return NearValue(a.x, b.x) && NearValue(a.y, b.y) && NearValue(a.z, b.z);
        }

        inline const PresetRow* FindPresetRow(GLOVEVERSION version)
        {
            for (const PresetRow& row : kPresetRows) {
                if (row.version == version) {
                    return &row;
                }
            }
            return nullptr;
        }

        inline bool UsesRightThumbProximal(bool rightHand, int joint)
        {
            return rightHand && joint == kThumbProximalJoint;
        }

    }  // namespace Detail

    //! True only for models with a preset row (PRO and PRO11).
    inline bool HasPreset(GLOVEVERSION version)
    {
        return Detail::FindPresetRow(version) != nullptr;
    }

    //! Preset tuning of a model for one hand. Requires HasPreset(version);
    //! otherwise every value is 0.
    inline Tuning Preset(GLOVEVERSION version, bool rightHand)
    {
        Tuning preset = {};
        const PresetRow* row = Detail::FindPresetRow(version);
        if (row == nullptr) {
            return preset;
        }
        preset.fingerSpacing = row->fingerSpacing;
        preset.finalFingerSpacing = row->finalFingerSpacing;
        for (int joint = 0; joint < kThumbJointCount; ++joint) {
            preset.thumbOffset[joint] = Detail::UsesRightThumbProximal(rightHand, joint)
                ? row->rightThumbProximal
                : row->otherThumbOffset;
        }
        return preset;
    }

    //! True when v is a FingerSpacing that an official SDK or demo has shipped.
    inline bool IsFactoryFingerSpacing(double v)
    {
        for (const double factory : kFactoryFingerSpacing) {
            if (Detail::NearValue(v, factory)) {
                return true;
            }
        }
        return false;
    }

    //! True when v is a FinalFingerSpacing that an official SDK or demo has shipped.
    inline bool IsFactoryFinalFingerSpacing(double v)
    {
        for (const double factory : kFactoryFinalFingerSpacing) {
            if (Detail::NearValue(v, factory)) {
                return true;
            }
        }
        return false;
    }

    //! True when v is a shipped thumb offset for this hand and joint
    //! (0 proximal, 1 intermediate, 2 distal). Any other joint index is false.
    inline bool IsFactoryThumbOffset(bool rightHand, int joint, const Vec3& v)
    {
        if (joint < 0 || joint >= kThumbJointCount) {
            return false;
        }
        if (Detail::UsesRightThumbProximal(rightHand, joint)) {
            for (const Offset& factory : kFactoryRightThumbProximal) {
                if (Detail::NearOffset(v, factory)) {
                    return true;
                }
            }
            return false;
        }
        for (const Offset& factory : kFactoryOtherThumbOffset) {
            if (Detail::NearOffset(v, factory)) {
                return true;
            }
        }
        return false;
    }

    //! Tuning to push for one hand. With useModelDefaults on and a preset for
    //! the effective model, each item that holds a factory value is replaced by
    //! that model's preset; every other item (the customer's own value) is kept.
    //! Otherwise current is returned unchanged.
    inline Tuning Resolve(const Tuning& current, GLOVEVERSION effective, bool rightHand, bool useModelDefaults)
    {
        if (!useModelDefaults || !HasPreset(effective)) {
            return current;
        }
        const Tuning preset = Preset(effective, rightHand);
        Tuning resolved = current;
        resolved.fingerSpacing = IsFactoryFingerSpacing(current.fingerSpacing)
            ? preset.fingerSpacing
            : current.fingerSpacing;
        resolved.finalFingerSpacing = IsFactoryFinalFingerSpacing(current.finalFingerSpacing)
            ? preset.finalFingerSpacing
            : current.finalFingerSpacing;
        for (int joint = 0; joint < kThumbJointCount; ++joint) {
            resolved.thumbOffset[joint] = IsFactoryThumbOffset(rightHand, joint, current.thumbOffset[joint])
                ? preset.thumbOffset[joint]
                : current.thumbOffset[joint];
        }
        return resolved;
    }

    //! Model in effect: the server-reported one when auto-detect is on and the
    //! server reported a model, otherwise the manually selected one.
    inline GLOVEVERSION EffectiveVersion(bool autoDetect, bool reported, GLOVEVERSION reportedVersion, GLOVEVERSION manual)
    {
        return (autoDetect && reported) ? reportedVersion : manual;
    }

    //! "DK1", "DK2", "PRO", "PRO7", "PRO11" or "PRO12"; "UNKNOWN" for any other value.
    inline const char* VersionName(GLOVEVERSION version)
    {
        switch (version) {
        case DK1:   return "DK1";
        case DK2:   return "DK2";
        case PRO:   return "PRO";
        case PRO7:  return "PRO7";
        case PRO11: return "PRO11";
        case PRO12: return "PRO12";
        default:    return "UNKNOWN";
        }
    }

}  // namespace ModelDefaults
}  // namespace VRTRIX
