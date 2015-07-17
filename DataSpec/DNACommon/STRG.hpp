#ifndef __COMMON_STRG_HPP__
#define __COMMON_STRG_HPP__

#include <string>
#include <fstream>
#include <angelscript.h>
#include <HECL/HECL.hpp>
#include <HECL/Database.hpp>
#include <Athena/FileWriter.hpp>
#include "DNACommon.hpp"

namespace Retro
{
struct ISTRG
{
    virtual ~ISTRG() {}

    virtual size_t count() const=0;
    virtual std::string getUTF8(const FourCC& lang, size_t idx) const=0;
    virtual std::wstring getUTF16(const FourCC& lang, size_t idx) const=0;
    virtual HECL::SystemString getSystemString(const FourCC& lang, size_t idx) const=0;
    virtual int32_t lookupIdx(const std::string& name) const=0;

    virtual bool readAngelScript(const AngelScript::asIScriptModule& in)=0;
    virtual void writeAngelScript(std::ofstream& out) const=0;

    template <class SUBCLS>
    static bool Extract(PAKEntryReadStream& rs, const HECL::ProjectPath& outPath)
    {
        SUBCLS strg;
        strg.read(rs);
        std::ofstream strgOut(outPath.getAbsolutePath());
        strg.writeAngelScript(strgOut);
        return true;
    }

    template <class SUBCLS>
    static bool Cook(const HECL::ProjectPath& inPath, const HECL::ProjectPath& outPath)
    {
        SUBCLS strg;
        HECL::Database::ASUniqueModule mod = HECL::Database::ASUniqueModule::CreateFromPath(inPath);
        if (!mod)
            return false;
        strg.readAngelScript(mod);
        Athena::io::FileWriter ws(outPath.getAbsolutePath());
        strg.write(ws);
        return true;
    }
};
std::unique_ptr<ISTRG> LoadSTRG(Athena::io::IStreamReader& reader);

extern HECL::Database::ASListType<std::string> ASTYPE_STRGLanguage;

}

#endif // __COMMON_STRG_HPP__
