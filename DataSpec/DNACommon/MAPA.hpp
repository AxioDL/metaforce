#ifndef __DNACOMMON_MAPA_HPP__
#define __DNACOMMON_MAPA_HPP__

#include "DNACommon.hpp"
#include "GX.hpp"


namespace DataSpec
{
namespace DNAMAPA
{
struct MAPA : BigDNA
{
    Delete _d;
    Value<atUint32> magic;
    Value<atUint32> version;
    struct IMAPAHeader : BigDNA
    {
        Delete _d;
        virtual atUint32 visMode() const=0;
        virtual atUint32 mappableObjectCount() const=0;
        virtual atUint32 vertexCount() const=0;
        virtual atUint32 surfaceCount() const=0;
    };

    struct HeaderMP1 : IMAPAHeader
    {
        DECL_DNA
        Value<atUint32> unknown1 = 0;
        Value<atUint32> mapVisMode = 0;
        Value<atVec3f>  boundingBox[2] = {};
        Value<atUint32> moCount = 0;
        Value<atUint32> vtxCount = 0;
        Value<atUint32> surfCount = 0;
        atUint32 visMode() const { return mapVisMode; }
        atUint32 mappableObjectCount() const { return moCount;}
        atUint32 vertexCount() const  { return vtxCount; }
        atUint32 surfaceCount() const { return surfCount; }
    };

    struct HeaderMP2 : IMAPAHeader
    {
        DECL_DNA
        Value<atUint32> unknown1 = 0;
        Value<atUint32> mapVisMode = 0;
        Value<atVec3f>  boundingBox[2] = {};
        Value<atUint32> unknown3 = 0;
        Value<atUint32> unknown4 = 0;
        Value<atUint32> unknown5 = 0;
        Value<atUint32> moCount = 0;
        Value<atUint32> vtxCount = 0;
        Value<atUint32> surfCount = 0;
        atUint32 visMode() const { return mapVisMode; }
        atUint32 mappableObjectCount() const { return moCount;}
        atUint32 vertexCount() const  { return vtxCount; }
        atUint32 surfaceCount() const { return surfCount; }
    };

    struct HeaderMP3 : IMAPAHeader
    {
        DECL_DNA
        Value<atUint32> unknown1 = 0;
        Value<atUint32> mapVisMode = 0;
        Value<atVec3f>  boundingBox[2] = {};
        Value<atUint32> unknown3 = 0;
        Value<atUint32> unknown4 = 0;
        Value<atUint32> unknown5 = 0;
        Value<atUint32> unknown6 = 0;
        Value<atUint32> moCount = 0;
        Value<atUint32> vtxCount = 0;
        Value<atUint32> surfCount = 0;
        Value<atUint32> internalNameLength = 0;
        Value<atUint32> unknown7 = 0;
        String<DNA_COUNT(internalNameLength)> internalName;
        atUint32 visMode() const { return mapVisMode; }
        atUint32 mappableObjectCount() const { return moCount;}
        atUint32 vertexCount() const  { return vtxCount; }
        atUint32 surfaceCount() const { return surfCount; }
    };


    void read(athena::io::IStreamReader& __dna_reader);
    void write(athena::io::IStreamWriter& __dna_writer) const;
    size_t binarySize(size_t __isz) const;

    std::unique_ptr<IMAPAHeader> header;

    struct IMappableObject : BigDNA
    {
        Delete _d;
        enum class Type : atUint32
        {
            BlueDoor         = 0,
            ShieldDoor       = 1,
            IceDoor          = 2,
            WaveDoor         = 3,
            PlasmaDoor       = 4,
            BigDoor1         = 5,
            BigDoor2         = 6,
            IceDoorCeiling   = 7,
            IceDoorFloor     = 8,
            WaveDoorCeiling  = 9,
            WaveDoorFloor    = 10,
            IceDoorFloor2    = 13,
            WaveDoorFloor2   = 14,
            DownArrowYellow  = 27, /* Maintenance Tunnel */
            UpArrowYellow    = 28, /* Phazon Processing Center */
            DownArrowGreen   = 29, /* Elevator A */
            UpArrowGreen     = 30, /* Elite Control Access */
            DownArrowRed     = 31, /* Elevator B */
            UpArrowRed       = 32, /* Fungal Hall Access */
            TransportLift    = 33,
            SaveStation      = 34,
            MissileStation   = 37
        };
        virtual ~IMappableObject()  {}
    };

    struct MappableObjectMP1_2 : IMappableObject
    {
        DECL_DNA
        Value<Type> type;
        Value<atUint32> visMode;
        Value<atUint32> sclyId;
        Value<atInt32> seek1 = -1;
        Value<atVec4f>  transformMtx[3];
        Value<atInt32> seek2[4] = {-1, -1, -1, -1};
        virtual ~MappableObjectMP1_2()  {}
    };

    struct MappableObjectMP3 : IMappableObject
    {
        DECL_DNA
        Value<Type> type;
        Value<atUint32> visMode;
        Value<atUint32> sclyId;
        Buffer<DNA_COUNT(0x10)> unknownHash;
        Value<atInt32> seek1 = -1;
        Value<atVec4f>  transformMtx[3];
        Value<atInt32> seek2[4] = {-1, -1, -1, -1};
        virtual ~MappableObjectMP3()  {}
    };

    std::vector<std::unique_ptr<IMappableObject>> mappableObjects;
    Vector<atVec3f, DNA_COUNT(header->vertexCount())> vertices;

    struct SurfaceHeader : BigDNA
    {
        DECL_DNA
        Value<atVec3f>  normal;
        Value<atVec3f>  centroid;
        Value<atUint32> polyOff;
        Value<atUint32> edgeOff;
    };

    Vector<SurfaceHeader, DNA_COUNT(header->surfaceCount())> surfaceHeaders;

    struct Surface : BigDNA
    {
        DECL_DNA
        Value<atUint32> primitiveCount;
        struct Primitive : BigDNA
        {
            DECL_DNA
            Value<atUint32> type;
            Value<atUint32> indexCount;
            Vector<atUint8, DNA_COUNT(indexCount)> indices;
            Align<4> align;
        };
        Vector<Primitive, DNA_COUNT(primitiveCount)> primitives;
        Value<atUint32> borderCount;
        struct Border : BigDNA
        {
            DECL_DNA
            Value<atUint32> indexCount;
            Vector<atUint8, DNA_COUNT(indexCount)> indices;
            Align<4> align;
        };
        Vector<Border, DNA_COUNT(borderCount)> borders;
    };

    Vector<Surface, DNA_COUNT(header->surfaceCount())> surfaces;


};

template <typename PAKRouter>
bool ReadMAPAToBlender(hecl::BlenderConnection& conn,
                       const MAPA& mapa,
                       const hecl::ProjectPath& outPath,
                       PAKRouter& pakRouter,
                       const typename PAKRouter::EntryType& entry,
                       bool force);

template <typename MAPAType>
bool Cook(const hecl::BlenderConnection::DataStream::MapArea& mapa, const hecl::ProjectPath& out);

}
}

#endif // __DNACOMMON_MAPA_HPP__
