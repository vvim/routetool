#include "geocode_data_manager.h"

#include <QJson/Parser>
#include <QMessageBox>
#include <QDebug>

#define vvimDebug()\
    qDebug() << "[" << Q_FUNC_INFO << "]"

GeocodeDataManager::GeocodeDataManager(QObject *parent) :
    QObject(parent)
{
    m_pNetworkAccessManager = new QNetworkAccessManager(this);
    markersToBeDone = new QList<SOphaalpunt>(); // needed so that we can delete it at the first call of pushListOfMarkers() + for the empty()-check at giveNextMarker()
    connect(m_pNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    connect(this, SIGNAL(coordinatesReady(double, double, QString)), this, SLOT(giveNextMarker()));
    connect(this, SIGNAL(coordinatesReady(double, double, SOphaalpunt)), this, SLOT(giveNextMarker()));
    marker_type = Adres;
}

void GeocodeDataManager::getCoordinates(const QString& address)
{
    name_of_marker = address;
    name_of_marker.replace("&","+");
    name_of_marker.replace("\n",",");

    QString address_encoded = name_of_marker;
    address_encoded.replace(" ","+");
    QString url = QString("https://maps.googleapis.com/maps/api/geocode/json?address=%1&key=%2&oe=utf8&sensor=false").arg(address_encoded).arg(settings.value("apiKey").toString());

vvimDebug() << "<vvim>" << "would the distance matrix work if we would only put the NAME of the ophaalpunt here, and NOT the address? based on the coordinates???";
    m_pNetworkAccessManager->get(QNetworkRequest(QUrl(url)));
}


void GeocodeDataManager::replyFinished(QNetworkReply* reply)
{
    QString json = reply->readAll();
    //vvimDebug() << "Reply = " << json;
    vvimDebug() << "URL = " << reply->url();
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

    if(result.toMap()["status"].toString() != "OK")
    {
        emit errorOccured(QString("Code of request is: %1").arg(result.toMap()["status"].toString()));
        return;
    }

    QVariantList nestedList = result.toMap()["results"].toList();
    QVariantList::Iterator it = nestedList.begin();
    if(it == nestedList.end())
    {
        emit errorOccured(QString("Cannot find any locations"));
        return;
    }
    QVariantMap locationOfResult = (*it).toMap()["geometry"].toMap()["location"].toMap();
    vvimDebug() << "location" << locationOfResult["lng"].toDouble() << locationOfResult["lat"].toDouble();

    if(marker_type == Adres)
        emit coordinatesReady(locationOfResult["lng"].toDouble(), locationOfResult["lat"].toDouble(),name_of_marker);
    else if(marker_type == Ophaalpunt)
    {
        marker_type = Adres;
        emit coordinatesReady(locationOfResult["lng"].toDouble(), locationOfResult["lat"].toDouble(),ophaalpunt_to_mark);
    }
    else if(marker_type == Levering)
    {
        marker_type = Adres;
        emit coordinatesReady(locationOfResult["lng"].toDouble(), locationOfResult["lat"].toDouble(),leveringToBeDone);
    }
}

void GeocodeDataManager::pushLevering(SLevering levering)
{
    marker_type = Levering;
    leveringToBeDone = levering;
    getCoordinates(leveringToBeDone.getAddress()); // getAddress() should be enough, while the marker will be named with getNameAndAddress()
}

void GeocodeDataManager::pushListOfMarkers(QList<SOphaalpunt> *list_of_markers)
{
    delete markersToBeDone;
    markersToBeDone = list_of_markers;

    giveNextMarker();
}

void GeocodeDataManager::giveNextMarker()
{
    if(!markersToBeDone->empty()) // markersToBeDone: not declared yet???
    {
        vvimDebug() << "markers to be done:" << markersToBeDone->size();
        foreach(SOphaalpunt marker, *markersToBeDone)
        {
            vvimDebug() << marker.naam;
        }

        marker_type = Ophaalpunt;
        vvimDebug() << "<vvim>: hier een korte pauze inlassen zodat de naam van de marker correct blijft?";
        ophaalpunt_to_mark = markersToBeDone->takeFirst(); // Removes the first item in the list and returns it.
        getCoordinates(ophaalpunt_to_mark.getNameAndAddress());
    }
}

GeocodeDataManager::~GeocodeDataManager()
{
    vvimDebug() << "start to deconstruct GeocodeDataManager()";
    delete markersToBeDone;
    delete m_pNetworkAccessManager;
    vvimDebug() << "GeocodeDataManager() deconstructed";
}
