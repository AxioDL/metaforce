#include "SCLY.hpp"
#include "ScriptObjects/ScriptTypes.hpp"

namespace Retro
{
namespace DNAMP1
{

void SCLY::read(Athena::io::IStreamReader& rs)
{
    fourCC = rs.readUint32Little();
    version = rs.readUint32Big();
    layerCount = rs.readUint32Big();
    rs.enumerateBig(layerSizes, layerCount);
    atUint32 i = 0;
    rs.enumerate<ScriptLayer>(layers, layerCount, [&i,this](Athena::io::IStreamReader& rs, ScriptLayer& layer) {
        atUint64 start = rs.position();
        layer.read(rs);
        rs.seek(start + layerSizes[i++], Athena::Begin);
    });
}

void SCLY::write(Athena::io::IStreamWriter& ws) const
{
    ws.writeUint32Big(fourCC);
    ws.writeUint32Big(version);
    ws.writeUint32Big(layerCount);
    ws.enumerateBig(layerSizes);
    ws.enumerate(layers);
}

size_t SCLY::binarySize(size_t __isz) const
{
    __isz += 12;
    __isz += layerSizes.size() * 4;
    return __EnumerateSize(__isz, layers);
}

void SCLY::exportToLayerDirectories(const PAK::Entry& entry, PAKRouter<PAKBridge> &pakRouter, bool force) const
{
    for (atUint32 i = 0; i < layerCount; i++)
    {
        HECL::ProjectPath layerPath = pakRouter.getAreaLayerWorking(entry.id, i);
        if (layerPath.getPathType() == HECL::ProjectPath::Type::None)
            layerPath.makeDir();

        HECL::ProjectPath yamlFile = HECL::ProjectPath(layerPath, _S("objects.yaml"));
        if (force || yamlFile.getPathType() == HECL::ProjectPath::Type::None)
        {
            FILE* yaml = HECL::Fopen(yamlFile.getAbsolutePath().c_str(), _S("wb"));
            layers[i].toYAMLFile(yaml);
            fclose(yaml);
        }
    }
}

void SCLY::addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
        std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo) const
{
    for (const ScriptLayer& layer : layers)
        layer.addCMDLRigPairs(pakRouter, addTo);
}

void SCLY::ScriptLayer::addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
        std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo) const
{
    for (const std::unique_ptr<IScriptObject>& obj : objects)
        obj->addCMDLRigPairs(pakRouter, addTo);
}

void SCLY::nameIDs(PAKRouter<PAKBridge>& pakRouter) const
{
    for (const ScriptLayer& layer : layers)
        layer.nameIDs(pakRouter);
}

void SCLY::ScriptLayer::nameIDs(PAKRouter<PAKBridge>& pakRouter) const
{
    for (const std::unique_ptr<IScriptObject>& obj : objects)
        obj->nameIDs(pakRouter);
}

void SCLY::read(Athena::io::YAMLDocReader& docin)
{
    fourCC = docin.readUint32("fourCC");
    version = docin.readUint32("version");
    layerCount = docin.readUint32("layerCount");
    docin.enumerate("layerSizes", layerSizes, layerCount);
    docin.enumerate("layers", layers, layerCount);
}

void SCLY::write(Athena::io::YAMLDocWriter& docout) const
{
    docout.writeUint32("fourCC", fourCC);
    docout.writeUint32("version", version);
    docout.writeUint32("layerCount", layerCount);
    docout.enumerate("layerSizes", layerSizes);
    docout.enumerate("layers", layers);
}

const char* SCLY::DNAType()
{
    return "Retro::DNAMP1::SCLY";
}

void SCLY::ScriptLayer::read(Athena::io::IStreamReader& rs)
{
    unknown = rs.readUByte();
    objectCount = rs.readUint32Big();
    objects.reserve(objectCount);
    for (atUint32 i = 0; i < objectCount; i++)
    {
        atUint8 type = rs.readUByte();
        atUint32 len = rs.readUint32Big();
        atUint64 start = rs.position();

        auto iter = std::find_if(SCRIPT_OBJECT_DB.begin(), SCRIPT_OBJECT_DB.end(), [&type](const ScriptObjectSpec* obj) -> bool
        { return obj->type == type; });

        if (iter != SCRIPT_OBJECT_DB.end())
        {
            std::unique_ptr<IScriptObject> obj((*iter)->a());
            obj->type = type;
            obj->read(rs);
            objects.push_back(std::move(obj));
            size_t actualLen = rs.position() - start;
            if (actualLen != len)
                Log.report(LogVisor::FatalError, _S("Error while reading object of type 0x%.2X, did not read the expected amount of data, read 0x%x, expected 0x%x"), (atUint32)type, actualLen, len);
            rs.seek(start + len, Athena::Begin);
        }
        else
            Log.report(LogVisor::FatalError, _S("Unable to find type 0x%X in object database"), (atUint32)type);
    }
}

void SCLY::ScriptLayer::read(Athena::io::YAMLDocReader& rs)
{
    unknown = rs.readUByte("unknown");
    objectCount = rs.readUint32("objectCount");
    objects.reserve(objectCount);
    rs.enterSubVector("objects");
    for (atUint32 i = 0; i < objectCount; i++)
    {
        rs.enterSubRecord(nullptr);
        atUint8 type = rs.readUByte("type");
        auto iter = std::find_if(SCRIPT_OBJECT_DB.begin(), SCRIPT_OBJECT_DB.end(), [&type](const ScriptObjectSpec* obj) -> bool
        { return obj->type == type; });

        if (iter != SCRIPT_OBJECT_DB.end())
        {
            std::unique_ptr<IScriptObject> obj((*iter)->a());
            obj->read(rs);
            obj->type = type;
            objects.push_back(std::move(obj));
        }
        else
            Log.report(LogVisor::FatalError, _S("Unable to find type 0x%X in object database"), (atUint32)type);

        rs.leaveSubRecord();
    }
    rs.leaveSubVector();
}

void SCLY::ScriptLayer::write(Athena::io::IStreamWriter& ws) const
{
    ws.writeUByte(unknown);
    ws.writeUint32Big(objectCount);
    for (const std::unique_ptr<IScriptObject>& obj : objects)
    {
        ws.writeByte(obj->type);
        obj->write(ws);
    }
}

size_t SCLY::ScriptLayer::binarySize(size_t __isz) const
{
    __isz += 5;
    for (const std::unique_ptr<IScriptObject>& obj : objects)
    {
        __isz += 1;
        __isz = obj->binarySize(__isz);
    }
    return __isz;
}

void SCLY::ScriptLayer::write(Athena::io::YAMLDocWriter& ws) const
{
    ws.writeUByte("unknown", unknown);
    ws.writeUint32("objectCount", objectCount);
    ws.enterSubVector("objects");
    for (const std::unique_ptr<IScriptObject>& obj : objects)
    {
        ws.enterSubRecord(nullptr);
        ws.writeUByte("type", obj->type);
        obj->write(ws);
        ws.leaveSubRecord();
    };
    ws.leaveSubVector();
}

const char* SCLY::ScriptLayer::DNAType()
{
    return "Retro::DNAMP1::SCLY::ScriptLayer";
}

}
}
