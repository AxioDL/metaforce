#include <cstddef>
#include "CMDL.hpp"
#include "DNAMP1.hpp"

namespace Retro
{
namespace DNAMP1
{

bool CMDL::ReadToBlender(HECL::BlenderConnection& conn, Athena::io::IStreamReader& reader)
{
    reader.setEndian(Athena::BigEndian);

    CMDL::Header head;
    head.read(reader);

    if (head.magic != 0xDEADBABE)
    {
        Log.report(LogVisor::Error, "invalid CMDL magic");
        return false;
    }

    if (head.version != 2)
    {
        Log.report(LogVisor::Error, "invalid CMDL version for MP1");
        return false;
    }

    /* Open Py Stream */
    HECL::BlenderConnection::PyOutStream os = conn.beginPythonOut();
    os << "bm = bmesh.new();\n";

    for (size_t s=0 ; s<head.secCount ; ++s)
    {
        atUint64 secStart = reader.position();

        std::unique_ptr<atVec3f[]> vertPos;
        std::unique_ptr<atVec3f[]> vertNorm;
        typedef atInt16 ShortVec3[3];
        std::unique_ptr<ShortVec3[]> vertNormShort;
        std::unique_ptr<atVec2f[]> vertUVs;
        typedef atInt16 ShortVec2[2];
        std::unique_ptr<ShortVec2[]> vertUVsShort;
        bool visitedDLOffsets = false;
        if (s < head.matSetCount)
        {
            MaterialSet matSet;
            matSet.read(reader);
        }
        else
        {
            switch (s-head.matSetCount)
            {
            case 0:
            {
                /* Positions */
                size_t vertCount = head.secSizes[s] / 12;
                vertPos.reset(new atVec3f[vertCount]);
                for (size_t i=0 ; i<vertCount ; ++i)
                    vertPos[i] = reader.readVec3f();
                break;
            }
            case 1:
            {
                /* Normals */
                if (head.flags.shortNormals())
                {
                    size_t normCount = head.secSizes[s] / 6;
                    vertNormShort.reset(new ShortVec3[normCount]);
                    for (size_t i=0 ; i<normCount ; ++i)
                    {
                        vertNormShort[i][0] = reader.readInt16();
                        vertNormShort[i][1] = reader.readInt16();
                        vertNormShort[i][2] = reader.readInt16();
                    }
                }
                else
                {
                    size_t normCount = head.secSizes[s] / 12;
                    vertNorm.reset(new atVec3f[normCount]);
                    for (size_t i=0 ; i<normCount ; ++i)
                        vertNorm[i] = reader.readVec3f();
                }
                break;
            }
            case 2:
            {
                /* Colors */
                break;
            }
            case 3:
            {
                /* Float UVs */
                size_t uvCount = head.secSizes[s] / 8;
                vertUVs.reset(new atVec2f[uvCount]);
                for (size_t i=0 ; i<uvCount ; ++i)
                    vertUVs[i] = reader.readVec2f();
                break;
            }
            case 4:
            {
                /* Short UVs */
                if (head.flags.shortUVs())
                {
                    size_t uvCount = head.secSizes[s] / 4;
                    vertUVsShort.reset(new ShortVec2[uvCount]);
                    for (size_t i=0 ; i<uvCount ; ++i)
                    {
                        vertUVsShort[i][0] = reader.readInt16();
                        vertUVsShort[i][1] = reader.readInt16();
                    }
                    break;
                }

                /* DL Offsets (here or next section) */
                visitedDLOffsets = true;
                break;
            }
            default:
            {
                if (!visitedDLOffsets)
                {
                    visitedDLOffsets = true;
                    break;
                }

                /* GX Display List (surface) */
                SurfaceHeader sHead;
                sHead.read(reader);

            }
            }
        }

        reader.seek(secStart + head.secSizes[s], Athena::Begin);
    }

    return true;
}

}
}
