#ifndef SMARKER_H
#define SMARKER_H

#include <QSettings>

extern QSettings settings;


//structure for save markers data
struct SMarker
{
    SMarker()
    {
        east = 0;
        north = 0;
        caption = "";
    };
    SMarker(double _east, double _north, QString _caption)
    {
        east = _east; north = _north; caption = _caption;
    };

    double east;
    double north;
    QString caption;
};


#endif // SMARKER_H
