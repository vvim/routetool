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
        distancematrixindex = -1;
    };
    SMarker(double _east, double _north, QString _caption)
    {
        east = _east; north = _north; caption = _caption; distancematrixindex = -1;
    };

    double east;
    double north;
    QString caption;
    int distancematrixindex;
};


#endif // SMARKER_H
