#include "geocode_data_manager.h"

#include <QJson/Parser>
#include <QMessageBox>
#include <QDebug>


GeocodeDataManager::GeocodeDataManager(QObject *parent) :
    QObject(parent)
{
    m_pNetworkAccessManager = new QNetworkAccessManager(this);
    connect(m_pNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
}

void GeocodeDataManager::getCoordinates(const QString& address)
{
//    QString url = QString("http://maps.google.com/maps/geo?q=%1&key=%2&output=json&oe=utf8&sensor=false").arg(address).arg(apiKey);
    QString url = QString("https://maps.googleapis.com/maps/api/geocode/json?address=%1&key=%2&oe=utf8&sensor=false").arg(address).arg(settings.value("apiKey").toString());
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

    emit coordinatesReady(locationOfResult["lng"].toDouble(), locationOfResult["lat"].toDouble()); // <vvim: handel omdraaien :-) eerst lng dan lat>
    // </vvim>

}
