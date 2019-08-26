#include "DownloadManager.hpp"
#include "Common.hpp"
#include <quazip.h>

#define KEY_PINNING 0

#if KEY_PINNING
static const char AxioDLPublicKeyPEM[] =
    "-----BEGIN PUBLIC KEY-----\n"
    "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAvtshImzoP1a++9P5RK0k\n"
    "btTOpwie7O7S/wWFZxwwbMezEPhjw2uu86TPqJe3P/1v+6xRKrEf9zFn/sKNygvD\n"
    "bO64ZkJre4M46IYd0XxwIhiu7PBR+13CD+fqbrbYwPkoG090CP4MtZZN6mt5NAKB\n"
    "QHoIj0wV5K/jJE9cBQxViwOqrxK05Cl/ivy0gRtpL7Ot6S+QHL3++rb6U2hWydIQ\n"
    "kS+ucufKCIL77RcDwAc9vwNvzxf9EUU2pmq+EsEtLgRw3fR6BInoltOI8P9X5Wo6\n"
    "/skeg92xZA++vv0neq5gjjDfa2A1zDgJRysz3Xps/AMlLOe55XCzXse9BpvChT+Z\n"
    "pwIDAQAB\n"
    "-----END PUBLIC KEY-----\n";

static const QSslKey AxioDLPublicKey = QSslKey({AxioDLPublicKeyPEM}, QSsl::Rsa, QSsl::Pem, QSsl::PublicKey);

static const char AxioDLEdgePublicKeyPEM[] =
    "-----BEGIN PUBLIC KEY-----\n"
    "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE4a8ZLg3LRU0FiK6m8g2pT3qVBTMA\n"
    "K2Uu5VGl7iamdGpUjynQ4uYWMx+WXf2Qkh7UZZgYvA6UeWHEs3M6ME8T6g==\n"
    "-----END PUBLIC KEY-----\n";

static const QSslKey AxioDLEdgePublicKey = QSslKey({AxioDLEdgePublicKeyPEM}, QSsl::Ec, QSsl::Pem, QSsl::PublicKey);
#endif

void DownloadManager::_validateCert(QNetworkReply* reply) {
#if KEY_PINNING
  QSslCertificate peerCert = reply->sslConfiguration().peerCertificate();
  QSslKey peerKey = peerCert.publicKey();
  if (peerKey != AxioDLPublicKey && peerKey != AxioDLEdgePublicKey) {
    auto cn = peerCert.subjectInfo(QSslCertificate::CommonName);
    if (!cn.empty())
      setError(QNetworkReply::SslHandshakeFailedError,
               QStringLiteral("Certificate pinning mismatch \"") + cn.first() + "\"");
    else
      setError(QNetworkReply::SslHandshakeFailedError, QStringLiteral("Certificate pinning mismatch"));
    reply->abort();
  }
#endif
}

static const QString Domain = QStringLiteral("https://releases.axiodl.com/");
static const QString Index = QStringLiteral("index.txt");

void DownloadManager::fetchIndex() {
  if (m_indexInProgress)
    return;

  resetError();
  QString track = QSettings().value("update_track").toString();
  QString url = Domain + track + '/' + CurPlatformString + '/' + Index;
  m_indexInProgress = m_netManager.get(QNetworkRequest(url));
  connect(m_indexInProgress, &QNetworkReply::finished, this, &DownloadManager::indexFinished);
  connect(m_indexInProgress, qOverload<QNetworkReply::NetworkError>(&QNetworkReply::error), this,
          &DownloadManager::indexError);
  connect(m_indexInProgress, &QNetworkReply::encrypted, this, &DownloadManager::indexValidateCert);
}

void DownloadManager::fetchBinary(const QString& str, const QString& outPath) {
  if (m_binaryInProgress)
    return;

  resetError();
  m_outPath = outPath;

  const QString track = QSettings().value("update_track").toString();
  const QString url = Domain + track + '/' + CurPlatformString + '/' + str;
  m_binaryInProgress = m_netManager.get(QNetworkRequest(url));
  connect(m_binaryInProgress, &QNetworkReply::finished, this, &DownloadManager::binaryFinished);
  connect(m_binaryInProgress, qOverload<QNetworkReply::NetworkError>(&QNetworkReply::error), this,
          &DownloadManager::binaryError);
  connect(m_binaryInProgress, &QNetworkReply::encrypted, this, &DownloadManager::binaryValidateCert);
  connect(m_binaryInProgress, &QNetworkReply::downloadProgress, this, &DownloadManager::binaryDownloadProgress);

  if (m_progBar) {
    m_progBar->setEnabled(true);
    m_progBar->setValue(0);
  }
}

void DownloadManager::indexFinished() {
  if (m_hasError)
    return;

  QStringList files;

  while (!m_indexInProgress->atEnd()) {
    QString line = QString::fromUtf8(m_indexInProgress->readLine()).trimmed();
    if (line.isEmpty())
      continue;
    files.push_back(line);
  }

  if (m_indexCompletionHandler)
    m_indexCompletionHandler(files);

  m_indexInProgress->deleteLater();
  m_indexInProgress = nullptr;
}

void DownloadManager::indexError(QNetworkReply::NetworkError error) {
  setError(error, m_indexInProgress->errorString());
  m_indexInProgress->deleteLater();
  m_indexInProgress = nullptr;
}

void DownloadManager::indexValidateCert() { _validateCert(m_indexInProgress); }

void DownloadManager::binaryFinished() {
  if (m_hasError)
    return;

  if (m_progBar)
    m_progBar->setValue(100);

  QByteArray all = m_binaryInProgress->readAll();
  QBuffer buff(&all);
  QuaZip zip(&buff);
  if (!zip.open(QuaZip::mdUnzip)) {
    setError(QNetworkReply::UnknownContentError, "Unable to open zip archive.");
    m_binaryInProgress->deleteLater();
    m_binaryInProgress = nullptr;
    return;
  }

  if (m_completionHandler)
    m_completionHandler(zip);

  m_binaryInProgress->deleteLater();
  m_binaryInProgress = nullptr;
}

void DownloadManager::binaryError(QNetworkReply::NetworkError error) {
  setError(error, m_binaryInProgress->errorString());
  m_binaryInProgress->deleteLater();
  m_binaryInProgress = nullptr;

  if (m_progBar)
    m_progBar->setEnabled(false);

  if (m_failedHandler)
    m_failedHandler();
}

void DownloadManager::binaryValidateCert() { _validateCert(m_binaryInProgress); }

void DownloadManager::binaryDownloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
  if (m_progBar) {
    if (bytesReceived == bytesTotal)
      m_progBar->setValue(100);
    else
      m_progBar->setValue(int(bytesReceived * 100 / bytesTotal));
  }
}
