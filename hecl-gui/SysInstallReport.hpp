#ifndef GUI_SYSINSTALLREPORT_HPP
#define GUI_SYSINSTALLREPORT_HPP

#include <QWidget>
#include <QLabel>

class SysInstallReport : public QWidget
{
    Q_OBJECT
    QLabel* m_installFile;
public:
    SysInstallReport(QWidget* parent = Q_NULLPTR);

    virtual QString installEntity() const { return QStringLiteral("Binary"); }
    virtual QString installFile(int version, QString os, QString architecture, QString vectorISA,
                                QString extension) const
    {
        QString ret;
        if (!extension.isEmpty())
            ret.sprintf("urde-%d-%s-%s-%s.%s", version, os.toUtf8().data(), architecture.toUtf8().data(),
                        vectorISA.toUtf8().data(), extension.toUtf8().data());
        else
            ret.sprintf("urde-%d-%s-%s-%s", version, os.toUtf8().data(), architecture.toUtf8().data(),
                        vectorISA.toUtf8().data());
        return ret;
    }
};

#endif // GUI_SYSINSTALLREPORT_HPP
