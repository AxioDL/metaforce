#pragma once

#include "DataSpec/DNACommon/DNACommon.hpp"
#include "DNAMP1.hpp"
#include <athena/FileWriter.hpp>
#include "athena/DNAOp.hpp"

namespace DataSpec::DNAMP1 {
struct FRME : BigDNA {
  AT_DECL_EXPLICIT_DNA
  Value<atUint32> version;
  Value<atUint32> unk1;
  Value<atUint32> modelCount; // Matches MODL widgets
  Value<atUint32> unk3;
  Value<atUint32> widgetCount;

  struct Widget : BigDNA {
    AT_DECL_EXPLICIT_DNA
    FRME* owner;
    DNAFourCC type;
    struct WidgetHeader : BigDNA {
      AT_DECL_DNA
      String<-1> name;
      String<-1> parent;
      Value<bool> useAnimController;
      Value<bool> defaultVisible;
      Value<bool> defaultActive;
      Value<bool> cullFaces;
      Value<atVec4f> color;
      Value<atUint32> modelDrawFlags;
    } header;

    struct IWidgetInfo : BigDNAV {
      Delete _dBase;
      virtual FourCC fourcc() const = 0;
    };

    std::unique_ptr<IWidgetInfo> widgetInfo;
    Value<bool> isWorker;
    Value<atUint16> workerId = 0;
    Value<atVec3f> origin;
    Value<atVec3f> basis[3];
    Value<atVec3f> rotationCenter;
    Value<atInt32> unk1;
    Value<atInt16> unk2;

    struct BWIGInfo : IWidgetInfo {
      AT_DECL_DNA
      const char* DNATypeV() const override { return "FRME::BWIG"; }
      FourCC fourcc() const override { return FOURCC('BWIG'); }
    };

    struct HWIGInfo : IWidgetInfo {
      AT_DECL_DNA
      const char* DNATypeV() const override { return "FRME::HWIG"; }
      FourCC fourcc() const override { return FOURCC('HWIG'); }
    };

    struct CAMRInfo : IWidgetInfo {
      AT_DECL_EXPLICIT_DNA
      const char* DNATypeV() const override { return "FRME::CAMR"; }
      enum class ProjectionType { Perspective, Orthographic };

      Value<ProjectionType> projectionType;
      struct IProjection : BigDNAV {
        Delete _d;
        const ProjectionType type;
        IProjection(ProjectionType t) : type(t) {}
      };

      struct PerspectiveProjection : IProjection {
        AT_DECL_DNA
        AT_DECL_DNAV
        PerspectiveProjection() : IProjection(ProjectionType::Perspective) {}
        Value<float> fov;
        Value<float> aspect;
        Value<float> znear;
        Value<float> zfar;
      };

      struct OrthographicProjection : IProjection {
        AT_DECL_DNA
        AT_DECL_DNAV
        OrthographicProjection() : IProjection(ProjectionType::Orthographic) {}
        Value<float> left;
        Value<float> right;
        Value<float> top;
        Value<float> bottom;
        Value<float> znear;
        Value<float> zfar;
      };
      std::unique_ptr<IProjection> projection;

      FourCC fourcc() const override { return FOURCC('CAMR'); }
    };

    struct MODLInfo : IWidgetInfo {
      AT_DECL_DNA
      const char* DNATypeV() const override { return "FRME::MODL"; }
      UniqueID32 model;
      enum class BlendMode { Unknown0, Unknown1, Unknown2, Additive };

      Value<atUint32> blendMode;
      Value<atUint32> lightMask;

      FourCC fourcc() const override { return FOURCC('MODL'); }
    };

    struct LITEInfo : IWidgetInfo {
      AT_DECL_EXPLICIT_DNA
      const char* DNATypeV() const override { return "FRME::LITE"; }
      enum class ELightType : atUint32 {
        Spot = 0,
        Point = 1,
        Directional = 2,
        LocalAmbient = 3,
        Custom = 4,
      };

      Value<ELightType> type;
      Value<float> distC;
      Value<float> distL;
      Value<float> distQ;
      Value<float> angC;
      Value<float> angL;
      Value<float> angQ;
      Value<atUint32> loadedIdx;
      Value<float> cutoff; /* Spot only */

      FourCC fourcc() const override { return FOURCC('LITE'); }
    };

    struct ENRGInfo : IWidgetInfo {
      AT_DECL_DNA
      const char* DNATypeV() const override { return "FRME::ENRG"; }
      UniqueID32 texture;

      FourCC fourcc() const override { return FOURCC('ENRG'); }
    };

    struct METRInfo : IWidgetInfo {
      AT_DECL_DNA
      const char* DNATypeV() const override { return "FRME::METR"; }
      Value<bool> unk1;
      Value<bool> noRoundUp;
      Value<atUint32> maxCapacity;
      Value<atUint32> workerCount;

      FourCC fourcc() const override { return FOURCC('METR'); }
    };

    struct GRUPInfo : IWidgetInfo {
      AT_DECL_DNA
      const char* DNATypeV() const override { return "FRME::GRUP"; }
      Value<atInt16> defaultWorker;
      Value<bool> unk3;

      FourCC fourcc() const override { return FOURCC('GRUP'); }
    };

    struct TBGPInfo : IWidgetInfo {
      AT_DECL_DNA
      const char* DNATypeV() const override { return "FRME::TBGP"; }
      Value<atUint16> elementCount;
      Value<atUint16> unk2;
      Value<atUint32> unkEnum;
      Value<atUint16> defaultSelection;
      Value<atUint16> un4;
      Value<bool> selectWraparound;
      Value<bool> unk6;
      Value<float> unkFloat1;
      Value<float> unkFloat2;
      Value<bool> unk7;
      Value<float> unkFloat3;
      Value<atUint16> unk8;
      Value<atUint16> unk9;
      Value<atUint16> unk10;
      Value<atUint16> unk11;

      FourCC fourcc() const override { return FOURCC('TBGP'); }
    };

    struct SLGPInfo : IWidgetInfo {
      AT_DECL_DNA
      const char* DNATypeV() const override { return "FRME::SLGP"; }
      Value<float> min;
      Value<float> max;
      Value<float> cur;
      Value<float> increment;

      FourCC fourcc() const override { return FOURCC('SLGP'); }
    };

    struct PANEInfo : IWidgetInfo {
      AT_DECL_DNA
      const char* DNATypeV() const override { return "FRME::PANE"; }
      Value<float> xDim;
      Value<float> zDim;
      Value<atVec3f> scaleCenter;

      FourCC fourcc() const override { return FOURCC('PANE'); }
    };

    struct TXPNInfo : IWidgetInfo {
      const char* DNATypeV() const override { return "FRME::TXPN"; }
      enum class Justification : atUint32 {
        Left = 0,
        Center,
        Right,
        Full,
        NLeft,
        NCenter,
        NRight,
        LeftMono,
        CenterMono,
        RightMono
      };

      enum class VerticalJustification : atUint32 {
        Top = 0,
        Center,
        Bottom,
        Full,
        NTop,
        NCenter,
        NBottom,
        LeftMono,
        CenterMono,
        RightMono
      };

      AT_DECL_EXPLICIT_DNA

      atUint32 version = 0;
      TXPNInfo() {}
      TXPNInfo(atUint32 version) : version(version) {}
      Value<float> xDim;
      Value<float> zDim;
      Value<atVec3f> scaleCenter;
      UniqueID32 font;
      Value<bool> wordWrap;
      Value<bool> horizontal;
      Value<Justification> justification;
      Value<VerticalJustification> verticalJustification;
      Value<atVec4f> fillColor;
      Value<atVec4f> outlineColor;
      Value<atVec2f> blockExtent; /* In points; converted to int by loader */
      /* The following is only found in V1 */
      UniqueID32 jpnFont;
      Value<atInt32> jpnPointScale[2] = {};

      FourCC fourcc() const override { return FOURCC('TXPN'); }
    };

    struct IMGPInfo : IWidgetInfo {
      AT_DECL_DNA
      const char* DNATypeV() const override { return "FRME::IMGP"; }
      UniqueID32 texture;
      Value<atUint32> unk1;
      Value<atUint32> unk2;
      Value<atUint32> quadCoordCount;
      Vector<atVec3f, AT_DNA_COUNT(quadCoordCount)> quadCoords;
      Value<atUint32> uvCoordCount;
      Vector<atVec2f, AT_DNA_COUNT(uvCoordCount)> uvCoords;

      FourCC fourcc() const override { return FOURCC('IMGP'); }
    };
  };

  Vector<Widget, AT_DNA_COUNT(widgetCount)> widgets;
  static bool Extract(const SpecBase& dataSpec, PAKEntryReadStream& rs, const hecl::ProjectPath& outPath,
                      PAKRouter<PAKBridge>& pakRouter, const PAK::Entry& entry, bool force, hecl::blender::Token& btok,
                      std::function<void(const hecl::SystemChar*)> fileChanged);
};

} // namespace DataSpec::DNAMP1
