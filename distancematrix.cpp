#include "distancematrix.h"
#include "geocode_data_manager.h"

#ifdef Q_OS_WIN
    #include <windows.h> // for Sleep
#endif

#define vvimDebug()\
    qDebug() << "[" << Q_FUNC_INFO << "]"

#include <QApplication>
#include <QJson/Parser>
#include <math.h>
#include <QDebug>
#include <QMessageBox>
#include <QTime>

//https://developers.google.com/maps/documentation/distancematrix/#Limits
#define MAX_URL_LENGTH 2000

// Google Distance Matrix doesn't allow to quary more than 10x10 places at once
#define MAX_NR_OF_CITIES 10

DistanceMatrix::DistanceMatrix(QObject *parent) :
    QObject(parent)
{
    m_pNetworkAccessManager = new QNetworkAccessManager(this);
    connect(m_pNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

    citynames = new QList<QString>;
    tsp_solution = new QList<int>();
    all_cities_marked = new QList<int>();
    distance_matrix_ready_to_process = DISTANCE_MATRIX_PART_AA;
    Vlaspit = 0;

    //calculate distance in meters or in seconds?
    distance_calc = DISTANCE_IN_METERS;
}

QString DistanceMatrix::buildQjsonUrl(QList <SMarker*> markers, int origins_start, int origins_end, int destinations_start, int destinations_end)
{
    // <vvim> testen of start <= end ?
    // <vvim> testen of start => markers.begin() ?
    // <vvim> testen of end <= markers.end() ?

    QString origins = "";
    QString destinations = "";

    /*
    QList<SMarker*>::Iterator it = markers.begin(); // <vvim> markers.at(start) ?
    while(it != markers.end())  // <vvim> markers.at(end) ?
    {
        //// op lng / lat : adres wijzigt een beetje maar url is korter
        //origins += QString::number((*it)->north) + "," + QString::number((*it)->east) + "|";

        // op naam : zoals gevraagd, maar url is langer
        origins += (*it)->caption + "|";
        ++it;
    }

    // chop trailing '|'
    if(origins.endsWith('|'))
        origins.chop(1);




    it = markers.begin(); // <vvim> markers.at(start) ?
    while(it != markers.end())  // <vvim> markers.at(end) ?
    {
        //// op lng / lat : adres wijzigt een beetje maar url is korter
        //destinations += QString::number((*it)->north) + "," + QString::number((*it)->east) + "|";

        // op naam : zoals gevraagd, maar url is langer
        destinations += (*it)->caption + "|";
        ++it;
    }

    // chop trailing '|'
    if(destinations.endsWith('|'))
        destinations.chop(1);
    */

    int i = 1;

    QList<SMarker*>::Iterator it = markers.begin(); // <vvim> markers.at(start) ?

    while(it != markers.end())  // <vvim> markers.at(end) ?
    {
        //QString address_to_look_up = prepareForUrl((*it)->caption);
        QString address_to_look_up = QString("%1,%2").arg((*it)->north).arg((*it)->east);

        vvimDebug() << "!! !!" << address_to_look_up << "!! !!";

        if( (i >= origins_start) && (i <= origins_end) )
            origins += address_to_look_up + "|";

        if((i >= destinations_start) && (i <= destinations_end) )
            destinations += address_to_look_up + "|";

        ++it;
        ++i;
    }

    if(origins.endsWith('|'))
        origins.chop(1);

    if(destinations.endsWith('|'))
        destinations.chop(1);

    // <vvim> TODO: if QString.length() > 2000 => gebruik coordinaten ipv adres!
    vvimDebug() << "<vvim> TODO: if QString.length() > 2000 => gebruik coordinaten ipv adres!";

    QString QjsonUrl = QString("https://maps.googleapis.com/maps/api/distancematrix/json?origins=%1&destinations=%2&key=%3&oe=utf8&sensor=false").arg(origins).arg(destinations).arg(settings.value("apiKey").toString());
    vvimDebug() << "<vvim> QjsonUrl:" << QjsonUrl;
    if(QjsonUrl.length() > MAX_URL_LENGTH)
        vvimDebug() << "url is too long:" << QjsonUrl.length() << "exceeds the maximum of" << MAX_URL_LENGTH;
    return QjsonUrl;

    return origins;
}

QString DistanceMatrix::buildQjsonUrl(QList <SMarker*> markers)
{
    QString origins = "";

    QList<SMarker*>::Iterator it = markers.begin();
    while(it != markers.end())
    {
        //// op lng / lat : adres wijzigt een beetje maar url is korter
        //origins += QString::number((*it)->north) + "," + QString::number((*it)->east) + "|";

        // op naam : zoals gevraagd, maar url is langer
        //QString address_to_look_up = prepareForUrl((*it)->caption);
        QString address_to_look_up = QString("%1,%2").arg((*it)->north).arg((*it)->east);
        origins += address_to_look_up + "|";
        ++it;
    }

    // chop trailing '|'
    if(origins.endsWith('|'))
        origins.chop(1);

    QString destinations = origins;

    // <vvim> TODO: if QString.length() > 2000 => gebruik coordinaten ipv adres!
    vvimDebug() << "<vvim> TODO: if QString.length() > 2000 => gebruik coordinaten ipv adres!";

    QString QjsonUrl = QString("https://maps.googleapis.com/maps/api/distancematrix/json?origins=%1&destinations=%2&key=%3&oe=utf8&sensor=false").arg(origins).arg(destinations).arg(settings.value("apiKey").toString());
    vvimDebug() << "<vvim> QjsonUrl:" << QjsonUrl;
    if(QjsonUrl.length() > MAX_URL_LENGTH)
        vvimDebug() << "url is too long:" << QjsonUrl.length() << "exceeds the maximum of" << MAX_URL_LENGTH;
    return QjsonUrl;
}

void DistanceMatrix::getDistances(QList <SMarker*> markers)
{
    if(m_markers.length() > 0)
    {

        vvimDebug() << "DistanceMatrix has been called before, so we must delete the matrices!";
        deleteTheMatrices();

    }

    m_markers = markers;

    vvimDebug() << "<vvim> TODO: mag dit, of moeten we een nieuwe m_markers creëren en deze laten invullen met 'markers'???";

    // <vvim> TODO: maak dat de eerste SMarker altijd Vlaspit is!!!! En voeg deze automatisch toe bij het opstarten
    vvimDebug() << "<vvim> TODO: maak dat de eerste SMarker altijd Vlaspit is!!!! En voeg deze automatisch toe bij het opstarten aub";

    // HIER ENKEL DE EERSTE (10) PAKKEN  ( -->  MAX_NR_OF_CITIES)
    vvimDebug() << "<vvim> TODO: check whether markers.length()" << markers.length() << " > " << MAX_NR_OF_CITIES << (markers.length() > MAX_NR_OF_CITIES);
    vvimDebug() << "<vvim> TODO:  + check whether m_markers.length()" << m_markers.length() << " > " << MAX_NR_OF_CITIES << (m_markers.length() > MAX_NR_OF_CITIES);
    vvimDebug() << "<vvim> TODO: check whether markers.size()" << markers.size() << " > " << MAX_NR_OF_CITIES << (markers.size() > MAX_NR_OF_CITIES);
    vvimDebug() << "<vvim> TODO:  + check whether m_markers.size()" << m_markers.size() << " > " << MAX_NR_OF_CITIES << (m_markers.size() > MAX_NR_OF_CITIES);

    if(m_markers.length() > (2*MAX_NR_OF_CITIES))
    {
        // can't handle this with AA + B + CC +D
        vvimDebug() << "<vvim> ERROR: aantal steden:"<< m_markers.length() <<" > 2x maximum dat Google toelaat om op te zoeken, nl" << MAX_NR_OF_CITIES << ". We kunnen dit niet oplossen. Google Maps API For Business aanschaffen?";
        emit errorOccured(QString("You exceeded the maximum number of places (%1 > %2) that we can calculate the distances from. Check the Google API Distance Matrix Limits.").arg(m_markers.length()).arg(2*MAX_NR_OF_CITIES));
        return;
    }
    else if(m_markers.length() > MAX_NR_OF_CITIES)
    {
        // telkens ook 1 seconde wachten aub
        vvimDebug() << "<vvim> SPECIAL SAUCE";
        distance_matrix_ready_to_process = DISTANCE_MATRIX_PART_AA; // origins = [0, MAX[, destinations = [0, MAX[
        QString url = buildQjsonUrl(m_markers,0,MAX_NR_OF_CITIES,0,MAX_NR_OF_CITIES);
        m_pNetworkAccessManager->get(QNetworkRequest(QUrl(url))); // staat er een delay op het antwoord? Of moeten we continu testen: "distance_matrix_ready_to_process == DISTANCE_MATRIX_PART_B;" ???

//emit errorOccured(QString("Het spijt me Geert, dit zijn meer dan 10 plaatsen en ik heb de code nog niet geschreven om dit op te lossen. Voorlopig maximum zijn 10 plaatsen."));
//return;

    }
    else
    {
        distance_matrix_ready_to_process = DISTANCE_MATRIX_READY;

        QString url = buildQjsonUrl(m_markers);

        m_pNetworkAccessManager->get(QNetworkRequest(QUrl(url)));
    }
}


void DistanceMatrix::replyFinished(QNetworkReply* reply)
{
    int nr_of_cities = m_markers.length();
    QString url;

    if(distance_matrix_ready_to_process == DISTANCE_MATRIX_PART_AA)
    {
// TODO: <vvim> this should be one level up, because all the same for AA, B, CC, D & MATRIX_READY
        vvimDebug() << "DISTANCE_MATRIX_PART_AA: preparing the distancematrix";
        // veel zaken een niveau hoger zetten,
        // veel variabele: classe-variabele maken

        citynames->clear(); // <vvim> dit is nodig, zie probleempje-20140423-citiesnames

        QString json = reply->readAll();
        //vvimDebug() << "AA Reply = " << json;
        //vvimDebug() << "URL = " << reply->url();
        QString strUrl = reply->url().toString();

        QJson::Parser parser;
        bool ok;

        QVariantMap result = parser.parse(json.toLatin1(), &ok).toMap();
        if (!ok) {
          emit errorOccured(QString("Cannot convert to QJson object: %1").arg(json));
          return;
        }

        // JSON: "{}" is een map, "[]" is een list
        QVariantList rowsList = result["rows"].toList(); // bevat de distance matrix in een list
        //vvimDebug() << "aantal rijen == aantal oorsprongsadressen" << rowsList.length() << result["origin_addresses"].toList().length(); // lengte van de rows is afhankelijk van het aantal "origin_addresses"
        //vvimDebug() << "aantal elementen eerste rij == aantal destinaties"  << rowsList.first().toMap()["elements"].toList().length() << result["destination_addresses"].toList().length(); // aantal "destination_addresses" bepaalt het aantal elementen per row -> [ { 'elements' [

        int row = 0;

        // aantal steden:
        vvimDebug() << "aantal steden: " << nr_of_cities;
        vvimDebug() << "<vvim> TODO: we kunnen hier testen of listOfElementsFromThisRow.length() == SMarkers.length() . if so => ok";
        // <vvim> TODO: we kunnen hier testen of listOfElementsFromThisRow.length() == SMarkers.length() . if so => ok

        if(rowsList.length() < 1)
        {
            //<vvim> TODO: STATUS = MAX_ELEMENTS_EXCEEDED ? Hoe oplossen? zie https://developers.google.com/maps/documentation/distancematrix/#Limits
            emit errorOccured(QString("No destinations found: %1").arg(json));
            return;
        }

        vvimDebug() << "DISTANCE_MATRIX_PART_AA: eerste deel van distance_matrix invullen met de AA-resultaten";
        // TSP 1. initieer de matrix afstanden_matrix:
        distance_matrix_in_meters = new int*[nr_of_cities];
        distance_matrix_in_seconds = new int*[nr_of_cities];
        for(int i=0; i < nr_of_cities; i++)
         {
             distance_matrix_in_meters[i] = new int[nr_of_cities];
             distance_matrix_in_seconds[i] = new int[nr_of_cities];
         }

        // <vvim> ??? nergens voor nodig, denk ik ????? => error in QJson, function should stop?
        for(int i=0; i < nr_of_cities; i++)
         {
             for(int j=0; j < nr_of_cities; j++)
             {
                 distance_matrix_in_seconds[i][j] = i+j;
                 distance_matrix_in_meters[i][j] = i+j;
             }
         }

// TODO: <vvim> this should be put in a procedure, because almost the same for AA, B, CC, D & MATRIX_READY
        QVariantList::Iterator it = rowsList.begin();
        while(it != rowsList.end())
        {
            QVariantList listOfElementsFromThisRow = (*it).toMap()["elements"].toList();

            citynames->push_back(result["origin_addresses"].toList()[row].toString());

            int element = 0;

            QVariantList::Iterator jt = listOfElementsFromThisRow.begin();
            while(jt != listOfElementsFromThisRow.end())
            {
                QString status = (*jt).toMap()["status"].toString();
                if(status == "OK")
                {
                    QVariantMap distance = (*jt).toMap()["distance"].toMap();
                    QVariantMap duration = (*jt).toMap()["duration"].toMap();
                    // <vvim> distance in km or sec -> "distance" or "duration"
                    distance_matrix_in_meters[row][element] = distance["value"].toInt();
                    distance_matrix_in_seconds[row][element] = duration["value"].toInt();
                }
                else
                    vvimDebug() << "status:" << status;
                ++jt;
                ++element;
            }
    /*
            QVariantMap mapOfResults = (*it).toMap();
            vvimDebug() << "formatted_address" << mapOfResults["formatted_address"].toString();
            QVariantMap locationOfResult = mapOfResults["geometry"].toMap()["location"].toMap();
            vvimDebug() << "location" << locationOfResult["lng"].toDouble() << locationOfResult["lat"].toDouble();
            // --> "geometry" ( toMap() )
    */
            ++it;
            ++row;
        }



        #ifdef Q_OS_WIN
            Sleep(2000);
        #else
            sleep(2);
        #endif

        distance_matrix_ready_to_process = DISTANCE_MATRIX_PART_B; // origins = [0, MAX[, destinations = [MAX, all]
        url = buildQjsonUrl(m_markers,0,MAX_NR_OF_CITIES, MAX_NR_OF_CITIES+1, m_markers.length());

        m_pNetworkAccessManager->get(QNetworkRequest(QUrl(url))); // staat er een delay op het antwoord? Of moeten we continu testen: "distance_matrix_ready_to_process == DISTANCE_MATRIX_PART_B;" ???
        return;
    }
    else if(distance_matrix_ready_to_process == DISTANCE_MATRIX_PART_B)
    {
        vvimDebug() << "DISTANCE_MATRIX_PART_B: preparing the distancematrix";

        QString json = reply->readAll();
        //vvimDebug() << "B Reply = " << json;
        //vvimDebug() << "URL = " << reply->url();
        QString strUrl = reply->url().toString();

        QJson::Parser parser;
        bool ok;

        QVariantMap result = parser.parse(json.toLatin1(), &ok).toMap();
        if (!ok) {
          emit errorOccured(QString("Cannot convert to QJson object: %1").arg(json));
          return;
        }

        // JSON: "{}" is een map, "[]" is een list
        QVariantList rowsList = result["rows"].toList(); // bevat de distance matrix in een list
        //vvimDebug() << "aantal rijen == aantal oorsprongsadressen" << rowsList.length() << result["origin_addresses"].toList().length(); // lengte van de rows is afhankelijk van het aantal "origin_addresses"
        //vvimDebug() << "aantal elementen eerste rij == aantal destinaties"  << rowsList.first().toMap()["elements"].toList().length() << result["destination_addresses"].toList().length(); // aantal "destination_addresses" bepaalt het aantal elementen per row -> [ { 'elements' [

        int row = 0;

        if(rowsList.length() < 1)
        {
            //<vvim> TODO: STATUS = MAX_ELEMENTS_EXCEEDED ? Hoe oplossen? zie https://developers.google.com/maps/documentation/distancematrix/#Limits
            emit errorOccured(QString("No destinations found: %1").arg(json));
            return;
        }

        vvimDebug() << "DISTANCE_MATRIX_PART_B: tweede deel van distance_matrix invullen met de B-resultaten";

        QVariantList::Iterator it = rowsList.begin();
        while(it != rowsList.end())
        {
            QVariantList listOfElementsFromThisRow = (*it).toMap()["elements"].toList();

            //enkel bij AA en D: citynames->push_back(result["origin_addresses"].toList()[row].toString());

            int element = MAX_NR_OF_CITIES;

            QVariantList::Iterator jt = listOfElementsFromThisRow.begin();
            while(jt != listOfElementsFromThisRow.end())
            {
                QString status = (*jt).toMap()["status"].toString();
                if(status == "OK")
                {
                    QVariantMap distance = (*jt).toMap()["distance"].toMap();
                    QVariantMap duration = (*jt).toMap()["duration"].toMap();
                    // <vvim> distance in km or sec -> "distance" or "duration"
                    distance_matrix_in_meters[row][element] = distance["value"].toInt();
                    distance_matrix_in_seconds[row][element] = duration["value"].toInt();
                }
                else
                    vvimDebug() << "status:" << status;
                ++jt;
                ++element;
            }
    /*
            QVariantMap mapOfResults = (*it).toMap();
            vvimDebug() << "formatted_address" << mapOfResults["formatted_address"].toString();
            QVariantMap locationOfResult = mapOfResults["geometry"].toMap()["location"].toMap();
            vvimDebug() << "location" << locationOfResult["lng"].toDouble() << locationOfResult["lat"].toDouble();
            // --> "geometry" ( toMap() )
    */
            ++it;
            ++row;
        }




        #ifdef Q_OS_WIN
            Sleep(2000);
        #else
            sleep(2);
        #endif

        distance_matrix_ready_to_process = DISTANCE_MATRIX_PART_CC; // origins = [MAX, all], destinations = [0, MAX[
        url = buildQjsonUrl(m_markers,MAX_NR_OF_CITIES+1, m_markers.length(), 0,MAX_NR_OF_CITIES);

        m_pNetworkAccessManager->get(QNetworkRequest(QUrl(url))); // staat er een delay op het antwoord? Of moeten we continu testen: "distance_matrix_ready_to_process == DISTANCE_MATRIX_PART_B;" ???
        return;
    }
    else if(distance_matrix_ready_to_process == DISTANCE_MATRIX_PART_CC)
    {
        vvimDebug() << "DISTANCE_MATRIX_PART_CC: preparing the distancematrix";

        QString json = reply->readAll();
        //vvimDebug() << "CC Reply = " << json;
        //vvimDebug() << "URL = " << reply->url();
        QString strUrl = reply->url().toString();

        QJson::Parser parser;
        bool ok;

        QVariantMap result = parser.parse(json.toLatin1(), &ok).toMap();
        if (!ok) {
          emit errorOccured(QString("Cannot convert to QJson object: %1").arg(json));
          return;
        }

        // JSON: "{}" is een map, "[]" is een list
        QVariantList rowsList = result["rows"].toList(); // bevat de distance matrix in een list
        //vvimDebug() << "aantal rijen == aantal oorsprongsadressen" << rowsList.length() << result["origin_addresses"].toList().length(); // lengte van de rows is afhankelijk van het aantal "origin_addresses"
        //vvimDebug() << "aantal elementen eerste rij == aantal destinaties"  << rowsList.first().toMap()["elements"].toList().length() << result["destination_addresses"].toList().length(); // aantal "destination_addresses" bepaalt het aantal elementen per row -> [ { 'elements' [

        int row = MAX_NR_OF_CITIES;

        if(rowsList.length() < 1)
        {
            //<vvim> TODO: STATUS = MAX_ELEMENTS_EXCEEDED ? Hoe oplossen? zie https://developers.google.com/maps/documentation/distancematrix/#Limits
            emit errorOccured(QString("No destinations found: %1").arg(json));
            return;
        }

        vvimDebug() << "DISTANCE_MATRIX_PART_CC: derde deel van distance_matrix invullen met de CC-resultaten";
        QVariantList::Iterator it = rowsList.begin();
        while(it != rowsList.end())
        {
            QVariantList listOfElementsFromThisRow = (*it).toMap()["elements"].toList();

            //enkel bij AA en D: citynames->push_back(result["origin_addresses"].toList()[row].toString());

            int element = 0;

            QVariantList::Iterator jt = listOfElementsFromThisRow.begin();
            while(jt != listOfElementsFromThisRow.end())
            {
                QString status = (*jt).toMap()["status"].toString();
                if(status == "OK")
                {
                    QVariantMap distance = (*jt).toMap()["distance"].toMap();
                    QVariantMap duration = (*jt).toMap()["duration"].toMap();
                    // <vvim> distance in km or sec -> "distance" or "duration"
                    distance_matrix_in_meters[row][element] = distance["value"].toInt();
                    distance_matrix_in_seconds[row][element] = duration["value"].toInt();
                }
                else
                    vvimDebug() << "status:" << status;
                ++jt;
                ++element;
            }
    /*
            QVariantMap mapOfResults = (*it).toMap();
            vvimDebug() << "formatted_address" << mapOfResults["formatted_address"].toString();
            QVariantMap locationOfResult = mapOfResults["geometry"].toMap()["location"].toMap();
            vvimDebug() << "location" << locationOfResult["lng"].toDouble() << locationOfResult["lat"].toDouble();
            // --> "geometry" ( toMap() )
    */
            ++it;
            ++row;
        }


        #ifdef Q_OS_WIN
            Sleep(2000);
        #else
            sleep(2);
        #endif
        distance_matrix_ready_to_process = DISTANCE_MATRIX_PART_D; // origins = [MAX, all], destinations = [MAX, all]
        url = buildQjsonUrl(m_markers,MAX_NR_OF_CITIES+1, m_markers.length(), MAX_NR_OF_CITIES+1, m_markers.length());
        m_pNetworkAccessManager->get(QNetworkRequest(QUrl(url))); // staat er een delay op het antwoord? Of moeten we continu testen: "distance_matrix_ready_to_process == DISTANCE_MATRIX_PART_B;" ???
        return;
    }
    else if(distance_matrix_ready_to_process == DISTANCE_MATRIX_PART_D)
    {
        vvimDebug() << "DISTANCE_MATRIX_PART_D: preparing the distancematrix";

        QString json = reply->readAll();
        //vvimDebug() << "D Reply = " << json;
        //vvimDebug() << "URL = " << reply->url();
        QString strUrl = reply->url().toString();

        QJson::Parser parser;
        bool ok;

        QVariantMap result = parser.parse(json.toLatin1(), &ok).toMap();
        if (!ok) {
          emit errorOccured(QString("Cannot convert to QJson object: %1").arg(json));
          return;
        }

        // JSON: "{}" is een map, "[]" is een list
        QVariantList rowsList = result["rows"].toList(); // bevat de distance matrix in een list
        //vvimDebug() << "aantal rijen == aantal oorsprongsadressen" << rowsList.length() << result["origin_addresses"].toList().length(); // lengte van de rows is afhankelijk van het aantal "origin_addresses"
        //vvimDebug() << "aantal elementen eerste rij == aantal destinaties"  << rowsList.first().toMap()["elements"].toList().length() << result["destination_addresses"].toList().length(); // aantal "destination_addresses" bepaalt het aantal elementen per row -> [ { 'elements' [

        int row = MAX_NR_OF_CITIES;
        int row_city_names = 0;

        if(rowsList.length() < 1)
        {
            //<vvim> TODO: STATUS = MAX_ELEMENTS_EXCEEDED ? Hoe oplossen? zie https://developers.google.com/maps/documentation/distancematrix/#Limits
            emit errorOccured(QString("No destinations found: %1").arg(json));
            return;
        }

        vvimDebug() << "DISTANCE_MATRIX_PART_D: laatste deel van distance_matrix invullen met de D-resultaten";
        QVariantList::Iterator it = rowsList.begin();
        while(it != rowsList.end())
        {
            QVariantList listOfElementsFromThisRow = (*it).toMap()["elements"].toList();

            // enkel bij DISTANCE_MATRIX_PART_AA en DISTANCE_MATRIX_PART_D
            citynames->push_back(result["origin_addresses"].toList()[row_city_names].toString());

            int element = MAX_NR_OF_CITIES;

            QVariantList::Iterator jt = listOfElementsFromThisRow.begin();
            while(jt != listOfElementsFromThisRow.end())
            {
                QString status = (*jt).toMap()["status"].toString();
                if(status == "OK")
                {
                    QVariantMap distance = (*jt).toMap()["distance"].toMap();
                    QVariantMap duration = (*jt).toMap()["duration"].toMap();
                    // <vvim> distance in km or sec -> "distance" or "duration"
                    distance_matrix_in_meters[row][element] = distance["value"].toInt();
                    distance_matrix_in_seconds[row][element] = duration["value"].toInt();
                }
                else
                    vvimDebug() << "status:" << status;
                ++jt;
                ++element;
            }
    /*
            QVariantMap mapOfResults = (*it).toMap();
            vvimDebug() << "formatted_address" << mapOfResults["formatted_address"].toString();
            QVariantMap locationOfResult = mapOfResults["geometry"].toMap()["location"].toMap();
            vvimDebug() << "location" << locationOfResult["lng"].toDouble() << locationOfResult["lat"].toDouble();
            // --> "geometry" ( toMap() )
    */
            ++it;
            ++row;
            ++row_city_names;
        }

        vvimDebug() << " *** TODO *** MATRIX VERWERKEN ZOALS bij DISTANCE_MATRIX_READY!";

        //return; // ?
    }
    else if(distance_matrix_ready_to_process == DISTANCE_MATRIX_READY)
    {

        // veel zaken een niveau hoger zetten,
        // veel variabele: classe-variabele maken

        citynames->clear(); // <vvim> dit is nodig, zie probleempje-20140423-citiesnames

        QString json = reply->readAll();
        //vvimDebug() << "Ready Reply = " << json;
        //vvimDebug() << "URL = " << reply->url();
        QString strUrl = reply->url().toString();

        QJson::Parser parser;
        bool ok;

        QVariantMap result = parser.parse(json.toLatin1(), &ok).toMap();
        if (!ok) {
          emit errorOccured(QString("Cannot convert to QJson object: %1").arg(json));
          return;
        }

        // JSON: "{}" is een map, "[]" is een list
        QVariantList rowsList = result["rows"].toList(); // bevat de distance matrix in een list
        //vvimDebug() << "aantal rijen == aantal oorsprongsadressen" << rowsList.length() << result["origin_addresses"].toList().length(); // lengte van de rows is afhankelijk van het aantal "origin_addresses"
        //vvimDebug() << "aantal elementen eerste rij == aantal destinaties"  << rowsList.first().toMap()["elements"].toList().length() << result["destination_addresses"].toList().length(); // aantal "destination_addresses" bepaalt het aantal elementen per row -> [ { 'elements' [

        int row = 0;

        if(nr_of_cities != rowsList.length())
            vvimDebug() << "WARNING: replyFinished() DistanceMatrixReady: nr_of_cities != rowsList.length() : " << nr_of_cities << rowsList.length();

        vvimDebug() << "aantal steden: " << nr_of_cities << "ofte" << rowsList.length();
        vvimDebug() << "<vvim> TODO: we kunnen hier testen of listOfElementsFromThisRow.length() == SMarkers.length() . if so => ok";
        // <vvim> TODO: we kunnen hier testen of listOfElementsFromThisRow.length() == SMarkers.length() . if so => ok

        if(rowsList.length() < 1)
        {
            //<vvim> TODO: STATUS = MAX_ELEMENTS_EXCEEDED ? Hoe oplossen? zie https://developers.google.com/maps/documentation/distancematrix/#Limits
            emit errorOccured(QString("No destinations found: %1").arg(json));
            return;
        }

        // TSP 1. initieer de matrix afstanden_matrix:
        distance_matrix_in_meters = new int*[nr_of_cities];
        distance_matrix_in_seconds = new int*[nr_of_cities];
        for(int i=0; i < nr_of_cities; i++)
         {
             distance_matrix_in_meters[i] = new int[nr_of_cities];
             distance_matrix_in_seconds[i] = new int[nr_of_cities];
         }

        // <vvim> ??? nergens voor nodig, denk ik ????? => error in QJson, function should stop?
        for(int i=0; i < nr_of_cities; i++)
         {
             for(int j=0; j < nr_of_cities; j++)
             {
                 distance_matrix_in_seconds[i][j] = i+j;
                 distance_matrix_in_meters[i][j] = i+j;
             }
         }

        QVariantList::Iterator it = rowsList.begin();
        while(it != rowsList.end())
        {
            QVariantList listOfElementsFromThisRow = (*it).toMap()["elements"].toList();

            citynames->push_back(result["origin_addresses"].toList()[row].toString());

            int element = 0;

            QVariantList::Iterator jt = listOfElementsFromThisRow.begin();
            while(jt != listOfElementsFromThisRow.end())
            {
                QString status = (*jt).toMap()["status"].toString();
                if(status == "OK")
                {
                    QVariantMap distance = (*jt).toMap()["distance"].toMap();
                    QVariantMap duration = (*jt).toMap()["duration"].toMap();
                    // <vvim> distance in km or sec -> "distance" or "duration"
                    distance_matrix_in_meters[row][element] = distance["value"].toInt();
                    distance_matrix_in_seconds[row][element] = duration["value"].toInt();
                }
                else
                    vvimDebug() << "status:" << status;
                ++jt;
                ++element;
            }
    /*
            QVariantMap mapOfResults = (*it).toMap();
            vvimDebug() << "formatted_address" << mapOfResults["formatted_address"].toString();
            QVariantMap locationOfResult = mapOfResults["geometry"].toMap()["location"].toMap();
            vvimDebug() << "location" << locationOfResult["lng"].toDouble() << locationOfResult["lat"].toDouble();
            // --> "geometry" ( toMap() )
    */
            ++it;
            ++row;
        }
    }

    emit new_distance_matrices(distance_matrix_in_meters, distance_matrix_in_seconds);
}

void DistanceMatrix::calculateOptimalRoute()
{
    vvimDebug() << "you should only call this function when the distance matrices have been filled in correctly";
    vvimDebug() << "that is why Form has the boolean `matrices_up_to_date` ";

//logOutputCitynamesDistanceMatrices(); //for debugging only:
    int nr_of_cities = m_markers.length();

    QString startTimeString = QDateTime::currentDateTime().toString();
    vvimDebug() << "Start:" << startTimeString;
    // TSP 2. al de rest resetten:
    reset_all_tsp(nr_of_cities);

    // TSP 3. start with empty path
    QList<int>* path = new QList<int>();

    // TSP 4. start from city "Vlaspit"
    path->push_back(Vlaspit);
    all_cities_marked->removeOne(Vlaspit);

    // TSP 5. route planning:
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor)); // set cursor to "Waiting cursor"
                                                              // best cursor aanpassen NA check van QJson, anders ook daar de cursor resetten als het mis ging

    tsp(path,0,all_cities_marked);


    vvimDebug() << "--- start:" << startTimeString;
    vvimDebug() << "--- done:" << QDateTime::currentDateTime().toString();

    // TSP 6. tsp_solution uitschrijven:
    // <vvim> TODO: wat als er geen tsp_solution is gevonden?
    vvimDebug() << "<vvim> TODO: wat als er geen tsp_solution is gevonden?";

    QApplication::restoreOverrideCursor(); // set cursor back to "Arrow cursor"

    //emit new_order_smarkers(tsp_solution, distance_matrix_in_meters, distance_matrix_in_seconds); // geef de voorgestelde route door aan de class Form zodat die ook in de GUI kan worden aangepast
    emit new_order_smarkers(tsp_solution); // geef de voorgestelde route door aan de class Form zodat die ook in de GUI kan worden aangepast

    vvimDebug() << "[DistanceMatrix::calculateOptimalRoute()]" << "destroy path!";
    delete path;

}

int DistanceMatrix::initialize_current_minimum_cost(int size)
{
    // we initialize the current_minimum_cost to be the "maximum" possible,namely adding all values from the distance_matrix together. Should do the trick.
    // Or we could take "MAX_INT", maybe that one is faster??
    vvimDebug() << "<vvim> TODO: to initialize the current_minimum_cost, maybe we should take MAX_INT instead? Faster no doubt.";
    int a = 0;
    for(int i=0; i < size; i++)
     {
         for(int j=0; j < size; j++)
         {
             if(distance_calc == DISTANCE_IN_METERS)
                a += distance_matrix_in_meters[i][j];
             else
                a += distance_matrix_in_seconds[i][j];
         }
     }
    a++;
    return a;
}

void DistanceMatrix::fill_all_cities(int size)
{
    // de link maken tussen afstanden_matrix[][] en stad_naam met een Map<QString, int>.

    // informatie uit SMarkers halen! -->nr, naam? of enkel nr zodat we de naam later weer uit de SMarkers halen???
    for(int a = 0; a<size; a++)
        all_cities_marked->push_back(a);
}

void DistanceMatrix::reset_all_tsp(int nr_of_cities)
{
    vvimDebug() << " + tsp_solution NUlL?" << (tsp_solution == NULL);
    vvimDebug() << "<vvim> TODO: tsp_solution op NULL zetten,  + tsp_solution NUlL?" << (tsp_solution == NULL);
    /*
    if(tsp_solution == NULL)
        tsp_solution = new QList<int>();
    */
    delete tsp_solution;              // } <vvim> TODO: is dit wel nodig?
    tsp_solution = new QList<int>();  // }
    vvimDebug() << " + tsp_solution clear";
    tsp_solution->clear();

    vvimDebug() << " + all_cities_marked NUlL?" << (all_cities_marked == NULL);
    /*
    if(all_cities_marked == NULL)
        all_cities_marked = new QList<int>();
    */
    delete all_cities_marked;
    all_cities_marked = new QList<int>();
    vvimDebug() << " + all_cities_marked clear";
    all_cities_marked->clear();

    vvimDebug() << " + fill all cities";
    fill_all_cities(nr_of_cities);

    vvimDebug() << " + current_minimum_cost";
    current_minimum_cost = initialize_current_minimum_cost(nr_of_cities);
}

void DistanceMatrix::tsp(QList<int>* path, int totale_kost, QList<int>* remaining_cities)
{
   // wat als we in plaats van remaining_cities en afstandenmatrix[][] gebruik maken van
   // een stack van vectoren (A, B, 20min)  (van, naar, kost)
   // en dan een array[] bijhouden van lengte "aantal steden" die eerst op 0 geinitialiseerd wordt
   // en daarna '1' voor elke stad die bezocht wordt. Als allen 1 => alles bezocht
   // antwoordt dit op "misschien is Antw -> Leuven -> Brussel -> Leuven -> Boechout -> Antwerpen ook een tsp_solution"
   // waar 2x langs Leuven gaan uitgesloten is bij matrix[][] en remaining_cities

    int last_visited_city = path->last();

/*
    // for Debugging only:
        QString path_string = "";
        foreach(int i, *path)
            path_string += QString::number(i) + " -> ";
        vvimDebug() << "onderzoeken van" << path_string << "kost" << totale_kost << "( huidig minimum: " << current_minimum_cost << ")"; // procedure SALESMAN(C, N, R, l_path)
        QString REMAININGCITIES_string = "";
        foreach(int i, *remaining_cities)
            REMAININGCITIES_string += QString::number(i) + ",";
        vvimDebug() << "ik kom van stad" << last_visited_city << "overschietende steden:" << REMAININGCITIES_string;
    // </Debugging>
*/

    if(totale_kost > current_minimum_cost)
    {
        //vvimDebug() << "kost hoger dan HUIDIG_MINIMUM" << totale_kost << ">" << current_minimum_cost << ":" << path_string << "=> ABORT\n\n";
        return;
    }

    if(remaining_cities->empty())
    {
       //vvimDebug() << "toerken gedaan: " << path_string;

       //vvimDebug() << "VLASPIT TOEVOEGEN:   path += VLASPIT en totale_kost += terug naar VLASPIT";
       path->push_back(Vlaspit);
       if(distance_calc == DISTANCE_IN_METERS)
          totale_kost += distance_matrix_in_meters[last_visited_city][Vlaspit];
       else
           totale_kost += distance_matrix_in_seconds[last_visited_city][Vlaspit];

       if (totale_kost <= current_minimum_cost)
       {
           //vvimDebug() << "kost" << totale_kost << "<= dan HUIDIG_MINIMUM" << current_minimum_cost << "dus potentiële opl" << path_string << "->"<<Vlaspit;
           current_minimum_cost = totale_kost;
           // path opslaan in een algemene variabele?? tsp_solution ??? of als RETURN value???
           tsp_solution->clear();
           foreach(int element , *path) // is dit "deep copy" ??
               tsp_solution->push_back(element);
           //tsp_solution = path; // kopieert enkel het adres, niet de inhoud!!! zie: http://stackoverflow.com/questions/1277121/qt-4-5-qlistqlistconst-qlist-is-this-a-deep-copy-constructor en http://stackoverflow.com/questions/6866183/when-does-a-deep-copy-happen-to-a-qlist
       }
       /*
       else
       {
           vvimDebug() << "kost" << totale_kost << "> dan HUIDIG_MINIMUM" << current_minimum_cost << "dus GEEN opl" << path_string;
           vvimDebug() << "dit komt voor omdat we totale_kost verhogen met de reis terug naar de Vlaspit";
       }
       vvimDebug() << "THE END\n\n";
       */
       return;
    }

    foreach(int nieuwe_stad , *remaining_cities)
    {
      QList<int>*newPATH = new QList<int>();
      newPATH->append(*path);
      newPATH->push_back(nieuwe_stad);

      int nieuweTOTALE_KOST = totale_kost; // kost uit Matrix halen
      if(distance_calc == DISTANCE_IN_METERS)
         nieuweTOTALE_KOST += distance_matrix_in_meters[last_visited_city][nieuwe_stad]; // kost uit Matrix halen
      else
         nieuweTOTALE_KOST += distance_matrix_in_seconds[last_visited_city][nieuwe_stad]; // kost uit Matrix halen


      QList<int>* newREMAINING_CITIES = new QList<int>();
      newREMAINING_CITIES->append(*remaining_cities); // KOPIEER de waarden van "all_cities_marked" naar "anderesteden"
      newREMAINING_CITIES->removeOne(nieuwe_stad);

      tsp ( newPATH , nieuweTOTALE_KOST , newREMAINING_CITIES );
      delete newPATH;
      delete newREMAINING_CITIES;
    }

    //vvimDebug() << "\n";
}

QString DistanceMatrix::seconds_human_readable(int totalseconds)
{
    QString human_readable = "";
    int hours = floor(totalseconds/3600.0);
    int minutes = floor(fmod(totalseconds,3600.0)/60.0);
    int seconds = fmod(totalseconds,60.0);

    /* // check for calculation error:
    int secondscheck = (((((0*24)+hours)*60) + minutes)*60) + seconds;
    if (secondscheck == totalseconds)
        vvimDebug() << "OK";
    else
        vvimDebug() << "[error]";
    */

    human_readable.sprintf("%01du %02dm %02ds", hours, minutes, seconds);
    return human_readable;
}


void DistanceMatrix::logOutputCitynamesDistanceMatrices()
{

    FILE *matrixlogfile;
    QDateTime currentdatetime = QDateTime::currentDateTime();
    QString filename = QString("matrix-%1.csv").arg(currentdatetime.toString());
    matrixlogfile = fopen(filename.toStdString().c_str(), "a");
    if (NULL == matrixlogfile) {
        QString error = "";
        error.append("cannot open matrixlogfile '%1'").arg(filename);
        perror(error.toStdString().c_str());
        exit(-1);
    }


    int nr_of_cities = m_markers.length();

      QString allcities = ";";
      for(int i = 0; i < nr_of_cities; i++)
      {
          allcities.append(QString("%1 ;").arg(citynames->at(i)));
      }

      vvimDebug() << "Distance Matrix in meters:\n" << allcities;
      fprintf(matrixlogfile, "Distance Matrix in meters:\n%s", allcities.toStdString().c_str());

      for(int i = 0; i < nr_of_cities; i++)
      {
          QString glob = citynames->at(i)+";";
          for(int j = 0; j < nr_of_cities; j++)
          {
              glob.append(QString("%1 ;").arg(distance_matrix_in_meters[i][j]));
          }
          vvimDebug() << glob;
          fprintf(matrixlogfile, "\n%s", glob.toStdString().c_str());
      }


      vvimDebug() << "\nDistance Matrix in seconds:\n" << allcities;
      fprintf(matrixlogfile, "\n\nDistance Matrix in seconds:\n%s", allcities.toStdString().c_str());


      for(int i = 0; i < nr_of_cities; i++)
      {
          QString glob = citynames->at(i)+";";
          for(int j = 0; j < nr_of_cities; j++)
          {
              glob.append(QString("%1 ;").arg(distance_matrix_in_seconds[i][j]));
          }
          vvimDebug() << glob;
          fprintf(matrixlogfile, "\n%s", glob.toStdString().c_str());
      }
      fclose(matrixlogfile);
}

void DistanceMatrix::deleteTheMatrices()
{
    int nr_of_cities = m_markers.length();

    if(nr_of_cities > 0)
    {
        for(int i = 0; i < nr_of_cities; i++) {
            delete [] distance_matrix_in_meters[i];
            delete [] distance_matrix_in_seconds[i];
        }
        delete [] distance_matrix_in_meters;
        delete [] distance_matrix_in_seconds;
    }
}

DistanceMatrix::~DistanceMatrix()
{
    vvimDebug() << "start to deconstruct DistanceMatrix()";
    deleteTheMatrices();
    delete m_pNetworkAccessManager;
    delete tsp_solution;
    delete all_cities_marked;
    delete citynames;
    vvimDebug() << "DistanceMatrix() deconstructed";
}

QString DistanceMatrix::prepareForUrl(QString string)
{
    QString return_string = QString(string);
    return_string.replace("&","+");
    return_string.replace("\n",",");
    return_string.replace(" ","+");
    return return_string;
}
