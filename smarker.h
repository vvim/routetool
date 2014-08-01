#ifndef SMARKER_H
#define SMARKER_H

#include <QSettings>
#include <QDebug>
#include "sophaalpunt.h"
#include "slevering.h"

extern QSettings settings;

//structure for save markers data
struct SMarker
{
    SMarker()
    {
        east = 0;
        north = 0;
        caption = "";
        distancematrixindex = -1;
        ophaling = false;
        levering = false;
    };

    SMarker(double _east, double _north, QString _caption)
    {
        east = _east; north = _north; caption = _caption; distancematrixindex = -1;
        ophaling = false; levering = false;
    };

    SMarker(double _east, double _north, SOphaalpunt _ophaalpunt)
    {
        east = _east; north = _north; caption = QString("%1, %2").arg(_ophaalpunt.naam).arg(_ophaalpunt.adres); ophaalpunt = _ophaalpunt; distancematrixindex = -1;
        ophaling = true; levering = false;
    };

    SMarker(double _east, double _north, SLevering _leveringspunt)
    {
        east = _east; north = _north; caption = _leveringspunt.getNameAndAddress(); leveringspunt = _leveringspunt; distancematrixindex = -1;
        ophaling = false; levering = true;
    };

    void PrintInformation()
    {
        qDebug() << "." << caption << "(" << east << north << ")" << "matrix:" << distancematrixindex;
        if((!ophaling) && (!levering))
        {
            qDebug() << ". type: Adres";
        }
        if(ophaling)
        {
            ophaalpunt.PrintInformation();
        }
        if(levering)
        {
            leveringspunt.PrintInformation();
        }
    };


    double east;
    double north;
    QString caption;
    int distancematrixindex;

    bool ophaling;
    bool levering;

    SOphaalpunt ophaalpunt;
    SLevering leveringspunt;
};


#endif // SMARKER_H
