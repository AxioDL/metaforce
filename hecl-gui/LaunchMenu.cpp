#include "LaunchMenu.hpp"
#include "hecl/CVarCommons.hpp"
#include "ArgumentEditor.hpp"
#include <QList>

extern hecl::CVar* hecl::com_developer;

LaunchMenu::LaunchMenu(hecl::CVarCommons& commons, QWidget* parent)
: QMenu(tr("Launch Menu"), parent)
, m_commons(commons)
, m_apiMenu(tr("Graphics API"), this)
, m_msaaMenu(tr("Anti-Aliasing"), this)
, m_anisoMenu(tr("Anisotropic Filtering"), this)
, m_apiGroup(this)
, m_msaaGroup(this)
, m_anisoGroup(this) {
  setToolTipsVisible(true);
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
  addMenu(&m_apiMenu)->setToolTip(QString::fromUtf8(m_commons.m_graphicsApi->rawHelp().data()));
  addMenu(&m_msaaMenu)->setToolTip(QString::fromUtf8(m_commons.m_drawSamples->rawHelp().data()));
  addMenu(&m_anisoMenu)->setToolTip(QString::fromUtf8(m_commons.m_texAnisotropy->rawHelp().data()));
  const QAction* argumentEditor = addAction(tr("Edit Runtime Arguments"));
  connect(argumentEditor, &QAction::triggered, this, &LaunchMenu::editRuntimeArgs);
  initDeepColor();
  initDeveloperMode();
  initCheats();
}

LaunchMenu::~LaunchMenu() = default;

void LaunchMenu::initApiAction(const QString& action) {
  QAction* act = m_apiGroup.addAction(action);
  connect(act, &QAction::triggered, this, &LaunchMenu::apiTriggered);
  act->setCheckable(true);
  if (!action.compare(QString::fromStdString(m_commons.getGraphicsApi()), Qt::CaseInsensitive))
    act->setChecked(true);
}

void LaunchMenu::initMsaaAction(const QString& action) {
  QAction* act = m_msaaGroup.addAction(action);
  connect(act, &QAction::triggered, this, &LaunchMenu::msaaTriggered);
  act->setCheckable(true);
  if (!action.compare(QString::number(m_commons.getSamples()), Qt::CaseInsensitive))
    act->setChecked(true);
}

void LaunchMenu::initAnisoAction(const QString& action) {
  QAction* act = m_anisoGroup.addAction(action);
  connect(act, &QAction::triggered, this, &LaunchMenu::anisoTriggered);
  act->setCheckable(true);
  if (!action.compare(QString::number(m_commons.getAnisotropy()), Qt::CaseInsensitive))
    act->setChecked(true);
}

void LaunchMenu::initDeepColor() {
  QAction* act = addAction(tr("Deep Color"));
  act->setToolTip(QString::fromUtf8(m_commons.m_deepColor->rawHelp().data()));
  act->setCheckable(true);
  act->setChecked(m_commons.getDeepColor());
  connect(act, &QAction::triggered, this, &LaunchMenu::deepColorTriggered);
}

void LaunchMenu::initDeveloperMode() {
  m_developerMode = addAction(tr("Developer Mode"));
  m_developerMode->setToolTip(QString::fromUtf8(hecl::com_developer->rawHelp().data()));
  m_developerMode->setCheckable(true);
  m_developerMode->setChecked(hecl::com_developer->toBoolean());
  connect(m_developerMode, &QAction::triggered, this, &LaunchMenu::developerModeTriggered);
}

void LaunchMenu::initCheats() {
  m_enableCheats = addAction(tr("Enable Cheats"));
  m_enableCheats->setToolTip(QString::fromUtf8(hecl::com_enableCheats->rawHelp().data()));
  m_enableCheats->setCheckable(true);
  m_enableCheats->setChecked(hecl::com_enableCheats->toBoolean());
  connect(m_enableCheats, &QAction::triggered, this, &LaunchMenu::cheatsTriggered);
}

void LaunchMenu::apiTriggered() {
  QString apiStr = qobject_cast<QAction*>(sender())->text();
  apiStr = apiStr.remove(QLatin1Char{'&'});
  m_commons.setGraphicsApi(apiStr.toStdString());
  m_commons.serialize();
}

void LaunchMenu::msaaTriggered() {
  m_commons.setSamples(qobject_cast<QAction*>(sender())->text().toUInt());
  m_commons.serialize();
}

void LaunchMenu::anisoTriggered() {
  m_commons.setAnisotropy(qobject_cast<QAction*>(sender())->text().toUInt());
  m_commons.serialize();
}

void LaunchMenu::deepColorTriggered() {
  m_commons.setDeepColor(qobject_cast<QAction*>(sender())->isChecked());
  m_commons.serialize();
}

void LaunchMenu::developerModeTriggered() {
  const bool isChecked = qobject_cast<QAction*>(sender())->isChecked();

  if (hecl::com_enableCheats->toBoolean() && !isChecked) {
    m_enableCheats->setChecked(false);
  }

  hecl::CVarManager::instance()->setDeveloperMode(isChecked, true);
  m_commons.serialize();
}

void LaunchMenu::cheatsTriggered() {
  const bool isChecked = qobject_cast<QAction*>(sender())->isChecked();

  if (!hecl::com_developer->toBoolean() && isChecked) {
    m_developerMode->setChecked(false);
  }

  hecl::CVarManager::instance()->setCheatsEnabled(isChecked, true);
  m_commons.serialize();
}

void LaunchMenu::editRuntimeArgs() {
  ArgumentEditor editor(this);
  editor.exec();
}
