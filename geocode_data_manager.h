#ifndef GEOCODE_DATA_MANAGER_H
#define GEOCODE_DATA_MANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>

extern QSettings settings;

class GeocodeDataManager : public QObject
{
    Q_OBJECT
public:
    explicit GeocodeDataManager(QObject *parent = 0);
    ~GeocodeDataManager();

    void getCoordinates(const QString& address);
    void pushListOfMarkers(QList<QString> *);

signals:
    void errorOccured(const QString&);
    void coordinatesReady(double east, double north, QString markername);
    void markerDone();

private slots:
    void replyFinished(QNetworkReply* reply);
    void giveNextMarker();

private:
    QNetworkAccessManager* m_pNetworkAccessManager;
    QList<QString> *markersToBeDone;
    QString name_of_marker;


};

#endif // GEOCODE_DATA_MANAGER_H
