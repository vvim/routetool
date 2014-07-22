#include "geocode_data_manager.h"

#include <QJson/Parser>
#include <QMessageBox>
#include <QDebug>


GeocodeDataManager::GeocodeDataManager(QObject *parent) :
    QObject(parent)
{
    m_pNetworkAccessManager = new QNetworkAccessManager(this);
    markersToBeDone = new QList<QString>();
    connect(m_pNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    connect(this, SIGNAL(coordinatesReady(double, double, QString)), this, SLOT(giveNextMarker()));
}

void GeocodeDataManager::getCoordinates(const QString& address)
{
    name_of_marker = address;
    name_of_marker.replace("&","+");
    name_of_marker.replace("\n",",");

    QString address_encoded = name_of_marker;
    address_encoded.replace(" ","+");
    QString url = QString("https://maps.googleapis.com/maps/api/geocode/json?address=%1&key=%2&oe=utf8&sensor=false").arg(address_encoded).arg(settings.value("apiKey").toString());

qDebug() << "<vvim>" << "would the distance matrix work if we would only put the NAME of the ophaalpunt here, and NOT the address? based on the coordinates???";
    m_pNetworkAccessManager->get(QNetworkRequest(QUrl(url)));
}


void GeocodeDataManager::replyFinished(QNetworkReply* reply)
{
    QString json = reply->readAll();
    //qDebug() << "Reply = " << json;
    qDebug() << "URL = " << reply->url();
    QString strUrl = reply->url().toString();

    QJson::Parser parser;

    bool ok;

    // json is a QString containing the data to convert
    QVariant result = parser.parse (json.toLatin1(), &ok);
    if(!ok)
    {
        emit errorOccured(QString("Cannot convert to QJson object: %1").arg(json));
        return;
    }

/* old version of Geocoding API Application v2
    int code = result.toMap()["Status"].toMap()["code"].toInt();
    if(code != 200)
    {
        emit errorOccured(QString("Code of request is: %1").arg(code));
        return;
    }
*/

    // <vvim> upgrade to v3
    if(result.toMap()["status"].toString() != "OK")
    {
        emit errorOccured(QString("Code of request is: %1").arg(result.toMap()["status"].toString()));
        return;
    }
    // </vvim>


/* old version of Geocoding API Application v2
    QVariantList placeMarks = result.toMap()["Placemark"].toList();
    if(placeMarks.count() == 0)
    {
        emit errorOccured(QString("Cannot find any locations"));
        return;
    }

    double east  = placeMarks[0].toMap()["Point"].toMap()["coordinates"].toList()[0].toDouble();
    double north = placeMarks[0].toMap()["Point"].toMap()["coordinates"].toList()[1].toDouble();

    emit coordinatesReady(east, north);
*/

    // <vvim> upgrade to v3
    QVariantList nestedList = result.toMap()["results"].toList();
    QVariantList::Iterator it = nestedList.begin();
    if(it == nestedList.end())
    {
        emit errorOccured(QString("Cannot find any locations"));
        return;
    }
    QVariantMap locationOfResult = (*it).toMap()["geometry"].toMap()["location"].toMap();
    qDebug() << "location" << locationOfResult["lng"].toDouble() << locationOfResult["lat"].toDouble();
/**
    QVariantMap address_name = (*it).toMap();
    QString markerName = address_name["formatted_address"].toString();
    qDebug() << "<vvim>: markername: " << markerName << "dit is enkel het adres, ook nog de naam van het opaahlpunt invullen!! Best met Inherited Classes? en een int om te differentiëren: 1) ophaalpunt 2) leveringsadres 3) gewoon een adres";
    emit coordinatesReady(locationOfResult["lng"].toDouble(), locationOfResult["lat"].toDouble(),markerName); // <vvim: handel omdraaien :-) eerst lng dan lat>
**/

    emit coordinatesReady(locationOfResult["lng"].toDouble(), locationOfResult["lat"].toDouble(),name_of_marker); // <vvim: handel omdraaien :-) eerst lng dan lat>
//    qDebug() << "\n\n  * * do we ever reach here??? * * \n\n";
//    emit markerDone();
    // </vvim>
}

void GeocodeDataManager::pushListOfMarkers(QList<QString> *list_of_markers)
{
    markersToBeDone = list_of_markers;

    qDebug() << "markers to be done:" << markersToBeDone->size();
    foreach(QString marker, *markersToBeDone)
    {
        qDebug() << marker;
    }

    if(!markersToBeDone->empty())
    {
        QString firstmarker = markersToBeDone->takeFirst(); // Removes the first item in the list and returns it.
        getCoordinates(firstmarker);
    }
}

void GeocodeDataManager::giveNextMarker()
{
    if(!markersToBeDone->empty()) // markersToBeDone: not declared yet???
    {
        qDebug() << "markers to be done:" << markersToBeDone->size();
        foreach(QString marker, *markersToBeDone)
        {
            qDebug() << marker;
        }
        qDebug() << "<vvim>: hier een korte pauze inlassen zodat de naam van de marker correct blijft?";
        QString firstmarker = markersToBeDone->takeFirst(); // Removes the first item in the list and returns it.
        getCoordinates(firstmarker);
    }
    else
        qDebug() << "shit is empty, nuttin' to see here boy!";
}

GeocodeDataManager::~GeocodeDataManager()
{
    delete markersToBeDone;
}
