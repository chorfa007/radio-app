#include "RadioModel.h"

RadioModel::RadioModel(QObject* parent)
    : QAbstractListModel(parent) {}

void RadioModel::setRadios(const std::vector<Radio>& radios) {
    beginResetModel();
    m_radios = radios;
    endResetModel();
    emit countChanged();
}

int RadioModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_radios.size());
}

QVariant RadioModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= rowCount()) return {};
    const auto& r = m_radios[index.row()];
    switch (role) {
        case IdRole:          return r.id;
        case NameRole:        return QString::fromStdString(r.name);
        case StreamUrlRole:   return QString::fromStdString(r.streamUrl);
        case GenreRole:       return QString::fromStdString(r.genre);
        case DescriptionRole: return QString::fromStdString(r.description);
        case LogoUrlRole:     return QString::fromStdString(r.logoUrl);
        case CountryIdRole:   return r.countryId;
        default:              return {};
    }
}

QHash<int, QByteArray> RadioModel::roleNames() const {
    return {
        { IdRole,          "stationId"   },
        { NameRole,        "stationName" },
        { StreamUrlRole,   "streamUrl"   },
        { GenreRole,       "genre"       },
        { DescriptionRole, "description" },
        { LogoUrlRole,     "logoUrl"     },
        { CountryIdRole,   "countryId"   },
    };
}
