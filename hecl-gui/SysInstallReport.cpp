#include "SysInstallReport.hpp"
#include <QFontDatabase>
#include <QPushButton>
#include <QFormLayout>

SysInstallReport::SysInstallReport(QWidget* parent)
: QWidget(parent)
{
    QFormLayout* formLayout = new QFormLayout(this);

    QString labelText;
    labelText.sprintf("Recommended URDE %s:", installEntity().toUtf8().data());
    m_installFile = new QLabel("fetching...", this);
    QFont mFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    mFont.setPointSize(m_installFile->font().pointSize());
    m_installFile->setFont(mFont);
    formLayout->addRow(labelText, m_installFile);

    QHBoxLayout* buttonLayout = new QHBoxLayout(this);

    QPushButton* b1 = new QPushButton("Install In Applications", this);
    b1->setEnabled(false);
    buttonLayout->addWidget(b1);

    QPushButton* b2 = new QPushButton("Just Download", this);
    b2->setEnabled(false);
    buttonLayout->addWidget(b2);

    formLayout->addRow(buttonLayout);
}
