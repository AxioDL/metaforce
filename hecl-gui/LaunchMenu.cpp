#include "LaunchMenu.hpp"
#include "hecl/CVarCommons.hpp"

LaunchMenu::LaunchMenu(hecl::CVarCommons& commons, QWidget* parent)
: QMenu("Launch Menu", parent),
  m_commons(commons),
  m_apiMenu("Graphics API", this),
  m_msaaMenu("Anti-Aliasing", this),
  m_anisoMenu("Anisotropic Filtering", this),
  m_apiGroup(this),
  m_msaaGroup(this),
  m_anisoGroup(this)
{
#ifdef _WIN32
    initApiAction(QStringLiteral("D3D11"));
    initApiAction(QStringLiteral("Vulkan"));
    initApiAction(QStringLiteral("OpenGL"));
#elif defined(__APPLE__)
    initApiAction(QStringLiteral("Metal"));
    initApiAction(QStringLiteral("OpenGL"));
#else
    initApiAction(QStringLiteral("OpenGL"));
    initApiAction(QStringLiteral("Vulkan"));
#endif

    initMsaaAction(QStringLiteral("1"));
    initMsaaAction(QStringLiteral("2"));
    initMsaaAction(QStringLiteral("4"));
    initMsaaAction(QStringLiteral("8"));
    initMsaaAction(QStringLiteral("16"));

    initAnisoAction(QStringLiteral("1"));
    initAnisoAction(QStringLiteral("2"));
    initAnisoAction(QStringLiteral("4"));
    initAnisoAction(QStringLiteral("8"));
    initAnisoAction(QStringLiteral("16"));

    m_apiMenu.addActions(m_apiGroup.actions());
    m_msaaMenu.addActions(m_msaaGroup.actions());
    m_anisoMenu.addActions(m_anisoGroup.actions());
    addMenu(&m_apiMenu);
    addMenu(&m_msaaMenu);
    addMenu(&m_anisoMenu);
}

void LaunchMenu::initApiAction(const QString& action)
{
    QAction* act = m_apiGroup.addAction(action);
    connect(act, SIGNAL(triggered()), this, SLOT(apiTriggered()));
    act->setCheckable(true);
    if (!action.compare(QString::fromStdString(m_commons.getGraphicsApi()), Qt::CaseInsensitive))
        act->setChecked(true);
}

void LaunchMenu::initMsaaAction(const QString& action)
{
    QAction* act = m_msaaGroup.addAction(action);
    connect(act, SIGNAL(triggered()), this, SLOT(msaaTriggered()));
    act->setCheckable(true);
    if (!action.compare(QString::number(m_commons.getSamples()), Qt::CaseInsensitive))
        act->setChecked(true);
}

void LaunchMenu::initAnisoAction(const QString& action)
{
    QAction* act = m_anisoGroup.addAction(action);
    connect(act, SIGNAL(triggered()), this, SLOT(anisoTriggered()));
    act->setCheckable(true);
    if (!action.compare(QString::number(m_commons.getAnisotropy()), Qt::CaseInsensitive))
        act->setChecked(true);
}

void LaunchMenu::apiTriggered()
{
    m_commons.setGraphicsApi(qobject_cast<QAction*>(sender())->text().toStdString());
    m_commons.serialize();
}

void LaunchMenu::msaaTriggered()
{
    m_commons.setSamples(qobject_cast<QAction*>(sender())->text().toInt());
    m_commons.serialize();
}

void LaunchMenu::anisoTriggered()
{
    m_commons.setAnisotropy(qobject_cast<QAction*>(sender())->text().toInt());
    m_commons.serialize();
}
