#ifndef SMARKER_H
#define SMARKER_H

#include <QSettings>
#include "sophaalpunt.h"

extern QSettings settings;

enum MarkerType { Adres, Levering, Ophaalpunt };

//structure for save markers data
struct SMarker
{
    SMarker()
    {
        east = 0;
        north = 0;
        caption = "";
        distancematrixindex = -1;
        marker_type = Adres;
    };

    SMarker(double _east, double _north, QString _caption)
    {
        east = _east; north = _north; caption = _caption; distancematrixindex = -1; marker_type = Adres;
    };

    SMarker(double _east, double _north, SOphaalpunt _ophaalpunt)
    {
        east = _east; north = _north; caption = QString("%1, %2").arg(_ophaalpunt.naam).arg(_ophaalpunt.adres); ophaalpunt = _ophaalpunt; distancematrixindex = -1; marker_type = Ophaalpunt;
    };

    double east;
    double north;
    QString caption;
    int distancematrixindex;

    MarkerType marker_type;
    SOphaalpunt ophaalpunt;
};


#endif // SMARKER_H
