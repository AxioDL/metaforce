#include "STRG.hpp"
#include "DNAMP1.hpp"

namespace DataSpec::DNAMP1
{

const std::vector<FourCC> skLanguages = {FOURCC('ENGL'), FOURCC('FREN'), FOURCC('GERM'), FOURCC('SPAN'),
                                         FOURCC('ITAL'), FOURCC('DUTC'), FOURCC('JAPN')};

static uint32_t ParseTag(const char16_t* str)
{
    char parseStr[9];
    int i;
    for (i = 0; i < 8 && str[i]; ++i)
        parseStr[i] = str[i];
    parseStr[i] = '\0';
    return strtoul(parseStr, nullptr, 16);
}

static std::u16string_view::const_iterator SkipCommas(std::u16string& ret, std::u16string_view str,
                                                      std::u16string_view::const_iterator it, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        auto cpos = str.find(u',', it - str.begin());
        if (cpos == std::u16string::npos)
            return str.end();
        auto end = str.begin() + cpos + 1;
        ret.insert(ret.end(), it, end);
        it = end;
    }
    return it;
}

static std::u16string_view::const_iterator UncookTextureList(std::u16string& ret, std::u16string_view str,
                                                             std::u16string_view::const_iterator it)
{
    while (true)
    {
        UniqueID32 id = ParseTag(&*it);
        hecl::ProjectPath path = UniqueIDBridge::TranslatePakIdToPath(id);
        ret.append(hecl::UTF8ToChar16(path ? path.getRelativePathUTF8() : id.toString()));
        it += 8;
        if (*it == u';')
        {
            ret.push_back(u';');
            return it + 1;
        }
        else if (*it == u',')
        {
            ret.push_back(u',');
            ++it;
        }
        else
        {
            break;
        }
    }

    /* Failsafe */
    auto scpos = str.find(u';', it - str.begin());
    if (scpos == std::u16string::npos)
        return str.end();
    return str.begin() + scpos + 1;
}

static std::u16string_view::const_iterator CookTextureList(std::u16string& ret, std::u16string_view str,
                                                           std::u16string_view::const_iterator it)
{
    while (true)
    {
        auto end = str.find_first_of(u",;", it - str.begin());
        if (end == std::u16string::npos)
            Log.report(logvisor::Fatal, "Missing comma/semicolon token while pasing font tag");
        auto endIt = str.begin() + end;
        hecl::ProjectPath path =
            UniqueIDBridge::MakePathFromString<UniqueID32>(hecl::Char16ToUTF8(std::u16string(it, endIt)));
        ret.append(hecl::UTF8ToChar16(UniqueID32(path).toString()));
        it = endIt;
        if (*it == u';')
        {
            ret.push_back(u';');
            return it + 1;
        }
        else if (*it == u',')
        {
            ret.push_back(u',');
            ++it;
        }
        else
        {
            break;
        }
    }

    /* Failsafe */
    auto scpos = str.find(u';', it - str.begin());
    if (scpos == std::u16string::npos)
        return str.end();
    return str.begin() + scpos + 1;
}

static std::u16string_view::const_iterator GatherTextureList(std::vector<hecl::ProjectPath>& pathsOut,
                                                             std::u16string_view str, std::u16string_view::const_iterator it)
{
    while (true)
    {
        auto end = str.find_first_of(u",;", it - str.begin());
        if (end == std::u16string::npos)
            Log.report(logvisor::Fatal, "Missing comma/semicolon token while pasing font tag");
        auto endIt = str.begin() + end;
        hecl::ProjectPath path =
            UniqueIDBridge::MakePathFromString<UniqueID32>(hecl::Char16ToUTF8(std::u16string(it, endIt)));
        if (path)
            pathsOut.push_back(path);

        it = endIt;
        if (*it == u';')
        {
            return it + 1;
        }
        else if (*it == u',')
        {
            ++it;
        }
        else
        {
            break;
        }
    }

    /* Failsafe */
    auto scpos = str.find(u';', it - str.begin());
    if (scpos == std::u16string::npos)
        return str.end();
    return str.begin() + scpos + 1;
}

static std::u16string UncookString(std::u16string_view str)
{
    std::u16string ret;
    ret.reserve(str.size());
    for (auto it = str.begin(); it != str.end();)
    {
        if (*it == u'&')
        {
            ret.push_back(u'&');
            ++it;
            if (!str.compare(it - str.begin(), 5, u"image"))
            {
                ret.append(u"image=");
                it += 6;
                if (!str.compare(it - str.begin(), 1, u"A"))
                {
                    it = SkipCommas(ret, str, it, 2);
                    it = UncookTextureList(ret, str, it);
                    continue;
                }
                else if (!str.compare(it - str.begin(), 2, u"SA"))
                {
                    it = SkipCommas(ret, str, it, 4);
                    it = UncookTextureList(ret, str, it);
                    continue;
                }
                else if (!str.compare(it - str.begin(), 2, u"SI"))
                {
                    it = SkipCommas(ret, str, it, 3);
                    it = UncookTextureList(ret, str, it);
                    continue;
                }
            }
            else if (!str.compare(it - str.begin(), 4, u"font"))
            {
                ret.append(u"font=");
                it += 5;
                UniqueID32 id = ParseTag(&*it);
                hecl::ProjectPath path = UniqueIDBridge::TranslatePakIdToPath(id, true);
                ret.append(hecl::UTF8ToChar16(path ? path.getRelativePathUTF8() : id.toString()));

                ret.push_back(u';');
                auto scpos = str.find(u';', it - str.begin());
                if (scpos == std::u16string::npos)
                    it = str.end();
                else
                    it = str.begin() + scpos + 1;
            }
            else
            {
                auto scpos = str.find(u';', it - str.begin());
                if (scpos == std::u16string::npos)
                {
                    it = str.end();
                }
                else
                {
                    auto end = str.begin() + scpos + 1;
                    ret.insert(ret.end(), it, end);
                    it = end;
                }
            }
        }
        else
        {
            ret.push_back(*it);
            ++it;
        }
    }
    return ret;
}

static std::u16string CookString(std::u16string_view str)
{
    std::u16string ret;
    ret.reserve(str.size());
    for (auto it = str.begin(); it != str.end();)
    {
        if (*it == u'&')
        {
            ret.push_back(u'&');
            ++it;
            if (!str.compare(it - str.begin(), 5, u"image"))
            {
                ret.append(u"image=");
                it += 6;
                if (!str.compare(it - str.begin(), 1, u"A"))
                {
                    it = SkipCommas(ret, str, it, 2);
                    it = CookTextureList(ret, str, it);
                    continue;
                }
                else if (!str.compare(it - str.begin(), 2, u"SA"))
                {
                    it = SkipCommas(ret, str, it, 4);
                    it = CookTextureList(ret, str, it);
                    continue;
                }
                else if (!str.compare(it - str.begin(), 2, u"SI"))
                {
                    it = SkipCommas(ret, str, it, 3);
                    it = CookTextureList(ret, str, it);
                    continue;
                }
            }
            else if (!str.compare(it - str.begin(), 4, u"font"))
            {
                ret.append(u"font=");
                it += 5;
                auto scpos = str.find(u';', it - str.begin());
                if (scpos == std::u16string::npos)
                    Log.report(logvisor::Fatal, "Missing semicolon token while pasing font tag");
                hecl::ProjectPath path = UniqueIDBridge::MakePathFromString<UniqueID32>(
                    hecl::Char16ToUTF8(std::u16string(it, str.begin() + scpos)));
                ret.append(hecl::UTF8ToChar16(UniqueID32(path).toString()));
                ret.push_back(u';');
                it = str.begin() + scpos + 1;
            }
            else
            {
                auto scpos = str.find(u';', it - str.begin());
                if (scpos == std::u16string::npos)
                {
                    it = str.end();
                }
                else
                {
                    auto end = str.begin() + scpos + 1;
                    ret.insert(ret.end(), it, end);
                    it = end;
                }
            }
        }
        else
        {
            ret.push_back(*it);
            ++it;
        }
    }
    return ret;
}

void STRG::gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const
{
    std::u16string skip;
    for (const auto& lang : langs)
    {
        for (const std::u16string& str : lang.second)
        {
            std::u16string_view strView(str);
            for (auto it = strView.begin(); it != strView.end();)
            {
                if (*it == u'&')
                {
                    ++it;
                    if (!str.compare(it - strView.begin(), 5, u"image"))
                    {
                        it += 6;
                        if (!str.compare(it - strView.begin(), 1, u"A"))
                        {
                            it = SkipCommas(skip, str, it, 2);
                            it = GatherTextureList(pathsOut, str, it);
                            continue;
                        }
                        else if (!str.compare(it - strView.begin(), 2, u"SA"))
                        {
                            it = SkipCommas(skip, str, it, 4);
                            it = GatherTextureList(pathsOut, str, it);
                            continue;
                        }
                        else if (!str.compare(it - strView.begin(), 2, u"SI"))
                        {
                            it = SkipCommas(skip, str, it, 3);
                            it = GatherTextureList(pathsOut, str, it);
                            continue;
                        }
                    }
                    else if (!str.compare(it - strView.begin(), 4, u"font"))
                    {
                        it += 5;
                        auto scpos = str.find(u';', it - strView.begin());
                        if (scpos == std::u16string::npos)
                            Log.report(logvisor::Fatal, "Missing semicolon token while pasing font tag");
                        hecl::ProjectPath path = UniqueIDBridge::MakePathFromString<UniqueID32>(
                            hecl::Char16ToUTF8(std::u16string(it, strView.begin() + scpos)));
                        if (path)
                            pathsOut.push_back(path);
                        it = strView.begin() + scpos + 1;
                    }
                    else
                    {
                        auto scpos = str.find(u';', it - strView.begin());
                        if (scpos == std::u16string::npos)
                            it = strView.end();
                        else
                            it = strView.begin() + scpos + 1;
                    }
                }
                else
                    ++it;
            }
        }
    }
}

void STRG::_read(athena::io::IStreamReader& reader)
{
    atUint32 langCount = reader.readUint32Big();
    atUint32 strCount = reader.readUint32Big();

    std::vector<std::pair<FourCC, atUint32>> readLangs;

    readLangs.reserve(langCount);
    for (atUint32 l = 0; l < langCount; ++l)
    {
        DNAFourCC lang;
        lang.read(reader);
        atUint32 off = reader.readUint32Big();
        readLangs.emplace_back(lang, off);
    }

    atUint32 tablesStart = reader.position();
    langs.clear();
    langs.reserve(skLanguages.size());
    for (const std::pair<FourCC, atUint32>& lang : readLangs)
    {
        std::vector<std::u16string> strs;
        reader.seek(tablesStart + lang.second, athena::SeekOrigin::Begin);
        reader.readUint32Big(); // table size
        atUint32 langStart = reader.position();
        for (atUint32 s = 0; s < strCount; ++s)
        {
            atUint32 strOffset = reader.readUint32Big();
            atUint32 tmpOffset = reader.position();
            reader.seek(langStart + strOffset, athena::SeekOrigin::Begin);
            strs.emplace_back(UncookString(reader.readU16StringBig()));
            reader.seek(tmpOffset, athena::SeekOrigin::Begin);
        }
        langs.emplace_back(lang.first, strs);
    }

    langMap.clear();
    langMap.reserve(langCount);
    for (std::pair<FourCC, std::vector<std::u16string>>& item : langs)
        langMap.emplace(item.first, &item.second);
}

template <>
void STRG::Enumerate<BigDNA::Read>(typename Read::StreamT& reader)
{
    atUint32 magic = reader.readUint32Big();
    if (magic != 0x87654321)
        Log.report(logvisor::Error, "invalid STRG magic");

    atUint32 version = reader.readUint32Big();
    if (version != 0)
        Log.report(logvisor::Error, "invalid STRG version");

    _read(reader);
}

template <>
void STRG::Enumerate<BigDNA::Write>(typename Write::StreamT& writer)
{
    writer.writeUint32Big(0x87654321);
    writer.writeUint32Big(0);
    writer.writeUint32Big(langs.size());
    atUint32 strCount = STRG::count();
    writer.writeUint32Big(strCount);

    std::vector<std::u16string> strings;
    strings.reserve(strCount * langs.size());

    atUint32 offset = 0;
    for (const std::pair<FourCC, std::vector<std::u16string>>& lang : langs)
    {
        DNAFourCC(lang.first).write(writer);
        writer.writeUint32Big(offset);
        offset += strCount * 4 + 4;
        atUint32 langStrCount = lang.second.size();
        for (atUint32 s = 0; s < strCount; ++s)
        {
            std::u16string str = CookString(lang.second[s]);
            atUint32 chCount = str.size();
            if (s < langStrCount)
                offset += (chCount + 1) * 2;
            else
                offset += 1;
            strings.push_back(std::move(str));
        }
    }

    auto langIt = strings.cbegin();
    for (const std::pair<FourCC, std::vector<std::u16string>>& lang : langs)
    {
        atUint32 langStrCount = lang.second.size();
        atUint32 tableSz = strCount * 4;
        auto strIt = langIt;
        for (atUint32 s = 0; s < strCount; ++s)
        {
            if (s < langStrCount)
                tableSz += ((strIt++)->size() + 1) * 2;
            else
                tableSz += 1;
        }
        writer.writeUint32Big(tableSz);

        offset = strCount * 4;
        strIt = langIt;
        for (atUint32 s = 0; s < strCount; ++s)
        {
            writer.writeUint32Big(offset);
            if (s < langStrCount)
                offset += ((strIt++)->size() + 1) * 2;
            else
                offset += 1;
        }

        strIt = langIt;
        for (atUint32 s = 0; s < strCount; ++s)
        {
            if (s < langStrCount)
                writer.writeU16StringBig(*strIt++);
            else
                writer.writeUByte(0);
        }

        langIt = strIt;
    }
}

template <>
void STRG::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& _s)
{
    _s += 16;
    _s += langs.size() * 12;

    size_t strCount = STRG::count();
    _s += langs.size() * strCount * 4;
    for (const std::pair<FourCC, std::vector<std::u16string>>& lang : langs)
    {
        atUint32 langStrCount = lang.second.size();
        for (atUint32 s = 0; s < strCount; ++s)
        {
            if (s < langStrCount)
                _s += (CookString(lang.second[s]).size() + 1) * 2;
            else
                _s += 1;
        }
    }
}

template <>
void STRG::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& reader)
{
    const athena::io::YAMLNode* root = reader.getRootNode();

    /* Validate Pass */
    if (root->m_type == YAML_MAPPING_NODE)
    {
        for (const auto& lang : root->m_mapChildren)
        {
            if (lang.first == "DNAType")
                continue;

            if (lang.first.size() != 4)
            {
                Log.report(logvisor::Warning, "STRG language string '%s' must be exactly 4 characters; skipping",
                           lang.first.c_str());
                return;
            }
            if (lang.second->m_type != YAML_SEQUENCE_NODE)
            {
                Log.report(logvisor::Warning, "STRG language string '%s' must contain a sequence; skipping",
                           lang.first.c_str());
                return;
            }
            for (const auto& str : lang.second->m_seqChildren)
            {
                if (str->m_type != YAML_SCALAR_NODE)
                {
                    Log.report(logvisor::Warning, "STRG language '%s' must contain all scalars; skipping",
                               lang.first.c_str());
                    return;
                }
            }
        }
    }
    else
    {
        Log.report(logvisor::Warning, "STRG must have a mapping root node; skipping");
        return;
    }

    /* Read Pass */
    langs.clear();
    for (const auto& lang : root->m_mapChildren)
    {
        if (lang.first == "DNAType")
            continue;

        std::vector<std::u16string> strs;
        for (const auto& str : lang.second->m_seqChildren)
            strs.emplace_back(hecl::UTF8ToChar16(str->m_scalarString));
        langs.emplace_back(FourCC(lang.first.c_str()), strs);
    }

    langMap.clear();
    langMap.reserve(langs.size());
    for (auto& item : langs)
        langMap.emplace(item.first, &item.second);
}

template <>
void STRG::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& writer)
{
    for (const auto& lang : langs)
    {
        if (auto v = writer.enterSubVector(lang.first.toString().c_str()))
            for (const std::u16string& str : lang.second)
                writer.writeU16String(nullptr, str);
    }
}

const char* STRG::DNAType() { return "urde::DNAMP1::STRG"; }
}
