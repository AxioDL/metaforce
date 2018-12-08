#include "testquazipnewinfo.h"

#include "qztest.h"

#include <QDir>
#include <QFileInfo>
#include <QtTest/QtTest>

#include <quazip/quazip.h>
#include <quazip/quazipfile.h>
#include <quazip/quazipnewinfo.h>
#include <quazip/quazipfileinfo.h>

TestQuaZipNewInfo::TestQuaZipNewInfo(QObject *parent) :
    QObject(parent)
{
}

void TestQuaZipNewInfo::setFileNTFSTimes()
{
    QString zipName = "newtimes.zip";
    QStringList testFiles;
    testFiles << "test.txt";
    QDir curDir;
    if (curDir.exists(zipName)) {
        if (!curDir.remove(zipName))
            QFAIL("Can't remove zip file");
    }
    if (!createTestFiles(testFiles)) {
        QFAIL("Can't create test file");
    }
    QDateTime base(QDate(1601, 1, 1), QTime(0, 0), Qt::UTC);
    quint64 mTicks, aTicks, cTicks;
    {
        // create
        QuaZip zip(zipName);
        QVERIFY(zip.open(QuaZip::mdCreate));
        QuaZipFile zipFile(&zip);
        QFileInfo fileInfo("tmp/test.txt");
        mTicks = base.msecsTo(fileInfo.lastModified()) * 10000;
        aTicks = base.msecsTo(fileInfo.lastRead()) * 10000;
        cTicks = base.msecsTo(fileInfo.created()) * 10000;
        QuaZipNewInfo newInfo("test.txt", "tmp/test.txt");
        newInfo.setFileNTFSTimes("tmp/test.txt");
        QVERIFY(zipFile.open(QIODevice::WriteOnly, newInfo));
        zipFile.close();
        zip.close();
    }
    {
        // check
        QuaZip zip(zipName);
        QVERIFY(zip.open(QuaZip::mdUnzip));
        zip.goToFirstFile();
        QuaZipFileInfo64 fileInfo;
        QVERIFY(zip.getCurrentFileInfo(&fileInfo));
        zip.close();
        QByteArray &extra = fileInfo.extra;
        bool ntfsFound = false;
        int timesFound = 0;
        for (int i = 0; i <= extra.size() - 4; ) {
            unsigned type = static_cast<unsigned>(static_cast<unsigned char>(
                                                      extra.at(i)))
                    | (static_cast<unsigned>(static_cast<unsigned char>(
                                                 extra.at(i + 1))) << 8);
            i += 2;
            unsigned length = static_cast<unsigned>(static_cast<unsigned char>(
                                                        extra.at(i)))
                    | (static_cast<unsigned>(static_cast<unsigned char>(
                                                 extra.at(i + 1))) << 8);
            i += 2;
            if (type == 0x000Au && length >= 32) {
                ntfsFound = true;
                i += 4; // reserved
                while (i <= extra.size() - 4) {
                    unsigned tag = static_cast<unsigned>(
                                static_cast<unsigned char>(extra.at(i)))
                            | (static_cast<unsigned>(
                                   static_cast<unsigned char>(extra.at(i + 1)))
                               << 8);
                    i += 2;
                    unsigned tagsize = static_cast<unsigned>(
                                static_cast<unsigned char>(extra.at(i)))
                            | (static_cast<unsigned>(
                                   static_cast<unsigned char>(extra.at(i + 1)))
                               << 8);
                    i += 2;
                    QCOMPARE(tagsize, static_cast<unsigned>(24));
                    if (tag == 0x0001u && tagsize >= 24) {
                        for (int timesPos = i;
                             i < timesPos + 24;
                             i += 8, tagsize -= 8) {
                            quint64 time = static_cast<quint64>(
                                static_cast<unsigned char>(extra.at(i)))
                            | (static_cast<quint64>(static_cast<unsigned char>(
                                                       extra.at(i + 1))) << 8)
                            | (static_cast<quint64>(static_cast<unsigned char>(
                                                       extra.at(i + 2))) << 16)
                            | (static_cast<quint64>(static_cast<unsigned char>(
                                                       extra.at(i + 3))) << 24)
                            | (static_cast<quint64>(static_cast<unsigned char>(
                                                       extra.at(i + 4))) << 32)
                            | (static_cast<quint64>(static_cast<unsigned char>(
                                                       extra.at(i + 5))) << 40)
                            | (static_cast<quint64>(static_cast<unsigned char>(
                                                       extra.at(i + 6))) << 48)
                            | (static_cast<quint64>(static_cast<unsigned char>(
                                                       extra.at(i + 7))) << 56);
                            ++timesFound;
                            if (i - timesPos == 0) {
                                QCOMPARE(time, mTicks);
                            } else if (i - timesPos == 8) {
                                QCOMPARE(time, aTicks);
                            } else if (i - timesPos == 16) {
                                QCOMPARE(time, cTicks);
                            } else {
                                QFAIL("Wrong position");
                            }
                        }
                        i += tagsize;
                    } else {
                        i += tagsize;
                    }

                }
            } else {
                i += length;
            }
        }
        QVERIFY(ntfsFound);
        QCOMPARE(timesFound, 3);
    }
    removeTestFiles(testFiles);
    curDir.remove(zipName);
}
