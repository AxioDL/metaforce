#ifndef GUI_EXTRACTZIP_HPP
#define GUI_EXTRACTZIP_HPP

class QStringList;
class QuaZip;
class QString;

class ExtractZip
{
public:
    static QStringList getFileList(QuaZip& zip);
    static bool extractFile(QuaZip& zip, QString fileName, QString fileDest);
    static bool extractDir(QuaZip& zip, QString dir);
};

#endif // GUI_EXTRACTZIP_HPP
