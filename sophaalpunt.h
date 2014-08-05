#ifndef SOPHAALPUNT_H
#define SOPHAALPUNT_H

#include <QSettings>
#include <QDebug>

extern QSettings settings;


//structure for save ophaalpunt data
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
        aanmelding_id = -1;
        ophaalpunt_id = -1;
    };

    SOphaalpunt(QString _naam, double _kg_kurk, double _kg_kaarsresten, double _zakken_kurk, double _zakken_kaarsresten, QString _adres, int _aanmelding_id, int _ophaalpunt_id)
    {
        naam = _naam;
        kg_kurk = _kg_kurk;
        kg_kaarsresten = _kg_kaarsresten;
        zakken_kurk = _zakken_kurk;
        zakken_kaarsresten = _zakken_kaarsresten;
        adres = _adres;
        aanmelding_id = _aanmelding_id;
        ophaalpunt_id = _ophaalpunt_id;
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

    void PrintInformation()
    {
        qDebug() << ". type: Ophaalpunt" << ophaalpunt_id << "( aanmelding:" << aanmelding_id << ")";
        qDebug() << "..." << naam;
        qDebug() << "..." << adres;
        qDebug() << "... kurk: " << kg_kurk << "kg, "<< zakken_kurk << "zakken";
        qDebug() << "... kaars: " << kg_kaarsresten << "kg, "<< zakken_kaarsresten << "zakken";
    }

    QString naam;
    double kg_kurk;
    double kg_kaarsresten;
    double zakken_kurk;
    double zakken_kaarsresten;
    QString adres;
    int aanmelding_id;
    int ophaalpunt_id;
};

#endif // SOPHAALPUNT_H
