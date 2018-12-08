#pragma once

#include "VectorISATableModel.hpp"

class VectorISATableModelIntel : public VectorISATableModel {
  Q_OBJECT
public:
  VectorISATableModelIntel(QObject* parent = Q_NULLPTR) : VectorISATableModel(parent) {}

  int columnCount(const QModelIndex& parent = QModelIndex()) const { return 7; }

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const {
    if (role != Qt::DisplayRole && role != Qt::UserRole)
      return {};

    if (role == Qt::UserRole) {
      switch (index.column()) {
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
    } else {
      switch (index.column()) {
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

  VectorISA getISA(int idx) const {
    switch (idx) {
    default:
      return VectorISA::Invalid;
    case 0:
      return VectorISA::X87;
    case 1:
      return VectorISA::SSE;
    case 2:
      return VectorISA::SSE2;
    case 3:
      return VectorISA::SSE3;
    case 4:
      return VectorISA::SSE41;
    case 5:
      return VectorISA::AVX;
    case 6:
      return VectorISA::AVX2;
    }
  }

  bool willRun(VectorISA visa) const {
    switch (visa) {
    default:
      return false;
    case VectorISA::X87:
      return true;
    case VectorISA::SSE:
      return m_features.SSE1;
    case VectorISA::SSE2:
      return m_features.SSE2;
    case VectorISA::SSE3:
      return m_features.SSE3;
    case VectorISA::SSE41:
      return m_features.SSE41;
    case VectorISA::AVX:
      return m_features.AVX;
    case VectorISA::AVX2:
      return m_features.AVX2;
    }
  }
};
