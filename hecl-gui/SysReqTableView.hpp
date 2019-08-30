#pragma once

#include <QTableView>
#include "VectorISATableView.hpp"

class QSequentialAnimationGroup;

class SysReqTableModel : public QAbstractTableModel {
  Q_OBJECT
  uint64_t m_cpuSpeed = 0;
  QString m_cpuSpeedStr;
  uint64_t m_memorySize = 0;
  QString m_memorySizeStr;
  qint64 m_freeDiskSpace = 0;
  QString m_freeDiskSpaceStr = tr("<Set Working Directory>");
#if __APPLE__
  int m_macosMajor = 0;
  int m_macosMinor = 0;
  int m_macosPatch = 0;
#elif _WIN32
  bool m_win7SP1OrGreater = false;
#endif
  QString m_osVersion;
  int m_blendMajor = 0;
  int m_blendMinor = 0;
  QString m_blendVersionStr;

public:
  SysReqTableModel(QObject* parent = Q_NULLPTR);
  int rowCount(const QModelIndex& parent = QModelIndex()) const;
  int columnCount(const QModelIndex& parent = QModelIndex()) const;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  bool isBlenderVersionOk() const { return m_blendMajor > 2 || (m_blendMajor == 2 && m_blendMinor >= 78); }
  void updateFreeDiskSpace(const QString& path);
};

class SysReqTableView : public QTableView {
  Q_OBJECT
  SysReqTableModel m_model;
  VectorISATableView m_vectorISATable;
  std::tuple<QWidget*, QSequentialAnimationGroup*, bool> m_backgroundWidgets[6] = {};

public:
  SysReqTableView(QWidget* parent = Q_NULLPTR);
  void paintEvent(QPaintEvent* e) Q_DECL_OVERRIDE;
  const SysReqTableModel& getModel() const { return m_model; }
  const VectorISATableView& getVectorISATable() const { return m_vectorISATable; }
  bool willRun(const URDEVersion& v) const {
    return v.getArchitecture() == CurArchitecture && v.getPlatform() == CurPlatform &&
           m_vectorISATable.willRun(v.getVectorISA());
  }
  bool isBlenderVersionOk() const { return m_model.isBlenderVersionOk(); }
  void updateFreeDiskSpace(const QString& path) { m_model.updateFreeDiskSpace(path); }
};
