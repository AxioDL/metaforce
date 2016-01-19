#ifndef _DNAMP1_FRME_HPP_
#define _DNAMP1_FRME_HPP_

#include "../DNACommon/DNACommon.hpp"
#include "DNAMP1.hpp"
#include <Athena/FileWriter.hpp>

namespace Retro
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
            Value<bool> unk1;
            Value<bool> unk2;
            Value<bool> unk3;
            Value<bool> unk4;
            Value<atVec4f> color;
            Value<atUint32> modelDrawFlags;
        } header;

        struct IWidgetInfo : BigDNA
        {
            Delete _d;
            virtual void read(Athena::io::IStreamReader&) {}
            void write(Athena::io::IStreamWriter&) const {}
            size_t binarySize(size_t __isz) const { return __isz; }
        };

        std::unique_ptr<IWidgetInfo> widgetInfo;
        Value<bool> hasGroup;
        Value<atUint16> group;
        Value<atVec3f> origin;
        Value<atVec3f> basis[3];
        Value<atVec3f> rotationCenter;
        Value<atUint32> unk2;
        Value<bool> unk3;
        Value<bool> unk4;

        struct BWIGInfo : IWidgetInfo
        { Delete _d; };

        struct HWIGInfo : IWidgetInfo
        { Delete _d; };

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
        };

        struct MODLInfo : IWidgetInfo
        {
            DECL_DNA
            UniqueID32 model;
            Value<atUint32> blendMode;
            Value<atUint32> lightMode;
        };

        struct LITEInfo : IWidgetInfo
        {
            DECL_DNA
            Value<atUint32> unk1;
            Value<float> unk2;
            Value<float> unk3;
            Value<float> unk4;
            Value<float> unk5;
            Value<float> unk6;
            Value<float> unk7;
            Value<atUint32> unk8;
        };

        struct ENRGInfo : IWidgetInfo
        {
            DECL_DNA
            UniqueID32 texture;
        };

        struct METRInfo : IWidgetInfo
        {
            DECL_DNA
            Value<atUint16> unk1;
            Value<atUint32> value1;
            Value<atUint32> value2;
        };

        struct GRUPInfo : IWidgetInfo
        {
            DECL_DNA
            Value<bool> unk1;
            Value<bool> unk2;
            Value<bool> unk3;
        };

        struct TBGPInfo : IWidgetInfo
        {
            DECL_DNA
            Value<atUint16> unk1;
            Value<atUint16> unk2;
            Value<atUint32> unkEnum;
            Value<atUint16> unk3;
            Value<atUint16> un4;
            Value<bool>  unk5;
            Value<bool>  unk6;
            Value<float> unkFloat1;
            Value<float> unkFloat2;
            Value<bool> unk7;
            Value<float> unkFloat3;
            Value<atUint16> unk8;
            Value<atUint16> unk9;
            Value<atUint16> unk10;
            Value<atUint16> unk11;
        };

        struct SLGPInfo : IWidgetInfo
        {
            DECL_DNA
            Value<float> min;
            Value<float> max;
            Value<float> unk1;
            Value<float> unk2;
        };

        struct TXPNInfo : IWidgetInfo
        {
            DECL_EXPLICIT_DNA
            atUint32 version = 0;
            TXPNInfo() {}
            TXPNInfo(atUint32 version)
                : version(version)
            {}
            Value<float> frameVals[5];
            UniqueID32 font;
            Value<bool> unk1;
            Value<bool> unk2;
            Value<atUint32> unk3;
            Value<atUint32> unk4;
            Value<atVec4f> fillColor;
            Value<atVec4f> outlineColor;
            Value<atVec2f> pointScale;
            /* The following is only found in V1 */
            UniqueID32 jpnFont;
            Value<atInt32> jpnPointScale[2];
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
        };
    };

    Vector<Widget, DNA_COUNT(widgetCount)> widgets;
    static bool Extract(const SpecBase& dataSpec,
                        PAKEntryReadStream& rs,
                        const HECL::ProjectPath& outPath,
                        PAKRouter<PAKBridge>& pakRouter,
                        const PAK::Entry& entry,
                        bool force,
                        std::function<void(const HECL::SystemChar*)> fileChanged)
    {
        FRME frme;
        frme.read(rs);
        return true;
    }
};

}
}

#endif // _DNAMP1_FRME_HPP_
