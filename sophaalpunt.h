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
        street = "";
        housenr = "";
        busnr = "";
        postalcode = "";
        plaats = "";
        country = "";
        kg_kurk = 0;
        kg_kaarsresten = 0;
        zakken_kurk = 0;
        zakken_kaarsresten = 0;
        aanmelding_id = -1;
        ophaalpunt_id = -1;
        opmerkingen = "";
        lat = 0.0;
        lng = 0.0;
    };

    SOphaalpunt(QString _naam, QString _street, QString _housenr, QString _busnr, QString _postalcode, QString _plaats, QString _country, double _kg_kurk, double _kg_kaarsresten, double _zakken_kurk, double _zakken_kaarsresten, int _aanmelding_id, int _ophaalpunt_id, QString _opmerkingen, double _lat = 0.0, double _lng = 0.0)
    {
        naam = _naam;
        kg_kurk = _kg_kurk;
        kg_kaarsresten = _kg_kaarsresten;
        zakken_kurk = _zakken_kurk;
        zakken_kaarsresten = _zakken_kaarsresten;
        street = _street;
        housenr = _housenr;
        busnr = _busnr;
        postalcode = _postalcode;
        plaats = _plaats;
        country = _country;
        aanmelding_id = _aanmelding_id;
        ophaalpunt_id = _ophaalpunt_id;
        opmerkingen = _opmerkingen;
        lat = _lat;
        lng = _lng;
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

    QString getNameAndAddress()
    {
        QString nameandaddress = "";

        if(!naam.trimmed().isEmpty())
            nameandaddress.append(naam).append(", ");

        nameandaddress.append(getAddress());

        return nameandaddress;
    };

    QString getAddress()
    {
        QString address = "";

        address.append(street).append(" ").append(housenr);

        if(!busnr.trimmed().isEmpty())
            address.append(" bus ").append(busnr);

        address.append(", ").append(postalcode).append(" ").append(plaats).append(", ").append(country);

        return address;
    };

    void PrintInformation()
    {
        qDebug() << ". type: Ophaalpunt #" << ophaalpunt_id << "( aanmelding:" << aanmelding_id << ")";
        qDebug() << "... naam       :" << naam;
        qDebug() << "... street     :" << street;
        qDebug() << "... housenr    :" << housenr;
        qDebug() << "... busnr      :" << busnr;
        qDebug() << "... postalcode :" << postalcode;
        qDebug() << "... plaats     :" << plaats;
        qDebug() << "... country    :" << country;
        qDebug() << "... ... getAddress()       :" << getAddress();
        qDebug() << "... ... getNameAndAddress():" << getNameAndAddress();
        qDebug() << "..." << opmerkingen;
        qDebug() << "... kurk: " << kg_kurk << "kg, "<< zakken_kurk << "zakken";
        qDebug() << "... kaars: " << kg_kaarsresten << "kg, "<< zakken_kaarsresten << "zakken";
    }


    int getOphaalpuntId()
    {
        return ophaalpunt_id;
    }

    double getLatitude()
    {
        return lat;
    }

    double getLongitude()
    {
        return lng;
    }

    QString naam;
    double kg_kurk;
    double kg_kaarsresten;
    double zakken_kurk;
    double zakken_kaarsresten;
    QString street;
    QString housenr;
    QString busnr;
    QString postalcode;
    QString plaats;
    QString country;
    int aanmelding_id;
    int ophaalpunt_id;
    QString opmerkingen;
    double lat;
    double lng;
};

#endif // SOPHAALPUNT_H
