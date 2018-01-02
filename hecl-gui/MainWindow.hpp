#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QProcess>
#include <memory>
#include "DownloadManager.hpp"
#include "Common.hpp"
class QTextEdit;
class QTextCharFormat;
class QPushButton;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Ui::MainWindow* m_ui;
    QString m_ansiString;
    QString m_path;
    QString m_urdePath;
    QString m_heclPath;
    QProcess m_heclProc;
    DownloadManager m_dlManager;
    QSettings m_settings;
    URDEVersion m_recommendedVersion;
    QPushButton* m_updateURDEButton;
    bool m_inContinueNote = false;
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setTextTermFormatting(const QString& text);
    void insertContinueNote(const QString& text);
    void parseEscapeSequence(int attribute, QListIterator<QString>& i, QTextCharFormat& textCharFormat,
                             const QTextCharFormat& defaultTextCharFormat);
private slots:
    void onExtract();
    void onExtractFinished(int exitCode);
    void onPackage();
    void onPackageFinished(int exitCode);
    void onLaunch();
    void onLaunchFinished(int exitCode);
    void doHECLTerminate();
    void onReturnPressed();
    void onDownloadPressed();
    void onUpdateURDEPressed();
private:
    bool checkDownloadedBinary();
    void setPath(const QString& path);
    void initSlots();
    void onIndexDownloaded(const QStringList& index);
    void onBinaryDownloaded(const QString& file);
    void onBinaryFailed(const QString& file);
    void disableOperations();
    void enableOperations();
};

#endif // MAINWINDOW_HPP
