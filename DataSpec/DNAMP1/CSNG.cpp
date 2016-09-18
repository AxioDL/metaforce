#include "CSNG.hpp"
#include "amuse/SongConverter.hpp"

namespace DataSpec
{
namespace DNAMP1
{

bool CSNG::Extract(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath)
{
    hecl::ProjectPath midPath = outPath.getWithExtension(_S(".mid"), true);
    hecl::ProjectPath yamlPath = outPath.getWithExtension(_S(".yaml"), true);

    Header head;
    head.read(rs);

    {
        athena::io::YAMLDocWriter dw("CSNG");
        dw.writeUint32("midiSetupId", head.midiSetupId);
        dw.writeUint32("songGroupId", head.songGroupId);
        dw.enterSubRecord("agscId");
        head.agscId.write(dw);
        dw.leaveSubRecord();

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

    return true;
}

bool CSNG::Cook(const hecl::ProjectPath& inPath, const hecl::ProjectPath& outPath)
{
    hecl::ProjectPath midPath = inPath.getWithExtension(_S(".mid"), true);
    hecl::ProjectPath yamlPath = inPath.getWithExtension(_S(".yaml"), true);

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
        dr.enterSubRecord("agscId");
        head.agscId.read(dr);
        dr.leaveSubRecord();
        head.sngLength = sngData.size();
        head.write(w);
    }

    w.writeUBytes(sngData.data(), sngData.size());

    return true;
}

}
}
