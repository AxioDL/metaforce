#include "SysReqTableView.hpp"
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QHeaderView>

#if _WIN32
#else
#include <unistd.h>
#endif

#if __APPLE__
#include "MacOSSystemVersion.hpp"
#endif

SysReqTableModel::SysReqTableModel(QObject* parent)
: QAbstractTableModel(parent)
{
    int regs[4] = {};
    zeus::getCpuInfo(0, regs);
    if (regs[0] >= 0x16)
    {
        zeus::getCpuInfo(0x16, regs);
        m_cpuSpeed = uint64_t(regs[0]);
    }
    m_cpuSpeedStr.sprintf("%g GHz", m_cpuSpeed / 1000.f);
#if _WIN32
#else
    m_memorySize = uint64_t(sysconf(_SC_PHYS_PAGES)) * sysconf(_SC_PAGESIZE);
#endif
    m_memorySizeStr.sprintf("%g GB", m_memorySize / 1024.f / 1024.f / 1024.f);
#ifdef __APPLE__
    GetMacOSSystemVersion(m_macosMajor, m_macosMinor, m_macosPatch);
    if (m_macosPatch == 0)
        m_osVersion.sprintf("macOS %d.%d", m_macosMajor, m_macosMinor);
    else
        m_osVersion.sprintf("macOS %d.%d.%d", m_macosMajor, m_macosMinor, m_macosPatch);
#endif
}

int SysReqTableModel::rowCount(const QModelIndex& parent) const
{
    return 5;
}

int SysReqTableModel::columnCount(const QModelIndex& parent) const
{
    return 2;
}

QVariant SysReqTableModel::data(const QModelIndex& index, int role) const
{
    if (role != Qt::DisplayRole && role != Qt::UserRole)
    {
        return {};
    }

    if (role == Qt::UserRole)
    {
        switch (index.row())
        {
        case 0:
            return true;
        case 1:
            return m_cpuSpeed >= 1500;
        case 2:
            return m_memorySize >= 0x100000000;
        case 3:
#ifdef __APPLE__
            return m_macosMajor > 10 || m_macosMinor >= 9;
#else
            return true;
#endif
        }
    }
    else
    {
        if (index.column() == 0)
        {
            /* Recommended */
            switch (index.row())
            {
            case 0:
#if ZEUS_ARCH_X86 || ZEUS_ARCH_X86_64
                return QStringLiteral("x86_64");
#else
                return {};
#endif
            case 1:
                return QStringLiteral("1.5 GHz");
            case 2:
                return QStringLiteral("4 GB");
            case 3:
#ifdef __APPLE__
                return QStringLiteral("macOS 10.9");
#else
                return {};
#endif
            }
        }
        else if (index.column() == 1)
        {
            /* Your System */
            switch (index.row())
            {
            case 0:
#if ZEUS_ARCH_X86 || ZEUS_ARCH_X86_64
                return CurArchitectureString;
#else
                return {};
#endif
            case 1:
                return m_cpuSpeedStr;
            case 2:
                return m_memorySizeStr;
            case 3:
                return m_osVersion;
            }
        }
    }
    return {};
}

QVariant SysReqTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return {};
    }

    if (orientation == Qt::Horizontal)
    {
        switch (section)
        {
        case 0:
        default:
            return QStringLiteral("Recommended");
        case 1:
            return QStringLiteral("Your System");
        }
    }
    else
    {
        switch (section)
        {
        case 0:
        default:
            return QStringLiteral("Architecture");
        case 1:
            return QStringLiteral("CPU Speed");
        case 2:
            return QStringLiteral("Memory");
        case 3:
            return QStringLiteral("OS");
        case 4:
            return QStringLiteral("Vector ISA");
        }
    }
}

void SysReqTableView::paintEvent(QPaintEvent* e)
{
    int tableWidth = columnWidth(0) + columnWidth(1);
    int tableX = verticalHeader()->width() + columnViewportPosition(0);
    int tableY = horizontalHeader()->height();
    for (int i = 0; i < 4; ++i)
    {
        QWidget* w = std::get<0>(m_backgroundWidgets[i]);
        QSequentialAnimationGroup* animation = std::get<1>(m_backgroundWidgets[i]);
        QPropertyAnimation* pAnimation = static_cast<QPropertyAnimation*>(animation->animationAt(1));
        bool& running = std::get<2>(m_backgroundWidgets[i]);
        if (!running)
        {
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

SysReqTableView::SysReqTableView(QWidget* parent)
: QTableView(parent), m_vectorISATable(this)
{
    setModel(&m_model);
    setIndexWidget(m_model.index(4, 0), &m_vectorISATable);
    setSpan(4, 0, 1, 2);

    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
    setFocusPolicy(Qt::NoFocus);

    for (int i = 0; i < 4; ++i)
    {
        QWidget* w = new QWidget(this);
        std::get<0>(m_backgroundWidgets[i]) = w;

        QPalette pal = palette();
        if (m_model.data(m_model.index(i, 0), Qt::UserRole).toBool())
            pal.setColor(QPalette::Background, QColor::fromRgbF(0.f, 1.f, 0.f, 0.2f));
        else
            pal.setColor(QPalette::Background, QColor::fromRgbF(1.f, 0.f, 0.f, 0.2f));

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
