#ifndef __DNACOMMON_NAMEDRESOURCECATALOG_HPP__
#define __DNACOMMON_NAMEDRESOURCECATALOG_HPP__

#include "DNACommon.hpp"
namespace DataSpec
{
template <class IDType>
struct NamedResourceCatalog : BigYAML
{
    Delete _d;
    Value<atUint32> namedResCount = 0;
    struct NamedResource : BigYAML
    {
        Delete _d;
        DNAFourCC type;
        String<-1> name;
        IDType uid;

        void read(Athena::io::IStreamReader& __dna_reader)
        {
            /* type */
            type.read(__dna_reader);
            /* name */
            name = __dna_reader.readString(-1);
            /* uid */
            uid.read(__dna_reader);
        }

        void write(Athena::io::IStreamWriter& __dna_writer) const
        {
            /* type */
            type.write(__dna_writer);
            /* name */
            __dna_writer.writeString(name, -1);
            /* uid */
            uid.write(__dna_writer);
        }

        void read(Athena::io::YAMLDocReader& __dna_docin)
        {
            /* type */
            __dna_docin.enumerate("type", type);
            /* name */
            name = __dna_docin.readString("name");
            /* uid */
            __dna_docin.enumerate("uid", uid);
        }

        void write(Athena::io::YAMLDocWriter& __dna_docout) const
        {
            /* type */
            __dna_docout.enumerate("type", type);
            /* name */
            __dna_docout.writeString("name", name);
            /* uid */
            __dna_docout.enumerate("uid", uid);
        }

        static const char* DNAType() { return "DataSpec::DNACommon::NameResourceCatalog::NamedResource"; }

        size_t binarySize(size_t __isz) const
        {
            __isz = type.binarySize(__isz);
            __isz += name.size() + 1;
            __isz = uid.binarySize(__isz);
            return __isz;
        }
    };
    Vector<NamedResource, DNA_COUNT(namedResCount)> namedResources;

    void read(Athena::io::IStreamReader& __dna_reader)
    {
        /* namedResCount */
        namedResCount = __dna_reader.readUint32Big();
        /* namedResources */
        __dna_reader.enumerate(namedResources, namedResCount);
    }

    void write(Athena::io::IStreamWriter& __dna_writer) const
    {
        /* namedResCount */
        __dna_writer.writeUint32Big(namedResCount);
        /* namedResources */
        __dna_writer.enumerate(namedResources);
    }

    void read(Athena::io::YAMLDocReader& __dna_docin)
    {
        /* namedResCount */
        namedResCount = __dna_docin.readUint32("namedResCount");
        /* namedResources */
        __dna_docin.enumerate("namedResources", namedResources, namedResCount);
    }

    void write(Athena::io::YAMLDocWriter& __dna_docout) const
    {
        /* namedResCount */
        __dna_docout.writeUint32("namedResCount", namedResCount);
        /* namedResources */
        __dna_docout.enumerate("namedResources", namedResources);
    }

    static const char* DNAType()
    {
        return "DataSpec::DNACommon::NameResourceCatalog";
    }

    size_t binarySize(size_t __isz) const
    {
        __isz = __EnumerateSize(__isz, namedResources);
        return __isz + 4;
    }

    void addNamedResource(const std::string& name, const IDType& id, const DNAFourCC& type)
    {
        NamedResource res;
        res.type = type;
        res.name = name;
        res.uid = id;
        auto it = std::find_if(namedResources.begin(), namedResources.end(), [res](const NamedResource& a)->bool
        { return (a.name == res.name && a.type == res.type && a.uid == res.uid); });

        if (it != namedResources.end())
            return;

        namedResources.push_back(std::move(res));
        namedResCount++;
    }
};
}

#endif // NAMEDRESOURCECATALOG_HPP
