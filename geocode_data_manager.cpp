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
    coordsToPutInDatabase = new QList<SOphaalpunt>(); // needed so that we can delete it at the first call of lookForCoordinatesToPutInDatabase() + for the empty()-check at lookupNextCoords()
    connect(m_pNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    connect(this, SIGNAL(coordinatesReady(double, double, QString)), this, SLOT(giveNextMarker()));
    connect(this, SIGNAL(coordinatesReady(double, double, SOphaalpunt)), this, SLOT(giveNextMarker()));
    connect(this, SIGNAL(putCoordinatesInDatabase(double,double, int)), this, SLOT(lookupNextCoords()));
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

void GeocodeDataManager::getCoordinates(const QString& address, const QString& name_and_address)
{
    name_of_marker = name_and_address;
    name_of_marker.replace("&","+");
    name_of_marker.replace("\n",",");

    QString address_encoded = address;
    address_encoded.replace("&","+");
    address_encoded.replace("\n",",");
    address_encoded.replace(" ","+");
    QString url = QString("https://maps.googleapis.com/maps/api/geocode/json?address=%1&key=%2&oe=utf8&sensor=false").arg(address_encoded).arg(settings.value("apiKey").toString());

    vvimDebug() << "issue #20, check:" << url;
vvimDebug() << "<vvim>" << "would the distance matrix work if we would only put the NAME of the ophaalpunt here, and NOT the address? based on the coordinates???";
    m_pNetworkAccessManager->get(QNetworkRequest(QUrl(url)));
}

void GeocodeDataManager::replyFinished(QNetworkReply* reply)
{
    QString json = reply->readAll();
    vvimDebug() << "URL = " << reply->url();
    //QString strUrl = reply->url().toString();

    QJson::Parser parser;

    bool ok;

    // json is a QString containing the data to convert
    QVariant result = parser.parse (json.toLatin1(), &ok);
    if(!ok)
    {
        vvimDebug() << "error occured, cannot convert to QJson object: %1" << json;
        emit errorOccured(QString("Cannot convert to QJson object: %1").arg(json));
        return;
    }

    if(result.toMap()["status"].toString() != "OK")
    {
        vvimDebug() << "error occured, status was wrong. Code of request is: %1" << result.toMap()["status"].toString();
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
    else if(marker_type == OnlyCoords)
    {
        marker_type = Adres;
        emit putCoordinatesInDatabase(locationOfResult["lng"].toDouble(), locationOfResult["lat"].toDouble(), ophaalpunt_to_mark.ophaalpunt_id);
    }
}

void GeocodeDataManager::pushLevering(SLevering levering)
{
    marker_type = Levering;
    leveringToBeDone = levering;
    getCoordinates(ophaalpunt_to_mark.getAddress(), ophaalpunt_to_mark.getNameAndAddress());
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
        getCoordinates(ophaalpunt_to_mark.getAddress(), ophaalpunt_to_mark.getNameAndAddress());
    }
}

GeocodeDataManager::~GeocodeDataManager()
{
    vvimDebug() << "start to deconstruct GeocodeDataManager()";
    delete markersToBeDone;
    delete coordsToPutInDatabase;
    delete m_pNetworkAccessManager;
    vvimDebug() << "GeocodeDataManager() deconstructed";
}

void GeocodeDataManager::lookForCoordinatesToPutInDatabase(QList<SOphaalpunt> *list_of_ophaalpunten)
{
    delete coordsToPutInDatabase;
    coordsToPutInDatabase = list_of_ophaalpunten;

    lookupNextCoords();
}

void GeocodeDataManager::lookupNextCoords()
{
    if(!coordsToPutInDatabase->empty()) // coordsToPutInDatabase: not declared yet???
    {
        vvimDebug() << "number of coords to put in database:" << coordsToPutInDatabase->size();
        foreach(SOphaalpunt marker, *coordsToPutInDatabase)
        {
            vvimDebug() << marker.naam;
        }

        marker_type = OnlyCoords;
        ophaalpunt_to_mark = coordsToPutInDatabase->takeFirst(); // Removes the first item in the list and returns it.
        getCoordinates(ophaalpunt_to_mark.getAddress(), ophaalpunt_to_mark.getNameAndAddress());
    }
}
