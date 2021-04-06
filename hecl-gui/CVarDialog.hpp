#pragma once

#include <QAbstractButton>
#include <QDialog>
#include <QItemSelection>
#include <QStringListModel>
#include <memory>

namespace Ui {
class CVarDialog;
} // namespace Ui

class CVarDialog : public QDialog {
  Q_OBJECT

public:
  explicit CVarDialog(QWidget* parent = nullptr);
  ~CVarDialog() override;

  QString textValue();

private:
  std::unique_ptr<Ui::CVarDialog> m_ui;
  QStringListModel m_model;

private slots:
  void on_buttonBox_accepted();
  void on_buttonBox_rejected();
  void handleSelectionChanged(const QItemSelection& selection);
};
