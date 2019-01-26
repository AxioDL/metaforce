#include "MainWindow.hpp"
#include "ui_MainWindow.h"
#include <QFontDatabase>
#include <QMessageBox>
#include "EscapeSequenceParser.hpp"
#include "FileDirDialog.hpp"
#include "ExtractZip.hpp"

#if _WIN32
#include <Windows.h>
#include <shellapi.h>
#include <TlHelp32.h>

static void KillProcessTree(QProcess& proc) {
  Q_PID pid = proc.pid();
  if (!pid)
    return;

  DWORD myprocID = pid->dwProcessId;
  PROCESSENTRY32 pe = {};
  pe.dwSize = sizeof(PROCESSENTRY32);

  HANDLE hSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

  if (::Process32First(hSnap, &pe)) {
    BOOL bContinue = TRUE;

    // kill child processes
    while (bContinue) {
      // only kill child processes
      if (pe.th32ParentProcessID == myprocID) {
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

MainWindow::MainWindow(QWidget* parent)
: QMainWindow(parent)
, m_fileMgr(_SYS_STR("urde"))
, m_cvarManager(m_fileMgr)
, m_cvarCommons(m_cvarManager)
, m_ui(new Ui::MainWindow)
, m_heclProc(this)
, m_dlManager(this)
, m_launchMenu(m_cvarCommons, this) {
  if (m_settings.value("urde_arguments").isNull())
    m_settings.setValue("urde_arguments", QStringList() << "--no-shader-warmup");

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

  m_updateURDEButton = new QPushButton(QStringLiteral("Update URDE"), m_ui->centralwidget);
  m_ui->gridLayout->addWidget(m_updateURDEButton, 2, 3, 1, 1);
  m_updateURDEButton->hide();
  QPalette pal = m_updateURDEButton->palette();
  pal.setColor(QPalette::Button, QColor(53, 53, 72));
  m_updateURDEButton->setPalette(pal);
  connect(m_updateURDEButton, SIGNAL(clicked()), this, SLOT(onUpdateURDEPressed()));

  m_dlManager.connectWidgets(m_ui->downloadProgressBar, m_ui->downloadErrorLabel,
                             std::bind(&MainWindow::onIndexDownloaded, this, std::placeholders::_1),
                             std::bind(&MainWindow::onBinaryDownloaded, this, std::placeholders::_1),
                             std::bind(&MainWindow::onBinaryFailed, this));

  initSlots();

  m_dlManager.fetchIndex();

  setPath(m_settings.value(QStringLiteral("working_dir")).toString());
}

MainWindow::~MainWindow() {
  KillProcessTree(m_heclProc);
  delete m_ui;
}

void MainWindow::onExtract() {
  if (m_path.isEmpty())
    return;
  QString imgPath = QFileDialog::getOpenFileName(this, QStringLiteral("Extract Image"), m_path,
                                                 QStringLiteral("Images (*.iso *.wbfs *.gcm)"));
  if (imgPath.isEmpty())
    return;

  m_ui->processOutput->clear();
  KillProcessTree(m_heclProc);
  m_heclProc.setProcessChannelMode(QProcess::ProcessChannelMode::MergedChannels);
  m_heclProc.setWorkingDirectory(m_path);
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("TERM", "xterm-color");
  env.insert("ConEmuANSI", "ON");
  m_heclProc.setProcessEnvironment(env);
  disconnect(&m_heclProc, SIGNAL(finished(int)), nullptr, nullptr);
  connect(&m_heclProc, SIGNAL(finished(int)), this, SLOT(onExtractFinished(int)));
  m_heclProc.start(m_heclPath, {"extract", "-y", "-g", "-o", m_path, imgPath},
                   QIODevice::ReadOnly | QIODevice::Unbuffered);

  m_ui->heclTabs->setCurrentIndex(0);

  disableOperations();
  m_ui->extractBtn->setText(QStringLiteral("Cancel"));
  m_ui->extractBtn->setEnabled(true);
  disconnect(m_ui->extractBtn, SIGNAL(clicked()), nullptr, nullptr);
  connect(m_ui->extractBtn, SIGNAL(clicked()), this, SLOT(doHECLTerminate()));
}

void MainWindow::onExtractFinished(int returnCode) {
  m_cursor.movePosition(QTextCursor::End);
  m_cursor.insertBlock();
  disconnect(m_ui->extractBtn, SIGNAL(clicked()), nullptr, nullptr);
  connect(m_ui->extractBtn, SIGNAL(clicked()), this, SLOT(onExtract()));
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
  env.insert("TERM", "xterm-color");
  env.insert("ConEmuANSI", "ON");
  m_heclProc.setProcessEnvironment(env);
  disconnect(&m_heclProc, SIGNAL(finished(int)), nullptr, nullptr);
  connect(&m_heclProc, SIGNAL(finished(int)), this, SLOT(onPackageFinished(int)));
  m_heclProc.start(m_heclPath, {"package", "-y", "-g"}, QIODevice::ReadOnly | QIODevice::Unbuffered);

  m_ui->heclTabs->setCurrentIndex(0);

  disableOperations();
  m_ui->packageBtn->setText(QStringLiteral("Cancel"));
  m_ui->packageBtn->setEnabled(true);
  disconnect(m_ui->packageBtn, SIGNAL(clicked()), nullptr, nullptr);
  connect(m_ui->packageBtn, SIGNAL(clicked()), this, SLOT(doHECLTerminate()));

  QSize size = QWidget::size();
  if (size.width() < 1100)
    size.setWidth(1100);
  resize(size);
}

void MainWindow::onPackageFinished(int returnCode) {
  m_cursor.movePosition(QTextCursor::End);
  m_cursor.insertBlock();
  disconnect(m_ui->packageBtn, SIGNAL(clicked()), nullptr, nullptr);
  connect(m_ui->packageBtn, SIGNAL(clicked()), this, SLOT(onPackage()));
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
  env.insert("TERM", "xterm-color");
  env.insert("ConEmuANSI", "ON");
  m_heclProc.setProcessEnvironment(env);
  disconnect(&m_heclProc, SIGNAL(finished(int)), nullptr, nullptr);
  connect(&m_heclProc, SIGNAL(finished(int)), this, SLOT(onLaunchFinished(int)));
  m_heclProc.start(m_urdePath, QStringList() << (m_path + "/out")
                                             << m_settings.value("urde_arguments").toStringList().join(' ').split(' '),
                   QIODevice::ReadOnly | QIODevice::Unbuffered);

  m_ui->heclTabs->setCurrentIndex(0);

  disableOperations();
}

void MainWindow::onLaunchFinished(int returnCode) {
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
  m_updateURDEButton->hide();
  QString filename = m_ui->binaryComboBox->currentData().value<URDEVersion>().fileString(true);
  disableOperations();
  m_ui->downloadButton->setEnabled(false);
  m_dlManager.fetchBinary(filename, m_path + '/' + filename);
}

void MainWindow::onUpdateURDEPressed() {
  m_ui->heclTabs->setCurrentIndex(1);
  onDownloadPressed();
}

void MainWindow::onBinaryDownloaded(QuaZip& file) {
  bool err = !ExtractZip::extractDir(file, m_path);

  if (err)
    m_ui->downloadErrorLabel->setText(QStringLiteral("Error extracting zip"));
  else
    m_ui->downloadErrorLabel->setText(QStringLiteral("Download successful"), true);

  m_ui->downloadButton->setEnabled(true);
  checkDownloadedBinary();

  if (!err && m_ui->extractBtn->isEnabled())
    m_ui->downloadErrorLabel->setText(QStringLiteral("Download successful - Press 'Extract' to continue."), true);
  if (!err && !m_ui->sysReqTable->isBlenderVersionOk())
    m_ui->downloadErrorLabel->setText(
        QStringLiteral("Blender 2.78+ must be installed. Please download via Steam or blender.org."));
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
}

void MainWindow::enableOperations() {
  disableOperations();
  m_ui->pathEdit->setEnabled(true);
  m_ui->browseBtn->setEnabled(true);

  if (m_path.isEmpty())
    return;

  m_ui->downloadButton->setEnabled(true);

  if (m_heclPath.isEmpty())
    return;

  m_ui->extractBtn->setText(QStringLiteral("Extract"));
  m_ui->packageBtn->setText(QStringLiteral("Package"));
  m_ui->launchBtn->setText(QStringLiteral("Launch"));

  m_ui->extractBtn->setEnabled(true);
  if (QFile::exists(m_path + "/MP1/!original_ids.yaml")) {
    m_ui->packageBtn->setEnabled(true);
    if (isPackageComplete())
      m_ui->launchBtn->setEnabled(true);
  }

  if (!m_ui->sysReqTable->isBlenderVersionOk())
    insertContinueNote("Blender 2.78+ must be installed. Please download via Steam or blender.org.");
  else if (m_ui->launchBtn->isEnabled())
    insertContinueNote("Package complete - Press 'Launch' to start URDE.");
  else if (m_ui->packageBtn->isEnabled())
    insertContinueNote("Extract complete - Press 'Package' to continue.");
  else if (m_ui->extractBtn->isEnabled())
    insertContinueNote("Press 'Extract' to begin.");
}

bool MainWindow::isPackageComplete() const {
  return
#if RUNTIME_ORIGINAL_IDS
      QFile::exists(m_path + "/out/files/!original_ids.upak") &&
#endif
      QFile::exists(m_path + "/out/files/AudioGrp.upak") && QFile::exists(m_path + "/out/files/GGuiSys.upak") &&
      QFile::exists(m_path + "/out/files/Metroid1.upak") && QFile::exists(m_path + "/out/files/Metroid2.upak") &&
      QFile::exists(m_path + "/out/files/Metroid3.upak") && QFile::exists(m_path + "/out/files/Metroid4.upak") &&
      QFile::exists(m_path + "/out/files/metroid5.upak") && QFile::exists(m_path + "/out/files/Metroid6.upak") &&
      QFile::exists(m_path + "/out/files/Metroid7.upak") && QFile::exists(m_path + "/out/files/Metroid8.upak") &&
      QFile::exists(m_path + "/out/files/MidiData.upak") && QFile::exists(m_path + "/out/files/MiscData.upak") &&
      QFile::exists(m_path + "/out/files/NoARAM.upak") && QFile::exists(m_path + "/out/files/SamGunFx.upak") &&
      QFile::exists(m_path + "/out/files/SamusGun.upak") && QFile::exists(m_path + "/out/files/SlideShow.upak") &&
      QFile::exists(m_path + "/out/files/TestAnim.upak") && QFile::exists(m_path + "/out/files/Tweaks.upak");
}

static bool GetDLPackage(const QString& path, QString& dlPackage) {
  QProcess proc;
  proc.start(path, {"--dlpackage"}, QIODevice::ReadOnly);
  if (proc.waitForStarted()) {
    proc.waitForFinished();
    if (proc.exitCode() == 100)
      dlPackage = QString::fromUtf8(proc.readLine()).trimmed();
    return true;
  }
  return false;
}

bool MainWindow::checkDownloadedBinary() {
  m_updateURDEButton->hide();

  m_urdePath = QString();
  m_heclPath = QString();

  if (m_path.isEmpty()) {
    m_ui->heclTabs->setCurrentIndex(1);
    m_ui->downloadErrorLabel->setText(QStringLiteral("Set working directory to continue."), true);
    enableOperations();
    return false;
  }

#if __APPLE__
  QString urdePath = m_path + "/URDE.app/Contents/MacOS/urde";
  QString heclPath = m_path + "/URDE.app/Contents/MacOS/hecl";
  QString visigenPath = m_path + "/URDE.app/Contents/MacOS/visigen";
#elif _WIN32
  QString urdePath = m_path + "/urde.exe";
  QString heclPath = m_path + "/hecl.exe";
  QString visigenPath = m_path + "/visigen.exe";
#else
  QString urdePath = m_path + "/urde";
  QString heclPath = m_path + "/hecl";
  QString visigenPath = m_path + "/visigen";
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
      if (m_recommendedVersion.isValid() && v.isValid() && m_recommendedVersion.getVersion() > v.getVersion()) {
        m_updateURDEButton->show();
      }
    } else {
      m_ui->currentBinaryLabel->setText(QStringLiteral("unknown -- re-download recommended"));
    }

    m_urdePath = urdePath;
    m_heclPath = heclPath;
    m_ui->downloadErrorLabel->setText({}, true);
    enableOperations();
    return true;
  } else {
    m_ui->currentBinaryLabel->setText(QStringLiteral("none"));
    m_ui->heclTabs->setCurrentIndex(1);
    m_ui->downloadErrorLabel->setText(QStringLiteral("Press 'Download' to fetch latest URDE binary."), true);
    enableOperations();
  }

  return false;
}

void MainWindow::setPath(const QString& path) {
  QFileInfo finfo(path);
  QString usePath;
  if (!path.isEmpty())
    usePath = finfo.absoluteFilePath();
  if (!usePath.isEmpty() && !finfo.exists()) {
    if (QMessageBox::question(this, QStringLiteral("Make Directory"),
                              QStringLiteral("%1 does not exist. Create it now?").arg(usePath)) == QMessageBox::Yes)
      QDir().mkpath(usePath);
    else
      usePath = QString();
  }

  if (!usePath.isEmpty() && !finfo.isDir()) {
    QMessageBox::warning(this, QStringLiteral("Directory Error"), QStringLiteral("%1 is not a directory").arg(usePath),
                         QMessageBox::Ok, QMessageBox::NoButton);
    usePath = QString();
  }

  m_path = usePath;
  m_settings.setValue(QStringLiteral("working_dir"), m_path);

  if (!m_path.isEmpty()) {
    m_ui->pathEdit->setText(m_path);
    m_ui->downloadButton->setToolTip(QString());
    m_ui->downloadButton->setEnabled(m_ui->binaryComboBox->isEnabled());
  } else {
    m_ui->downloadButton->setToolTip(QStringLiteral("Working directory must be set"));
    m_ui->downloadButton->setEnabled(false);
    m_ui->currentBinaryLabel->setText(QStringLiteral("none"));
  }

  m_ui->sysReqTable->updateFreeDiskSpace(m_path);
  checkDownloadedBinary();
}

void MainWindow::initSlots() {
  connect(&m_heclProc, &QProcess::readyRead, [=]() {
    QByteArray bytes = m_heclProc.readAll();
    setTextTermFormatting(bytes);
  });

  connect(m_ui->extractBtn, SIGNAL(clicked()), this, SLOT(onExtract()));
  connect(m_ui->packageBtn, SIGNAL(clicked()), this, SLOT(onPackage()));
  connect(m_ui->launchBtn, SIGNAL(clicked()), this, SLOT(onLaunch()));
  m_ui->launchMenuBtn->setMenu(&m_launchMenu);

  connect(m_ui->browseBtn, &QPushButton::clicked, [=]() {
    FileDirDialog dialog(this);
    dialog.setDirectory(m_path);
    dialog.setWindowTitle("Select Working Directory");
    int res = dialog.exec();
    if (res == QFileDialog::Rejected)
      return;

    if (dialog.selectedFiles().size() <= 0)
      return;

    setPath(dialog.selectedFiles().at(0));
  });

  connect(m_ui->downloadButton, SIGNAL(clicked()), this, SLOT(onDownloadPressed()));
}

void MainWindow::setTextTermFormatting(const QString& text) {
  m_inContinueNote = false;

  QRegExp const escapeSequenceExpression(R"(\x1B\[([\d;\?FA]+)([mlh]?))");
  QTextCharFormat defaultTextCharFormat = m_cursor.charFormat();
  int offset = escapeSequenceExpression.indexIn(text);
  ReturnInsert(m_cursor, text.mid(0, offset));
  QTextCharFormat textCharFormat = defaultTextCharFormat;
  while (offset >= 0) {
    int previousOffset = offset + escapeSequenceExpression.matchedLength();
    QStringList captures = escapeSequenceExpression.capturedTexts();
    if (captures.size() >= 3 && captures[2] == "m") {
      QStringList capturedTexts = captures[1].split(';');
      QListIterator<QString> i(capturedTexts);
      while (i.hasNext()) {
        bool ok = false;
        int attribute = i.next().toInt(&ok);
        Q_ASSERT(ok);
        ParseEscapeSequence(attribute, i, textCharFormat, defaultTextCharFormat);
      }
    } else if (captures.size() >= 2 && (captures[1].endsWith('F') || captures[1].endsWith('A'))) {
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
