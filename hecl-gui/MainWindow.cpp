#include "MainWindow.hpp"
#include "ui_MainWindow.h"
#include <QFontDatabase>
#include <QProcess>
#include <QScrollBar>
#include <QFileInfo>
#include <QDebug>
#include <QTextCursor>
#include "FileDirDialog.hpp"

#if _WIN32
#include <Windows.h>
#include <shellapi.h>
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow)
  , m_heclProc(this)
  , m_dlManager(this)
  , m_settings("AxioDL", "HECL", this)
{
    m_ui->setupUi(this);
    m_ui->heclTabs->setCurrentIndex(0);
    m_ui->splitter->setSizes({0,100});

    m_ui->aboutIcon->setPixmap(QApplication::windowIcon().pixmap(256, 256));

    QFont mFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    mFont.setPointSize(m_ui->currentBinaryLabel->font().pointSize());
    m_ui->currentBinaryLabel->setFont(mFont);
    m_ui->recommendedBinaryLabel->setFont(mFont);
    mFont.setPointSize(10);
    m_ui->processOutput->setFont(mFont);

    m_updateURDEButton = new QPushButton(QStringLiteral("Update URDE"), m_ui->centralwidget);
    m_ui->gridLayout->addWidget(m_updateURDEButton, 2, 3, 1, 1);
    m_updateURDEButton->hide();
    QPalette pal = m_updateURDEButton->palette();
    pal.setColor(QPalette::Button, QColor(53,53,72));
    m_updateURDEButton->setPalette(pal);
    connect(m_updateURDEButton, SIGNAL(clicked()), this, SLOT(onUpdateURDEPressed()));

    m_dlManager.connectWidgets(m_ui->downloadProgressBar, m_ui->downloadErrorLabel,
                               std::bind(&MainWindow::onIndexDownloaded, this, std::placeholders::_1),
                               std::bind(&MainWindow::onBinaryDownloaded, this, std::placeholders::_1),
                               std::bind(&MainWindow::onBinaryFailed, this, std::placeholders::_1));

    initSlots();

    m_dlManager.fetchIndex();

    setPath(m_settings.value(QStringLiteral("working_dir")).toString());
}

MainWindow::~MainWindow()
{
    m_heclProc.close();
    m_heclProc.terminate();
    delete m_ui;
}
/* TODO: more complete Vt102 emulation */
// based on information: http://en.m.wikipedia.org/wiki/ANSI_escape_code
// http://misc.flogisoft.com/bash/tip_colors_and_formatting
// http://invisible-island.net/xterm/ctlseqs/ctlseqs.html
void MainWindow::parseEscapeSequence(int attribute, QListIterator<QString>& i, QTextCharFormat& textCharFormat,
                                     const QTextCharFormat& defaultTextCharFormat)
{
    switch (attribute) {
    case 0 : { // Normal/Default (reset all attributes)
        textCharFormat = defaultTextCharFormat;
        break;
    }
    case 1 : { // Bold/Bright (bold or increased intensity)
        textCharFormat.setFontWeight(QFont::Bold);
        break;
    }
    case 2 : { // Dim/Faint (decreased intensity)
        textCharFormat.setFontWeight(QFont::Light);
        break;
    }
    case 3 : { // Italicized (italic on)
        textCharFormat.setFontItalic(true);
        break;
    }
    case 4 : { // Underscore (single underlined)
        textCharFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
        textCharFormat.setFontUnderline(true);
        break;
    }
    case 5 : { // Blink (slow, appears as Bold)
        textCharFormat.setFontWeight(QFont::Bold);
        break;
    }
    case 6 : { // Blink (rapid, appears as very Bold)
        textCharFormat.setFontWeight(QFont::Black);
        break;
    }
    case 7 : { // Reverse/Inverse (swap foreground and background)
        QBrush foregroundBrush = textCharFormat.foreground();
        textCharFormat.setForeground(textCharFormat.background());
        textCharFormat.setBackground(foregroundBrush);
        break;
    }
    case 8 : { // Concealed/Hidden/Invisible (usefull for passwords)
        textCharFormat.setForeground(textCharFormat.background());
        break;
    }
    case 9 : { // Crossed-out characters
        textCharFormat.setFontStrikeOut(true);
        break;
    }
    case 10 : { // Primary (default) font
        textCharFormat.setFont(defaultTextCharFormat.font());
        break;
    }
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19: {
        QFontDatabase fontDatabase;
        QString fontFamily = textCharFormat.fontFamily();
        QStringList fontStyles = fontDatabase.styles(fontFamily);
        int fontStyleIndex = attribute - 11;
        if (fontStyleIndex < fontStyles.length()) {
            textCharFormat.setFont(fontDatabase.font(fontFamily, fontStyles.at(fontStyleIndex), textCharFormat.font().pointSize()));
        }
        break;
    }
    case 20 : { // Fraktur (unsupported)
        break;
    }
    case 21 : { // Set Bold off
        textCharFormat.setFontWeight(QFont::Normal);
        break;
    }
    case 22 : { // Set Dim off
        textCharFormat.setFontWeight(QFont::Normal);
        break;
    }
    case 23 : { // Unset italic and unset fraktur
        textCharFormat.setFontItalic(false);
        break;
    }
    case 24 : { // Unset underlining
        textCharFormat.setUnderlineStyle(QTextCharFormat::NoUnderline);
        textCharFormat.setFontUnderline(false);
        break;
    }
    case 25 : { // Unset Blink/Bold
        textCharFormat.setFontWeight(QFont::Normal);
        break;
    }
    case 26 : { // Reserved
        break;
    }
    case 27 : { // Positive (non-inverted)
        QBrush backgroundBrush = textCharFormat.background();
        textCharFormat.setBackground(textCharFormat.foreground());
        textCharFormat.setForeground(backgroundBrush);
        break;
    }
    case 28 : {
        textCharFormat.setForeground(defaultTextCharFormat.foreground());
        textCharFormat.setBackground(defaultTextCharFormat.background());
        break;
    }
    case 29 : {
        textCharFormat.setUnderlineStyle(QTextCharFormat::NoUnderline);
        textCharFormat.setFontUnderline(false);
        break;
    }
    case 30:
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:
    {
        int colorIndex = attribute - 30;
        QColor color;
        if (QFont::Normal < textCharFormat.fontWeight()) {
            switch (colorIndex) {
            case 0 : {
                color = Qt::darkGray;
                break;
            }
            case 1 : {
                color = Qt::red;
                break;
            }
            case 2 : {
                color = Qt::green;
                break;
            }
            case 3 : {
                color = Qt::yellow;
                break;
            }
            case 4 : {
                color = Qt::blue;
                break;
            }
            case 5 : {
                color = Qt::magenta;
                break;
            }
            case 6 : {
                color = Qt::cyan;
                break;
            }
            case 7 : {
                color = Qt::white;
                break;
            }
            default : {
                Q_ASSERT(false);
            }
            }
        } else {
            switch (colorIndex) {
            case 0 : {
                color = Qt::black;
                break;
            }
            case 1 : {
                color = Qt::darkRed;
                break;
            }
            case 2 : {
                color = Qt::darkGreen;
                break;
            }
            case 3 : {
                color = Qt::darkYellow;
                break;
            }
            case 4 : {
                color = Qt::darkBlue;
                break;
            }
            case 5 : {
                color = Qt::darkMagenta;
                break;
            }
            case 6 : {
                color = Qt::darkCyan;
                break;
            }
            case 7 : {
                color = Qt::lightGray;
                break;
            }
            default : {
                Q_ASSERT(false);
            }
            }
        }
        textCharFormat.setForeground(color);
        break;
    }
    case 38 : {
        if (i.hasNext()) {
            bool ok = false;
            int selector = i.next().toInt(&ok);
            Q_ASSERT(ok);
            QColor color;
            switch (selector) {
            case 2 : {
                if (!i.hasNext()) {
                    break;
                }
                int red = i.next().toInt(&ok);
                Q_ASSERT(ok);
                if (!i.hasNext()) {
                    break;
                }
                int green = i.next().toInt(&ok);
                Q_ASSERT(ok);
                if (!i.hasNext()) {
                    break;
                }
                int blue = i.next().toInt(&ok);
                Q_ASSERT(ok);
                color.setRgb(red, green, blue);
                break;
            }
            case 5 :
            {
                if (!i.hasNext()) {
                    break;
                }
                int index = i.next().toInt(&ok);
                Q_ASSERT(ok);
                if (index >= 0 && index <= 0x07)
                { // 0x00-0x07:  standard colors (as in ESC [ 30..37 m)
                    return parseEscapeSequence(index - 0x00 + 30, i, textCharFormat, defaultTextCharFormat);
                }
                else if (index >= 0x08 && index <= 0x0F)
                { // 0x08-0x0F:  high intensity colors (as in ESC [ 90..97 m)
                    return parseEscapeSequence(index - 0x08 + 90, i, textCharFormat, defaultTextCharFormat);
                }
                else if (index >= 0x10 && index <= 0xE7)
                { // 0x10-0xE7:  6*6*6=216 colors: 16 + 36*r + 6*g + b (0≤r,g,b≤5)
                    index -= 0x10;
                    int red = index % 6;
                    index /= 6;
                    int green = index % 6;
                    index /= 6;
                    int blue = index % 6;
                    index /= 6;
                    Q_ASSERT(index == 0);
                    color.setRgb(red, green, blue);
                    break;
                }
                else if (index >= 0xE8 && index <= 0xFF)
                { // 0xE8-0xFF:  grayscale from black to white in 24 steps
                    qreal intensity = qreal(index - 0xE8) / (0xFF - 0xE8);
                    color.setRgbF(intensity, intensity, intensity);
                    break;
                }
                textCharFormat.setForeground(color);
                break;
            }
            default : {
                break;
            }
            }
        }
        break;
    }
    case 39 : {
        textCharFormat.setForeground(defaultTextCharFormat.foreground());
        break;
    }
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
    case 45:
    case 46:
    case 47: {
        int colorIndex = attribute - 40;
        QColor color;
        switch (colorIndex) {
        case 0 : {
            color = Qt::darkGray;
            break;
        }
        case 1 : {
            color = Qt::red;
            break;
        }
        case 2 : {
            color = Qt::green;
            break;
        }
        case 3 : {
            color = Qt::yellow;
            break;
        }
        case 4 : {
            color = Qt::blue;
            break;
        }
        case 5 : {
            color = Qt::magenta;
            break;
        }
        case 6 : {
            color = Qt::cyan;
            break;
        }
        case 7 : {
            color = Qt::white;
            break;
        }
        default : {
            Q_ASSERT(false);
        }
        }
        textCharFormat.setBackground(color);
        break;
    }
    case 48 : {
        if (i.hasNext()) {
            bool ok = false;
            int selector = i.next().toInt(&ok);
            Q_ASSERT(ok);
            QColor color;
            switch (selector) {
            case 2 : {
                if (!i.hasNext()) {
                    break;
                }
                int red = i.next().toInt(&ok);
                Q_ASSERT(ok);
                if (!i.hasNext()) {
                    break;
                }
                int green = i.next().toInt(&ok);
                Q_ASSERT(ok);
                if (!i.hasNext()) {
                    break;
                }
                int blue = i.next().toInt(&ok);
                Q_ASSERT(ok);
                color.setRgb(red, green, blue);
                break;
            }
            case 5 : {
                if (!i.hasNext()) {
                    break;
                }
                int index = i.next().toInt(&ok);
                Q_ASSERT(ok);
                if (index >= 0x00 && index <= 0x07)
                { // 0x00-0x07:  standard colors (as in ESC [ 40..47 m)
                    return parseEscapeSequence(index - 0x00 + 40, i, textCharFormat, defaultTextCharFormat);
                }
                else if (index >= 0x08 && index <= 0x0F)
                { // 0x08-0x0F:  high intensity colors (as in ESC [ 100..107 m)
                    return parseEscapeSequence(index - 0x08 + 100, i, textCharFormat, defaultTextCharFormat);
                }
                else if (index >= 0x10 && index <= 0xE7)
                { // 0x10-0xE7:  6*6*6=216 colors: 16 + 36*r + 6*g + b (0≤r,g,b≤5)
                    index -= 0x10;
                    int red = index % 6;
                    index /= 6;
                    int green = index % 6;
                    index /= 6;
                    int blue = index % 6;
                    index /= 6;
                    Q_ASSERT(index == 0);
                    color.setRgb(red, green, blue);
                    break;
                }
                else if (index >= 0xE8 && index <= 0xFF)
                { // 0xE8-0xFF:  grayscale from black to white in 24 steps
                    qreal intensity = qreal(index - 0xE8) / (0xFF - 0xE8);
                    color.setRgbF(intensity, intensity, intensity);
                }
            }
                textCharFormat.setBackground(color);
                break;
            }
        }
        break;
    }
    case 49: {
        textCharFormat.setBackground(defaultTextCharFormat.background());
        break;
    }
    case 90:
    case 91:
    case 92:
    case 93:
    case 94:
    case 95:
    case 96:
    case 97: {
        int colorIndex = attribute - 90;
        QColor color;
        switch (colorIndex) {
        case 0 : {
            color = Qt::darkGray;
            break;
        }
        case 1 : {
            color = Qt::red;
            break;
        }
        case 2 : {
            color = Qt::green;
            break;
        }
        case 3 : {
            color = Qt::yellow;
            break;
        }
        case 4 : {
            color = Qt::blue;
            break;
        }
        case 5 : {
            color = Qt::magenta;
            break;
        }
        case 6 : {
            color = Qt::cyan;
            break;
        }
        case 7 : {
            color = Qt::white;
            break;
        }
        default : {
            Q_ASSERT(false);
        }
        }
        color.setRedF(color.redF() * 0.8);
        color.setGreenF(color.greenF() * 0.8);
        color.setBlueF(color.blueF() * 0.8);
        textCharFormat.setForeground(color);
        break;
    }
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    {
        int colorIndex = attribute - 100;
        QColor color;
        switch (colorIndex) {
        case 0 : {
            color = Qt::darkGray;
            break;
        }
        case 1 : {
            color = Qt::red;
            break;
        }
        case 2 : {
            color = Qt::green;
            break;
        }
        case 3 : {
            color = Qt::yellow;
            break;
        }
        case 4 : {
            color = Qt::blue;
            break;
        }
        case 5 : {
            color = Qt::magenta;
            break;
        }
        case 6 : {
            color = Qt::cyan;
            break;
        }
        case 7 : {
            color = Qt::white;
            break;
        }
        default : {
            Q_ASSERT(false);
        }
        }
        color.setRedF(color.redF() * 0.8);
        color.setGreenF(color.greenF() * 0.8);
        color.setBlueF(color.blueF() * 0.8);
        textCharFormat.setBackground(color);
        break;
    }
    default : {
        break;
    }
    }
}

void MainWindow::onExtract()
{
    if (m_path.isEmpty())
        return;
    QString imgPath = QFileDialog::getOpenFileName(this, QStringLiteral("Extract Image"), {},
                                                   QStringLiteral("Images (*.iso *.wbfs *.gcm)"));
    m_ansiString.clear();
    m_ui->processOutput->clear();
    m_heclProc.close();
    m_heclProc.terminate();
    m_heclProc.setProcessChannelMode(QProcess::ProcessChannelMode::MergedChannels);
    m_heclProc.setWorkingDirectory(m_path);
    m_heclProc.start(m_heclPath, {"extract", "-y", imgPath, m_path});

    m_ui->heclTabs->setCurrentIndex(0);

    disableOperations();
    m_ui->extractBtn->setText(QStringLiteral("Cancel"));
    m_ui->extractBtn->setEnabled(true);
    disconnect(m_ui->extractBtn, SIGNAL(clicked()), nullptr, nullptr);
    connect(m_ui->extractBtn, SIGNAL(clicked()), this, SLOT(doHECLTerminate()));

    disconnect(&m_heclProc, SIGNAL(finished(int)), nullptr, nullptr);
    connect(&m_heclProc, SIGNAL(finished(int)), this, SLOT(onExtractFinished(int)));
}

void MainWindow::onExtractFinished(int returnCode)
{
    disconnect(m_ui->extractBtn, SIGNAL(clicked()), nullptr, nullptr);
    connect(m_ui->extractBtn, SIGNAL(clicked()), this, SLOT(onExtract()));
    checkDownloadedBinary();
}

void MainWindow::onPackage()
{
    if (m_path.isEmpty())
        return;
    m_ansiString.clear();
    m_ui->processOutput->clear();
    m_heclProc.close();
    m_heclProc.terminate();
    m_heclProc.setProcessChannelMode(QProcess::ProcessChannelMode::MergedChannels);
    m_heclProc.setWorkingDirectory(m_path);
    m_heclProc.start(m_heclPath, {"package", "-y"});

    m_ui->heclTabs->setCurrentIndex(0);

    disableOperations();
    m_ui->packageBtn->setText(QStringLiteral("Cancel"));
    m_ui->packageBtn->setEnabled(true);
    disconnect(m_ui->packageBtn, SIGNAL(clicked()), nullptr, nullptr);
    connect(m_ui->packageBtn, SIGNAL(clicked()), this, SLOT(doHECLTerminate()));

    disconnect(&m_heclProc, SIGNAL(finished(int)), nullptr, nullptr);
    connect(&m_heclProc, SIGNAL(finished(int)), this, SLOT(onPackageFinished(int)));
}

void MainWindow::onPackageFinished(int returnCode)
{
    disconnect(m_ui->packageBtn, SIGNAL(clicked()), nullptr, nullptr);
    connect(m_ui->packageBtn, SIGNAL(clicked()), this, SLOT(onPackage()));
    checkDownloadedBinary();
}

void MainWindow::onLaunch()
{
    if (m_path.isEmpty())
        return;
    m_ansiString.clear();
    m_ui->processOutput->clear();
    m_heclProc.close();
    m_heclProc.terminate();
    m_heclProc.setProcessChannelMode(QProcess::ProcessChannelMode::MergedChannels);
    m_heclProc.setWorkingDirectory(m_path);
    m_heclProc.start(m_urdePath, {"--no-shader-warmup", m_path + "/out"});

    m_ui->heclTabs->setCurrentIndex(0);

    disableOperations();

    disconnect(&m_heclProc, SIGNAL(finished(int)), nullptr, nullptr);
    connect(&m_heclProc, SIGNAL(finished(int)), this, SLOT(onLaunchFinished(int)));
}

void MainWindow::onLaunchFinished(int returnCode)
{
    checkDownloadedBinary();
}

void MainWindow::doHECLTerminate()
{
    m_heclProc.terminate();
}

void MainWindow::onReturnPressed()
{
    if (sender() == m_ui->pathEdit && !m_ui->pathEdit->text().isEmpty())
        setPath(m_ui->pathEdit->text());
}

void MainWindow::onIndexDownloaded(const QStringList& index)
{
    int bestVersion = 0;
    m_ui->binaryComboBox->clear();
    for (const QString& str : index)
    {
        URDEVersion version(str);
        if (m_ui->sysReqTable->willRun(version))
            bestVersion = m_ui->binaryComboBox->count();
        m_ui->binaryComboBox->addItem(version.fileString(false), QVariant::fromValue(version));
    }
    m_ui->binaryComboBox->setCurrentIndex(bestVersion);
    m_recommendedVersion = m_ui->binaryComboBox->itemData(bestVersion).value<URDEVersion>();
    m_ui->recommendedBinaryLabel->setText(m_recommendedVersion.fileString(false));
    m_ui->binaryComboBox->setEnabled(true);

    if (!m_path.isEmpty())
    {
        checkDownloadedBinary();
        m_ui->downloadButton->setEnabled(true);
    }
}

void MainWindow::onDownloadPressed()
{
    m_updateURDEButton->hide();
    QString filename = m_ui->binaryComboBox->currentData().value<URDEVersion>().fileString(true);
    disableOperations();
    m_ui->downloadButton->setEnabled(false);
    m_dlManager.fetchBinary(filename, m_path + '/' + filename);
}

void MainWindow::onUpdateURDEPressed()
{
    m_ui->heclTabs->setCurrentIndex(1);
    onDownloadPressed();
}

void MainWindow::onBinaryDownloaded(const QString& file)
{
    QFileInfo path(file);
#ifndef _WIN32
    QProcess unzip;
    unzip.setWorkingDirectory(path.dir().absolutePath());
    unzip.start("unzip", {"-o", path.fileName()});
    unzip.waitForFinished();
    int err = unzip.exitCode();
#else
    SHFILEOPSTRUCT fileOp = {};
    fileOp.wFunc = FO_COPY;
    fileOp.pFrom = (path.fileName().toStdWString() + L"\\*.*\0\0").c_str();
    fileOp.pTo = (path.dir().absolutePath().toStdWString() + L"\0\0").c_str();
    fileOp.fFlags |= FOF_NOCONFIRMATION;
    int err = SHFileOperationW(&fileOp);
    if (fileOp.fAnyOperationsAborted)
        err = 1;
#endif
    QFile::remove(file);
    if (err)
        m_ui->downloadErrorLabel->setText(QStringLiteral("Error extracting ") + path.fileName());
    else
        m_ui->downloadErrorLabel->setText(QStringLiteral("Download successful"), true);
    m_ui->downloadButton->setEnabled(true);
    checkDownloadedBinary();
    if (!err && m_ui->extractBtn->isEnabled())
        m_ui->downloadErrorLabel->setText(QStringLiteral("Download successful - Press 'Extract' to continue."), true);
}

void MainWindow::onBinaryFailed(const QString& file)
{
    m_ui->downloadButton->setEnabled(true);
    checkDownloadedBinary();
}

void MainWindow::disableOperations()
{
    m_ui->extractBtn->setEnabled(false);
    m_ui->packageBtn->setEnabled(false);
    m_ui->launchBtn->setEnabled(false);
}

void MainWindow::enableOperations()
{
    disableOperations();
    if (m_path.isEmpty())
        return;

    m_ui->extractBtn->setText(QStringLiteral("Extract"));
    m_ui->packageBtn->setText(QStringLiteral("Package"));
    m_ui->launchBtn->setText(QStringLiteral("Launch"));

    m_ui->extractBtn->setEnabled(true);
    if (QFile::exists(m_path + "/MP1/!original_ids.yaml"))
    {
        m_ui->packageBtn->setEnabled(true);
        if (QFile::exists(m_path + "/out/MP1/!original_ids.upak"))
            m_ui->launchBtn->setEnabled(true);
    }

    if (m_ui->launchBtn->isEnabled())
        insertContinueNote("Package complete - Press 'Launch' to start URDE.");
    else if (m_ui->packageBtn->isEnabled())
        insertContinueNote("Extract complete - Press 'Package' to continue.");
    else if (m_ui->extractBtn->isEnabled())
        insertContinueNote("Press 'Extract' to begin.");
}

static bool GetDLPackage(const QString& path, QString& dlPackage)
{
    QProcess proc;
    proc.start(path, {"--dlpackage"}, QIODevice::ReadOnly);
    if (proc.waitForStarted())
    {
        proc.waitForFinished();
        if (proc.exitCode() == 100)
            dlPackage = QString::fromUtf8(proc.readLine()).trimmed();
        return true;
    }
    return false;
}

bool MainWindow::checkDownloadedBinary()
{
    disableOperations();
    m_updateURDEButton->hide();

    m_urdePath = QString();
    m_heclPath = QString();

    if (m_path.isEmpty())
    {
        m_ui->heclTabs->setCurrentIndex(1);
        m_ui->downloadErrorLabel->setText(QStringLiteral("Set working directory to continue."), true);
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
    if (GetDLPackage(urdePath, urdeDlPackage) &&
        GetDLPackage(heclPath, heclDlPackage) &&
        GetDLPackage(visigenPath, visigenDlPackage))
    {
        if (!urdeDlPackage.isEmpty() &&
            urdeDlPackage == heclDlPackage &&
            urdeDlPackage == visigenDlPackage)
        {
            URDEVersion v(urdeDlPackage);
            m_ui->currentBinaryLabel->setText(v.fileString(false));
            if (m_recommendedVersion.isValid() && v.isValid() &&
                m_recommendedVersion.getVersion() > v.getVersion())
            {
                m_updateURDEButton->show();
            }
        }
        else
        {
            m_ui->currentBinaryLabel->setText(QStringLiteral("unknown... re-download recommended"));
        }

        enableOperations();
        m_urdePath = urdePath;
        m_heclPath = heclPath;
        return true;
    }
    else
    {
        m_ui->currentBinaryLabel->setText(QStringLiteral("none"));
        m_ui->heclTabs->setCurrentIndex(1);
        m_ui->downloadErrorLabel->setText(QStringLiteral("Press 'Download' to fetch latest URDE binary."), true);
    }

    return false;
}

void MainWindow::setPath(const QString& path)
{
    m_path = path;
    m_settings.setValue(QStringLiteral("working_dir"), m_path);

    if (!path.isEmpty())
    {
        m_ui->pathEdit->setText(m_path);
        m_ui->downloadButton->setToolTip(QString());
        m_ui->downloadButton->setEnabled(m_ui->binaryComboBox->isEnabled());
    }
    else
    {
        m_ui->downloadButton->setToolTip(QStringLiteral("Working directory must be set"));
        m_ui->downloadButton->setEnabled(false);
        m_ui->currentBinaryLabel->setText(QStringLiteral("none"));
    }

    checkDownloadedBinary();
}

void MainWindow::initSlots()
{
#ifdef Q_OS_WIN
    m_heclProc.setEnvironment(QProcessEnvironment::systemEnvironment().toStringList() + QStringList("ConEmuANSI=ON"));
#endif
    connect(&m_heclProc, &QProcess::readyRead, [=](){
        m_ansiString = m_heclProc.readAll();
        setTextTermFormatting(m_ansiString);
        m_ui->processOutput->ensureCursorVisible();
    });

    connect(m_ui->extractBtn, SIGNAL(clicked()), this, SLOT(onExtract()));
    connect(m_ui->packageBtn, SIGNAL(clicked()), this, SLOT(onPackage()));
    connect(m_ui->launchBtn, SIGNAL(clicked()), this, SLOT(onLaunch()));

    connect(m_ui->browseBtn, &QPushButton::clicked, [=]() {
        FileDirDialog dialog(this);
        dialog.setWindowTitle("Select Working Directory");
        int res = dialog.exec();//QFileDialog::getOpenFileName(this, "Select ISO or Directory");
        if (res == QFileDialog::Rejected)
            return;

        if (dialog.selectedFiles().size() <= 0)
            return;

        /* TODO: Add beacon detection */
        setPath(dialog.selectedFiles().at(0));
    });

    connect(m_ui->downloadButton, SIGNAL(clicked()), this, SLOT(onDownloadPressed()));
}

static void ReturnInsert(QTextCursor& cur, const QString& text)
{
    QStringList list = text.split('\r');
    if (!list.front().isEmpty())
        cur.insertText(list.front());
    if (list.size() > 1)
    {
        for (auto it = list.begin() + 1; it != list.end(); ++it)
        {
            cur.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
            if (!it->isEmpty())
                cur.insertText(*it);
        }
    }
}

static void ReturnInsert(QTextCursor& cur, const QString& text, const QTextCharFormat& format)
{
    QStringList list = text.split('\r');
    if (!list.front().isEmpty())
        cur.insertText(list.front(), format);
    if (list.size() > 1)
    {
        for (auto it = list.begin() + 1; it != list.end(); ++it)
        {
            cur.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
            if (!it->isEmpty())
                cur.insertText(*it, format);
        }
    }
}

void MainWindow::setTextTermFormatting(const QString& text)
{
    m_inContinueNote = false;

    QTextDocument* document = m_ui->processOutput->document();
    QRegExp const escapeSequenceExpression(R"(\x1B\[([\d;]+)m)");
    QTextCursor cursor(document);
    cursor.movePosition(QTextCursor::End);
    QTextCharFormat defaultTextCharFormat = cursor.charFormat();
    cursor.beginEditBlock();
    int offset = escapeSequenceExpression.indexIn(text);
    ReturnInsert(cursor, text.mid(0, offset));
    QTextCharFormat textCharFormat = defaultTextCharFormat;
    while (!(offset < 0)) {
        int previousOffset = offset + escapeSequenceExpression.matchedLength();
        QStringList capturedTexts = escapeSequenceExpression.capturedTexts().back().split(';');
        QListIterator< QString > i(capturedTexts);
        while (i.hasNext()) {
            bool ok = false;
            int attribute = i.next().toInt(&ok);
            Q_ASSERT(ok);
            parseEscapeSequence(attribute, i, textCharFormat, defaultTextCharFormat);
        }
        offset = escapeSequenceExpression.indexIn(text, previousOffset);
        if (offset < 0) {
            ReturnInsert(cursor, text.mid(previousOffset), textCharFormat);
        } else {
            ReturnInsert(cursor, text.mid(previousOffset, offset - previousOffset), textCharFormat);
        }
    }
    cursor.setCharFormat(defaultTextCharFormat);
    cursor.endEditBlock();
    cursor.movePosition(QTextCursor::End);
    m_ui->processOutput->setTextCursor(cursor);
}

void MainWindow::insertContinueNote(const QString& text)
{
    if (m_inContinueNote)
        return;
    m_inContinueNote = true;

    QTextDocument* document = m_ui->processOutput->document();
    QTextCursor cursor(document);
    cursor.movePosition(QTextCursor::End);
    QTextCharFormat textCharFormat = cursor.charFormat();
    textCharFormat.setForeground(QColor(0,255,0));
    cursor.beginEditBlock();
    cursor.insertText(text + '\n', textCharFormat);
    cursor.endEditBlock();
    cursor.movePosition(QTextCursor::End);
    m_ui->processOutput->setTextCursor(cursor);
}
