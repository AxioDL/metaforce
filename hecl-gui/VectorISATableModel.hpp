#ifndef GUI_VECTORISATABLEMODEL_HPP
#define GUI_VECTORISATABLEMODEL_HPP

#include <QTableView>
#include "zeus/Math.hpp"

class VectorISATableModel : public QAbstractTableModel
{
    Q_OBJECT
protected:
    const zeus::CPUInfo& m_features = zeus::cpuFeatures();
public:
    VectorISATableModel(QObject* parent = Q_NULLPTR) : QAbstractTableModel(parent) {}
    int rowCount(const QModelIndex& parent = QModelIndex()) const { return 1; }
};

#endif // GUI_VECTORISATABLEMODEL_HPP
