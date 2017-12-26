#ifndef GUI_COMMON_HPP
#define GUI_COMMON_HPP

#include <QString>
#include "zeus/Math.hpp"

enum class Platform
{
    Invalid,
    MacOS,
    Win32,
    Linux,
    MAXPlatform
};
QString PlatformToString(Platform plat);
Platform StringToPlatform(const QString& str);

#if __APPLE__
constexpr Platform CurPlatform = Platform::MacOS;
#elif _WIN32
constexpr Platform CurPlatform = Platform::Win32;
#elif __linux__
constexpr Platform CurPlatform = Platform::Linux;
#endif

extern const QString CurPlatformString;

enum class Architecture
{
    Invalid,
    X86,
    X86_64,
    ARM,
    AARCH64,
    MAXArchitecture
};
QString ArchitectureToString(Architecture arch);
Architecture StringToArchitecture(const QString& str);

#if ZEUS_ARCH_X86_64
constexpr Architecture CurArchitecture = Architecture::X86_64;
#elif ZEUS_ARCH_X86
constexpr Architecture CurArchitecture = Architecture::X86;
#elif ZEUS_ARCH_ARM
constexpr Architecture CurArchitecture = Architecture::ARM;
#elif ZEUS_ARCH_AARCH64
constexpr Architecture CurArchitecture = Architecture::AARCH64;
#endif

extern const QString CurArchitectureString;

enum class VectorISA
{
    Invalid,
    X87,
    SSE,
    SSE2,
    SSE3,
    SSE41,
    AVX,
    AVX2,
    MAXVectorISA
};
QString VectorISAToString(VectorISA visa);
VectorISA StringToVectorISA(const QString& str);

class URDEVersion
{
    int m_version = -1;
    VectorISA m_vectorISA;
public:
    explicit URDEVersion(const QString& filename);
    bool isValid() const { return m_version >= 0; }

};

#endif // GUI_COMMON_HPP
