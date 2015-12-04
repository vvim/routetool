#include "form.h"
#include "ui_form.h"
#include <QDebug>
#include <QWebFrame>
#include <QWebElement>
#include <QMessageBox>
#include <QFile>
#include <QStringListModel>
#include <QKeyEvent>
#include <QVariant>
#include <QSqlQuery>
#include <QSqlError>
#include "globalfunctions.h"

#ifdef Q_OS_WIN
    #include <windows.h> // for Sleep
#endif

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    completer = NULL;
    matrices_up_to_date = false;
    after_calculating_distance_matrix_continue_to_tsp = false;
    after_calculating_distance_matrix_continue_to_transportationlist = false;

    normal = new QPalette();
    normal->setColor(QPalette::Text,Qt::AutoColor);

    warning = new QPalette();
    warning->setColor(QPalette::Text,Qt::red);

    ui->setupUi(this);
    ui->webView->setPage(new myWebPage());

    ui->totalWeightEdit->setPalette(*normal);
    ui->totalVolumeEdit->setPalette(*normal);
    ui->totalBagsKurkEdit->setPalette(*normal);
    ui->totalBagsParafineEdit->setPalette(*normal);
    ui->totalKilometersEdit->setPalette(*normal);
    ui->totalTimeEdit->setPalette(*normal);

    connect(ui->goButton, SIGNAL(clicked()), this, SLOT(goClicked()));
    connect(ui->lePostalAddress, SIGNAL(returnPressed()), this, SLOT(goClicked()));

    //ui: buttons should be disabled until optimal route has been calculated:
    ui->pbOptimizeRoute->setEnabled(true);
    ui->pbRemoveMarker->setEnabled(true);
    ui->pbShowRouteAsDefined->setEnabled(true);
    ui->pbTransportationList->setEnabled(true);
    ui->pbRouteOmdraaien->setEnabled(true);

    /*
    connect(ui->lwMarkers, SIGNAL(itemPressed(QListWidgetItem*)), this, SLOT(reorderMarkers()));
        can only be used to identify the START of a drag/drop from ui->lwMarkers:
        it cannot SIGNAL the drop itself.
    */

    connect(ui->pbShowRouteAsDefined, SIGNAL(clicked()), this, SLOT(drawRoute()));

    connect(&m_geocodeDataManager, SIGNAL(coordinatesReady(double,double,QString)), this, SLOT(showCoordinates(double,double,QString)));
    connect(&m_geocodeDataManager, SIGNAL(coordinatesReady(double,double,SOphaalpunt)), this, SLOT(showOphaalpunt(double,double,SOphaalpunt)));
    connect(&m_geocodeDataManager, SIGNAL(coordinatesReady(double,double,SLevering)), this, SLOT(showLevering(double,double,SLevering)));
    connect(&m_geocodeDataManager, SIGNAL(errorOccured(QString)), this, SLOT(errorOccured(QString)));
    connect(&m_geocodeDataManager, SIGNAL(putCoordinatesInDatabase(double,double,int)), this, SLOT(putCoordinatesInDatabase(double,double,int)));

    connect(&m_distanceMatrix, SIGNAL(errorOccured(QString)), this, SLOT(errorOccured(QString)));
    connect(&m_distanceMatrix, SIGNAL(new_order_smarkers(QList<int> *)), this, SLOT(process_result_distancematrix(QList<int> *)));
    connect(&m_distanceMatrix, SIGNAL(new_distance_matrices(int**, int**)), this, SLOT(reload_distancematrix(int**, int**)));
    connect(ui->webView->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(populateJavaScriptWindowObject()));

    QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);
    ui->lePostalAddress->setText("");
    //////    ui->webView->setUrl(QUrl("qrc:/html/google_maps.html"));
    ui->webView->setHtml("<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"initial-scale=1.0, user-scalable=no\" /><style type=\"text/css\">html { height: 100% } body { height: 100%; margin: 0; padding: 0 } #map_canvas { height: 100% } </style> <script src=\"https://maps.googleapis.com/maps/api/js?v=3&key="+settings.value("apiKey").toString()+"\"> </script> <script> var map; var markers = []; function initialize() { var myOptions = { center: new google.maps.LatLng(50.9801, 4.97517), zoom: 15, mapTypeId: google.maps.MapTypeId.ROADMAP, panControl: true }; map = new google.maps.Map(document.getElementById(\"map_canvas\"), myOptions); } </script> </head> <body onload=\"initialize()\"> <div id=\"map_canvas\" style=\"width:100%; height:100%\"></div> </body></html>");
    ui->lwMarkers->setMaximumWidth(180);

    reloadCompleter();

    ui->lePostalAddress->setText(settings.value("startpunt").toString());
    goClicked();

    setTotalWeightTotalVolume();
    resetTotalDistanceAndTotalTime()
}

Form::~Form()
{
    vvimDebug() << "start to deconstruct Form()";
    delete ui; // does this delete everything? Also the QPalettes ?
    if(completer)
        delete completer;
    vvimDebug() << "<vvim> ~Form() deconstructor: no need to delete distance_matrix_in_meters and distance_matrix_in_seconds, this has been done by the class DistanceMatrix";
    vvimDebug() << "<vvim> ~Form() deconstructor: must we also delete all contents of QList <SMarker*> m_markers ?";
    vvimDebug() << "Form() deconstructed";
}

void Form::showLevering(double east, double north, SLevering levering, bool saveMarker)
{
    vvimDebug() << "Form, showLevering" << east << north;

    QString str =
            QString("var newLoc = new google.maps.LatLng(%1, %2); ").arg(north).arg(east) +
            QString("map.setCenter(newLoc);") +
            QString("map.setZoom(%1);").arg(ui->zoomSpinBox->value());

     vvimDebug() << str;

    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);

    if (saveMarker)
        setMarker(east, north, levering);
}

void Form::showOphaalpunt(double east, double north, SOphaalpunt ophaalpunt, bool saveMarker)
{
    vvimDebug() << "Form, showOphaalpunt" << east << north;

    QString str =
            QString("var newLoc = new google.maps.LatLng(%1, %2); ").arg(north).arg(east) +
            QString("map.setCenter(newLoc);") +
            QString("map.setZoom(%1);").arg(ui->zoomSpinBox->value());

     vvimDebug() << str;

    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);

    if (saveMarker)
        setMarker(east, north, ophaalpunt);
}

void Form::putCoordinatesInDatabase(double east, double north, int ophaalpunt_id)
{
    /** this function is only to put the coordinates in the database, NOT to put a marker on the screen **/


    vvimDebug() << "update db: lat / lng / ophaalpunt" << north << east << ophaalpunt_id;

    QString SQLquery_update_coords = "UPDATE ophaalpunten SET lat = :lat, lng = :lng WHERE id = :ophaalpunt_id";

    QSqlQuery query_update_coords;
    query_update_coords.prepare(SQLquery_update_coords);
    query_update_coords.bindValue(":lat", north);
    query_update_coords.bindValue(":lng", east);
    query_update_coords.bindValue(":ophaalpunt_id", ophaalpunt_id);

    vvimDebug() << "step 1" << "update ophaalpunt with lat/lng";
    if(!query_update_coords.exec())
    {
        if(!reConnectToDatabase(query_update_coords.lastError(), SQLquery_update_coords, QString("[%1]").arg(Q_FUNC_INFO)))
        {
            vvimDebug() << "unable to reconnect to DB, halting";
            QMessageBox::information(this, tr("Fout bij verbinding met de databank ").arg(Q_FUNC_INFO), tr("De databank kon niet geraadpleegd worden, het programma zal zich nu afsluiten.\n\nProbeer later opnieuw. Als deze fout zich blijft voordoen, stuur het logbestand naar Wim of neem contact op met de systeembeheerder."));
            return;
        }
        QSqlQuery query_temp;
        query_temp.prepare(SQLquery_update_coords);
        query_update_coords = query_temp;
        query_update_coords = QSqlQuery(SQLquery_update_coords);
        if(!query_update_coords.exec())
        {
            vvimDebug() << "FATAL:" << "Something went wrong, could not execute query:" << SQLquery_update_coords << query_update_coords.lastError();
            qFatal(QString("Something went wrong, could not execute query: %1").arg(SQLquery_update_coords).toStdString().c_str());
            QMessageBox::information(this, tr("Fout bij verbinding met de databank ").arg(Q_FUNC_INFO), tr("De databank kon niet geraadpleegd worden, het programma zal zich nu afsluiten.\n\nProbeer later opnieuw. Als deze fout zich blijft voordoen, stuur het logbestand naar Wim of neem contact op met de systeembeheerder."));
            return;
        }
    }
}


void Form::showCoordinates(double east, double north, QString markername, bool saveMarker)
{
    vvimDebug() << "Form, showCoordinates" << east << north;

    QString str =
            QString("var newLoc = new google.maps.LatLng(%1, %2); ").arg(north).arg(east) +
            QString("map.setCenter(newLoc);") +
            QString("map.setZoom(%1);").arg(ui->zoomSpinBox->value());

     vvimDebug() << str;

    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);

    if (saveMarker)
        setMarker(east, north, markername);
        //setMarker(east, north, ui->lePostalAddress->text());
}

void Form::setMarker(double east, double north, QString caption)
{
    for (int i=0; i<m_markers.size(); i++)
    {
        if (m_markers[i]->caption == caption) return;
    }

    QString str =
            QString("var marker = new google.maps.Marker({") +
            QString("position: new google.maps.LatLng(%1, %2),").arg(north).arg(east) +
            QString("map: map,") +
            QString("title: %1").arg("\""+caption+"\"") +
            QString("});") +
            QString("markers.push(marker);");
    vvimDebug() << str;
    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);


    SMarker *_marker = new SMarker(east, north, caption);
    m_markers.append(_marker);

    //adding capton to ListWidget
    ui->lwMarkers->addItem(caption);
    link_lwMarkers_mmarkers[caption] = _marker;
    matrices_up_to_date = false;
    setTotalDistanceAndTotalTime();
}

void Form::setMarker(double east, double north, SOphaalpunt ophaalpunt)
{
    QString caption = ophaalpunt.getNameAndAddress();
    for (int i=0; i<m_markers.size(); i++)
    {
        if (m_markers[i]->caption == caption)
        {
            // overschrijven met info ophaalpunt???
            vvimDebug() << "<vvim> TODO: wat als OPHAALPUNT & LEVERING ?";
            vvimDebug() << "found marker with the same caption" << caption << "ophaling" << m_markers[i]->ophaling << ", levering" << m_markers[i]->levering << ". Overwriting with SOphaalpunt data.";
            m_markers[i]->ophaling = true;
            m_markers[i]->ophaalpunt = ophaalpunt;
            setTotalWeightTotalVolume();
            return;
        }
    }

    QString str =
            QString("var marker = new google.maps.Marker({") +
            QString("position: new google.maps.LatLng(%1, %2),").arg(north).arg(east) +
            QString("map: map,") +
            QString("title: %1").arg("\"Ophaalpunt: "+ophaalpunt.naam+"\"") +
            QString("});") +
            QString("markers.push(marker);");
    vvimDebug() << str;
    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);


    SMarker *_marker = new SMarker(east, north, ophaalpunt);
    m_markers.append(_marker);

    //adding capton to ListWidget
    ui->lwMarkers->addItem(caption);
    link_lwMarkers_mmarkers[caption] = _marker;

    matrices_up_to_date = false;

    setTotalDistanceAndTotalTime();
    setTotalWeightTotalVolume();
}

void Form::setMarker(double east, double north, SLevering levering)
{
    QString caption = levering.getNameAndAddress();
    for (int i=0; i<m_markers.size(); i++)
    {
        if (m_markers[i]->caption == caption)
        {
            // overschrijven met info levering???
            vvimDebug() << "found marker with the same caption" << caption << "ophaling" << m_markers[i]->ophaling << ", levering" << m_markers[i]->levering << ". Overwriting with SLevering data.";
            m_markers[i]->levering = true;
            m_markers[i]->leveringspunt = levering;
            setTotalWeightTotalVolume();
            // anders bij levering dan bij ophaling, neen??? Levering wordt eerst gedaan, daarna is de camion leeg, dan de ophaling
            return;
        }
    }

    QString str =
            QString("var marker = new google.maps.Marker({") +
            QString("position: new google.maps.LatLng(%1, %2),").arg(north).arg(east) +
            QString("map: map,") +
            QString("title: %1").arg("\"Levering: "+levering.name+"\"") +
            QString("});") +
            QString("markers.push(marker);");
    vvimDebug() << str;
    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);


    SMarker *_marker = new SMarker(east, north, levering);
    m_markers.append(_marker);

    //adding capton to ListWidget
    ui->lwMarkers->addItem(caption);
    link_lwMarkers_mmarkers[caption] = _marker;
    matrices_up_to_date = false;

    setTotalDistanceAndTotalTime();
    setTotalWeightTotalVolume();
    // anders bij levering dan bij ophaling, neen??? Levering wordt eerst gedaan, daarna is de camion leeg, dan de ophaling
}

void Form::goClicked()
{
    QString address = ui->lePostalAddress->text();
    m_geocodeDataManager.getCoordinates(address);
    ui->lePostalAddress->clear();
}



void Form::errorOccured(const QString& error)
{
    QMessageBox::warning(this, tr("Geocode Error"), error);
}



void Form::on_lwMarkers_currentRowChanged(int currentRow)
{
    if (currentRow < 0) return;
    QString str =
            QString("var newLoc = new google.maps.LatLng(%1, %2); ").arg(m_markers[currentRow]->north).arg(m_markers[currentRow]->east) +
            QString("map.setCenter(newLoc);");

    vvimDebug() << str;

    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);
}

void Form::on_pbRemoveMarker_clicked()
{
    if (ui->lwMarkers->currentRow() < 0) return;

    if((ui->lwMarkers->currentRow()+2) == ui->lwMarkers->count())
    {
        vvimDebug() << "Dangerous business";
        QMessageBox::critical(this, tr("WAARSCHUWING"), tr("Het wissen van de 'voorlaatste' marker in de lijst resulteert in een fout die ik nog niet heb kunnen opsporen.\n\nAls je deze marker echt wilt wissen, versleep hem dan eerst tot onderaan de lijst, en verwijder hem dan."));
        return;
    }

    matrices_up_to_date = false;
    setTotalDistanceAndTotalTime(); // not necessary because 'drawRoute()' will be called, but in case future changes remove that call, this line is for avoiding bugs

    // after a Drag and Drop, the order might have changed
    reorderMarkers();

    /*
        because of the Drag and Drop in lwMarkers,
        the order of the markers[] array in JavaScript can be incorrect
        therefore we do not use these lines anymore

    QString str =
            QString("markers[%1].setMap(null); markers.splice(%1, 1);").arg(ui->lwMarkers->currentRow());
    vvimDebug() << str;
    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);

    */

    //deleteing caption from markers list
    /** THIS LINE GIVES THE TROUBLE, some sort of interaction with lwMarkers? **/
    //<vvim> TODO: programma crasht als er maar 2 markers zijn en je wil de tweede verwijderen. Waarom?
    delete m_markers.takeAt(ui->lwMarkers->currentRow());

    //deleteing caption from ListWidget
    delete ui->lwMarkers->takeItem(ui->lwMarkers->currentRow());

    drawRoute();
}

void Form::on_zoomSpinBox_valueChanged(int arg1)
{
    QString str =
            QString("map.setZoom(%1);").arg(arg1);
    vvimDebug() << str;
    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);
}

void Form::on_pbOptimizeRoute_clicked()
{
    //reorder Markers:: see BUG
          // <vvim> BUG: er is geen SIGNAL voor Drag/Drop vanuit QListWidget. Het programma weet dus niet
          //             of er markers van plaats zijn veranderd. Zeer vervelend.
          //             Bestaat er een extensie voor Qt die deze SIGNAL wel heeft?
    // reorderMarkers(); -> not necessary, the order will be changed any way

    if(matrices_up_to_date)
    {
        // no need to recalculate the distance matrices, just go straight to TSP:
        after_calculating_distance_matrix_continue_to_tsp = false;
        after_calculating_distance_matrix_continue_to_transportationlist = false;
        m_distanceMatrix.calculateOptimalRoute();
        setTotalDistanceAndTotalTime();
    }
    else
    {
        // we need to first calculate the distance matrices
        after_calculating_distance_matrix_continue_to_tsp = true;
        after_calculating_distance_matrix_continue_to_transportationlist = false;
        m_distanceMatrix.getDistances(m_markers);
    }

    //extra funcationalities
    ui->pbTransportationList->setEnabled(true);
    ui->pbRouteOmdraaien->setEnabled(true);
    ui->pbShowRouteAsDefined->setEnabled(true);
}

void Form::reload_distancematrix(int** matrix_in_meters, int ** matrix_in_seconds)
{
    distance_matrix_in_meters = matrix_in_meters;
    distance_matrix_in_seconds = matrix_in_seconds;
    matrix_dimensions = m_markers.length();

    matrices_up_to_date = true;

    if(after_calculating_distance_matrix_continue_to_tsp)
    {
        // continue to TSP:
        after_calculating_distance_matrix_continue_to_tsp = false;
        m_distanceMatrix.calculateOptimalRoute();
    }
    else
        /*  github 96a8b5577056d3108c6ac36ea4ec78ebf5cbd181
            after calculating the distance matrices, we also have to show which
            marker corresponds to which index of those matrices. Even if the order
            is the same (fixed)
        */
    {
        QList<int> *refreshindex = new QList<int>();
        for(int i = 0 ; i < m_markers.length() ; i++)
        {
            refreshindex->push_back(i);
        }
        process_result_distancematrix(refreshindex);
    }

    if(after_calculating_distance_matrix_continue_to_transportationlist)
    {
        // continue to Transportation List:
        after_calculating_distance_matrix_continue_to_transportationlist = false;
        buildTransportationList();
    }
}

void Form::process_result_distancematrix(QList<int> *tsp_order_smarkers)
{
        /**********************************************************************************
          function to re-arrange the QList<SMarker *>
          based on the Traveling Salesman Problem run in DistanceMatrix::

          good to know: tsp_order_smarkers->length() == m_markers.length() + 1
          because tsp_order_smarkers has the same beginning as endpoint (to make a loop)
        **********************************************************************************/

        /*
            QString volgorde = "Volgorde van de oplossing:\n";

            foreach(int i, *tsp_order_smarkers)
            {
                volgorde += " * " + m_markers.at(i)->caption;
                volgorde += "\n";
            }

            volgorde += "\n\nVolgorde van de Markers:\n";
            foreach(SMarker* i, m_markers)
            {
                volgorde += " * " + i->caption;
                volgorde += "\n";
            }

        */
        /////////////////

        QList<SMarker*> temp;
        ui->lwMarkers->clear();

        for(int i = 0; i < m_markers.length(); i++)
        {
            SMarker * _marker = m_markers.at(tsp_order_smarkers->at(i));
            _marker->distancematrixindex = tsp_order_smarkers->at(i);
            vvimDebug() << _marker->caption << _marker->distancematrixindex;
            temp.push_back(_marker);
            ui->lwMarkers->addItem(m_markers.at(tsp_order_smarkers->at(i))->caption);
            //vvimDebug() << m_markers.at(tsp_order_smarkers->at(i))->caption;
        }

        m_markers = temp; // does this leave garbage in memory? should I delete something??
                          // TODO <vvim> ask StackOverflow
        ui->lwMarkers->update();

        /*
            QMessageBox msgBox;
            msgBox.setText("message "+QString::number(tsp_order_smarkers->length())+" "+QString::number(m_markers.length())+" "+QString::number(temp.length()));
            msgBox.exec();

            volgorde += "\n\nVolgorde van de nieuwe Markers:\n";
            foreach(SMarker* i, temp)
            {
                volgorde += " * " + i->caption;
                volgorde += "\n";
            }

            msgBox.setText(volgorde);
            msgBox.exec();

        */

        drawRoute();
}

void Form::add_aanmeldingen(QList<SOphaalpunt> *aanmeldingen)
{
    m_geocodeDataManager.pushListOfMarkers(aanmeldingen);
}

void Form::add_levering(SLevering levering)
{
    m_geocodeDataManager.pushLevering(levering);
}

void Form::keyPressEvent( QKeyEvent *k )
{
    if(k->key() == Qt::Key_Delete)
        on_pbRemoveMarker_clicked();
}


void Form::drawRoute()
{
    setTotalDistanceAndTotalTime();

    // this only makes sense when there is more than 1 marker in m_markers
    // (as the first marker is the STARTING POINT and DESTINATION POINT of the route)
    if(m_markers.length() > 1)
    {
        //reorder Markers:: see BUG
              // <vvim> BUG: er is geen SIGNAL voor Drag/Drop vanuit QListWidget. Het programma weet dus niet
              //             of er markers van plaats zijn veranderd. Zeer vervelend.
              //             Bestaat er een extensie voor Qt die deze SIGNAL wel heeft?
        reorderMarkers();

        /**  ***********************************************  **
             Google API doesn't allow more than 8 WayPoints
             the workaround used here is courtesy of 'lemonharpy':
                   * http://lemonharpy.wordpress.com/2011/12/15/working-around-8-waypoint-limit-in-google-maps-directions-api/
                   * http://jsfiddle.net/ZyHnk/
         **  ***********************************************  **/

        QString html_top = QString("<!DOCTYPE html> <html> <head> <meta name=\"viewport\" content=\"initial-scale=1.0, user-scalable=no\" /> <style type=\"text/css\"> html { height: 100% } body { height: 100%; margin: 0; padding: 0 } #map_canvas { height: 100% } </style> <script src=\"https://maps.googleapis.com/maps/api/js?v=3&key=")+
                settings.value("apiKey").toString()+
                QString("\"> </script> <script> var markers = []; var map; function Tour_startUp(stops) { if (!window.tour) window.tour = { updateStops: function (newStops) { stops = newStops; }, loadMap: function (map, directionsDisplay) { var myOptions = { zoom: 15, center: new window.google.maps.LatLng(50.9801, 4.97517), mapTypeId: window.google.maps.MapTypeId.ROADMAP }; map.setOptions(myOptions); directionsDisplay.setMap(map); }, fitBounds: function (map) { var bounds = new window.google.maps.LatLngBounds(); jQuery.each(stops, function (key, val) { var myLatlng = new window.google.maps.LatLng(val.Geometry.Latitude, val.Geometry.Longitude); bounds.extend(myLatlng); }); map.fitBounds(bounds); }, calcRoute: function (directionsService, directionsDisplay) { var batches = []; var itemsPerBatch = 10; var itemsCounter = 0; var wayptsExist = stops.length > 0; while (wayptsExist) { var subBatch = []; var subitemsCounter = 0; for (var j = itemsCounter; j < stops.length; j++) { subitemsCounter++; subBatch.push({ location: new window.google.maps.LatLng(stops[j].Geometry.Latitude, stops[j].Geometry.Longitude), stopover: true }); if (subitemsCounter == itemsPerBatch) break; } itemsCounter += subitemsCounter; batches.push(subBatch); wayptsExist = itemsCounter < stops.length; itemsCounter--; } var combinedResults; var unsortedResults = [{}]; var directionsResultsReturned = 0; for (var k = 0; k < batches.length; k++) { var lastIndex = batches[k].length - 1; var start = batches[k][0].location; var end = batches[k][lastIndex].location; var waypts = []; waypts = batches[k]; waypts.splice(0, 1); waypts.splice(waypts.length - 1, 1); var request = { origin: start, destination: end, waypoints: waypts, travelMode: window.google.maps.TravelMode.WALKING }; (function (kk) { directionsService.route(request, function (result, status) { if (status == window.google.maps.DirectionsStatus.OK) { var unsortedResult = { order: kk, result: result }; unsortedResults.push(unsortedResult); directionsResultsReturned++; if (directionsResultsReturned == batches.length) { unsortedResults.sort(function (a, b) { return parseFloat(a.order) - parseFloat(b.order); }); var count = 0; for (var key in unsortedResults) { if (unsortedResults[key].result != null) { if (unsortedResults.hasOwnProperty(key)) { if (count == 0) combinedResults = unsortedResults[key].result; else { combinedResults.routes[0].legs = combinedResults.routes[0].legs.concat(unsortedResults[key].result.routes[0].legs); combinedResults.routes[0].overview_path = combinedResults.routes[0].overview_path.concat(unsortedResults[key].result.routes[0].overview_path); combinedResults.routes[0].bounds = combinedResults.routes[0].bounds.extend(unsortedResults[key].result.routes[0].bounds.getNorthEast()); combinedResults.routes[0].bounds = combinedResults.routes[0].bounds.extend(unsortedResults[key].result.routes[0].bounds.getSouthWest()); } count++; } } } directionsDisplay.setDirections(combinedResults); } } }); })(k); } } }; } function initialize() { var stops = [");

        QString html_bottom = QString("] ; var myOptions = { center: new google.maps.LatLng(50.9801, 4.97517), zoom: 15, mapTypeId: google.maps.MapTypeId.ROADMAP, panControl: true }; map = new window.google.maps.Map(document.getElementById(\"map_canvas\"), myOptions); var directionsDisplay = new window.google.maps.DirectionsRenderer(); var directionsService = new window.google.maps.DirectionsService(); Tour_startUp(stops); window.tour.loadMap(map, directionsDisplay); if (stops.length > 1) window.tour.calcRoute(directionsService, directionsDisplay); }; </script> </head> <body onLoad='initialize()'> <div id=\"map_canvas\" style=\"width:100%;height:100%\"> </div> </body> </html>");

        foreach(SMarker* marker, m_markers)
        {
            QString str = QString("{\"Geometry\":{\"Latitude\": %1 ,\"Longitude\": %2 }},\n").arg(marker->north).arg(marker->east);
            html_top += str;
        }

        // return again to starting point:
        QString str = QString("{\"Geometry\":{\"Latitude\": %1 ,\"Longitude\": %2 }}\n").arg(m_markers[0]->north).arg(m_markers[0]->east);
        html_top += str;

        vvimDebug() << "complete HTML:" << html_top+html_bottom;
        ui->webView->setHtml(html_top+html_bottom);
    }
}

void Form::on_pbRouteOmdraaien_clicked()
{
    //reorder Markers:: see BUG
          // <vvim> BUG: er is geen SIGNAL voor Drag/Drop vanuit QListWidget. Het programma weet dus niet
          //             of er markers van plaats zijn veranderd. Zeer vervelend.
          //             Bestaat er een extensie voor Qt die deze SIGNAL wel heeft?
    reorderMarkers();

    if(m_markers.length() > 0) // has no use without markers, right?
    {
        QList<SMarker*> temp;
        ui->lwMarkers->clear();

        // reverse the route, but keep the same starting point!
        temp.push_front(m_markers.front());
        ui->lwMarkers->addItem(m_markers.front()->caption);

        for(int i = m_markers.length() - 1; i > 0; i--)
        {
            temp.push_back(m_markers.at(i));
            ui->lwMarkers->addItem(m_markers.at(i)->caption);
            vvimDebug() << m_markers.at(i)->caption;
        }


        m_markers = temp; // does this leave garbage in memory? should I delete something??
                          // TODO <vvim> ask StackOverflow
        ui->lwMarkers->update();
    }

    drawRoute();
}


void Form::reorderMarkers()
{
    QList<SMarker*> temp;
    /**/vvimDebug() << "write out of m_markers BEFORE the reordering";
    /**/logOutputMarkers();

    for(int i = 0; i < ui->lwMarkers->count(); i++)
    {
        temp.push_back(link_lwMarkers_mmarkers[ui->lwMarkers->item(i)->text()]);
    }

    m_markers = temp; // does this leave garbage in memory? should I delete something??
                      // TODO <vvim> ask StackOverflow

    /**/vvimDebug() << "write out of m_markers AFTER the reordering";
    /**/logOutputMarkers();
}

void Form::logOutputMarkers()
{
    // function used for debugging
    vvimDebug() << "Debug information: output markers. Length: " << m_markers.length();
    vvimDebug() << "Are the distance matrices up to date?" << matrices_up_to_date;
    for(int i = 0; i < m_markers.length(); i++)
    {
        vvimDebug() << "Marker" << i << ":";
        m_markers[i]->PrintInformation();
    }
}

void Form::setTotalWeightTotalVolume()
{
    // anders bij levering dan bij ophaling, neen??? Levering wordt eerst gedaan, daarna is de camion leeg, dan de ophaling
    double total_weight = 0;
    double total_volume = 0;
    int total_bags_kurk = 0;
    int total_bags_kaarsresten = 0;

    for(int i = 0; i < m_markers.length(); i++)
    {
        if(m_markers[i]->ophaling)
        {
            //total_weight += m_markers[i]->ophaalpunt.kg_kurk + m_markers[i]->ophaalpunt.kg_kaarsresten;
            total_weight += m_markers[i]->ophaalpunt.getWeight();
            //total_volume += (m_markers[i]->ophaalpunt.zakken_kurk * settings.value("zak_kurk_volume").toDouble()) + (m_markers[i]->ophaalpunt.zakken_kaarsresten * settings.value("zak_kaarsresten_volume").toDouble());
            total_volume += m_markers[i]->ophaalpunt.getVolume();
            total_bags_kurk += m_markers[i]->ophaalpunt.zakken_kurk;
            total_bags_kaarsresten += m_markers[i]->ophaalpunt.zakken_kaarsresten;
        }
        if(m_markers[i]->levering)
        {
            vvimDebug() << ". type: Levering";
            vvimDebug() << "... anders bij levering dan bij ophaling, neen??? Levering wordt eerst gedaan, daarna is de camion leeg, dan de ophaling";
        }
    }


    ui->totalWeightEdit->setText(QString("%1 kg").arg(total_weight));
    ui->totalVolumeEdit->setText(QString("%1 liter").arg(total_volume));
    ui->totalBagsKurkEdit->setText(QString("%1").arg(total_bags_kurk));
    ui->totalBagsParafineEdit->setText(QString("%1").arg(total_bags_kaarsresten));

    if(total_weight > settings.value("max_gewicht_vrachtwagen").toDouble())
        ui->totalWeightEdit->setPalette(*warning);
    else
        ui->totalWeightEdit->setPalette(*normal);

    if(total_volume > settings.value("max_volume_vrachtwagen").toDouble())
        ui->totalVolumeEdit->setPalette(*warning);
    else
        ui->totalVolumeEdit->setPalette(*normal);

}

void Form::resetTotalDistanceAndTotalTime()
{
    ui->totalTimeEdit->setText(tr("onbekend"));
    ui->totalKilometersEdit->setText(tr("onbekend"));
}

void Form::setTotalDistanceAndTotalTime()
{
    vvimDebug() << "is transportationlist ready?" << transportationlistWriter.getReady();
    if(matrices_up_to_date)
    {
        int total_distance_in_meters, total_time_on_the_road_in_seconds;
        total_distance_in_meters = transportationlistWriter.getTotalMetersOfRoute();
        total_time_on_the_road_in_seconds = transportationlistWriter.getTotalSecondsOfRoute();
        vvimDebug() << "matrices are up to date" << matrices_up_to_date << "=> set total # kilometers and # time";
        vvimDebug() << total_time_on_the_road_in_seconds << " seconds is " << seconds_human_readable(total_time_on_the_road_in_seconds);
        ui->totalTimeEdit->setText(tr("%1").arg(seconds_human_readable(total_time_on_the_road_in_seconds)));
        double kilometers = (double) total_distance_in_meters / 1000;
        vvimDebug() << total_distance_in_meters << " meters is " << kilometers;
        ui->totalKilometersEdit->setText(tr("%1 km").arg(kilometers));
    }
    else
    {
        vvimDebug() << "matrices are up to date (no)" << matrices_up_to_date << "=> RESET";
        vvimDebug() << "reset MainForm: total # kilometers and # time is unknown unless 'Route Optimalisation' is pressed.";
        ui->totalTimeEdit->setText(tr("onbekend"));
        ui->totalKilometersEdit->setText(tr("onbekend"));
    }
}

void Form::on_pbTransportationList_clicked()
{
    vvimDebug() << "This is where we should work on making the Transportation List (using distance matrices and Document Writer)";
    // for the map: see http://qt-project.org/doc/qt-4.8/desktop-screenshot.html
    //              see http://stackoverflow.com/questions/681148/how-to-print-a-qt-dialog-or-window


    // 1. prepare the Maps for a screenshot
    drawRoute();
    // drawRoute() also calls `reorderMarkers()` so that is taken care of: "make sure Drag and Drop changes are in place"


    // 2. check if we need to recalculate the distance matrices
    if(!matrices_up_to_date)
    {
        // first fill in the distance matrices, then go to build the transportation list
        after_calculating_distance_matrix_continue_to_tsp = false;
        after_calculating_distance_matrix_continue_to_transportationlist = true;
        m_distanceMatrix.getDistances(m_markers);
    }
    else
        buildTransportationList();
}

void Form::buildTransportationList()
{
    // 3. distance matrices should be filled in correctly
    transportationlistWriter.prepare(m_markers, distance_matrix_in_meters, distance_matrix_in_seconds, ui->webView);
    setTotalDistanceAndTotalTime();
    transportationlistWriter.show();
    return;
}


void Form::logOutputLwMarkers()
{
    vvimDebug() << "\n\nSTART lwMarkers";
    // function used for debugging
    vvimDebug() << "Debug information: output lwmarkers. Count: " << ui->lwMarkers->count();
    vvimDebug() << "Current row:" << ui->lwMarkers->currentRow();
    for(int i = 0; i < ui->lwMarkers->count(); i++)
    {
        vvimDebug() << "Row" << i << ":" << ui->lwMarkers->item(i)->text();
    }
    vvimDebug() << "STOP\n\n";
}

void Form::reloadCompleter()
{
    vvimDebug() << "database has been changed, so we should reload the Completer";

    /** version from database **/
    QStringList words; // "don't come easy, to me, la la la laaa la la"

    vvimDebug() << "1. made QStringList words";
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif
    vvimDebug() << "2. setOverrideCursor";

    QString SQLquery = "SELECT naam, straat, nr, bus, postcode, plaats, land FROM ophaalpunten WHERE kurk > 0 OR parafine > 0";
    QSqlQuery query(SQLquery);

    vvimDebug() << "3. SELECT query ready";
    if(!query.exec())
    {
        if(!reConnectToDatabase(query.lastError(), SQLquery, QString("[%1]").arg(Q_FUNC_INFO)))
        {
            vvimDebug() << "unable to reconnect to DB, halting";
            QMessageBox::information(this, tr("Fout bij verbinding met de databank ").arg(Q_FUNC_INFO), tr("De databank kon niet geraadpleegd worden, probeer opnieuw. Als deze fout zich blijft voordoen, stuur het logbestand naar Wim of neem contact op met de systeembeheerder."));
            exit(-1);
        }
        query = QSqlQuery(SQLquery);
        if(!query.exec())
        {
            vvimDebug() << "query failed after reconnecting to DB, halting" << SQLquery;
            QMessageBox::information(this, tr("Fout bij verbinding met heruitvoeren query ").arg(Q_FUNC_INFO), tr("De query kon niet uitgevoerd worden na reconnectie met databank, probeer opnieuw. Als deze fout zich blijft voordoen, stuur het logbestand naar Wim of neem contact op met de systeembeheerder."));
            exit(-1);
        }
    }

    vvimDebug() << "4. no SQL query";
    while (query.next()) {
        QString naam	= query.value(0).toString();
        QString straat	= query.value(1).toString();
        QString nr	    = query.value(2).toString();
        QString bus	    = query.value(3).toString();
        QString postcode	= query.value(4).toString();
        QString plaats	= query.value(5).toString();
        QString land	= query.value(6).toString();
        words << naam.append(", %1 %2, %3 %4, %5").arg(straat).arg(nr).arg(postcode).arg(plaats).arg(land);
    }

    vvimDebug() << "5. TOTAL of ophaalpunten loaded in completer : " << words.length();

    vvimDebug() << "6. IF completer -> delete it, thank you" << completer;
    vvimDebug() << "6. IF completer -> delete it, thank you - value:" << completer;
    vvimDebug() << "!!! check in all classes if completer is set to NULL in constructor, thank you";

    if(completer)
    {
        vvimDebug() << "7. - completer TRUE => delete completer";
        delete completer;
    }
    else
        vvimDebug() << "7. - completer FALSE, no need to delete it";



    completer = new MyCompleter(words, this);
    vvimDebug() << "8. new completer 'words'";
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    vvimDebug() << "9. Case insensitive";

    ui->lePostalAddress->setCompleter(completer);
    vvimDebug() << "10. lePostalAddress";

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
    vvimDebug() << "done, completer (re)loaded.";
}

void Form::on_showOphaalpunten_clicked()
{
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif

    vvimDebug() << "toon bekende ophaalpunten!" << "inspiration: https://developers.google.com/maps/documentation/javascript/examples/marker-simple" << "and" << "https://developers.google.com/maps/documentation/javascript/markers";

    vvimDebug() << "** [A] ** update the ophaalpunten without lat/lng - Google Maps API doesn't allow more than 15 queries in one go, maximum = 15";
    QSqlQuery query_no_lat_lng("SELECT * FROM ophaalpunten WHERE lat is NULL or lng is NULL LIMIT 15");
    if(query_no_lat_lng.exec())
        vvimDebug() << "query runs";
    else
        vvimDebug() << "query error:" << query_no_lat_lng.lastError();
    QList<SOphaalpunt> * listOfOphaalpunten = new QList<SOphaalpunt>();
    vvimDebug() << "QList aangemaakt";
    while(query_no_lat_lng.next())
    {
        vvimDebug() << "inside resultlist of ophaalpunten without lat/lng";
        vvimDebug() << "name of ophaalpunt:" << query_no_lat_lng.value(2).toString();
        SOphaalpunt _ophaalpunt(
                        query_no_lat_lng.value(2).toString(),                      //_naam
                        query_no_lat_lng.value(7).toString(),                      //_street
                        query_no_lat_lng.value(8).toString(),                      //_housenr
                        query_no_lat_lng.value(9).toString(),                      //_busnr
                        query_no_lat_lng.value(10).toString(),                      //_postalcode
                        query_no_lat_lng.value(11).toString(),                      //_plaats
                        query_no_lat_lng.value(12).toString(),                      //_country
                        0,                      //_kg_kurk
                        0,                      //_kg_kaarsresten
                        0,                      //_zakken_kurk
                        0,                      //_zakken_kaarsresten
                        -1,                      //_aanmelding_id
                        query_no_lat_lng.value(0).toInt(),                      //_ophaalpunt_id
                        query_no_lat_lng.value(23).toString()                      //_opmerkingen
                    );
        listOfOphaalpunten->append(_ophaalpunt);

        _ophaalpunt.PrintInformation();
    }

    m_geocodeDataManager.lookForCoordinatesToPutInDatabase(listOfOphaalpunten);
    vvimDebug() << "** [A] ** done";




    /**
      de al getoonde markers staan in QList <SMarker*> m_markers;
      eerst een lijst maken van de bestaande markers? Die maken automatisch deel uit van de route, en mogen dus
      genegeerd worden
    **/

    vvimDebug() << "** [B] ** lijst maken van bestaande markers om 'doubles' te voorkomen";

    QSet<int> *ophaalpunten_in_route = getOphaalpuntIdFromRoute();
    vvimDebug() << "aantal ophaalpunten in de huidige route:" << ophaalpunten_in_route->size();

    QString SQLquery_all_aanmeldingen = "SELECT ophaalpunt FROM aanmelding WHERE ophaalronde_datum is NULL ORDER by ophaalpunt;";

    QSqlQuery query_all_aanmeldingen(SQLquery_all_aanmeldingen);


    vvimDebug() << "++++ step 1" << "get all aanmeldingen";
    if(!query_all_aanmeldingen.exec())
    {
        if(!reConnectToDatabase(query_all_aanmeldingen.lastError(), SQLquery_all_aanmeldingen, QString("[%1]").arg(Q_FUNC_INFO)))
        {
            vvimDebug() << "unable to reconnect to DB, halting";
            QMessageBox::information(this, tr("Fout bij verbinding met de databank ").arg(Q_FUNC_INFO), tr("De databank kon niet geraadpleegd worden, het programma zal zich nu afsluiten.\n\nProbeer later opnieuw. Als deze fout zich blijft voordoen, stuur het logbestand naar Wim of neem contact op met de systeembeheerder."));
            return;
        }
        query_all_aanmeldingen = QSqlQuery(SQLquery_all_aanmeldingen);
        if(!query_all_aanmeldingen.exec())
        {
            vvimDebug() << "FATAL:" << "Something went wrong, could not execute query:" << SQLquery_all_aanmeldingen << query_all_aanmeldingen.lastError();
            qFatal(QString("Something went wrong, could not execute query: %1").arg(SQLquery_all_aanmeldingen).toStdString().c_str());
            QMessageBox::information(this, tr("Fout bij verbinding met de databank ").arg(Q_FUNC_INFO), tr("De databank kon niet geraadpleegd worden, het programma zal zich nu afsluiten.\n\nProbeer later opnieuw. Als deze fout zich blijft voordoen, stuur het logbestand naar Wim of neem contact op met de systeembeheerder."));
            return;
        }
    }

    QSet<int> ophaalpunten_met_aanmelding;
    QSet<SOphaalpunt*> markers_met_aanmelding;
    QSet<SOphaalpunt*> markers_zonder_aanmelding;

    while (query_all_aanmeldingen.next())
    {
        ophaalpunten_met_aanmelding.insert(query_all_aanmeldingen.value(0).toInt());
    }


    vvimDebug() << "++++ step 1" << "done" << ophaalpunten_met_aanmelding.size() << "ophaalpunten have an 'aanmelding' (so we should mark them with a different color)";

    vvimDebug() << "++++ step 2" << "get all ophaalpunten";

    QString SQLquery_all_ophaalpunten = "SELECT id, naam, postcode, "
                                        "       straat, nr, bus, plaats, land, extra_informatie, lat, lng "
                    "FROM ophaalpunten WHERE kurk > 0 or parafine > 0 "
                    "ORDER BY postcode";

    QSqlQuery query_all_ophaalpunten(SQLquery_all_ophaalpunten);


    if(!query_all_ophaalpunten.exec())
    {
        if(!reConnectToDatabase(query_all_ophaalpunten.lastError(), SQLquery_all_ophaalpunten, QString("[%1]").arg(Q_FUNC_INFO)))
        {
            vvimDebug() << "unable to reconnect to DB, halting";
            QMessageBox::information(this, tr("Fout bij verbinding met de databank ").arg(Q_FUNC_INFO), tr("De databank kon niet geraadpleegd worden, het programma zal zich nu afsluiten.\n\nProbeer later opnieuw. Als deze fout zich blijft voordoen, stuur het logbestand naar Wim of neem contact op met de systeembeheerder."));
            return;
        }
        query_all_ophaalpunten = QSqlQuery(SQLquery_all_ophaalpunten);
        if(!query_all_ophaalpunten.exec())
        {
            vvimDebug() << "FATAL:" << "Something went wrong, could not execute query:" << SQLquery_all_ophaalpunten << query_all_ophaalpunten.lastError();
            qFatal(QString("Something went wrong, could not execute query: %1").arg(SQLquery_all_ophaalpunten).toStdString().c_str());
            QMessageBox::information(this, tr("Fout bij verbinding met de databank ").arg(Q_FUNC_INFO), tr("De databank kon niet geraadpleegd worden, het programma zal zich nu afsluiten.\n\nProbeer later opnieuw. Als deze fout zich blijft voordoen, stuur het logbestand naar Wim of neem contact op met de systeembeheerder."));
            return;
        }
    }

    vvimDebug() << "++++ step 3" << "get all ophaalpunten and filter those who are already marked or have an 'aanmelding'";
    while (query_all_ophaalpunten.next())
    {

       int ophaalpunt_id = query_all_ophaalpunten.value(0).toInt();
       if(ophaalpunten_in_route->contains(ophaalpunt_id))
       {
           // abort, this ophaalpunt is already in the route and does not have to be displayed again
           ;
       }
       else
       {
           // ophaalpunt not in route => display. Only question is: does it have an aanmelding or not? (see if-test)

           QString ophaalpunt_naam = query_all_ophaalpunten.value(1).toString();
           QString ophaalpunt_postcode = query_all_ophaalpunten.value(2).toString();

           QString ophaalpunt_straat = query_all_ophaalpunten.value(3).toString();
           QString ophaalpunt_huisnr = query_all_ophaalpunten.value(4).toString();
           QString ophaalpunt_busnr = query_all_ophaalpunten.value(5).toString();
           QString ophaalpunt_plaats = query_all_ophaalpunten.value(6).toString();
           QString ophaalpunt_land = query_all_ophaalpunten.value(7).toString();
           QString ophaalpunt_opmerkingen = query_all_ophaalpunten.value(8).toString();
           double ophaalpunt_latitude = query_all_ophaalpunten.value(9).toDouble();
           double ophaalpunt_longitude = query_all_ophaalpunten.value(10).toDouble();

           SOphaalpunt* nieuw = new SOphaalpunt(ophaalpunt_naam, ophaalpunt_straat, ophaalpunt_huisnr, ophaalpunt_busnr,
                             ophaalpunt_postcode, ophaalpunt_plaats, ophaalpunt_land,
                             -1, -1, -1, -1, // woudl be nice to add information of the 'aanmelding' to ophaalpunten_met_aanmelding: kg kurg, kg parafine, aanmelding_id, ...
                             -1, ophaalpunt_id, ophaalpunt_opmerkingen,
                             ophaalpunt_latitude, ophaalpunt_longitude
                             );

           if(ophaalpunten_met_aanmelding.contains(ophaalpunt_id))
           {
               //TOEVOEGEN AAN LIJST MET OPHAALPUNTEN DIE AANMELDINGEN GEDAAN HEBBEN
               markers_met_aanmelding.insert(nieuw);
           }
           else
           {
               //toevoegen aan lijst met ophaalpunten ZONDER aanmelding
               markers_zonder_aanmelding.insert(nieuw);
           }
       }
   }


/// could be done better, see example at http://stackoverflow.com/a/3059129 with extra argument for the addListener
    vvimDebug() << "++++ step 4" << "form javascript with all ophaalpunten and put them as clickable markers on the map" << "see http://stackoverflow.com/questions/3059044/";
    vvimDebug() << "inspiration: voor effectieve interactie tussen marker en javascript function" << "http://stackoverflow.com/questions/3059044/google-maps-js-api-v3-simple-multiple-marker-example" << "en" << "http://stackoverflow.com/a/23322162";

    //                [id, lat, lng, 'title', icon, zIndex],  // icon: see http://stackoverflow.com/questions/7095574/google-maps-api-3-custom-marker-color-for-default-dot-marker/18623391#18623391
    QString str = "\n\t[%5, %1, %2,   '%3', 'http://maps.google.com/mapfiles/ms/icons/%4-dot.png'],";

   QString markers_js = "var ophaalpunten = [ ";

   QSet<SOphaalpunt*>::Iterator it = markers_met_aanmelding.begin();
    while(it != markers_met_aanmelding.end())
    {
        //showing locations with aanmelding in blue
        markers_js.append(str.arg((*it)->getLatitude()).arg((*it)->getLongitude()).arg((*it)->getNameAndAddress().replace("\n"," ").replace("'","\\'")).arg("blue").arg((*it)->getOphaalpuntId()));
       ++it;
    }

    it = markers_zonder_aanmelding.begin();
    while(it != markers_zonder_aanmelding.end())
    {
      //showing locations without aanmelding in yellow
      markers_js.append(str.arg((*it)->getLatitude()).arg((*it)->getLongitude()).arg((*it)->getNameAndAddress().replace("\n"," ").replace("'","\\'")).arg("yellow").arg((*it)->getOphaalpuntId()));
      ++it;
    }

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    if(markers_js.length() < 25) // length "var ophaalpunten = [ " < 25 => no ophaalpunten to be marked
    {
        vvimDebug() << "no ophaalpunten found to be marked, returning";

        vvimDebug() << "******************";
        vvimDebug() << "*** < debug >  ***";
        vvimDebug() << "******************";
        vvimDebug() << "";
        vvimDebug() << "markers met aanmelding:" << markers_met_aanmelding.size();
        vvimDebug() << "markers zonder aanmelding:" << markers_zonder_aanmelding.size();
        vvimDebug() << "ophaalpunten in route:" << ophaalpunten_in_route->size();
        vvimDebug() << "totaal aantal:" << markers_met_aanmelding.size() + markers_zonder_aanmelding.size() + ophaalpunten_in_route->size();
        vvimDebug() << "";
        vvimDebug() << "******************";
        vvimDebug() << "*** </ debug > ***";
        vvimDebug() << "******************";
        vvimDebug() << "\n\nJavascript:" << markers_js << "\n\n";
        return;
    }

    // chop last ',' from markers_js
    markers_js.chop(1);

    str = "\n];\n"
          "for (var i = 0; i < ophaalpunten.length; i++) { \n"
          "      var ophaalpunt = ophaalpunten[i]; \n"
          "      var marker = new google.maps.Marker({ \n"
          "          position: {lat: ophaalpunt[1], lng: ophaalpunt[2]}, \n"
          "          map: map, \n"
          "          title: ophaalpunt[3], \n"
          "          icon: ophaalpunt[4], \n"
          "          zIndex: i \n"          /// a marker with zIndex 0 , does it get shown?
          "          }); \n"
          "      google.maps.event.addListener(marker, 'dblclick', (function(marker, i) {\n"
          "          return function() {\n"
          "            VlaspitRoutetool.askMainProgramToShowOphaalpuntInfo(ophaalpunten[i][0]);\n"
          "      }\n"
          " })(marker, i));\n"
          "  }\n";
    markers_js.append(str);
    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(markers_js);
    vvimDebug() << "\n\nJavascript:" << markers_js << "\n\n";
}

QSet<int>* Form::getOphaalpuntIdFromRoute()
{
    QSet<int>* ophaalpunt_in_route = new QSet<int>();

    QList<SMarker*>::Iterator it = m_markers.begin();
    while(it != m_markers.end())
    {
        if((*it)->getOphaalpuntId() > -1)   // valid ophaalpunt
        {
            ophaalpunt_in_route->insert((*it)->getOphaalpuntId());
        }
        else
            vvimDebug() << "non valid ophaalpunt" << (*it)->getOphaalpuntId() << (*it)->caption;
        ++it;
    }

    // <debug>
    QSet<int>::Iterator qq = ophaalpunt_in_route->begin();
    while(qq != ophaalpunt_in_route->end())
    {
        vvimDebug() << "ophaalpunt in route, id: " << (*qq);
        ++qq;
    }
    // </debug>

    return ophaalpunt_in_route;
}

void Form::populateJavaScriptWindowObject()
{
    qDebug() << "Populate";
    ui->webView->page()->mainFrame()->addToJavaScriptWindowObject("VlaspitRoutetool", this);
}

void Form::askMainProgramToShowOphaalpuntInfo(int ophaalpunt_id)
{
    vvimDebug() << "double clicked on marker, show info ophaalpunt" << ophaalpunt_id;
    emit showOphaalpuntInfo(ophaalpunt_id);
}
