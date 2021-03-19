#include "MainWindow.hpp"
#include "ui_MainWindow.h"
#include "LayerDialog.hpp"

#include <QFontDatabase>
#include <QMessageBox>
#include <QComboBox>
#include <QLabel>
#include <QTreeView>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtCore5Compat>
#endif
#include "EscapeSequenceParser.hpp"
#include "FileDirDialog.hpp"
#include "ExtractZip.hpp"

#if _WIN32
#include <Windows.h>
#include <shellapi.h>
#include <TlHelp32.h>

static void KillProcessTree(QProcess& proc) {
  quint64 pid = proc.processId();
  if (pid == 0) {
    return;
  }

  PROCESSENTRY32 pe = {};
  pe.dwSize = sizeof(PROCESSENTRY32);

  HANDLE hSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

  if (::Process32First(hSnap, &pe) == TRUE) {
    BOOL bContinue = TRUE;

    // kill child processes
    while (bContinue != FALSE) {
      // only kill child processes
      if (pe.th32ParentProcessID == pid) {
        HANDLE hChildProc = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID);

        if (hChildProc) {
          ::TerminateProcess(hChildProc, 1);
          ::CloseHandle(hChildProc);
        }
      }

      bContinue = ::Process32Next(hSnap, &pe);
    }
  }

  proc.close();
  proc.terminate();
}
#else
static void KillProcessTree(QProcess& proc) {
  proc.close();
  proc.terminate();
}
#endif

const QStringList MainWindow::skUpdateTracks = {QStringLiteral("stable"), QStringLiteral("dev"), QStringLiteral("continuous")};

MainWindow::MainWindow(QWidget* parent)
: QMainWindow(parent)
, m_ui(std::make_unique<Ui::MainWindow>())
, m_fileMgr(_SYS_STR("urde"))
, m_cvarManager(m_fileMgr)
, m_cvarCommons(m_cvarManager)
, m_heclProc(this)
, m_dlManager(this) {
  if (m_settings.value(QStringLiteral("urde_arguments")).isNull()) {
    m_settings.setValue(QStringLiteral("urde_arguments"), QStringList{QStringLiteral("--no-shader-warmup")});
  }
  if (m_settings.value(QStringLiteral("update_track")).isNull()) {
    m_settings.setValue(QStringLiteral("update_track"), QStringLiteral("dev"));
  }

  m_ui->setupUi(this);
  m_ui->heclTabs->setCurrentIndex(0);

  m_ui->aboutIcon->setPixmap(QApplication::windowIcon().pixmap(256, 256));

  QFont mFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  mFont.setPointSize(m_ui->currentBinaryLabel->font().pointSize());
  m_ui->currentBinaryLabel->setFont(mFont);
  m_ui->recommendedBinaryLabel->setFont(mFont);
  mFont.setPointSize(10);
  m_ui->processOutput->setFont(mFont);
  m_cursor = QTextCursor(m_ui->processOutput->document());
  connect(m_ui->saveLogButton, &QPushButton::pressed, this, [this] {
    QString defaultFileName = QStringLiteral("urde-") + QDateTime::currentDateTime().toString(Qt::DateFormat::ISODate) +
                              QStringLiteral(".log");
    defaultFileName.replace(QLatin1Char(':'), QLatin1Char('-'));
    const QString fileName =
        QFileDialog::getSaveFileName(this, tr("Save Log"), defaultFileName, QStringLiteral("*.log"));
    if (fileName.isEmpty()) {
      return;
    }
    QFile file = QFile(fileName);
    if (file.open(QFile::OpenModeFlag::WriteOnly | QFile::OpenModeFlag::Truncate | QFile::OpenModeFlag::Text)) {
      QTextStream stream(&file);
      stream << m_ui->processOutput->toPlainText();
      stream.flush();
      file.close();
    } else {
      QMessageBox::critical(this, tr("Save Log"), tr("Failed to open log file"));
    }
  });

//  m_updateURDEButton = new QPushButton(tr("Update URDE"), m_ui->centralwidget);
//  m_ui->gridLayout->addWidget(m_updateURDEButton, 2, 3, 1, 1);
//  m_updateURDEButton->hide();
//  QPalette pal = m_updateURDEButton->palette();
//  pal.setColor(QPalette::Button, QColor(53, 53, 72));
//  m_updateURDEButton->setPalette(pal);
//  connect(m_updateURDEButton, &QPushButton::clicked, this, &MainWindow::onUpdateURDEPressed);
  qDebug() << "Stored track " << m_settings.value(QStringLiteral("update_track"));
  const int index = skUpdateTracks.indexOf(m_settings.value(QStringLiteral("update_track")).toString());
  m_ui->devTrackWarning->setVisible(index == 1);
  m_ui->continuousTrackWarning->setVisible(index == 2);
  m_ui->updateTrackComboBox->setCurrentIndex(index);
  connect(m_ui->updateTrackComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this,
          &MainWindow::onUpdateTrackChanged);

  m_dlManager.connectWidgets(m_ui->downloadProgressBar, m_ui->downloadErrorLabel,
                             std::bind(&MainWindow::onIndexDownloaded, this, std::placeholders::_1),
                             std::bind(&MainWindow::onBinaryDownloaded, this, std::placeholders::_1),
                             std::bind(&MainWindow::onBinaryFailed, this));
#if !PLATFORM_ZIP_DOWNLOAD
  m_ui->downloadProgressBar->hide();
#endif

  initOptions();
  initSlots();

  m_dlManager.fetchIndex();

  setPath(m_settings.value(QStringLiteral("working_dir")).toString());
  resize(1024, 768);
}

MainWindow::~MainWindow() { KillProcessTree(m_heclProc); }

void MainWindow::onExtract() {
  if (m_path.isEmpty()) {
    return;
  }

  const QString imgPath =
      QFileDialog::getOpenFileName(this, tr("Extract Image"), m_path, tr("Images (*.iso *.wbfs *.gcm)"));
  if (imgPath.isEmpty()) {
    return;
  }

  m_ui->processOutput->clear();
  KillProcessTree(m_heclProc);
  m_heclProc.setProcessChannelMode(QProcess::ProcessChannelMode::MergedChannels);
  m_heclProc.setWorkingDirectory(m_path);
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert(QStringLiteral("TERM"), QStringLiteral("xterm-color"));
  env.insert(QStringLiteral("ConEmuANSI"), QStringLiteral("ON"));
  m_heclProc.setProcessEnvironment(env);
  disconnect(&m_heclProc, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), nullptr, nullptr);
  connect(&m_heclProc, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, &MainWindow::onExtractFinished);

  const QStringList heclProcArguments{
      QStringLiteral("extract"), QStringLiteral("-y"), QStringLiteral("-g"), QStringLiteral("-o"), m_path, imgPath};
  m_heclProc.start(m_heclPath, heclProcArguments, QIODevice::ReadOnly | QIODevice::Unbuffered);

  m_ui->heclTabs->setCurrentIndex(0);

  disableOperations();
  m_ui->extractBtn->setText(tr("&Cancel"));
  m_ui->extractBtn->setEnabled(true);
  disconnect(m_ui->extractBtn, &QPushButton::clicked, nullptr, nullptr);
  connect(m_ui->extractBtn, &QPushButton::clicked, this, &MainWindow::doHECLTerminate);
}

void MainWindow::onExtractFinished(int returnCode, QProcess::ExitStatus) {
  m_cursor.movePosition(QTextCursor::End);
  m_cursor.insertBlock();
  disconnect(m_ui->extractBtn, &QPushButton::clicked, nullptr, nullptr);
  connect(m_ui->extractBtn, &QPushButton::clicked, this, &MainWindow::onExtract);
  checkDownloadedBinary();
}

void MainWindow::onPackage() {
  if (m_path.isEmpty())
    return;
  m_ui->processOutput->clear();
  KillProcessTree(m_heclProc);
  m_heclProc.setProcessChannelMode(QProcess::ProcessChannelMode::MergedChannels);
  m_heclProc.setWorkingDirectory(m_path);
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert(QStringLiteral("TERM"), QStringLiteral("xterm-color"));
  env.insert(QStringLiteral("ConEmuANSI"), QStringLiteral("ON"));
  m_heclProc.setProcessEnvironment(env);
  disconnect(&m_heclProc, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), nullptr, nullptr);
  connect(&m_heclProc, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, &MainWindow::onPackageFinished);

  const QStringList heclProcArguments{QStringLiteral("package"), QStringLiteral("MP1"), QStringLiteral("-y"),
                                      QStringLiteral("-g")};
  m_heclProc.start(m_heclPath, heclProcArguments, QIODevice::ReadOnly | QIODevice::Unbuffered);

  m_ui->heclTabs->setCurrentIndex(0);

  disableOperations();
  m_ui->packageBtn->setText(tr("&Cancel"));
  m_ui->packageBtn->setEnabled(true);
  disconnect(m_ui->packageBtn, &QPushButton::clicked, nullptr, nullptr);
  connect(m_ui->packageBtn, &QPushButton::clicked, this, &MainWindow::doHECLTerminate);

  QSize size = QWidget::size();
  if (size.width() < 1100)
    size.setWidth(1100);
  resize(size);
}

void MainWindow::onPackageFinished(int returnCode, QProcess::ExitStatus) {
  m_cursor.movePosition(QTextCursor::End);
  m_cursor.insertBlock();
  disconnect(m_ui->packageBtn, &QPushButton::clicked, nullptr, nullptr);
  connect(m_ui->packageBtn, &QPushButton::clicked, this, &MainWindow::onPackage);
  checkDownloadedBinary();
}

void MainWindow::onLaunch() {
  if (m_path.isEmpty())
    return;
  m_ui->processOutput->clear();
  KillProcessTree(m_heclProc);
  m_heclProc.setProcessChannelMode(QProcess::ProcessChannelMode::MergedChannels);
  m_heclProc.setWorkingDirectory(m_path);
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert(QStringLiteral("TERM"), QStringLiteral("xterm-color"));
  env.insert(QStringLiteral("ConEmuANSI"), QStringLiteral("ON"));
  m_heclProc.setProcessEnvironment(env);
  disconnect(&m_heclProc, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), nullptr, nullptr);
  connect(&m_heclProc, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, &MainWindow::onLaunchFinished);

  const auto heclProcArguments = QStringList{m_path + QStringLiteral("/out/MP1")}
                                 << m_warpSettings << QStringLiteral("-l")
                                 << m_settings.value(QStringLiteral("urde_arguments"))
                                        .toStringList()
                                        .join(QLatin1Char{' '})
                                        .split(QLatin1Char{' '});
  m_heclProc.start(m_urdePath, heclProcArguments, QIODevice::ReadOnly | QIODevice::Unbuffered);

  m_ui->heclTabs->setCurrentIndex(0);

  disableOperations();
}

void MainWindow::onLaunchFinished(int returnCode, QProcess::ExitStatus) {
  m_cursor.movePosition(QTextCursor::End);
  m_cursor.insertBlock();
  checkDownloadedBinary();
}

void MainWindow::doHECLTerminate() { KillProcessTree(m_heclProc); }

void MainWindow::onReturnPressed() {
  if (sender() == m_ui->pathEdit)
    setPath(m_ui->pathEdit->text());
}

void MainWindow::onIndexDownloaded(const QStringList& index) {
  int bestVersion = 0;
  m_ui->binaryComboBox->clear();
  for (const QString& str : index) {
    URDEVersion version(str);
    if (m_ui->sysReqTable->willRun(version))
      bestVersion = m_ui->binaryComboBox->count();
    m_ui->binaryComboBox->addItem(version.fileString(false), QVariant::fromValue(version));
  }
  m_ui->binaryComboBox->setCurrentIndex(bestVersion);
  m_recommendedVersion = m_ui->binaryComboBox->itemData(bestVersion).value<URDEVersion>();
  m_ui->recommendedBinaryLabel->setText(m_recommendedVersion.fileString(false));
  m_ui->binaryComboBox->setEnabled(true);

  if (!m_path.isEmpty()) {
    checkDownloadedBinary();
    m_ui->downloadButton->setEnabled(true);
  }
}

void MainWindow::onDownloadPressed() {
//  m_updateURDEButton->hide();
  QString filename = m_ui->binaryComboBox->currentData().value<URDEVersion>().fileString(true);
#if PLATFORM_ZIP_DOWNLOAD
  disableOperations();
  m_ui->downloadButton->setEnabled(false);
#endif
  m_dlManager.fetchBinary(filename, m_path + QLatin1Char{'/'} + filename);
}

//void MainWindow::onUpdateURDEPressed() {
//  m_ui->heclTabs->setCurrentIndex(2);
//  onDownloadPressed();
//}

void MainWindow::onBinaryDownloaded(QuaZip& file) {
  const bool err = !ExtractZip::extractDir(file, m_path);

  if (err) {
    m_ui->downloadErrorLabel->setText(tr("Error extracting zip"));
  } else {
    m_ui->downloadErrorLabel->setText(tr("Download successful"), true);
  }

  m_ui->downloadButton->setEnabled(true);
  checkDownloadedBinary();

  if (!err && m_ui->extractBtn->isEnabled()) {
    m_ui->downloadErrorLabel->setText(tr("Download successful - Press 'Extract' to continue."), true);
  }
  if (!err && !m_ui->sysReqTable->isBlenderVersionOk()) {
    m_ui->downloadErrorLabel->setText(
        tr("Blender 2.90 or greater must be installed. Please download via Steam or blender.org."));
  }
}

void MainWindow::onBinaryFailed() {
  m_ui->downloadButton->setEnabled(true);
  checkDownloadedBinary();
}

void MainWindow::disableOperations() {
  m_ui->extractBtn->setEnabled(false);
  m_ui->packageBtn->setEnabled(false);
  m_ui->launchBtn->setEnabled(false);
  m_ui->pathEdit->setEnabled(false);
  m_ui->browseBtn->setEnabled(false);
  m_ui->downloadButton->setEnabled(false);
  m_ui->warpBtn->setEnabled(false);
}

void MainWindow::enableOperations() {
  disableOperations();
  m_ui->pathEdit->setEnabled(true);
  m_ui->browseBtn->setEnabled(true);

  if (hecl::com_enableCheats->toBoolean()) {
    m_ui->warpBtn->show();
  } else {
    m_ui->warpBtn->hide();
  }

  if (m_path.isEmpty())
    return;

  m_ui->downloadButton->setEnabled(true);

  if (m_heclPath.isEmpty())
    return;

  m_ui->extractBtn->setText(tr("&Extract"));
  m_ui->packageBtn->setText(tr("&Package"));
  m_ui->launchBtn->setText(tr("&Launch"));

  m_ui->extractBtn->setEnabled(true);
  if (QFile::exists(m_path + QStringLiteral("/out/files/MP1/version.yaml"))) {
    m_ui->packageBtn->setEnabled(true);
    if (isPackageComplete()) {
      m_ui->launchBtn->setEnabled(true);
      if (hecl::com_enableCheats->toBoolean()) {
        m_ui->warpBtn->setEnabled(true);
      }
    }
  }

  if (!m_ui->sysReqTable->isBlenderVersionOk()) {
    insertContinueNote(tr("Blender 2.90 or greater must be installed. Please download via Steam or blender.org."));
  } else if (m_ui->launchBtn->isEnabled()) {
    insertContinueNote(tr("Package complete - Press 'Launch' to start URDE."));
  } else if (m_ui->packageBtn->isEnabled()) {
    insertContinueNote(tr("Extract complete - Press 'Package' to continue."));
  } else if (m_ui->extractBtn->isEnabled()) {
    insertContinueNote(tr("Press 'Extract' to begin."));
  }
}

bool MainWindow::isPackageComplete() const {
  return QFile::exists(m_path + QStringLiteral("/out/files/MP1/AudioGrp.upak")) &&
         QFile::exists(m_path + QStringLiteral("/out/files/MP1/GGuiSys.upak")) &&
         QFile::exists(m_path + QStringLiteral("/out/files/MP1/Metroid1.upak")) &&
         QFile::exists(m_path + QStringLiteral("/out/files/MP1/Metroid2.upak")) &&
         QFile::exists(m_path + QStringLiteral("/out/files/MP1/Metroid3.upak")) &&
         QFile::exists(m_path + QStringLiteral("/out/files/MP1/Metroid4.upak")) &&
         QFile::exists(m_path + QStringLiteral("/out/files/MP1/metroid5.upak")) &&
         QFile::exists(m_path + QStringLiteral("/out/files/MP1/Metroid6.upak")) &&
         QFile::exists(m_path + QStringLiteral("/out/files/MP1/Metroid7.upak")) &&
         QFile::exists(m_path + QStringLiteral("/out/files/MP1/Metroid8.upak")) &&
         QFile::exists(m_path + QStringLiteral("/out/files/MP1/MidiData.upak")) &&
         QFile::exists(m_path + QStringLiteral("/out/files/MP1/MiscData.upak")) &&
         QFile::exists(m_path + QStringLiteral("/out/files/MP1/NoARAM.upak")) &&
         QFile::exists(m_path + QStringLiteral("/out/files/MP1/SamGunFx.upak")) &&
         QFile::exists(m_path + QStringLiteral("/out/files/MP1/SamusGun.upak")) &&
         QFile::exists(m_path + QStringLiteral("/out/files/MP1/SlideShow.upak")) &&
         QFile::exists(m_path + QStringLiteral("/out/files/MP1/TestAnim.upak")) &&
         QFile::exists(m_path + QStringLiteral("/out/files/MP1/Tweaks.upak")) &&
         QFile::exists(m_path + QStringLiteral("/out/files/MP1/URDE.upak"));
}

static bool GetDLPackage(const QString& path, QString& dlPackage) {
  QProcess proc;
  proc.start(path, {QStringLiteral("--dlpackage")}, QIODevice::ReadOnly);
  if (proc.waitForStarted()) {
    proc.waitForFinished();
    if (proc.exitCode() == 100)
      dlPackage = QString::fromUtf8(proc.readLine()).trimmed();
    return true;
  }
  return false;
}

bool MainWindow::checkDownloadedBinary() {
//  m_updateURDEButton->hide();

  m_urdePath = QString();
  m_heclPath = QString();

  if (m_path.isEmpty()) {
    m_ui->heclTabs->setCurrentIndex(2);
    m_ui->downloadErrorLabel->setText(tr("Set working directory to continue."), true);
    enableOperations();
    return false;
  }

  const QString dir = QApplication::instance()->applicationDirPath();
#if _WIN32
  QString urdePath = dir + QStringLiteral("/urde.exe");
  QString heclPath = dir + QStringLiteral("/hecl.exe");
  QString visigenPath = dir + QStringLiteral("/visigen.exe");
  if (!QFileInfo::exists(urdePath) || !QFileInfo::exists(heclPath) || !QFileInfo::exists(visigenPath)) {
    urdePath = m_path + QStringLiteral("/urde.exe");
    heclPath = m_path + QStringLiteral("/hecl.exe");
    visigenPath = m_path + QStringLiteral("/visigen.exe");
  }
#else
  QString urdePath = dir + QStringLiteral("/urde");
  QString heclPath = dir + QStringLiteral("/hecl");
  QString visigenPath = dir + QStringLiteral("/visigen");
#endif
  urdePath = QFileInfo(urdePath).absoluteFilePath();
  heclPath = QFileInfo(heclPath).absoluteFilePath();
  visigenPath = QFileInfo(visigenPath).absoluteFilePath();

  QString urdeDlPackage, heclDlPackage, visigenDlPackage;
  if (GetDLPackage(urdePath, urdeDlPackage) && GetDLPackage(heclPath, heclDlPackage) &&
      GetDLPackage(visigenPath, visigenDlPackage)) {
    if (!urdeDlPackage.isEmpty() && urdeDlPackage == heclDlPackage && urdeDlPackage == visigenDlPackage) {
      URDEVersion v(urdeDlPackage);
      m_ui->currentBinaryLabel->setText(v.fileString(false));
//      if (m_recommendedVersion.isValid() && v.isValid() && m_recommendedVersion.getVersion() > v.getVersion()) {
//        m_updateURDEButton->show();
//      }
    } else {
      m_ui->currentBinaryLabel->setText(tr("unknown -- re-download recommended"));
    }

    m_urdePath = urdePath;
    m_heclPath = heclPath;
    m_ui->downloadErrorLabel->setText({}, true);
    enableOperations();
    return true;
  }

  m_ui->currentBinaryLabel->setText(tr("none"));
  m_ui->heclTabs->setCurrentIndex(2);
  m_ui->downloadErrorLabel->setText(tr("Press 'Download' to fetch latest URDE binary."), true);
  enableOperations();
  return false;
}

void MainWindow::setPath(const QString& path) {
  const QFileInfo finfo(path);

  QString usePath;
  if (!path.isEmpty()) {
    usePath = finfo.absoluteFilePath();
  }

  if (!usePath.isEmpty() && !finfo.exists()) {
    if (QMessageBox::question(this, tr("Make Directory"), tr("%1 does not exist. Create it now?").arg(usePath)) ==
        QMessageBox::Yes) {
      QDir().mkpath(usePath);
    } else {
      usePath = QString();
    }
  }

  if (!usePath.isEmpty() && !finfo.isDir()) {
    QMessageBox::warning(this, tr("Directory Error"), tr("%1 is not a directory").arg(usePath), QMessageBox::Ok,
                         QMessageBox::NoButton);
    usePath = QString();
  }

  m_path = usePath;
  m_settings.setValue(QStringLiteral("working_dir"), m_path);

  if (!m_path.isEmpty()) {
    m_ui->pathEdit->setText(m_path);
    m_ui->downloadButton->setToolTip(QString());
    m_ui->downloadButton->setEnabled(m_ui->binaryComboBox->isEnabled());
  } else {
    m_ui->downloadButton->setToolTip(tr("Working directory must be set"));
    m_ui->downloadButton->setEnabled(false);
    m_ui->currentBinaryLabel->setText(tr("none"));
  }

  m_ui->sysReqTable->updateFreeDiskSpace(m_path);
  checkDownloadedBinary();
}

void MainWindow::initSlots() {
  connect(&m_heclProc, &QProcess::readyRead, [this]() {
    const QByteArray bytes = m_heclProc.readAll();
    setTextTermFormatting(QString::fromUtf8(bytes));
  });

  connect(m_ui->extractBtn, &QPushButton::clicked, this, &MainWindow::onExtract);
  connect(m_ui->packageBtn, &QPushButton::clicked, this, &MainWindow::onPackage);
  connect(m_ui->launchBtn, &QPushButton::clicked, this, &MainWindow::onLaunch);

  connect(m_ui->browseBtn, &QPushButton::clicked, [this]() {
    FileDirDialog dialog(this);
    dialog.setDirectory(m_path);
    dialog.setWindowTitle(tr("Select Working Directory"));
    int res = dialog.exec();
    if (res == QFileDialog::Rejected)
      return;

    if (dialog.selectedFiles().size() <= 0)
      return;

    setPath(dialog.selectedFiles().at(0));
  });
  connect(m_ui->pathEdit, &QLineEdit::editingFinished, [this]() { setPath(m_ui->pathEdit->text()); });

  connect(m_ui->downloadButton, &QPushButton::clicked, this, &MainWindow::onDownloadPressed);
}

void MainWindow::setTextTermFormatting(const QString& text) {
  m_inContinueNote = false;

  m_cursor.beginEditBlock();
  // TODO: Migrate QRegExp to QRegularExpression
  QRegExp const escapeSequenceExpression(QStringLiteral(R"(\x1B\[([\d;\?FA]+)([mlh]?))"));
  QTextCharFormat defaultTextCharFormat = m_cursor.charFormat();
  int offset = escapeSequenceExpression.indexIn(text);
  ReturnInsert(m_cursor, text.mid(0, offset));
  QTextCharFormat textCharFormat = defaultTextCharFormat;
  while (offset >= 0) {
    int previousOffset = offset + escapeSequenceExpression.matchedLength();
    QStringList captures = escapeSequenceExpression.capturedTexts();
    if (captures.size() >= 3 && captures[2] == QStringLiteral("m")) {
      QStringList capturedTexts = captures[1].split(QLatin1Char{';'});
      QListIterator<QString> i(capturedTexts);
      while (i.hasNext()) {
        bool ok = false;
        int attribute = i.next().toInt(&ok);
        Q_ASSERT(ok);
        ParseEscapeSequence(attribute, i, textCharFormat, defaultTextCharFormat);
      }
    } else if (captures.size() >= 2 &&
               (captures[1].endsWith(QLatin1Char{'F'}) || captures[1].endsWith(QLatin1Char{'A'}))) {
      int lineCount = captures[1].chopped(1).toInt();
      if (!lineCount)
        lineCount = 1;
      for (int i = 0; i < lineCount; ++i) {
        m_cursor.movePosition(QTextCursor::PreviousBlock);
        m_cursor.select(QTextCursor::BlockUnderCursor);
        m_cursor.removeSelectedText();
        m_cursor.insertBlock();
      }
    }
    offset = escapeSequenceExpression.indexIn(text, previousOffset);
    if (offset < 0) {
      ReturnInsert(m_cursor, text.mid(previousOffset), textCharFormat);
    } else {
      ReturnInsert(m_cursor, text.mid(previousOffset, offset - previousOffset), textCharFormat);
    }
  }
  m_cursor.setCharFormat(defaultTextCharFormat);
  m_cursor.endEditBlock();
  m_ui->processOutput->ensureCursorVisible();
}

void MainWindow::insertContinueNote(const QString& text) {
  if (m_inContinueNote)
    return;
  m_inContinueNote = true;

  m_cursor.movePosition(QTextCursor::End);
  QTextCharFormat textCharFormat = m_cursor.charFormat();
  textCharFormat.setForeground(QColor(0, 255, 0));
  m_cursor.insertText(text, textCharFormat);
  m_cursor.insertBlock();
  m_ui->processOutput->ensureCursorVisible();
}

void MainWindow::onUpdateTrackChanged(int index) {
  qDebug() << "Track changed from " << m_settings.value(QStringLiteral("update_track")) << " to "
           << skUpdateTracks[index];
  m_settings.setValue(QStringLiteral("update_track"), skUpdateTracks[index]);
  m_dlManager.fetchIndex();
  m_ui->devTrackWarning->setVisible(index == 1);
  m_ui->continuousTrackWarning->setVisible(index == 2);
}

void MainWindow::initOptions() {
  initGraphicsApiOption(m_ui->metalOption, CurPlatform != Platform::MacOS, DEFAULT_GRAPHICS_API == "Metal"sv);
  initGraphicsApiOption(m_ui->vulkanOption, CurPlatform == Platform::MacOS, DEFAULT_GRAPHICS_API == "Vulkan"sv);
  initGraphicsApiOption(m_ui->d3d11Option, CurPlatform != Platform::Win32, DEFAULT_GRAPHICS_API == "D3D11"sv);
  initNumberComboOption(m_ui->anistropicFilteringBox, m_cvarCommons.m_texAnisotropy);
  initNumberComboOption(m_ui->antialiasingBox, m_cvarCommons.m_drawSamples);
  initCheckboxOption(m_ui->deepColor, m_cvarCommons.m_deepColor);

  m_ui->developerModeBox->setToolTip(QString::fromUtf8(hecl::com_developer->rawHelp().data()));
  m_ui->developerModeBox->setChecked(hecl::com_developer->toBoolean());
  m_ui->developerOptionsGroup->setVisible(hecl::com_developer->toBoolean());
  connect(m_ui->developerModeBox, &QCheckBox::stateChanged, this, [this](int state) {
    bool isChecked = state == Qt::Checked;
    if (hecl::com_enableCheats->toBoolean() && !isChecked) {
      m_ui->enableCheatsBox->setChecked(false);
      // m_ui->tweaksOptionsGroup->setVisible(false);
      m_ui->warpBtn->setVisible(false);
    }
    m_ui->developerOptionsGroup->setVisible(isChecked);
    hecl::CVarManager::instance()->setDeveloperMode(isChecked, true);
    m_cvarManager.serialize();
  });

  m_ui->enableCheatsBox->setToolTip(QString::fromUtf8(hecl::com_enableCheats->rawHelp().data()));
  m_ui->enableCheatsBox->setChecked(hecl::com_enableCheats->toBoolean());
  m_ui->tweaksOptionsGroup->setVisible(false); // hecl::com_enableCheats->toBoolean()
  m_ui->warpBtn->setVisible(hecl::com_enableCheats->toBoolean());
  connect(m_ui->enableCheatsBox, &QCheckBox::stateChanged, this, [this](int state) {
    bool isChecked = state == Qt::Checked;
    if (!hecl::com_developer->toBoolean() && isChecked) {
      m_ui->developerModeBox->setChecked(true);
      m_ui->developerOptionsGroup->setVisible(true);
    }
    // m_ui->tweaksOptionsGroup->setVisible(isChecked);
    m_ui->warpBtn->setVisible(isChecked);
    hecl::CVarManager::instance()->setCheatsEnabled(isChecked, true);
    m_cvarManager.serialize();
  });

  initCheckboxOption(m_ui->developerModeBox, hecl::com_developer);
  initCheckboxOption(m_ui->enableCheatsBox, hecl::com_enableCheats);
  initCheckboxOption(m_ui->variableDtBox, m_cvarCommons.m_variableDt);
  initCheckboxOption(m_ui->areaInfoOverlayBox, m_cvarCommons.m_debugOverlayAreaInfo);
  initCheckboxOption(m_ui->playerInfoOverlayBox, m_cvarCommons.m_debugOverlayPlayerInfo);
  initCheckboxOption(m_ui->worldInfoOverlayBox, m_cvarCommons.m_debugOverlayWorldInfo);
  initCheckboxOption(m_ui->frameCounterBox, m_cvarCommons.m_debugOverlayShowFrameCounter);
  initCheckboxOption(m_ui->inGameTimeBox, m_cvarCommons.m_debugOverlayShowInGameTime);
  initCheckboxOption(m_ui->resourceStatsOverlayBox, m_cvarCommons.m_debugOverlayShowResourceStats);
  initCheckboxOption(m_ui->drawLighting, m_cvarCommons.m_debugToolDrawLighting);
  initCheckboxOption(m_ui->drawAiPaths, m_cvarCommons.m_debugToolDrawAiPath);
  initCheckboxOption(m_ui->drawCollisionActors, m_cvarCommons.m_debugToolDrawCollisionActors);
  initCheckboxOption(m_ui->drawMazePath, m_cvarCommons.m_debugToolDrawMazePath);
  initCheckboxOption(m_ui->drawPlatformCollision, m_cvarCommons.m_debugToolDrawPlatformCollision);
  initCheckboxOption(m_ui->logScriptingBox,
                     // TODO centralize
                     hecl::CVarManager::instance()->findOrMakeCVar(
                         "stateManager.logScripting"sv, "Prints object communication to the console", false,
                         hecl::CVar::EFlags::ReadOnly | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::Game));
  initCheckboxOption(m_ui->skipSplashScreensBox,
                     // TODO centralize
                     hecl::CVarManager::instance()->findOrMakeCVar(
                         "tweak.game.SplashScreensDisabled"sv, "Skip splash screens on game startup", false,
                         hecl::CVar::EFlags::ReadOnly | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::Game));

  m_launchOptionsModel.setStringList(QSettings().value(QStringLiteral("urde_arguments")).toStringList());
  m_ui->launchOptionsList->setModel(&m_launchOptionsModel);

  connect(m_ui->launchOptionAddButton, &QPushButton::clicked, this, [this] {
    int row = m_launchOptionsModel.rowCount();
    if (m_launchOptionsModel.insertRow(row)) {
      QModelIndex index = m_launchOptionsModel.index(row);
      m_ui->launchOptionsList->selectionModel()->select(index, QItemSelectionModel::SelectionFlag::ClearAndSelect);
      m_ui->launchOptionsList->edit(index);
    }
  });
  connect(m_ui->launchOptionDeleteButton, &QPushButton::clicked, this, [this] {
    QItemSelectionModel* selection = m_ui->launchOptionsList->selectionModel();
    if (selection == nullptr) {
      return;
    }
    QModelIndexList list = selection->selectedRows();
    for (QModelIndex index : list) {
      m_launchOptionsModel.removeRow(index.row());
    }
  });
  connect(&m_launchOptionsModel, &QStringListModel::dataChanged, this,
          [this]() { QSettings().setValue(QStringLiteral("urde_arguments"), m_launchOptionsModel.stringList()); });
  connect(&m_launchOptionsModel, &QStringListModel::rowsRemoved, this,
          [this]() { QSettings().setValue(QStringLiteral("urde_arguments"), m_launchOptionsModel.stringList()); });
  connect(m_ui->warpBtn, &QPushButton::clicked, this, [this] {
    QFileInfo areaPath(
        QFileDialog::getOpenFileName(this, tr("Select area to warp to..."), m_path, QStringLiteral("*.blend")));
    if (!areaPath.exists() || areaPath.suffix() != QStringLiteral("blend") ||
        !areaPath.fileName().contains(QStringLiteral("!area_"))) {
      m_warpSettings.clear();
      return;
    }

    auto ret =
        QMessageBox::question(this, tr("Select enabled layers?"), tr("Do you want to only enable certain layers?"));

    QString layerBits;
    if (ret == QMessageBox::StandardButton::Yes) {
      QList<Layer> layers;
      QDirIterator iter(areaPath.absolutePath(), QDir::Filter::Dirs);
      while (iter.hasNext()) {
        QFileInfo f(iter.next());
        if (f.isDir()) {
          QDir dir(f.absoluteFilePath());
          if (dir.exists(QStringLiteral("!objects.yaml"))) {
            bool active = dir.exists(QStringLiteral("!defaultactive"));
            layers.push_back({f.baseName(), active});
          }
        }
      }
      std::sort(layers.begin(), layers.end(), [&](const auto& a, const auto& b) { return a.name < b.name; });

      LayerDialog layer(this);
      layer.createLayerCheckboxes(layers);
      auto code = layer.exec();
      if (code == QDialog::DialogCode::Accepted) {
        layerBits = layer.getLayerBits();
      }
    }
    QString directoryPath = areaPath.path();
#if _WIN32
    directoryPath.replace(QLatin1Char('/'), QDir::separator());
#else
    directoryPath.replace(QLatin1Char('\\'), QDir::separator());
#endif
    auto list = directoryPath.split(QDir::separator());
    std::string areaDirectory = list.last().toLower().toStdString();
    list.pop_back();
    list.pop_back();
    std::string worldDir = list.last().toLower().toStdString();
    quint32 worldIndex;
    std::sscanf(worldDir.c_str(), "metroid%i", &worldIndex);
    quint32 areaIndex;
    char areaName[2048];
    std::sscanf(areaDirectory.c_str(), "%2i%[^\n]", &areaIndex, areaName);
    if (layerBits.isEmpty()) {
      m_warpSettings = QStringLiteral("--warp %1 %2").arg(worldIndex).arg(areaIndex).split(QLatin1Char(' '));
    } else {
      m_warpSettings =
          QStringLiteral("--warp %1 %2 %3").arg(worldIndex).arg(areaIndex).arg(layerBits).split(QLatin1Char(' '));
    }
    onLaunch();
    m_warpSettings.clear();
  });
}

void MainWindow::initNumberComboOption(QComboBox* action, hecl::CVar* cvar) {
  QString itemString;
  for (int i = 0; !(itemString = action->itemText(i)).isEmpty(); ++i) {
    if (itemString.toInt() == cvar->toSigned()) {
      action->setCurrentIndex(i);
      break;
    }
  }
  action->setToolTip(QString::fromUtf8(cvar->rawHelp().data()));
  connect(action, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
          [this, action, cvar](const int i) {
            cvar->fromInteger(action->itemText(i).toInt());
            m_cvarManager.serialize();
          });
}

void MainWindow::initCheckboxOption(QCheckBox* action, hecl::CVar* cvar) {
  action->setToolTip(QString::fromUtf8(cvar->rawHelp().data()));
  action->setChecked(cvar->toBoolean());
  connect(action, &QCheckBox::stateChanged, this, [this, cvar](int state) {
    cvar->fromBoolean(state == Qt::Checked);
    m_cvarManager.serialize();
  });
}

void MainWindow::initGraphicsApiOption(QRadioButton* action, bool hidden, bool isDefault) {
  if (hidden) {
    action->hide();
    return;
  }
  const std::string& currApi = m_cvarCommons.getGraphicsApi();
  action->setChecked(action->text().compare(QString::fromUtf8(currApi.data()), Qt::CaseInsensitive) == 0 ||
                     (isDefault && currApi.empty()));
  connect(action, &QRadioButton::toggled, this, [this, action](bool checked) {
    if (checked) {
      m_cvarCommons.setGraphicsApi(action->text().toStdString());
      m_cvarCommons.serialize();
    }
  });
}
