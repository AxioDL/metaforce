#ifndef HECL_HPP
#define HECL_HPP

#if _WIN32
char* win_realpath(const char* name, char* restrict resolved);
#else
#include <stdlib.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <dirent.h>
#endif

#include <functional>
#include <string>
#include <regex>
#include <stdexcept>
#include "../extern/blowfish/blowfish.h"

namespace HECL
{

#if _WIN32
typedef std::basic_string<TCHAR> TSystemPath;
#else
typedef std::string TSystemPath;
#endif

class ProjectRootPath;
static const std::regex regGLOB("\\*", std::regex::ECMAScript|std::regex::optimize);
static const std::regex regPATHCOMP("/([^/]+)", std::regex::ECMAScript|std::regex::optimize);
static const std::regex regDRIVELETTER("^([^/]*)/", std::regex::ECMAScript|std::regex::optimize);

/**
 * @brief Severity of a log event
 */
enum LogType
{
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
};

/**
 * @brief Logger callback type
 */
typedef std::function<void(LogType, std::string&)> TLogger;

/**
 * @brief FourCC representation used within HECL's database
 *
 * FourCCs are efficient, mnemonic four-char-sequences used to represent types
 * while fitting comfortably in a 32-bit word. HECL uses a four-char array
 * to remain endian-independent.
 */
class FourCC
{
    union
    {
        char fcc[4];
        uint32_t num;
    };
public:
    FourCC() /* Sentinel FourCC */
    : num(0) {}
    FourCC(const char* name)
    : num(*(uint32_t*)name) {}
    inline bool operator==(FourCC& other) {return num == other.num;}
    inline bool operator!=(FourCC& other) {return num != other.num;}
    inline std::string toString() {return std::string(fcc, 4);}
};

/**
 * @brief Hash representation used for all storable and comparable objects
 *
 * Hashes are used within HECL to avoid redundant storage of objects;
 * providing a rapid mechanism to compare for equality.
 */
class ObjectHash
{
    int64_t hash;
public:
    ObjectHash(const void* buf, size_t len)
    : hash(Blowfish_hash(buf, len)) {}
    ObjectHash(int64_t hashin)
    : hash(hashin) {}
    inline bool operator==(ObjectHash& other) {return hash == other.hash;}
    inline bool operator!=(ObjectHash& other) {return hash != other.hash;}
    inline bool operator<(ObjectHash& other) {return hash < other.hash;}
    inline bool operator>(ObjectHash& other) {return hash > other.hash;}
    inline bool operator<=(ObjectHash& other) {return hash <= other.hash;}
    inline bool operator>=(ObjectHash& other) {return hash >= other.hash;}
};

/**
 * @brief Canonicalized project path representation using POSIX conventions
 *
 * HECL uses POSIX-style paths (with '/' separator) and directory tokens
 * ('.','..') to resolve files within a project. The database internally
 * uses this representation to track working files.
 *
 * This class provides a convenient way to resolve paths relative to the
 * project root. Part of this representation involves resolving symbolic
 * links to regular file/directory paths and determining its type.
 *
 * NOTE THAT PROJECT PATHS ARE TREATED AS CASE SENSITIVE!!
 */
class ProjectPath
{
protected:
    TSystemPath m_absPath;
    const char* m_relPath = NULL;
    ProjectPath() {}
    bool _canonAbsPath(const TSystemPath& path)
    {
#if _WIN32
#else
        char resolvedPath[PATH_MAX];
        if (!realpath(path.c_str(), resolvedPath))
        {
            throw std::invalid_argument("Unable to resolve '" + path + "' as a canonicalized path");
            return false;
        }
        m_absPath = resolvedPath;
#endif
        return true;
    }
public:
    /**
     * @brief Construct a project subpath representation
     * @param rootPath previously constructed ProjectRootPath held by HECLDatabase::IProject
     * @param path valid filesystem-path (relative or absolute) to subpath
     */
    ProjectPath(const ProjectRootPath& rootPath, const TSystemPath& path)
    {
        _canonAbsPath(path);
        if (m_absPath.size() < ((ProjectPath&)rootPath).m_absPath.size() ||
            m_absPath.compare(0, ((ProjectPath&)rootPath).m_absPath.size(),
                              ((ProjectPath&)rootPath).m_absPath))
        {
            throw std::invalid_argument("'" + m_absPath + "' is not a subpath of '" +
                                        ((ProjectPath&)rootPath).m_absPath + "'");
            return;
        }
        if (m_absPath.size() == ((ProjectPath&)rootPath).m_absPath.size())
        {
            /* Copies of the project root are permitted */
            return;
        }
        m_relPath = m_absPath.c_str() + ((ProjectPath&)rootPath).m_absPath.size();
        if (m_relPath[0] == '/')
            ++m_relPath;
        if (m_relPath[0] == '\0')
            m_relPath = NULL;
    }
    /**
     * @brief Determine if ProjectPath represents project root directory
     * @return true if project root directory
     */
    inline bool isRoot() {return (m_relPath == NULL);}

    /**
     * @brief Access fully-canonicalized absolute path
     * @return Absolute path reference
     */
    inline const TSystemPath& getAbsolutePath() {return m_absPath;}

    /**
     * @brief Access fully-canonicalized project-relative path
     * @return Relative pointer to within absolute-path or "." for project root-directory (use isRoot to detect)
     */
    inline const char* getRelativePath()
    {
        if (m_relPath)
            return m_relPath;
        return ".";
    }

    /**
     * @brief Type of path
     */
    enum PathType
    {
        PT_NONE, /**< If path doesn't reference a valid filesystem entity, this is returned */
        PT_FILE, /**< Singular file path (confirmed with filesystem) */
        PT_DIRECTORY, /**< Singular directory path (confirmed with filesystem) */
        PT_GLOB /**< Glob-path (whenever one or more '*' occurs in syntax) */
    };

    /**
     * @brief Get type of path based on syntax and filesystem queries
     * @return Type of path
     */
    PathType getPathType()
    {
        if (std::regex_search(m_absPath, regGLOB))
            return PT_GLOB;
#if _WIN32
#else
        struct stat theStat;
        if (stat(m_absPath.c_str(), &theStat))
            return PT_NONE;
        if (S_ISDIR(theStat.st_mode))
            return PT_DIRECTORY;
        if (S_ISREG(theStat.st_mode))
            return PT_FILE;
        return PT_NONE;
#endif
    }

    void getGlobResults(std::vector<TSystemPath>& outPaths)
    {
#if _WIN32
        std::string itStr;
        std::smatch letterMatch;
        if (m_absPath.compare(0, 2, "//"))
            itStr = "\\\\";
        else if (std::regex_search(m_absPath, letterMatch, regDRIVELETTER))
            if (letterMatch[1].str().size())
                itStr = letterMatch[1];
#else
        std::string itStr = "/";
#endif
        bool needSlash = false;

        std::sregex_token_iterator pathComps(m_absPath.begin(), m_absPath.end(), regPATHCOMP);
        for (; pathComps != std::sregex_token_iterator() ; ++pathComps)
        {
            const std::string& comp = *pathComps;
            if (!std::regex_search(comp, regGLOB))
            {
                if (needSlash)
                    itStr += '/';
                else
                    needSlash = true;
                itStr += comp;
                continue;
            }
#if _WIN32
#else
            DIR* dir = opendir("");
#endif
        }
    }
};

/**
 * @brief Special ProjectRootPath subclass for opening HECLDatabase::IProject instances
 *
 * Constructing a ProjectPath requires supplying a ProjectRootPath to consistently
 * resolve canonicalized relative paths.
 */
class ProjectRootPath : public ProjectPath
{
public:
    ProjectRootPath(const TSystemPath& path)
    {
        _canonAbsPath(path);
    }
};



/* Type-sensitive byte swappers */
static inline int16_t bswap(int16_t val)
{
#if __GNUC__
    return __builtin_bswap16(val);
#elif _WIN32
    return _byteswap_ushort(val);
#else
    return (val = (val << 8) | ((val >> 8) & 0xFF));
#endif
}

static inline uint16_t bswap(uint16_t val)
{
#if __GNUC__
    return __builtin_bswap16(val);
#elif _WIN32
    return _byteswap_ushort(val);
#else
    return (val = (val << 8) | ((val >> 8) & 0xFF));
#endif
}

static inline int32_t bswap(int32_t val)
{
#if __GNUC__
    return __builtin_bswap32(val);
#elif _WIN32
    return _byteswap_ulong(val);
#else
    val = (val & 0x0000FFFF) << 16 | (val & 0xFFFF0000) >> 16;
    val = (val & 0x00FF00FF) << 8 | (val & 0xFF00FF00) >> 8;
    return val;
#endif
}

static inline uint32_t bswap(uint32_t val)
{
#if __GNUC__
    return __builtin_bswap32(val);
#elif _WIN32
    return _byteswap_ulong(val);
#else
    val = (val & 0x0000FFFF) << 16 | (val & 0xFFFF0000) >> 16;
    val = (val & 0x00FF00FF) << 8 | (val & 0xFF00FF00) >> 8;
    return val;
#endif
}

static inline int64_t bswap(int64_t val)
{
#if __GNUC__
    return __builtin_bswap64(val);
#elif _WIN32
    return _byteswap_uint64(val);
#else
    return (val = ((atInt64)((((atInt64)(val) & 0xFF00000000000000ULL) >> 56) |
                             (((atInt64)(val) & 0x00FF000000000000ULL) >> 40) |
                             (((atInt64)(val) & 0x0000FF0000000000ULL) >> 24) |
                             (((atInt64)(val) & 0x000000FF00000000ULL) >>  8) |
                             (((atInt64)(val) & 0x00000000FF000000ULL) <<  8) |
                             (((atInt64)(val) & 0x0000000000FF0000ULL) << 24) |
                             (((atInt64)(val) & 0x000000000000FF00ULL) << 40) |
                             (((atInt64)(val) & 0x00000000000000FFULL) << 56))));
#endif
}

static inline uint64_t bswap(uint64_t val)
{
#if __GNUC__
    return __builtin_bswap64(val);
#elif _WIN32
    return _byteswap_uint64(val);
#else
    return (val = ((atInt64)((((atInt64)(val) & 0xFF00000000000000ULL) >> 56) |
                             (((atInt64)(val) & 0x00FF000000000000ULL) >> 40) |
                             (((atInt64)(val) & 0x0000FF0000000000ULL) >> 24) |
                             (((atInt64)(val) & 0x000000FF00000000ULL) >>  8) |
                             (((atInt64)(val) & 0x00000000FF000000ULL) <<  8) |
                             (((atInt64)(val) & 0x0000000000FF0000ULL) << 24) |
                             (((atInt64)(val) & 0x000000000000FF00ULL) << 40) |
                             (((atInt64)(val) & 0x00000000000000FFULL) << 56))));
#endif
}

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define HECLMakeBig(val) HECL::bswap(val)
#define HECLMakeLittle(val) (val)
#else
#define HECLMakeBig(val) (val)
#define HECLMakeLittle(val) HECL::bswap(val)
#endif

}

#endif // HECL_HPP
