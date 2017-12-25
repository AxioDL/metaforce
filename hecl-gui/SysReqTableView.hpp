#ifndef GUI_SYSREQTABLEVIEW_HPP
#define GUI_SYSREQTABLEVIEW_HPP

#include <QTableView>
#include "VectorISATableView.hpp"

class QSequentialAnimationGroup;

class SysReqTableModel : public QAbstractTableModel
{
    Q_OBJECT
    uint64_t m_cpuSpeed = 0;
    QString m_cpuSpeedStr;
    uint64_t m_memorySize = 0;
    QString m_memorySizeStr;
#if __APPLE__
    int m_macosMajor = 0;
    int m_macosMinor = 0;
    int m_macosPatch = 0;
#endif
    QString m_osVersion;
    bool m_is64Bit = true;
public:
    SysReqTableModel(QObject* parent = Q_NULLPTR);
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
};

class SysReqTableView : public QTableView
{
    Q_OBJECT
    SysReqTableModel m_model;
    VectorISATableView m_vectorISATable;
    std::tuple<QWidget*, QSequentialAnimationGroup*, bool> m_backgroundWidgets[4] = {};
public:
    SysReqTableView(QWidget* parent = Q_NULLPTR);
    void paintEvent(QPaintEvent* e) Q_DECL_OVERRIDE;
};

#endif // GUI_SYSREQTABLEVIEW_HPP
