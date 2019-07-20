#include "CSNG.hpp"
#include "amuse/SongConverter.hpp"

namespace DataSpec::DNAMP1 {

bool CSNG::Extract(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath) {
  hecl::ProjectPath midPath = outPath.getWithExtension(_SYS_STR(".mid"), true);
  hecl::ProjectPath yamlPath = outPath.getWithExtension(_SYS_STR(".yaml"), true);

  Header head;
  head.read(rs);

  {
    athena::io::YAMLDocWriter dw("CSNG");
    dw.writeUint32("midiSetupId", head.midiSetupId);
    dw.writeUint32("songGroupId", head.songGroupId);
    if (auto rec = dw.enterSubRecord("agscId"))
      head.agscId.write(dw);

    athena::io::FileWriter w(yamlPath.getAbsolutePath());
    if (w.hasError())
      return false;
    dw.finish(&w);
  }

  {
    auto sng = rs.readUBytes(head.sngLength);
    int version;
    bool isBig;
    auto midi = amuse::SongConverter::SongToMIDI(sng.get(), version, isBig);

    athena::io::FileWriter w(midPath.getAbsolutePath());
    if (w.hasError())
      return false;
    w.writeUBytes(midi.data(), midi.size());
  }

  /* Update !songs.yaml for Amuse editor */
  hecl::ProjectPath audGrp(outPath.getParentPath().getParentPath(), _SYS_STR("AudioGrp"));
  audGrp.makeDirChain(true);
  hecl::ProjectPath songsPath(audGrp, _SYS_STR("!songs.yaml"));
  std::optional<athena::io::FileReader> r;
  if (songsPath.isFile())
    r.emplace(songsPath.getAbsolutePath());
  athena::io::YAMLDocWriter ydw("amuse::Songs", r ? &*r : nullptr);
  r = std::nullopt;
  ydw.writeString(fmt::format(fmt("{:04X}"), head.midiSetupId).c_str(),
    fmt::format(fmt("../MidiData/{}"), midPath.getLastComponentUTF8()));
  athena::io::FileWriter w(songsPath.getAbsolutePath());
  ydw.finish(&w);

  return true;
}

bool CSNG::Cook(const hecl::ProjectPath& inPath, const hecl::ProjectPath& outPath) {
  hecl::ProjectPath midPath = inPath.getWithExtension(_SYS_STR(".mid"), true);
  hecl::ProjectPath yamlPath = inPath.getWithExtension(_SYS_STR(".yaml"), true);

  std::vector<uint8_t> sngData;
  {
    athena::io::FileReader midR(midPath.getAbsolutePath());
    if (midR.hasError())
      return false;

    uint32_t midLen = midR.length();
    std::vector<uint8_t> midData;
    midData.resize(midLen);
    midR.readUBytesToBuf(midData.data(), midLen);
    sngData = amuse::SongConverter::MIDIToSong(midData, 1, true);
  }

  athena::io::FileWriter w(outPath.getAbsolutePath());
  if (w.hasError())
    return false;

  {
    athena::io::FileReader yamlR(yamlPath.getAbsolutePath());
    if (yamlR.hasError())
      return false;
    athena::io::YAMLDocReader dr;
    if (!dr.parse(&yamlR))
      return false;

    Header head;
    head.midiSetupId = dr.readUint32("midiSetupId");
    head.songGroupId = dr.readUint32("songGroupId");
    if (auto rec = dr.enterSubRecord("agscId"))
      head.agscId.read(dr);
    head.sngLength = sngData.size();
    head.write(w);
  }

  w.writeUBytes(sngData.data(), sngData.size());

  return true;
}

} // namespace DataSpec::DNAMP1
