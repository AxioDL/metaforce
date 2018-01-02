#ifndef GUI_DOWNLOADMANAGER_HPP
#define GUI_DOWNLOADMANAGER_HPP

#include <QObject>
#include <QtNetwork>
#include <QNetworkAccessManager>
#include <QProgressBar>
#include <QLabel>

class DownloadManager : public QObject
{
    Q_OBJECT
    QNetworkAccessManager m_netManager;
    QNetworkReply* m_indexInProgress = nullptr;
    QNetworkReply* m_binaryInProgress = nullptr;
    QString m_outPath;
    bool m_hasError = false;
    QProgressBar* m_progBar = nullptr;
    QLabel* m_errorLabel = nullptr;
    std::function<void(const QStringList& index)> m_indexCompletionHandler;
    std::function<void(const QString& file)> m_completionHandler;
    std::function<void(const QString& file)> m_failedHandler;

    void resetError()
    {
        m_hasError = false;
        if (m_errorLabel)
            m_errorLabel->setText(QString());
    }

    void setError(QNetworkReply::NetworkError error, const QString& errStr)
    {
        if (m_hasError && error == QNetworkReply::OperationCanceledError)
            return;
        m_hasError = true;
        if (m_errorLabel)
            m_errorLabel->setText(errStr);
    }

    void _validateCert(QNetworkReply* reply);

public:
    explicit DownloadManager(QObject* parent = Q_NULLPTR)
    : QObject(parent), m_netManager(this) {}
    void connectWidgets(QProgressBar* progBar, QLabel* errorLabel,
                        std::function<void(const QStringList& index)>&& indexCompletionHandler,
                        std::function<void(const QString& file)>&& completionHandler,
                        std::function<void(const QString& file)>&& failedHandler)
    {
        m_progBar = progBar;
        m_errorLabel = errorLabel;
        m_indexCompletionHandler = std::move(indexCompletionHandler);
        m_completionHandler = std::move(completionHandler);
        m_failedHandler = std::move(failedHandler);
    }
    void fetchIndex();
    void fetchBinary(const QString& str, const QString& outPath);
    bool hasError() const { return m_hasError; }

public slots:
    void indexFinished();
    void indexError(QNetworkReply::NetworkError error);
    void indexValidateCert();

    void binaryFinished();
    void binaryError(QNetworkReply::NetworkError error);
    void binaryValidateCert();
    void binaryDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

};

#endif // GUI_DOWNLOADMANAGER_HPP
