#ifndef _DNAMP1_FRME_HPP_
#define _DNAMP1_FRME_HPP_

#include "../DNACommon/DNACommon.hpp"
#include "DNAMP1.hpp"
#include <athena/FileWriter.hpp>

namespace DataSpec
{
namespace DNAMP1
{
struct FRME : BigDNA
{
    DECL_EXPLICIT_DNA
    Value<atUint32> version;
    Value<atUint32> unk1;
    Value<atUint32> modelCount; // Matches MODL widgets
    Value<atUint32> unk3;
    Value<atUint32> widgetCount;

    struct Widget : BigDNA
    {
        DECL_EXPLICIT_DNA
        FRME* owner;
        DNAFourCC type;
        struct WidgetHeader : BigDNA
        {
            DECL_DNA
            String<-1> name;
            String<-1> parent;
            Value<bool> useAnimController;
            Value<bool> defaultVisible;
            Value<bool> defaultActive;
            Value<bool> cullFaces;
            Value<atVec4f> color;
            Value<atUint32> modelDrawFlags;
        } header;

        struct IWidgetInfo : BigDNA
        {
            Delete _d;
            virtual void read(athena::io::IStreamReader&) {}
            void write(athena::io::IStreamWriter&) const {}
            size_t binarySize(size_t __isz) const { return __isz; }
            virtual FourCC fourcc() const=0;
        };

        std::unique_ptr<IWidgetInfo> widgetInfo;
        Value<bool> isWorker;
        Value<atUint16> workerId = 0;
        Value<atVec3f> origin;
        Value<atVec3f> basis[3];
        Value<atVec3f> rotationCenter;
        Value<atInt32> unk1;
        Value<atInt16> unk2;

        struct BWIGInfo : IWidgetInfo
        {
            Delete _d2;
            FourCC fourcc() const { return FOURCC('BWIG'); }
        };

        struct HWIGInfo : IWidgetInfo
        {
            Delete _d2;
            FourCC fourcc() const { return FOURCC('HWIG'); }
        };

        struct CAMRInfo : IWidgetInfo
        {
            DECL_EXPLICIT_DNA
            enum class ProjectionType
            {
                Perspective,
                Orthographic
            };

            Value<ProjectionType> projectionType;
            struct IProjection : BigDNA
            {
                Delete _d;
                const ProjectionType type;
                IProjection(ProjectionType t) : type(t) {}
            };

            struct PerspectiveProjection : IProjection
            {
                DECL_DNA
                PerspectiveProjection() : IProjection(ProjectionType::Perspective) {}
                Value<float> fov;
                Value<float> aspect;
                Value<float> znear;
                Value<float> zfar;
            };

            struct OrthographicProjection : IProjection
            {
                DECL_DNA
                OrthographicProjection() : IProjection(ProjectionType::Orthographic) {}
                Value<float> left;
                Value<float> right;
                Value<float> top;
                Value<float> bottom;
                Value<float> znear;
                Value<float> zfar;
            };
            std::unique_ptr<IProjection> projection;

            FourCC fourcc() const { return FOURCC('CAMR'); }
        };

        struct MODLInfo : IWidgetInfo
        {
            DECL_DNA
            UniqueID32 model;
            enum class BlendMode
            {
                Unknown0,
                Unknown1,
                Unknown2,
                Additive
            };

            Value<atUint32> blendMode;
            Value<atUint32> lightMask;

            FourCC fourcc() const { return FOURCC('MODL'); }
        };

        struct LITEInfo : IWidgetInfo
        {
            DECL_EXPLICIT_DNA
            enum class ELightType : atUint32
            {
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

            FourCC fourcc() const { return FOURCC('LITE'); }
        };

        struct ENRGInfo : IWidgetInfo
        {
            DECL_DNA
            UniqueID32 texture;

            FourCC fourcc() const { return FOURCC('ENRG'); }
        };

        struct METRInfo : IWidgetInfo
        {
            DECL_DNA
            Value<bool> unk1;
            Value<bool> noRoundUp;
            Value<atUint32> maxCapacity;
            Value<atUint32> workerCount;

            FourCC fourcc() const { return FOURCC('METR'); }
        };

        struct GRUPInfo : IWidgetInfo
        {
            DECL_DNA
            Value<atInt16> defaultWorker;
            Value<bool> unk3;

            FourCC fourcc() const { return FOURCC('GRUP'); }
        };

        struct TBGPInfo : IWidgetInfo
        {
            DECL_DNA
            Value<atUint16> elementCount;
            Value<atUint16> unk2;
            Value<atUint32> unkEnum;
            Value<atUint16> defaultSelection;
            Value<atUint16> un4;
            Value<bool>  selectWraparound;
            Value<bool>  unk6;
            Value<float> unkFloat1;
            Value<float> unkFloat2;
            Value<bool> unk7;
            Value<float> unkFloat3;
            Value<atUint16> unk8;
            Value<atUint16> unk9;
            Value<atUint16> unk10;
            Value<atUint16> unk11;

            FourCC fourcc() const { return FOURCC('TBGP'); }
        };

        struct SLGPInfo : IWidgetInfo
        {
            DECL_DNA
            Value<float> min;
            Value<float> max;
            Value<float> cur;
            Value<float> increment;

            FourCC fourcc() const { return FOURCC('SLGP'); }
        };

        struct PANEInfo : IWidgetInfo
        {
            DECL_DNA
            Value<float> xDim;
            Value<float> zDim;
            Value<atVec3f> scaleCenter;

            FourCC fourcc() const { return FOURCC('PANE'); }
        };

        struct TXPNInfo : IWidgetInfo
        {
            enum class Justification : atUint32
            {
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

            enum class VerticalJustification : atUint32
            {
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

            DECL_EXPLICIT_DNA
            atUint32 version = 0;
            TXPNInfo() {}
            TXPNInfo(atUint32 version)
                : version(version)
            {}
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

            FourCC fourcc() const { return FOURCC('TXPN'); }
        };

        struct IMGPInfo : IWidgetInfo
        {
            DECL_DNA
            UniqueID32 texture;
            Value<atUint32> unk1;
            Value<atUint32> unk2;
            Value<atUint32> quadCoordCount;
            Vector<atVec3f, DNA_COUNT(quadCoordCount)> quadCoords;
            Value<atUint32> uvCoordCount;
            Vector<atVec2f, DNA_COUNT(uvCoordCount)>  uvCoords;

            FourCC fourcc() const { return FOURCC('IMGP'); }
        };
    };

    Vector<Widget, DNA_COUNT(widgetCount)> widgets;
    static bool Extract(const SpecBase& dataSpec,
                        PAKEntryReadStream& rs,
                        const hecl::ProjectPath& outPath,
                        PAKRouter<PAKBridge>& pakRouter,
                        const PAK::Entry& entry,
                        bool force,
                        hecl::BlenderToken& btok,
                        std::function<void(const hecl::SystemChar*)> fileChanged);
};

}
}

#endif // _DNAMP1_FRME_HPP_
