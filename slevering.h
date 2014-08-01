#ifndef SLEVERING_H
#define SLEVERING_H

#include <QString>
#include <QDebug>

//structure for save levering data
struct SLevering
{
    SLevering()
    {
        name = "";
        street = "";
        housenr = "";
        busnr = "";
        postalcode = "";
        plaats = "";
        country = "";
        contactperson = "";
        telephone = "";
        weight = 0;
        volume = 0;
        timeneeded = 0;
        // ophaalpunt_id? -> later?
    };

    SLevering(QString _name, QString _street, QString _housenr, QString _busnr, QString _postalcode, QString _plaats, QString _country, QString _contactperson, QString _telephone, double _weight, double _volume, double _timeneeded)
    {
        name = _name;
        street = _street;
        housenr = _housenr;
        busnr = _busnr;
        postalcode = _postalcode;
        plaats = _plaats;
        country = _country;
        contactperson = _contactperson;
        telephone = _telephone;
        weight = _weight;
        volume = _volume;
        timeneeded = _timeneeded;
        // ophaalpunt_id? -> later?
    };

    QString getNameAndAddress()
    {
        QString nameandaddress = "";

        if(!name.trimmed().isEmpty())
            nameandaddress.append(name).append(", ");

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
        qDebug() << ". type: Levering"; //  ophaalpunt_id? -> later?
        qDebug() << "... name       :" << name;
        qDebug() << "... street     :" << street;
        qDebug() << "... housenr    :" << housenr;
        qDebug() << "... busnr      :" << busnr;
        qDebug() << "... postalcode :" << postalcode;
        qDebug() << "... plaats     :" << plaats;
        qDebug() << "... country    :" << country;
        qDebug() << "... ... getAddress()       :" << getAddress();
        qDebug() << "... ... getNameAndAddress():" << getNameAndAddress();
        qDebug() << "... contactperson:" << contactperson;
        qDebug() << "... telephone    :" << telephone;
        qDebug() << "... weight :" << weight;
        qDebug() << "... volume :" << volume;
        qDebug() << "... timeneeded   :" << timeneeded << "minutes";
    };

    QString name;
    QString street;
    QString housenr;
    QString busnr;
    QString postalcode;
    QString plaats;
    QString country;
    QString contactperson;
    QString telephone;
    double weight;
    double volume;
    double timeneeded;
    // ophaalpunt_id? -> later?
};
#endif // SLEVERING_H
