#include "ExtractZip.hpp"
#include <QDir>
#include <quazip.h>
#include <quazipfile.h>

/**
 * Modified JICompress utilities to operate on in-memory zip.
 * Only contains directory extraction functionality.
 */

static bool copyData(QIODevice& inFile, QIODevice& outFile) {
  while (!inFile.atEnd()) {
    char buf[4096];
    qint64 readLen = inFile.read(buf, 4096);
    if (readLen <= 0)
      return false;
    if (outFile.write(buf, readLen) != readLen)
      return false;
  }
  return true;
}

QStringList ExtractZip::getFileList(QuaZip& zip) {
  // Estraggo i nomi dei file
  QStringList lst;
  QuaZipFileInfo64 info;
  for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile()) {
    if (!zip.getCurrentFileInfo(&info))
      return {};
    lst << info.name;
    // info.name.toLocal8Bit().constData()
  }

  return lst;
}

/**OK
 * Estrae il file fileName, contenuto nell'oggetto zip, con il nome fileDest.
 * Se la funzione fallisce restituisce false e cancella il file che si e tentato di estrarre.
 *
 * La funzione fallisce se:
 * * zip==NULL;
 * * l'oggetto zip e stato aperto in una modalita non compatibile con l'estrazione di file;
 * * non e possibile aprire il file all'interno dell'oggetto zip;
 * * non e possibile creare il file estratto;
 * * si e rilevato un errore nella copia dei dati (1);
 * * non e stato possibile chiudere il file all'interno dell'oggetto zip (1);
 *
 * (1): prima di uscire dalla funzione cancella il file estratto.
 */
bool ExtractZip::extractFile(QuaZip& zip, QString fileName, QString fileDest) {
  // zip: oggetto dove aggiungere il file
  // filename: nome del file reale
  // fileincompress: nome del file all'interno del file compresso

  // Controllo l'apertura dello zip
  if (zip.getMode() != QuaZip::mdUnzip)
    return false;

  // Apro il file compresso
  if (!fileName.isEmpty())
    zip.setCurrentFile(fileName);
  QuaZipFile inFile(&zip);
  if (!inFile.open(QIODevice::ReadOnly) || inFile.getZipError() != UNZ_OK)
    return false;

  // Controllo esistenza cartella file risultato
  QDir curDir;
  if (fileDest.endsWith('/')) {
    if (!curDir.mkpath(fileDest)) {
      return false;
    }
  } else {
    if (!curDir.mkpath(QFileInfo(fileDest).absolutePath())) {
      return false;
    }
  }

  QuaZipFileInfo64 info;
  if (!zip.getCurrentFileInfo(&info))
    return false;

  QFile::Permissions srcPerm = info.getPermissions();
  if (fileDest.endsWith('/') && QFileInfo(fileDest).isDir()) {
    if (srcPerm != 0) {
      QFile(fileDest).setPermissions(srcPerm);
    }
    return true;
  }

  // Apro il file risultato
  QFile outFile;
  outFile.setFileName(fileDest);
  if (!outFile.open(QIODevice::WriteOnly))
    return false;

  // Copio i dati
  if (!copyData(inFile, outFile) || inFile.getZipError() != UNZ_OK) {
    outFile.close();
    return false;
  }
  outFile.close();

  // Chiudo i file
  inFile.close();
  if (inFile.getZipError() != UNZ_OK) {
    return false;
  }

  if (srcPerm != 0) {
    outFile.setPermissions(srcPerm);
  }
  return true;
}

/**OK
 * Estrae il file fileCompressed nella cartella dir.
 * Se dir = "" allora il file viene estratto nella cartella corrente.
 * Se la funzione fallisce cancella i file che si e tentato di estrarre.
 * Restituisce i nomi assoluti dei file estratti.
 *
 * La funzione fallisce se:
 * * non si riesce ad aprire l'oggetto zip;
 * * la compressione di un file fallisce;
 * * non si riesce a chiudere l'oggetto zip;
 */
bool ExtractZip::extractDir(QuaZip& zip, QString dir) {
  QDir directory(dir);
  if (!zip.goToFirstFile()) {
    return false;
  }
  do {
    QString name = zip.getCurrentFileName();
    QString absFilePath = directory.absoluteFilePath(name);
    if (!extractFile(zip, "", absFilePath))
      return false;
  } while (zip.goToNextFile());

  return true;
}
