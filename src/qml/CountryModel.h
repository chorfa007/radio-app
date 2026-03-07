#pragma once
#include "../models/Country.h"
#include <QAbstractListModel>
#include <vector>

class CountryModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        IdRole   = Qt::UserRole + 1,
        NameRole,
        CodeRole,
        FlagRole
    };
    Q_ENUM(Roles)

    explicit CountryModel(QObject* parent = nullptr);

    void setCountries(const std::vector<Country>& countries);

    int     rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    std::vector<Country> m_countries;
};
