#pragma once

#include <QTableView>

class QSequentialAnimationGroup;

class SysReqTableModel : public QAbstractTableModel {
  Q_OBJECT
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
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  bool isBlenderVersionOk() const;
  void updateFreeDiskSpace(const QString& path);
};

class SysReqTableView : public QTableView {
  Q_OBJECT
  SysReqTableModel m_model;
  std::tuple<QWidget*, QSequentialAnimationGroup*, bool> m_backgroundWidgets[6] = {};

public:
  SysReqTableView(QWidget* parent = Q_NULLPTR);
  void paintEvent(QPaintEvent* e) override;
  const SysReqTableModel& getModel() const { return m_model; }
  bool isBlenderVersionOk() const { return m_model.isBlenderVersionOk(); }
  void updateFreeDiskSpace(const QString& path) { m_model.updateFreeDiskSpace(path); }
};
