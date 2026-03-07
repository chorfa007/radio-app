#include "CountryModel.h"

CountryModel::CountryModel(QObject* parent)
    : QAbstractListModel(parent) {}

void CountryModel::setCountries(const std::vector<Country>& countries) {
    beginResetModel();
    m_countries = countries;
    endResetModel();
}

int CountryModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_countries.size());
}

QVariant CountryModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= rowCount()) return {};
    const auto& c = m_countries[index.row()];
    switch (role) {
        case IdRole:   return c.id;
        case NameRole: return QString::fromStdString(c.name);
        case CodeRole: return QString::fromStdString(c.code);
        case FlagRole: return QString::fromStdString(c.flag);
        default:       return {};
    }
}

QHash<int, QByteArray> CountryModel::roleNames() const {
    return {
        { IdRole,   "id"   },
        { NameRole, "name" },
        { CodeRole, "code" },
        { FlagRole, "flag" },
    };
}
