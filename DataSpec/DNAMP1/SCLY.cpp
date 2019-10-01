#include "SCLY.hpp"
#include "ScriptObjects/ScriptTypes.hpp"

namespace DataSpec::DNAMP1 {

template <>
void SCLY::Enumerate<BigDNA::Read>(athena::io::IStreamReader& rs) {
  fourCC.read(rs);
  version = rs.readUint32Big();
  layerCount = rs.readUint32Big();
  rs.enumerateBig(layerSizes, layerCount);
  atUint32 i = 0;
  rs.enumerate<ScriptLayer>(layers, layerCount, [&i, this](athena::io::IStreamReader& rs, ScriptLayer& layer) {
    atUint64 start = rs.position();
    layer.read(rs);
    rs.seek(start + layerSizes[i++], athena::Begin);
  });
}

template <>
void SCLY::Enumerate<BigDNA::Write>(athena::io::IStreamWriter& ws) {
  fourCC.write(ws);
  ws.writeUint32Big(version);
  ws.writeUint32Big(layerCount);
  ws.enumerateBig(layerSizes);
  ws.enumerate(layers);
}

template <>
void SCLY::Enumerate<BigDNA::BinarySize>(size_t& __isz) {
  __isz += 12;
  __isz += layerSizes.size() * 4;
  for (const ScriptLayer& layer : layers)
    layer.binarySize(__isz);
}

void SCLY::exportToLayerDirectories(const PAK::Entry& entry, PAKRouter<PAKBridge>& pakRouter, bool force) const {
  for (atUint32 i = 0; i < layerCount; i++) {
    bool active;
    hecl::ProjectPath layerPath = pakRouter.getAreaLayerWorking(entry.id, i, active);
    if (layerPath.isNone())
      layerPath.makeDirChain(true);

    if (active) {
      const hecl::ProjectPath activePath(layerPath, "!defaultactive");
      [[maybe_unused]] const auto fp = hecl::FopenUnique(activePath.getAbsolutePath().data(), _SYS_STR("wb"));
    }

    hecl::ProjectPath yamlFile(layerPath, _SYS_STR("!objects.yaml"));
    if (force || yamlFile.isNone()) {
      athena::io::FileWriter writer(yamlFile.getAbsolutePath());
      athena::io::ToYAMLStream(layers[i], writer);
    }
  }
}

void SCLY::addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter, CharacterAssociations<UniqueID32>& charAssoc) const {
  for (const ScriptLayer& layer : layers)
    layer.addCMDLRigPairs(pakRouter, charAssoc);
}

void SCLY::ScriptLayer::addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
                                        CharacterAssociations<UniqueID32>& charAssoc) const {
  for (const std::unique_ptr<IScriptObject>& obj : objects)
    obj->addCMDLRigPairs(pakRouter, charAssoc);
}

void SCLY::nameIDs(PAKRouter<PAKBridge>& pakRouter) const {
  for (const ScriptLayer& layer : layers)
    layer.nameIDs(pakRouter);
}

void SCLY::ScriptLayer::nameIDs(PAKRouter<PAKBridge>& pakRouter) const {
  for (const std::unique_ptr<IScriptObject>& obj : objects)
    obj->nameIDs(pakRouter);
}

template <>
void SCLY::Enumerate<BigDNA::ReadYaml>(athena::io::YAMLDocReader& docin) {
  Do<BigDNA::ReadYaml>(athena::io::PropId{"fourCC"}, fourCC, docin);
  version = docin.readUint32("version");
  layerCount = docin.enumerate("layerSizes", layerSizes);
  docin.enumerate("layers", layers);
}

template <>
void SCLY::Enumerate<BigDNA::WriteYaml>(athena::io::YAMLDocWriter& docout) {
  Do<BigDNA::WriteYaml>(athena::io::PropId{"fourCC"}, fourCC, docout);
  docout.writeUint32("version", version);
  docout.enumerate("layerSizes", layerSizes);
  docout.enumerate("layers", layers);
}

std::string_view SCLY::DNAType() { return "urde::DNAMP1::SCLY"sv; }

template <>
void SCLY::ScriptLayer::Enumerate<BigDNA::Read>(athena::io::IStreamReader& rs) {
  unknown = rs.readUByte();
  objectCount = rs.readUint32Big();
  objects.clear();
  objects.reserve(objectCount);
  for (atUint32 i = 0; i < objectCount; i++) {
    atUint8 type = rs.readUByte();
    atUint32 len = rs.readUint32Big();
    atUint64 start = rs.position();

    auto iter = std::find_if(SCRIPT_OBJECT_DB.begin(), SCRIPT_OBJECT_DB.end(),
                             [&type](const ScriptObjectSpec* obj) -> bool { return obj->type == type; });

    if (iter != SCRIPT_OBJECT_DB.end()) {
      std::unique_ptr<IScriptObject> obj((*iter)->a());
      obj->type = type;
      obj->read(rs);
      objects.push_back(std::move(obj));
      size_t actualLen = rs.position() - start;
      if (actualLen != len)
        Log.report(logvisor::Fatal,
                   fmt(_SYS_STR("Error while reading object of type 0x{:02X}, did not read the expected amount of "
                                "data, read 0x{:x}, expected 0x{:x}")),
                   (atUint32)type, actualLen, len);
      rs.seek(start + len, athena::Begin);
    } else
      Log.report(logvisor::Fatal, fmt(_SYS_STR("Unable to find type 0x{:X} in object database")), (atUint32)type);
  }
}

template <>
void SCLY::ScriptLayer::Enumerate<BigDNA::ReadYaml>(athena::io::YAMLDocReader& rs) {
  unknown = rs.readUByte("unknown");
  size_t objCount;
  objects.clear();
  if (auto v = rs.enterSubVector("objects", objCount)) {
    objectCount = objCount;
    objects.reserve(objCount);
    for (atUint32 i = 0; i < objectCount; i++) {
      if (auto rec = rs.enterSubRecord()) {
        atUint8 type = rs.readUByte("type");
        auto iter = std::find_if(SCRIPT_OBJECT_DB.begin(), SCRIPT_OBJECT_DB.end(),
                                 [&type](const ScriptObjectSpec* obj) -> bool { return obj->type == type; });

        if (iter != SCRIPT_OBJECT_DB.end()) {
          std::unique_ptr<IScriptObject> obj((*iter)->a());
          obj->read(rs);
          obj->type = type;
          objects.push_back(std::move(obj));
        } else
          Log.report(logvisor::Fatal, fmt(_SYS_STR("Unable to find type 0x{:X} in object database")), (atUint32)type);
      }
    }
  } else
    objectCount = 0;
}

template <>
void SCLY::ScriptLayer::Enumerate<BigDNA::Write>(athena::io::IStreamWriter& ws) {
  ws.writeUByte(unknown);
  ws.writeUint32Big(objectCount);
  for (const std::unique_ptr<IScriptObject>& obj : objects) {
    ws.writeByte(obj->type);
    size_t expLen = 0;
    obj->binarySize(expLen);
    ws.writeUint32Big(expLen);
    auto start = ws.position();
    obj->write(ws);
    auto wrote = ws.position() - start;
    if (wrote != expLen)
      Log.report(logvisor::Error, fmt("expected writing {} byte SCLY obj; wrote {}"), expLen, wrote);
  }
}

template <>
void SCLY::ScriptLayer::Enumerate<BigDNA::BinarySize>(size_t& __isz) {
  __isz += 5;
  for (const std::unique_ptr<IScriptObject>& obj : objects) {
    __isz += 5;
    obj->binarySize(__isz);
  }
}

template <>
void SCLY::ScriptLayer::Enumerate<BigDNA::WriteYaml>(athena::io::YAMLDocWriter& ws) {
  ws.writeUByte("unknown", unknown);
  if (auto v = ws.enterSubVector("objects")) {
    for (const std::unique_ptr<IScriptObject>& obj : objects) {
      if (auto rec = ws.enterSubRecord()) {
        ws.writeUByte("type", obj->type);
        obj->write(ws);
      }
    }
  }
}

std::string_view SCLY::ScriptLayer::DNAType() { return "urde::DNAMP1::SCLY::ScriptLayer"sv; }

} // namespace DataSpec::DNAMP1
