#pragma once

class QStringList;
class QuaZip;
class QString;

class ExtractZip {
public:
  static QStringList getFileList(QuaZip& zip);
  static bool extractFile(QuaZip& zip, QString fileName, QString fileDest);
  static bool extractDir(QuaZip& zip, QString dir);
};
