#ifndef SOPHAALPUNT_H
#define SOPHAALPUNT_H

#include <QSettings>

extern QSettings settings;


//structure for save markers data
struct SOphaalpunt
{
    SOphaalpunt()
    {
        naam = "";
        kg_kurk = 0;
        kg_kaarsresten = 0;
        zakken_kurk = 0;
        zakken_kaarsresten = 0;
        adres = "";
        id = -1;
    };

    SOphaalpunt(QString _naam, double _kg_kurk, double _kg_kaarsresten, double _zakken_kurk, double _zakken_kaarsresten, QString _adres, int _id)
    {
        naam = _naam;
        kg_kurk = _kg_kurk;
        kg_kaarsresten = _kg_kaarsresten;
        zakken_kurk = _zakken_kurk;
        zakken_kaarsresten = _zakken_kaarsresten;
        adres = _adres;
        id = _id;
    };

    double getVolume()
    {
        double volume = (zakken_kurk * settings.value("zak_kurk_volume").toDouble()) + (zakken_kaarsresten * settings.value("zak_kaarsresten_volume").toDouble());
        return volume;
    }

    double getWeight()
    {
        return kg_kurk + kg_kaarsresten;
    }

    QString naam;
    double kg_kurk;
    double kg_kaarsresten;
    double zakken_kurk;
    double zakken_kaarsresten;
    QString adres;
    int id;
};

#endif // SOPHAALPUNT_H
