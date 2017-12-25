#ifndef GUI_VECTORISATABLEMODELINTEL_HPP
#define GUI_VECTORISATABLEMODELINTEL_HPP

#include "VectorISATableModel.hpp"

class VectorISATableModelIntel : public VectorISATableModel
{
    Q_OBJECT
public:
    VectorISATableModelIntel(QObject* parent = Q_NULLPTR) : VectorISATableModel(parent) {}

    int columnCount(const QModelIndex &parent = QModelIndex()) const { return 7; }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        if (role != Qt::DisplayRole && role != Qt::UserRole)
            return {};

        if (role == Qt::UserRole)
        {
            switch (index.column())
            {
            case 0:
            default:
                return true;
            case 1:
                return m_features.SSE1;
            case 2:
                return m_features.SSE2;
            case 3:
                return m_features.SSE3;
            case 4:
                return m_features.SSE41;
            case 5:
                return m_features.AVX;
            case 6:
                return m_features.AVX2;
            }
        }
        else
        {
            switch (index.column())
            {
            case 0:
            default:
                return QStringLiteral("x87");
            case 1:
                return QStringLiteral("SSE");
            case 2:
                return QStringLiteral("SSE2");
            case 3:
                return QStringLiteral("SSE3");
            case 4:
                return QStringLiteral("SSE4.1");
            case 5:
                return QStringLiteral("AVX");
            case 6:
                return QStringLiteral("AVX2");
            }
        }
    }

    QString getISAString(int idx) const
    {
        switch (idx)
        {
        case 0:
        default:
            return QStringLiteral("x87");
        case 1:
            return QStringLiteral("sse");
        case 2:
            return QStringLiteral("sse2");
        case 3:
            return QStringLiteral("sse3");
        case 4:
            return QStringLiteral("sse41");
        case 5:
            return QStringLiteral("avx");
        case 6:
            return QStringLiteral("avx2");
        }
    }
};

#endif // GUI_VECTORISATABLEMODELINTEL_HPP
