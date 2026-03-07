#pragma once
#include "../models/Radio.h"
#include <QAbstractListModel>
#include <vector>

class RadioModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum Roles {
        IdRole          = Qt::UserRole + 1,
        NameRole,
        StreamUrlRole,
        GenreRole,
        DescriptionRole,
        LogoUrlRole,
        CountryIdRole
    };
    Q_ENUM(Roles)

    explicit RadioModel(QObject* parent = nullptr);

    void setRadios(const std::vector<Radio>& radios);

    int      rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

signals:
    void countChanged();

private:
    std::vector<Radio> m_radios;
};
