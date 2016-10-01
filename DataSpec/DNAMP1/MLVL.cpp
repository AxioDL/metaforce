#include "MLVL.hpp"
#include "SCLY.hpp"

namespace DataSpec
{
namespace DNAMP1
{

bool MLVL::Extract(const SpecBase& dataSpec,
                   PAKEntryReadStream& rs,
                   const hecl::ProjectPath& outPath,
                   PAKRouter<PAKBridge>& pakRouter,
                   const PAK::Entry& entry,
                   bool force,
                   hecl::BlenderToken& btok,
                   std::function<void(const hecl::SystemChar*)> fileChanged)
{
    MLVL mlvl;
    mlvl.read(rs);
    athena::io::FileWriter writer(outPath.getWithExtension(_S(".yaml"), true).getAbsolutePath());
    mlvl.toYAMLStream(writer, static_cast<YAMLWriteMemberFn>(&MLVL::writeMeta));
    hecl::BlenderConnection& conn = btok.getBlenderConnection();
    return DNAMLVL::ReadMLVLToBlender(conn, mlvl, outPath, pakRouter,
                                      entry, force, fileChanged);
}

bool MLVL::Cook(const hecl::ProjectPath& outPath, const hecl::ProjectPath& inPath, const World& wld)
{
    MLVL mlvl = {};
    athena::io::FileReader reader(inPath.getWithExtension(_S(".yaml"), true).getAbsolutePath());
    mlvl.fromYAMLStream(reader, static_cast<YAMLReadMemberFn>(&MLVL::readMeta));

    mlvl.magic = 0xDEAFBABE;
    mlvl.version = 0x11;
    mlvl.saveWorldId = inPath.ensureAuxInfo(_S(".SAVW"));

    size_t areaIdx = 0;
    for (const World::Area& area : wld.areas)
    {
        if (area.path.getPathType() != hecl::ProjectPath::Type::Directory)
            continue;
        hecl::DirectoryEnumerator dEnum(area.path.getAbsolutePath(),
                                        hecl::DirectoryEnumerator::Mode::DirsSorted);
        bool areaInit = false;

        for (const hecl::DirectoryEnumerator::Entry& e : dEnum)
        {
            hecl::SystemString layerName;
            hecl::SystemChar* endCh = nullptr;
            hecl::StrToUl(e.m_name.c_str(), &endCh, 0);
            if (!endCh)
                layerName = e.m_name;
            else
                layerName = hecl::StringUtils::TrimWhitespace(hecl::SystemString(endCh));

            hecl::ProjectPath objectsPath(area.path, e.m_name + _S("/!objects.yaml"));
            if (objectsPath.isNone())
                continue;

            SCLY::ScriptLayer layer;
            {
                athena::io::FileReader freader(objectsPath.getAbsolutePath());
                if (!freader.isOpen())
                    continue;
                if (!BigYAML::ValidateFromYAMLStream<DNAMP1::SCLY::ScriptLayer>(freader))
                    continue;

                athena::io::YAMLDocReader reader;
                if (!reader.parse(&freader))
                    continue;

                layer.read(reader);
            }

            hecl::ProjectPath defActivePath(area.path, e.m_name + _S("/!defaultactive"));
            bool active = defActivePath.isNone() ? false : true;

            if (!areaInit)
            {
                mlvl.areas.emplace_back();
                MLVL::Area& areaOut = mlvl.areas.back();
                areaInit = true;
            }
        }
        ++areaIdx;
    }
    mlvl.memRelayLinkCount = mlvl.memRelayLinks.size();
    mlvl.areaCount = mlvl.areas.size();

}

}
}
