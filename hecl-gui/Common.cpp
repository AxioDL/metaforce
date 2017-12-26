#include "Common.hpp"

#if __APPLE__
const QString CurPlatformString = QStringLiteral("macos");
#elif _WIN32
const QString CurPlatformString = QStringLiteral("win32");
#elif __linux__
const QString CurPlatformString = QStringLiteral("linux");
#else
#error HECL does not know which OS to fetch for
#endif

QString PlatformToString(Platform plat)
{
    switch (plat)
    {
    case Platform::MacOS:
        return QStringLiteral("macos");
    case Platform::Win32:
        return QStringLiteral("win32");
    case Platform::Linux:
        return QStringLiteral("linux");
    default:
        return QString();
    }
}

#if ZEUS_ARCH_X86_64
const QString CurArchitectureString = QStringLiteral("x86_64");
#elif ZEUS_ARCH_X86
const QString CurArchitectureString = QStringLiteral("x86");
#elif ZEUS_ARCH_ARM
const QString CurArchitectureString = QStringLiteral("arm");
#elif ZEUS_ARCH_AARCH64
const QString CurArchitectureString = QStringLiteral("aarch64");
#else
#error HECL does not know which architecture to fetch for
#endif

Platform StringToPlatform(const QString& str)
{
    for (int i = 1; i < int(Platform::MAXPlatform); ++i)
        if (str.contains(PlatformToString(Platform(i)), Qt::CaseInsensitive))
            return Platform(i);
    return Platform::Invalid;
}

QString ArchitectureToString(Architecture arch)
{
    switch (arch)
    {
    case Architecture::X86:
        return QStringLiteral("x86");
    case Architecture::X86_64:
        return QStringLiteral("x86_64");
    case Architecture::ARM:
        return QStringLiteral("arm");
    case Architecture::AARCH64:
        return QStringLiteral("aarch64");
    default:
        return QString();
    }
}

Architecture StringToArchitecture(const QString& str)
{
    for (int i = 1; i < int(Architecture::MAXArchitecture); ++i)
        if (str.contains(ArchitectureToString(Architecture(i)), Qt::CaseInsensitive))
            return Architecture(i);
    return Architecture::Invalid;
}

QString VectorISAToString(VectorISA visa)
{
    switch (visa)
    {
    case VectorISA::X87:
        return QStringLiteral("x87");
    case VectorISA::SSE:
        return QStringLiteral("sse");
    case VectorISA::SSE2:
        return QStringLiteral("sse2");
    case VectorISA::SSE3:
        return QStringLiteral("sse3");
    case VectorISA::SSE41:
        return QStringLiteral("sse41");
    case VectorISA::AVX:
        return QStringLiteral("avx");
    case VectorISA::AVX2:
        return QStringLiteral("avx2");
    default:
        return QString();
    }
}

VectorISA StringToVectorISA(const QString& str)
{
    for (int i = 1; i < int(VectorISA::MAXVectorISA); ++i)
        if (str.contains(VectorISAToString(VectorISA(i)), Qt::CaseInsensitive))
            return VectorISA(i);
    return VectorISA::Invalid;
}
