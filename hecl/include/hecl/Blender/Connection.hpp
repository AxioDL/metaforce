#ifndef BLENDERCONNECTION_HPP
#define BLENDERCONNECTION_HPP

#if _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <cstdint>
#include <cstdio>
#include <cfloat>
#include <string>
#include <functional>
#include <iostream>
#include <unordered_map>
#include <atomic>

#include "hecl/hecl.hpp"
#include "hecl/HMDLMeta.hpp"
#include "athena/Types.hpp"
#include "athena/MemoryWriter.hpp"
#include "optional.hpp"
#include "Token.hpp"

namespace hecl::blender
{

extern logvisor::Module BlenderLog;
class HMDLBuffers;
class Connection;

struct PoolSkinIndex
{
    size_t m_poolSz = 0;
    std::unique_ptr<uint32_t[]> m_poolToSkinIndex;

    void allocate(size_t poolSz)
    {
        m_poolSz = poolSz;
        if (poolSz)
            m_poolToSkinIndex.reset(new uint32_t[poolSz]);
    }
};

enum class ANIMCurveType
{
    Rotate,
    Translate,
    Scale
};

class ANIMOutStream
{
    Connection* m_parent;
    unsigned m_curCount = 0;
    unsigned m_totalCount = 0;
    bool m_inCurve = false;
public:
    using CurveType = ANIMCurveType;
    ANIMOutStream(Connection* parent);
    ~ANIMOutStream();
    void changeCurve(CurveType type, unsigned crvIdx, unsigned keyCount);
    void write(unsigned frame, float val);
};

class PyOutStream : public std::ostream
{
    friend class Connection;
    Connection* m_parent;
    bool m_deleteOnError;
    struct StreamBuf : std::streambuf
    {
        PyOutStream& m_parent;
        std::string m_lineBuf;
        bool m_deleteOnError;
        StreamBuf(PyOutStream& parent, bool deleteOnError)
        : m_parent(parent), m_deleteOnError(deleteOnError) {}
        StreamBuf(const StreamBuf& other) = delete;
        StreamBuf(StreamBuf&& other) = default;
        int_type overflow(int_type ch);
    } m_sbuf;
    PyOutStream(Connection* parent, bool deleteOnError);
public:
    PyOutStream(const PyOutStream& other) = delete;
    PyOutStream(PyOutStream&& other)
    : std::ostream(&m_sbuf), m_parent(other.m_parent), m_sbuf(std::move(other.m_sbuf))
    {other.m_parent = nullptr;}
    ~PyOutStream() {close();}
    void close();
#if __GNUC__
    __attribute__((__format__ (__printf__, 2, 3)))
#endif
    void format(const char* fmt, ...);
    void linkBlend(const char* target, const char* objName, bool link=true);
    void linkBackground(const char* target, const char* sceneName);
    void AABBToBMesh(const atVec3f& min, const atVec3f& max);
    void centerView();

    ANIMOutStream beginANIMCurve()
    {
        return ANIMOutStream(m_parent);
    }
    Connection& getConnection() {return *m_parent;}
};

/* Vector types with integrated stream reading constructor */
struct Vector2f
{
    atVec2f val;
    Vector2f() = default;
    void read(Connection& conn);
    Vector2f(Connection& conn) {read(conn);}
    operator const atVec2f&() const {return val;}
};
struct Vector3f
{
    atVec3f val;
    Vector3f() = default;
    void read(Connection& conn);
    Vector3f(Connection& conn) {read(conn);}
    operator const atVec3f&() const {return val;}
};
struct Vector4f
{
    atVec4f val;
    Vector4f() = default;
    void read(Connection& conn);
    Vector4f(Connection& conn) {read(conn);}
    operator const atVec4f&() const {return val;}
};
struct Matrix3f
{
    atVec3f m[3];
    inline atVec3f& operator[](size_t idx) {return m[idx];}
    inline const atVec3f& operator[](size_t idx) const {return m[idx];}
};
struct Matrix4f
{
    atVec4f val[4];
    Matrix4f() = default;
    void read(Connection& conn);
    Matrix4f(Connection& conn) {read(conn);}
    const atVec4f& operator[] (size_t idx) const {return val[idx];}
};
struct Index
{
    uint32_t val;
    Index() = default;
    void read(Connection& conn);
    Index(Connection& conn) {read(conn);}
    operator const uint32_t&() const {return val;}
};

atVec3f MtxVecMul4RM(const Matrix4f& mtx, const Vector3f& vec);
atVec3f MtxVecMul3RM(const Matrix4f& mtx, const Vector3f& vec);

/** HECL source and metadata of each material */
struct Material
{
    std::string name;
    std::string source;
    std::vector<ProjectPath> texs;
    std::unordered_map<std::string, int32_t> iprops;
    bool transparent;

    Material(Connection& conn);
    bool operator==(const Material& other) const
    {
        return source == other.source && texs == other.texs && iprops == other.iprops;
    }
};

/** Intermediate mesh representation prepared by blender from a single mesh object */
struct Mesh
{
    HMDLTopology topology;

    /* Object transform in scene */
    Matrix4f sceneXf;

    /* Cumulative AABB */
    Vector3f aabbMin;
    Vector3f aabbMax;

    std::vector<std::vector<Material>> materialSets;

    /* Vertex buffer data */
    std::vector<Vector3f> pos;
    std::vector<Vector3f> norm;
    uint32_t colorLayerCount = 0;
    std::vector<Vector3f> color;
    uint32_t uvLayerCount = 0;
    std::vector<Vector2f> uv;

    /* Skinning data */
    std::vector<std::string> boneNames;
    struct SkinBind
    {
        uint32_t boneIdx;
        float weight;
        SkinBind(Connection& conn);
    };
    std::vector<std::vector<SkinBind>> skins;
    std::vector<size_t> contiguousSkinVertCounts;

    void normalizeSkinBinds();

    /** Islands of the same material/skinBank are represented here */
    struct Surface
    {
        Vector3f centroid;
        Index materialIdx;
        Vector3f aabbMin;
        Vector3f aabbMax;
        Vector3f reflectionNormal;
        uint32_t skinBankIdx;

        /** Vertex indexing data (all primitives joined as degenerate tri-strip) */
        struct Vert
        {
            uint32_t iPos;
            uint32_t iNorm;
            uint32_t iColor[4] = {uint32_t(-1)};
            uint32_t iUv[8] = {uint32_t(-1)};
            uint32_t iSkin;
            uint32_t iBankSkin = -1;

            Vert(Connection& conn, const Mesh& parent);
            bool operator==(const Vert& other) const;
        };
        std::vector<Vert> verts;

        Surface(Connection& conn, Mesh& parent, int skinSlotCount);
    };
    std::vector<Surface> surfaces;

    std::unordered_map<std::string, std::string> customProps;

    struct SkinBanks
    {
        struct Bank
        {
            std::vector<uint32_t> m_skinIdxs;
            std::vector<uint32_t> m_boneIdxs;

            void addSkins(const Mesh& parent, const std::vector<uint32_t>& skinIdxs);
            size_t lookupLocalBoneIdx(uint32_t boneIdx) const;
        };
        std::vector<Bank> banks;
        std::vector<Bank>::iterator addSkinBank(int skinSlotCount);
        uint32_t addSurface(const Mesh& mesh, const Surface& surf, int skinSlotCount);
    } skinBanks;

    using SurfProgFunc = std::function<void(int)>;
    Mesh(Connection& conn, HMDLTopology topology, int skinSlotCount, SurfProgFunc& surfProg);

    Mesh getContiguousSkinningVersion() const;

    /** Prepares mesh representation for indexed access on modern APIs.
     *  Mesh must remain resident for accessing reference members
     */
    HMDLBuffers getHMDLBuffers(bool absoluteCoords, PoolSkinIndex& poolSkinIndex) const;
};

/** Intermediate collision mesh representation prepared by blender from a single mesh object */
struct ColMesh
{
    /** HECL source and metadata of each material */
    struct Material
    {
        std::string name;
        bool unknown;
        bool surfaceStone;
        bool surfaceMetal;
        bool surfaceGrass;
        bool surfaceIce;
        bool pillar;
        bool surfaceMetalGrating;
        bool surfacePhazon;
        bool surfaceDirt;
        bool surfaceLava;
        bool surfaceSPMetal;
        bool surfaceStoneRock;
        bool surfaceSnow;
        bool surfaceMudSlow;
        bool surfaceFabric;
        bool halfPipe;
        bool surfaceMud;
        bool surfaceGlass;
        bool unused3;
        bool unused4;
        bool surfaceShield;
        bool surfaceSand;
        bool surfaceMothOrSeedOrganics;
        bool surfaceWeb;
        bool projPassthrough;
        bool solid;
        bool u20;
        bool camPassthrough;
        bool surfaceWood;
        bool surfaceOrganic;
        bool u24;
        bool surfaceRubber;
        bool seeThrough;
        bool scanPassthrough;
        bool aiPassthrough;
        bool ceiling;
        bool wall;
        bool floor;
        bool aiBlock;
        bool jumpNotAllowed;
        bool spiderBall;
        bool screwAttackWallJump;

        Material(Connection& conn);
    };
    std::vector<Material> materials;

    std::vector<Vector3f> verts;

    struct Edge
    {
        uint32_t verts[2];
        bool seam;
        Edge(Connection& conn);
    };
    std::vector<Edge> edges;

    struct Triangle
    {
        uint32_t edges[3];
        uint32_t matIdx;
        bool flip;
        Triangle(Connection& conn);
    };
    std::vector<Triangle> trianges;

    ColMesh(Connection& conn);
};

/** Intermediate world representation */
struct World
{
    struct Area
    {
        ProjectPath path;
        Vector3f aabb[2];
        Matrix4f transform;
        struct Dock
        {
            Vector3f verts[4];
            Index targetArea;
            Index targetDock;
            Dock(Connection& conn);
        };
        std::vector<Dock> docks;
        Area(Connection& conn);
    };
    std::vector<Area> areas;
    World(Connection& conn);
};

/** Intermediate lamp representation */
struct Light
{
    /* Object transform in scene */
    Matrix4f sceneXf;
    Vector3f color;

    uint32_t layer;

    enum class Type : uint32_t
    {
        Ambient,
        Directional,
        Custom,
        Spot
    } type;

    float energy;
    float spotCutoff;
    float constant;
    float linear;
    float quadratic;
    bool shadow;

    std::string name;

    Light(Connection& conn);
};

/** Intermediate MapArea representation */
struct MapArea
{
    Index visType;
    std::vector<Vector3f> verts;
    std::vector<Index> indices;
    struct Surface
    {
        Vector3f normal;
        Vector3f centerOfMass;
        Index start;
        Index count;
        std::vector<std::pair<Index, Index>> borders;
        Surface(Connection& conn);
    };
    std::vector<Surface> surfaces;
    struct POI
    {
        uint32_t type;
        uint32_t visMode;
        uint32_t objid;
        Matrix4f xf;
        POI(Connection& conn);
    };
    std::vector<POI> pois;
    MapArea(Connection& conn);
};

/** Intermediate MapUniverse representation */
struct MapUniverse
{
    hecl::ProjectPath hexagonPath;
    struct World
    {
        std::string name;
        Matrix4f xf;
        std::vector<Matrix4f> hexagons;
        Vector4f color;
        hecl::ProjectPath worldPath;
        World(Connection& conn);
    };
    std::vector<World> worlds;
    MapUniverse(Connection& conn);
};

/** Intermediate bone representation used in Armature */
struct Bone
{
    std::string name;
    Vector3f origin;
    int32_t parent = -1;
    std::vector<int32_t> children;
    Bone(Connection& conn);
};

/** Intermediate armature representation used in Actor */
struct Armature
{
    std::string name;
    std::vector<Bone> bones;
    const Bone* lookupBone(const char* name) const;
    const Bone* getParent(const Bone* bone) const;
    const Bone* getChild(const Bone* bone, size_t child) const;
    const Bone* getRoot() const;
    Armature(Connection& conn);
};

/** Intermediate action representation used in Actor */
struct Action
{
    std::string name;
    float interval;
    bool additive;
    bool looping;
    std::vector<int32_t> frames;
    struct Channel
    {
        std::string boneName;
        uint32_t attrMask;
        struct Key
        {
            Vector4f rotation;
            Vector3f position;
            Vector3f scale;
            Key(Connection& conn, uint32_t attrMask);
        };
        std::vector<Key> keys;
        Channel(Connection& conn);
    };
    std::vector<Channel> channels;
    std::vector<std::pair<Vector3f, Vector3f>> subtypeAABBs;
    Action(Connection& conn);
};

/** Intermediate actor representation prepared by blender from a single HECL actor blend */
struct Actor
{
    std::vector<Armature> armatures;

    struct Subtype
    {
        std::string name;
        ProjectPath mesh;
        int32_t armature = -1;
        std::vector<std::pair<std::string, ProjectPath>> overlayMeshes;
        Subtype(Connection& conn);
    };
    std::vector<Subtype> subtypes;
    std::vector<Action> actions;

    Actor(Connection& conn);
};

class DataStream
{
    friend class Connection;
    Connection* m_parent;
    DataStream(Connection* parent);
public:
    DataStream(const DataStream& other) = delete;
    DataStream(DataStream&& other)
    : m_parent(other.m_parent) {other.m_parent = nullptr;}
    ~DataStream() {close();}
    void close();
    std::vector<std::string> getMeshList();
    std::vector<std::string> getLightList();
    std::pair<atVec3f, atVec3f> getMeshAABB();
    static const char* MeshOutputModeString(HMDLTopology topology);

    /** Compile mesh by context (MESH blends only) */
    Mesh compileMesh(HMDLTopology topology, int skinSlotCount=10,
                     Mesh::SurfProgFunc surfProg=[](int){});

    /** Compile mesh by name (AREA blends only) */
    Mesh compileMesh(std::string_view name, HMDLTopology topology, int skinSlotCount=10,
                     Mesh::SurfProgFunc surfProg=[](int){});

    /** Compile collision mesh by name (AREA blends only) */
    ColMesh compileColMesh(std::string_view name);

    /** Compile all meshes as collision meshes (CMESH blends only) */
    std::vector<ColMesh> compileColMeshes();

    /** Compile all meshes into one (AREA blends only) */
    Mesh compileAllMeshes(HMDLTopology topology, int skinSlotCount=10, float maxOctantLength=5.0,
                          Mesh::SurfProgFunc surfProg=[](int){});

    /** Compile world intermediate (WORLD blends only) */
    World compileWorld();

    /** Gather all lights in scene (AREA blends only) */
    std::vector<Light> compileLights();

    /** Compile GUI into FRME data (FRAME blends only) */
    void compileGuiFrame(std::string_view pathOut, int version);

    /** Gather all texture paths in scene */
    std::vector<ProjectPath> getTextures();

    Actor compileActor();
    Actor compileActorCharacterOnly();
    Action compileActionChannelsOnly(std::string_view name);
    std::vector<std::string> getArmatureNames();
    std::vector<std::string> getSubtypeNames();
    std::vector<std::string> getActionNames();
    std::vector<std::string> getSubtypeOverlayNames(std::string_view name);

    std::unordered_map<std::string, Matrix3f> getBoneMatrices(std::string_view name);

    bool renderPvs(std::string_view path, const atVec3f& location);
    bool renderPvsLight(std::string_view path, std::string_view lightName);

    MapArea compileMapArea();
    MapUniverse compileMapUniverse();
};

class Connection
{
    friend class DataStream;
    friend class PyOutStream;
    friend class ANIMOutStream;
    friend struct PyOutStream::StreamBuf;
    friend struct Mesh;
    friend struct Material;
    friend struct ColMesh;
    friend struct World;
    friend struct Light;
    friend struct MapArea;
    friend struct MapUniverse;
    friend struct Actor;
    friend struct Armature;
    friend struct Action;
    friend struct Bone;
    friend struct Vector2f;
    friend struct Vector3f;
    friend struct Vector4f;
    friend struct Matrix3f;
    friend struct Matrix4f;
    friend struct Index;

    std::atomic_bool m_lock = {false};
    bool m_pyStreamActive = false;
    bool m_dataStreamActive = false;
#if _WIN32
    PROCESS_INFORMATION m_pinfo = {};
    std::thread m_consoleThread;
    bool m_consoleThreadRunning = true;
#else
    pid_t m_blenderProc = 0;
#endif
    int m_readpipe[2];
    int m_writepipe[2];
    bool m_hasSlerp;
    BlendType m_loadedType = BlendType::None;
    bool m_loadedRigged = false;
    ProjectPath m_loadedBlend;
    std::string m_startupBlend;
    hecl::SystemString m_errPath;
    uint32_t _readStr(char* buf, uint32_t bufSz);
    uint32_t _writeStr(const char* str, uint32_t len, int wpipe);
    uint32_t _writeStr(const char* str, uint32_t len) { return _writeStr(str, len, m_writepipe[1]); }
    uint32_t _writeStr(const char* str) { return _writeStr(str, strlen(str)); }
    size_t _readBuf(void* buf, size_t len);
    size_t _writeBuf(const void* buf, size_t len);
    void _closePipe();
    void _blenderDied();

public:
    Connection(int verbosityLevel=1);
    ~Connection();

    Connection(const Connection&)=delete;
    Connection& operator=(const Connection&)=delete;
    Connection(Connection&&)=delete;
    Connection& operator=(Connection&&)=delete;

    bool hasSLERP() const {return m_hasSlerp;}

    bool createBlend(const ProjectPath& path, BlendType type);
    BlendType getBlendType() const {return m_loadedType;}
    const ProjectPath& getBlendPath() const {return m_loadedBlend;}
    bool getRigged() const {return m_loadedRigged;}
    bool openBlend(const ProjectPath& path, bool force=false);
    bool saveBlend();
    void deleteBlend();

    PyOutStream beginPythonOut(bool deleteOnError=false)
    {
        bool expect = false;
        if (!m_lock.compare_exchange_strong(expect, true))
            BlenderLog.report(logvisor::Fatal, "lock already held for blender::Connection::beginPythonOut()");
        return PyOutStream(this, deleteOnError);
    }

    DataStream beginData()
    {
        bool expect = false;
        if (!m_lock.compare_exchange_strong(expect, true))
            BlenderLog.report(logvisor::Fatal, "lock already held for blender::Connection::beginDataIn()");
        return DataStream(this);
    }

    void quitBlender();

    void closeStream()
    {
        if (m_lock)
            deleteBlend();
    }

    static Connection& SharedConnection();
    static void Shutdown();
};

class HMDLBuffers
{
public:
    struct Surface;
private:
    friend struct Mesh;
    HMDLBuffers(HMDLMeta&& meta,
                size_t vboSz, const std::vector<atUint32>& iboData,
                std::vector<Surface>&& surfaces,
                const Mesh::SkinBanks& skinBanks);
public:
    HMDLMeta m_meta;
    size_t m_vboSz;
    std::unique_ptr<uint8_t[]> m_vboData;
    size_t m_iboSz;
    std::unique_ptr<uint8_t[]> m_iboData;

    struct Surface
    {
        Surface(const Mesh::Surface& origSurf,
                atUint32 start, atUint32 count)
        : m_origSurf(origSurf), m_start(start), m_count(count) {}
        const Mesh::Surface& m_origSurf;
        atUint32 m_start;
        atUint32 m_count;
    };
    std::vector<Surface> m_surfaces;

    const Mesh::SkinBanks& m_skinBanks;
};

}

#endif // BLENDERCONNECTION_HPP
