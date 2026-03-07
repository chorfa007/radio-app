#include "RadioApp.h"

RadioApp::RadioApp(Database& db, QObject* parent)
    : QObject(parent)
    , m_db(db)
    , m_countryModel(new CountryModel(this))
    , m_radioModel(new RadioModel(this))
    , m_player(new PlayerController(this))
{
    m_countryModel->setCountries(m_db.getCountries());
}

void RadioApp::selectCountry(int countryId, const QString& name, const QString& flag) {
    m_selectedId   = countryId;
    m_selectedName = name;
    m_selectedFlag = flag;
    m_radioModel->setRadios(m_db.getRadiosByCountry(countryId));
    emit selectedCountryChanged();
}
