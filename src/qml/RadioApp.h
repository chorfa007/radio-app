#pragma once
#include "../database/Database.h"
#include "CountryModel.h"
#include "RadioModel.h"
#include "PlayerController.h"
#include <QObject>
#include <QString>

class RadioApp : public QObject {
    Q_OBJECT

    Q_PROPERTY(CountryModel*    countries          READ countries          CONSTANT)
    Q_PROPERTY(RadioModel*      radios             READ radios             CONSTANT)
    Q_PROPERTY(PlayerController* player            READ player             CONSTANT)
    Q_PROPERTY(int              selectedCountryId  READ selectedCountryId  NOTIFY selectedCountryChanged)
    Q_PROPERTY(QString          selectedCountryName READ selectedCountryName NOTIFY selectedCountryChanged)
    Q_PROPERTY(QString          selectedCountryFlag READ selectedCountryFlag NOTIFY selectedCountryChanged)

public:
    explicit RadioApp(Database& db, QObject* parent = nullptr);

    CountryModel*    countries()           const { return m_countryModel; }
    RadioModel*      radios()              const { return m_radioModel;   }
    PlayerController* player()            const { return m_player;        }
    int              selectedCountryId()   const { return m_selectedId;   }
    QString          selectedCountryName() const { return m_selectedName; }
    QString          selectedCountryFlag() const { return m_selectedFlag; }

    Q_INVOKABLE void selectCountry(int countryId,
                                   const QString& name,
                                   const QString& flag);

signals:
    void selectedCountryChanged();

private:
    Database&        m_db;
    CountryModel*    m_countryModel{nullptr};
    RadioModel*      m_radioModel{nullptr};
    PlayerController* m_player{nullptr};

    int     m_selectedId{-1};
    QString m_selectedName;
    QString m_selectedFlag;
};
