#include "Common.hpp"
#include <QStringList>
#include <csignal>

#ifndef _WIN32
#include <unistd.h>
#endif

#if __APPLE__
const QString CurPlatformString = QStringLiteral("macos");
#elif _WIN32
const QString CurPlatformString = QStringLiteral("win32");
#elif __linux__
const QString CurPlatformString = QStringLiteral("linux");
#else
#error HECL does not know which OS to fetch for
#endif

QString PlatformToString(Platform plat) {
  switch (plat) {
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

Architecture CurArchitecture = Architecture::Invalid;
QString CurArchitectureString;

Platform StringToPlatform(const QString& str) {
  for (int i = 1; i < int(Platform::MAXPlatform); ++i)
    if (!str.compare(PlatformToString(Platform(i)), Qt::CaseInsensitive))
      return Platform(i);
  return Platform::Invalid;
}

QString ArchitectureToString(Architecture arch) {
  switch (arch) {
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

Architecture StringToArchitecture(const QString& str) {
  for (int i = 1; i < int(Architecture::MAXArchitecture); ++i)
    if (!str.compare(ArchitectureToString(Architecture(i)), Qt::CaseInsensitive))
      return Architecture(i);
  return Architecture::Invalid;
}

QString VectorISAToString(VectorISA visa) {
  switch (visa) {
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

VectorISA StringToVectorISA(const QString& str) {
  for (int i = 1; i < int(VectorISA::MAXVectorISA); ++i)
    if (!str.compare(VectorISAToString(VectorISA(i)), Qt::CaseInsensitive))
      return VectorISA(i);
  return VectorISA::Invalid;
}

URDEVersion::URDEVersion(const QString& filename) {
  int idx;
  QString useFilename = filename;
  if ((idx = filename.lastIndexOf(QLatin1Char{'.'})) >= 0) {
    m_extension = QString(filename).remove(0, idx);
    useFilename.truncate(idx);
  }

  const QStringList list = useFilename.split(QLatin1Char{'-'});
  enum {
    version,
    platform,
    architecture,
    vectorISA,
    extra
  } state = version;
  for (size_t i = 1; i < list.size(); ++i) {
    if (state == version) {
      if (m_version.isEmpty()) {
        m_version = list[i];
        continue;
      }
      bool ok;
      int patch = list[i].toInt(&ok);
      if (ok) {
        m_version += QLatin1Char('-') + QString::number(patch);
        continue;
      }
      state = platform;
    }
    if (state == platform) {
      state = architecture;
      Platform platValue = StringToPlatform(list[i]);
      if (platValue != Platform::Invalid) {
        m_platform = platValue;
        continue;
      }
    }
    if (state == architecture) {
      state = vectorISA;
      Architecture archValue = StringToArchitecture(list[i]);
      if (archValue != Architecture::Invalid) {
        m_architecture = archValue;
        continue;
      }
    }
    if (state == vectorISA) {
      state = extra;
      VectorISA isa = StringToVectorISA(list[i]);
      if (isa != VectorISA::Invalid) {
        m_vectorISA = isa;
        continue;
      }
    }
    m_extra += QLatin1Char('-') + list[i];
  }
}

QString URDEVersion::fileString(bool withExtension) const {
  if (m_version.isEmpty()) {
    return {};
  }

  if (withExtension && !m_extension.isEmpty()) {
    return QStringLiteral("urde-%1-%2-%3-%4%5%6")
        .arg(m_version, PlatformToString(m_platform), ArchitectureToString(m_architecture),
             VectorISAToString(m_vectorISA), m_extra, m_extension);
  } else {
    return QStringLiteral("urde-%1-%2-%3-%4%5")
        .arg(m_version, PlatformToString(m_platform), ArchitectureToString(m_architecture),
             VectorISAToString(m_vectorISA), m_extra);
  }
}

#ifndef _WIN32
static void HUPHandler(int) {}
#endif

void InitializePlatform() {
#ifndef _WIN32
  /* This can happen when terminating hecl - do nothing */
  signal(SIGHUP, HUPHandler);
#endif

#if ZEUS_ARCH_X86_64
  const_cast<Architecture&>(CurArchitecture) = Architecture::X86_64;
#elif ZEUS_ARCH_X86
#if !defined(__APPLE__) && !defined(_WIN32)
  const_cast<Architecture&>(CurArchitecture) = (sysconf(_SC_WORD_BIT) == 64 ? Architecture::X86_64 : Architecture::X86);
#elif _WIN32
  bool isWOW = false;
  IsWow64Process(GetCurrentProcess(), &isWOW);
  const_cast<Architecture&>(CurArchitecture) = (isWOW ? Architecture::X86_64 : Architecture::X86);
#endif
#endif
  const_cast<QString&>(CurArchitectureString) = ArchitectureToString(CurArchitecture);
}
