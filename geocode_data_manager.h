#ifndef GEOCODE_DATA_MANAGER_H
#define GEOCODE_DATA_MANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include "smarker.h"

extern QSettings settings;

enum MarkerType { Adres, Levering, Ophaalpunt };

class GeocodeDataManager : public QObject
{
    Q_OBJECT
public:
    explicit GeocodeDataManager(QObject *parent = 0);
    ~GeocodeDataManager();

    void getCoordinates(const QString& address);
    void pushListOfMarkers(QList<SOphaalpunt> *);
    void pushLevering(SLevering);

signals:
    void errorOccured(const QString&);
    void coordinatesReady(double east, double north, QString markername);
    void coordinatesReady(double east, double north, SOphaalpunt ophaalpunt);
    void coordinatesReady(double east, double north, SLevering levering);
    void markerDone();

private slots:
    void replyFinished(QNetworkReply* reply);
    void giveNextMarker();

private:
    QNetworkAccessManager* m_pNetworkAccessManager;
    QList<SOphaalpunt> *markersToBeDone;
    SLevering leveringToBeDone;
    MarkerType marker_type;
    QString name_of_marker;
    SOphaalpunt ophaalpunt_to_mark;
};

#endif // GEOCODE_DATA_MANAGER_H
