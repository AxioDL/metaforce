#include "SysReqTableView.hpp"
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QHeaderView>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDomDocument>
#include <QProcess>
#include <QStorageInfo>
#include "FindBlender.hpp"
#include <QDebug>

#if _WIN32
#include <Windows.h>
#include <VersionHelpers.h>
#else
#include <unistd.h>
#endif

#if __APPLE__
#include "MacOSSystemVersion.hpp"
#elif _WIN32
static QString GetWindowsVersionString() {
  if (IsWindows10OrGreater())
    return QObject::tr("Windows 10");
  else if (IsWindows8Point1OrGreater())
    return QObject::tr("Windows 8.1");
  else if (IsWindows8OrGreater())
    return QObject::tr("Windows 8");
  else if (IsWindows7SP1OrGreater())
    return QObject::tr("Windows 7 SP1");
  else if (IsWindows7OrGreater())
    return QObject::tr("Windows 7");
  else if (IsWindowsVistaOrGreater())
    return QObject::tr("Windows Vista");
  else if (IsWindowsXPOrGreater())
    return QObject::tr("Windows XP");
  else
    return QObject::tr("Windows Old And Won't Work");
}
#endif

SysReqTableModel::SysReqTableModel(QObject* parent) : QAbstractTableModel(parent) {
#if _WIN32
  ULONGLONG memSize;
  GetPhysicallyInstalledSystemMemory(&memSize);
  m_memorySize = memSize * 1024;
#else
  m_memorySize = uint64_t(sysconf(_SC_PHYS_PAGES)) * sysconf(_SC_PAGESIZE);
#endif
  m_memorySizeStr = tr("%1 GiB").arg(m_memorySize / 1024.f / 1024.f / 1024.f);
#ifdef __APPLE__
  GetMacOSSystemVersion(m_macosMajor, m_macosMinor, m_macosPatch);
  if (m_macosPatch == 0) {
    m_osVersion = tr("macOS %1.%2").arg(m_macosMajor, m_macosMinor);
  } else {
    m_osVersion = tr("macOS %1.%2.%3")
                      .arg(QString::number(m_macosMajor), QString::number(m_macosMinor), QString::number(m_macosPatch));
  }
#elif _WIN32
  m_win7SP1OrGreater = IsWindows7SP1OrGreater();
  m_osVersion = GetWindowsVersionString();
#elif __linux__
  m_osVersion = tr("Linux");
#endif
  hecl::blender::FindBlender(m_blendMajor, m_blendMinor);
  if (m_blendMajor != 0) {
    m_blendVersionStr = tr("Blender %1.%2").arg(QString::number(m_blendMajor), QString::number(m_blendMinor));
  } else {
    m_blendVersionStr = tr("Not Found");
  }
}

void SysReqTableModel::updateFreeDiskSpace(const QString& path) {
  if (path.isEmpty()) {
    m_freeDiskSpace = 0;
    m_freeDiskSpaceStr = tr("<Set Working Directory>");
  } else {
    m_freeDiskSpace = QStorageInfo(path).bytesFree();
    m_freeDiskSpaceStr = tr("%1 GB").arg(m_freeDiskSpace / 1000.f / 1000.f / 1000.f, 1, 'f', 1);
  }
  emit dataChanged(index(1, 0), index(1, 0));
}

int SysReqTableModel::rowCount(const QModelIndex& parent) const { return 4; }

int SysReqTableModel::columnCount(const QModelIndex& parent) const { return 2; }

QVariant SysReqTableModel::data(const QModelIndex& index, int role) const {
  if (role != Qt::DisplayRole && role != Qt::UserRole) {
    return {};
  }

  if (role == Qt::UserRole) {
    switch (index.row()) {
    case 0:
      return m_memorySize >= 0xC0000000;
    case 1:
      return m_freeDiskSpace >= qint64(5) * 1000 * 1000 * 1000;
    case 2:
#ifdef __APPLE__
      return m_macosMajor > 10 || m_macosMinor >= 11;
#elif defined(_WIN32)
      return m_win7SP1OrGreater;
#else
      return true;
#endif
    case 3:
      return isBlenderVersionOk();
    }
  } else {
    if (index.column() == 0) {
      /* Recommended */
      switch (index.row()) {
      case 0:
        return tr("3 GiB");
      case 1:
        return tr("5 GB (MP1)");
      case 2:
#ifdef __APPLE__
        return tr("macOS 10.11");
#elif defined(_WIN32)
        return tr("Windows 7 SP1");
#elif defined(__linux__)
        return tr("Linux");
#else
        return {};
#endif
      case 3:
        return QStringLiteral("Blender %1.%2+")
            .arg(hecl::blender::MinBlenderMajorSearch)
            .arg(hecl::blender::MinBlenderMinorSearch);
      }
    } else if (index.column() == 1) {
      /* Your System */
      switch (index.row()) {
      case 0:
        return m_memorySizeStr;
      case 1:
        return m_freeDiskSpaceStr;
      case 2:
        return m_osVersion;
      case 3:
        return m_blendVersionStr;
      }
    }
  }
  return {};
}

QVariant SysReqTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role != Qt::DisplayRole) {
    return {};
  }

  if (orientation == Qt::Horizontal) {
    switch (section) {
    case 0:
    default:
      return tr("Recommended");
    case 1:
      return tr("Your System");
    }
  } else {
    switch (section) {
    default:
    case 0:
      return tr("Memory");
    case 1:
      return tr("Disk Space");
    case 2:
      return tr("OS");
    case 3:
      return tr("Blender");
    }
  }
}

bool SysReqTableModel::isBlenderVersionOk() const {
  return (m_blendMajor >= hecl::blender::MinBlenderMajorSearch &&
          m_blendMajor <= hecl::blender::MaxBlenderMajorSearch) &&
         (m_blendMinor >= hecl::blender::MinBlenderMinorSearch && m_blendMinor <= hecl::blender::MaxBlenderMinorSearch);
}

void SysReqTableView::paintEvent(QPaintEvent* e) {
  int tableWidth = columnWidth(0) + columnWidth(1);
  int tableX = verticalHeader()->width() + columnViewportPosition(0);
  int tableY = horizontalHeader()->height();
  for (int i = 0; i < 6; ++i) {
    QWidget* w = std::get<0>(m_backgroundWidgets[i]);

    QPalette pal = palette();
    if (m_model.data(m_model.index(i, 0), Qt::UserRole).toBool())
      pal.setColor(QPalette::Window, QColor::fromRgbF(0.f, 1.f, 0.f, 0.2f));
    else
      pal.setColor(QPalette::Window, QColor::fromRgbF(1.f, 0.f, 0.f, 0.2f));
    w->setPalette(pal);

    QSequentialAnimationGroup* animation = std::get<1>(m_backgroundWidgets[i]);
    QPropertyAnimation* pAnimation = static_cast<QPropertyAnimation*>(animation->animationAt(1));
    bool& running = std::get<2>(m_backgroundWidgets[i]);
    if (!running) {
      w->setGeometry(QRect(tableX, tableY + rowViewportPosition(i), 0, rowHeight(i)));
      pAnimation->setStartValue(QRect(tableX, tableY + rowViewportPosition(i), 0, rowHeight(i)));
      pAnimation->setEndValue(QRect(tableX, tableY + rowViewportPosition(i), tableWidth, rowHeight(i)));
      animation->start();
      running = true;
    }
    if (animation->state() == QAbstractAnimation::State::Running)
      pAnimation->setEndValue(QRect(tableX, tableY + rowViewportPosition(i), tableWidth, rowHeight(i)));
    else
      w->setGeometry(QRect(tableX, tableY + rowViewportPosition(i), tableWidth, rowHeight(i)));
  }
  QTableView::paintEvent(e);
}

SysReqTableView::SysReqTableView(QWidget* parent) : QTableView(parent) {
  setModel(&m_model);

  horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
  setFocusPolicy(Qt::NoFocus);

  for (int i = 0; i < 6; ++i) {
    QWidget* w = new QWidget(this);
    std::get<0>(m_backgroundWidgets[i]) = w;

    QPalette pal = palette();
    if (m_model.data(m_model.index(i, 0), Qt::UserRole).toBool())
      pal.setColor(QPalette::Window, QColor::fromRgbF(0.f, 1.f, 0.f, 0.2f));
    else
      pal.setColor(QPalette::Window, QColor::fromRgbF(1.f, 0.f, 0.f, 0.2f));

    w->setAutoFillBackground(true);
    w->setPalette(pal);
    w->lower();
    w->show();

    QPropertyAnimation* animation = new QPropertyAnimation(w, "geometry", this);
    animation->setDuration(2000);
    animation->setEasingCurve(QEasingCurve::Type::InOutCubic);

    QSequentialAnimationGroup* seq = new QSequentialAnimationGroup(this);
    std::get<1>(m_backgroundWidgets[i]) = seq;
    seq->addPause(i * 100);
    seq->addAnimation(animation);
  }
}
