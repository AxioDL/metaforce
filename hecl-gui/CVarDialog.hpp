#ifndef CVARDIALOG_H
#define CVARDIALOG_H

#include <QDialog>
#include <QStringListModel>
#include <QAbstractButton>
#include <QItemSelection>

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

#endif // CVARDIALOG_H
