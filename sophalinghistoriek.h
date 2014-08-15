#ifndef SOPHAALHISTORIEK_H
#define SOPHAALHISTORIEK_H

#include <QDate>
#include <QDebug>
#include <QSettings>

extern QSettings settings;

//structure to save ophaling data
struct SOphalingHistoriek
{
    SOphalingHistoriek()
    {
        historiek_id = -1;
        ophalingsdatum = QDate();
        chauffeur = "";
        ophaalpunt_id = -1;
        kg_kurk = 0;
        kg_kaarsresten = 0;
        zakken_kurk = 0;
        zakken_kaarsresten = 0;
        opmerkingen = "";
    };

    SOphalingHistoriek(int _historiek_id, QDate _ophalingsdatum, QString _chauffeur, int _ophaalpunt_id, double _kg_kurk, double _kg_kaarsresten, double _zakken_kurk, double _zakken_kaarsresten, QString _opmerkingen)
    {
        historiek_id = _historiek_id;
        ophalingsdatum = _ophalingsdatum;
        chauffeur = _chauffeur;
        ophaalpunt_id = _ophaalpunt_id;
        kg_kurk = _kg_kurk;
        kg_kaarsresten = _kg_kaarsresten;
        zakken_kurk = _zakken_kurk;
        zakken_kaarsresten = _zakken_kaarsresten;
        opmerkingen = _opmerkingen;
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
        qDebug() << "Ophalinghistoriek: ( #" << historiek_id << ")";
        qDebug() << "... datum      :" << QLocale().toString(ophalingsdatum,"dd MMM yyyy");
        qDebug() << "... chauffeur  :" << chauffeur;
        qDebug() << "... ophaalpunt :" << ophaalpunt_id;
        qDebug() << "... kurk       :" << kg_kurk << "kg, "<< zakken_kurk << "zakken";
        qDebug() << "... kaars      :" << kg_kaarsresten << "kg, "<< zakken_kaarsresten << "zakken";
        qDebug() << "... gewicht    :" << getWeight();
        qDebug() << "... volume     :" << getVolume();
        qDebug() << "... opmerkingen:" << opmerkingen;
    }

    int historiek_id;
    QDate ophalingsdatum;
    QString chauffeur;
    int ophaalpunt_id;
    double kg_kurk;
    double kg_kaarsresten;
    double zakken_kurk;
    double zakken_kaarsresten;
    QString opmerkingen;
};

#endif // SOPHAALHISTORIEK_H
