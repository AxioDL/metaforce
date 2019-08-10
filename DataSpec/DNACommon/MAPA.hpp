#pragma once

#include "DNACommon.hpp"
#include "GX.hpp"

namespace DataSpec::DNAMAPA {
struct MAPA : BigDNA {
  AT_DECL_EXPLICIT_DNA
  Value<atUint32> magic;
  Value<atUint32> version;
  struct IMAPAHeader : BigDNAV {
    Delete _d;
    virtual atUint32 visMode() const = 0;
    virtual atUint32 mappableObjectCount() const = 0;
    virtual atUint32 vertexCount() const = 0;
    virtual atUint32 surfaceCount() const = 0;
  };

  struct HeaderMP1 : IMAPAHeader {
    AT_DECL_DNA
    AT_DECL_DNAV
    Value<atUint32> unknown1 = 0;
    Value<atUint32> mapVisMode = 0;
    Value<atVec3f> boundingBox[2] = {};
    Value<atUint32> moCount = 0;
    Value<atUint32> vtxCount = 0;
    Value<atUint32> surfCount = 0;
    atUint32 visMode() const override { return mapVisMode; }
    atUint32 mappableObjectCount() const override { return moCount; }
    atUint32 vertexCount() const override { return vtxCount; }
    atUint32 surfaceCount() const override { return surfCount; }
  };

  struct HeaderMP2 : IMAPAHeader {
    AT_DECL_DNA
    AT_DECL_DNAV
    Value<atUint32> unknown1 = 0;
    Value<atUint32> mapVisMode = 0;
    Value<atVec3f> boundingBox[2] = {};
    Value<atUint32> unknown3 = 0;
    Value<atUint32> unknown4 = 0;
    Value<atUint32> unknown5 = 0;
    Value<atUint32> moCount = 0;
    Value<atUint32> vtxCount = 0;
    Value<atUint32> surfCount = 0;
    atUint32 visMode() const override { return mapVisMode; }
    atUint32 mappableObjectCount() const override { return moCount; }
    atUint32 vertexCount() const override { return vtxCount; }
    atUint32 surfaceCount() const override { return surfCount; }
  };

  struct HeaderMP3 : IMAPAHeader {
    AT_DECL_DNA
    AT_DECL_DNAV
    Value<atUint32> unknown1 = 0;
    Value<atUint32> mapVisMode = 0;
    Value<atVec3f> boundingBox[2] = {};
    Value<atUint32> unknown3 = 0;
    Value<atUint32> unknown4 = 0;
    Value<atUint32> unknown5 = 0;
    Value<atUint32> unknown6 = 0;
    Value<atUint32> moCount = 0;
    Value<atUint32> vtxCount = 0;
    Value<atUint32> surfCount = 0;
    Value<atUint32> internalNameLength = 0;
    Value<atUint32> unknown7 = 0;
    String<AT_DNA_COUNT(internalNameLength)> internalName;
    atUint32 visMode() const override { return mapVisMode; }
    atUint32 mappableObjectCount() const override { return moCount; }
    atUint32 vertexCount() const override { return vtxCount; }
    atUint32 surfaceCount() const override { return surfCount; }
  };

  std::unique_ptr<IMAPAHeader> header;

  struct IMappableObject : BigDNAV {
    Delete _d;
    enum class Type : atUint32 {
      BlueDoor = 0,
      ShieldDoor = 1,
      IceDoor = 2,
      WaveDoor = 3,
      PlasmaDoor = 4,
      BigDoor1 = 5,
      BigDoor2 = 6,
      IceDoorCeiling = 7,
      IceDoorFloor = 8,
      WaveDoorCeiling = 9,
      WaveDoorFloor = 10,
      IceDoorFloor2 = 13,
      WaveDoorFloor2 = 14,
      DownArrowYellow = 27, /* Maintenance Tunnel */
      UpArrowYellow = 28,   /* Phazon Processing Center */
      DownArrowGreen = 29,  /* Elevator A */
      UpArrowGreen = 30,    /* Elite Control Access */
      DownArrowRed = 31,    /* Elevator B */
      UpArrowRed = 32,      /* Fungal Hall Access */
      TransportLift = 33,
      SaveStation = 34,
      MissileStation = 37
    };
  };

  struct MappableObjectMP1_2 : IMappableObject {
    AT_DECL_DNA
    AT_DECL_DNAV
    Value<Type> type;
    Value<atUint32> visMode;
    Value<atUint32> sclyId;
    Value<atInt32> seek1 = -1;
    Value<atVec4f> transformMtx[3];
    Value<atInt32> seek2[4] = {-1, -1, -1, -1};
  };

  struct MappableObjectMP3 : IMappableObject {
    AT_DECL_DNA
    AT_DECL_DNAV
    Value<Type> type;
    Value<atUint32> visMode;
    Value<atUint32> sclyId;
    Buffer<AT_DNA_COUNT(0x10)> unknownHash;
    Value<atInt32> seek1 = -1;
    Value<atVec4f> transformMtx[3];
    Value<atInt32> seek2[4] = {-1, -1, -1, -1};
  };

  std::vector<std::unique_ptr<IMappableObject>> mappableObjects;
  Vector<atVec3f, AT_DNA_COUNT(header->vertexCount())> vertices;

  struct SurfaceHeader : BigDNA {
    AT_DECL_DNA
    Value<atVec3f> normal;
    Value<atVec3f> centroid;
    Value<atUint32> polyOff;
    Value<atUint32> edgeOff;
  };

  Vector<SurfaceHeader, AT_DNA_COUNT(header->surfaceCount())> surfaceHeaders;

  struct Surface : BigDNA {
    AT_DECL_DNA
    Value<atUint32> primitiveCount;
    struct Primitive : BigDNA {
      AT_DECL_DNA
      Value<atUint32> type;
      Value<atUint32> indexCount;
      Vector<atUint8, AT_DNA_COUNT(indexCount)> indices;
      Align<4> align;
    };
    Vector<Primitive, AT_DNA_COUNT(primitiveCount)> primitives;
    Value<atUint32> borderCount;
    struct Border : BigDNA {
      AT_DECL_DNA
      Value<atUint32> indexCount;
      Vector<atUint8, AT_DNA_COUNT(indexCount)> indices;
      Align<4> align;
    };
    Vector<Border, AT_DNA_COUNT(borderCount)> borders;
  };

  Vector<Surface, AT_DNA_COUNT(header->surfaceCount())> surfaces;
};

template <typename PAKRouter>
bool ReadMAPAToBlender(hecl::blender::Connection& conn, const MAPA& mapa, const hecl::ProjectPath& outPath,
                       PAKRouter& pakRouter, const typename PAKRouter::EntryType& entry, bool force);

template <typename MAPAType>
bool Cook(const hecl::blender::MapArea& mapa, const hecl::ProjectPath& out);

} // namespace DataSpec::DNAMAPA
