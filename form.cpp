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

#ifdef Q_OS_WIN
    #include <windows.h> // for Sleep
#endif

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    matrices_up_to_date = false;
    after_calculating_distance_matrix_continue_to_tsp = false;
    after_calculating_distance_matrix_continue_to_transportationlist = false;

    normal = new QPalette();
    normal->setColor(QPalette::Text,Qt::AutoColor);

    warning = new QPalette();
    warning->setColor(QPalette::Text,Qt::red);


    ui->setupUi(this);
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

    connect(&m_distanceMatrix, SIGNAL(errorOccured(QString)), this, SLOT(errorOccured(QString)));
    connect(&m_distanceMatrix, SIGNAL(new_order_smarkers(QList<int> *)), this, SLOT(process_result_distancematrix(QList<int> *)));
    connect(&m_distanceMatrix, SIGNAL(new_distance_matrices(int**, int**)), this, SLOT(reload_distancematrix(int**, int**)));

    QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);
    ui->lePostalAddress->setText("");
    //////    ui->webView->setUrl(QUrl("qrc:/html/google_maps.html"));
    ui->webView->setHtml("<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"initial-scale=1.0, user-scalable=no\" /><style type=\"text/css\">html { height: 100% } body { height: 100%; margin: 0; padding: 0 } #map_canvas { height: 100% } </style> <script type=\"text/javascript\" src=\"http://maps.googleapis.com/maps/api/js?key="+settings.value("apiKey").toString()+"&sensor=false\"> </script> <script type=\"text/javascript\"> var map; var markers = []; function initialize() { var myOptions = { center: new google.maps.LatLng(50.9801, 4.97517), zoom: 15, mapTypeId: google.maps.MapTypeId.ROADMAP, panControl: true }; map = new google.maps.Map(document.getElementById(\"map_canvas\"), myOptions); } </script> </head> <body onload=\"initialize()\"> <div id=\"map_canvas\" style=\"width:100%; height:100%\"></div> </body></html>");
    ui->lwMarkers->setMaximumWidth(180);

    /* version from file: can't get it to work properly
        <ugly>

      TODO vvim: make MyCompleter read from File and make modelFromFile
        unfortunately: I cannot get it to work (maybe problem in constructor? maybe in MyCompleter::update() ??)
        therefore I now convert the file to a QStringList, ugghh...


    completer = new MyCompleter(this);
    completer->setModel(modelFromFile(":/files/plaatsen.txt"));
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setWrapAround(false);
       </ugly>

    */

    /** version from file: this one works
    QFile file(":/files/plaatsen.txt");
    QStringList words; // "don't come easy, to me, la la la laaa la la"

    if (!file.open(QFile::ReadOnly))
        qDebug() << "<vvim> TODO: kan plaatsnamen niet vinden, iets doen? Nope, gewoon geen completer, toch?";
    else
    {

        #ifndef QT_NO_CURSOR
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        #endif

            while (!file.atEnd()) {
                QByteArray line = file.readLine();
                if (!line.isEmpty())
                    words << line.trimmed();
            }

        #ifndef QT_NO_CURSOR
            QApplication::restoreOverrideCursor();
        #endif
    }
    **/

    /** version from database **/
    QStringList words; // "don't come easy, to me, la la la laaa la la"

    #ifndef QT_NO_CURSOR
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    #endif

    QSqlQuery query("SELECT naam, straat, nr, bus, postcode, plaats, land FROM ophaalpunten");
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

    #ifndef QT_NO_CURSOR
        QApplication::restoreOverrideCursor();
    #endif



    completer = new MyCompleter(words, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);

    ui->lePostalAddress->setCompleter(completer);

    ui->lePostalAddress->setText(settings.value("startpunt").toString());
    goClicked();

    setTotalWeightTotalVolume();
}

Form::~Form()
{
    qDebug() << "start to deconstruct Form()";
    delete ui;
    delete completer;
    qDebug() << "<vvim> ~Form() deconstructor: no need to delete distance_matrix_in_meters and distance_matrix_in_seconds, this has been done by the class DistanceMatrix";
    qDebug() << "<vvim> ~Form() deconstructor: must we also delete all contents of QList <SMarker*> m_markers ?";
    qDebug() << "Form() deconstructed";
}

void Form::showLevering(double east, double north, SLevering levering, bool saveMarker)
{
    qDebug() << "Form, showLevering" << east << north;

    QString str =
            QString("var newLoc = new google.maps.LatLng(%1, %2); ").arg(north).arg(east) +
            QString("map.setCenter(newLoc);") +
            QString("map.setZoom(%1);").arg(ui->zoomSpinBox->value());

     qDebug() << str;

    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);

    if (saveMarker)
        setMarker(east, north, levering);
}

void Form::showOphaalpunt(double east, double north, SOphaalpunt ophaalpunt, bool saveMarker)
{
    qDebug() << "Form, showOphaalpunt" << east << north;

    QString str =
            QString("var newLoc = new google.maps.LatLng(%1, %2); ").arg(north).arg(east) +
            QString("map.setCenter(newLoc);") +
            QString("map.setZoom(%1);").arg(ui->zoomSpinBox->value());

     qDebug() << str;

    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);

    if (saveMarker)
        setMarker(east, north, ophaalpunt);
}


void Form::showCoordinates(double east, double north, QString markername, bool saveMarker)
{
    qDebug() << "Form, showCoordinates" << east << north;

    QString str =
            QString("var newLoc = new google.maps.LatLng(%1, %2); ").arg(north).arg(east) +
            QString("map.setCenter(newLoc);") +
            QString("map.setZoom(%1);").arg(ui->zoomSpinBox->value());

     qDebug() << str;

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
    qDebug() << str;
    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);


    SMarker *_marker = new SMarker(east, north, caption);
    m_markers.append(_marker);

    //adding capton to ListWidget
    ui->lwMarkers->addItem(caption);
    link_lwMarkers_mmarkers[caption] = _marker;
    matrices_up_to_date = false;
}

void Form::setMarker(double east, double north, SOphaalpunt ophaalpunt)
{
    QString caption = QString("%1, %2").arg(ophaalpunt.naam).arg(ophaalpunt.adres);
    for (int i=0; i<m_markers.size(); i++)
    {
        if (m_markers[i]->caption == caption)
        {
            // overschrijven met info ophaalpunt???
            qDebug() << "<vvim> TODO: wat als OPHAALPUNT & LEVERING ?";
            qDebug() << "found marker with the same caption" << caption << "ophaling" << m_markers[i]->ophaling << ", levering" << m_markers[i]->levering << ". Overwriting with SOphaalpunt data.";
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
            QString("title: %1").arg("\""+caption+"\"") +
            QString("});") +
            QString("markers.push(marker);");
    qDebug() << str;
    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);


    SMarker *_marker = new SMarker(east, north, ophaalpunt);
    m_markers.append(_marker);

    //adding capton to ListWidget
    ui->lwMarkers->addItem(caption);
    link_lwMarkers_mmarkers[caption] = _marker;
    matrices_up_to_date = false;

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
            qDebug() << "found marker with the same caption" << caption << "ophaling" << m_markers[i]->ophaling << ", levering" << m_markers[i]->levering << ". Overwriting with SLevering data.";
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
            QString("title: %1").arg("\""+caption+"\"") +
            QString("});") +
            QString("markers.push(marker);");
    qDebug() << str;
    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);


    SMarker *_marker = new SMarker(east, north, levering);
    m_markers.append(_marker);

    //adding capton to ListWidget
    ui->lwMarkers->addItem(caption);
    link_lwMarkers_mmarkers[caption] = _marker;
    matrices_up_to_date = false;

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

    qDebug() << str;

    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);
}

void Form::on_pbRemoveMarker_clicked()
{
    if (ui->lwMarkers->currentRow() < 0) return;

    if((ui->lwMarkers->currentRow()+2) == ui->lwMarkers->count())
    {
        qDebug() << "Dangerous business";
        QMessageBox::critical(this, tr("WAARSCHUWING"), tr("Het wissen van de 'voorlaatste' marker in de lijst resulteert in een fout die ik nog niet heb kunnen opsporen.\n\nAls je deze marker echt wilt wissen, versleep hem dan eerst tot onderaan de lijst, en verwijder hem dan."));
        return;
    }

    matrices_up_to_date = false;

    // after a Drag and Drop, the order might have changed
    reorderMarkers();

    /*
        because of the Drag and Drop in lwMarkers,
        the order of the markers[] array in JavaScript can be incorrect
        therefore we do not use these lines anymore

    QString str =
            QString("markers[%1].setMap(null); markers.splice(%1, 1);").arg(ui->lwMarkers->currentRow());
    qDebug() << str;
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
    qDebug() << str;
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
    {
        QList<int> *test = new QList<int>();
        for(int i = 0 ; i < m_markers.length() ; i++)
        {
            test->push_back(i);
        }
        process_result_distancematrix(test);
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
            qDebug() << _marker->caption << _marker->distancematrixindex;
            temp.push_back(_marker);
            ui->lwMarkers->addItem(m_markers.at(tsp_order_smarkers->at(i))->caption);
            //qDebug()<< m_markers.at(tsp_order_smarkers->at(i))->caption;
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

/**

    I believe this function is a copy/paste accident, must try out. Was already included in version 20140519

QAbstractItemModel *Form::modelFromFile(const QString& fileName)
{
    // <vvim> is this function ever used???
    qDebug() << "\n\n++++++++" << "<vvim> is this function ever used???" << "QAbstractItemModel *Form::modelFromFile(const QString& fileName) in form.cpp" << "++++++++\n\n";
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return new QStringListModel(completer);

#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif
    QStringList words;

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        if (!line.isEmpty())
            words << line.trimmed();
    }

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
    return new QStringListModel(words, completer);
}

**/

void Form::keyPressEvent( QKeyEvent *k )
{
    if(k->key() == Qt::Key_Delete)
        on_pbRemoveMarker_clicked();
}


void Form::drawRoute()
{
    //reorder Markers:: see BUG
          // <vvim> BUG: er is geen SIGNAL voor Drag/Drop vanuit QListWidget. Het programma weet dus niet
          //             of er markers van plaats zijn veranderd. Zeer vervelend.
          //             Bestaat er een extensie voor Qt die deze SIGNAL wel heeft?
    reorderMarkers();

    qDebug() << "<vvim>: Google MAX is 8 WayPoints. To beat this, see http://lemonharpy.wordpress.com/2011/12/15/working-around-8-waypoint-limit-in-google-maps-directions-api/ and http://jsfiddle.net/ZyHnk/";

    //info: https://www.youtube.com/watch?v=nN85QMYZzQQ


    // this only makes sense when there is more than 1 marker in m_markers
    // (as the first marker is the STARTING POINT and DESTINATION POINT of the route)
    if(m_markers.length() > 1)
    {
        QString html = QString("<!DOCTYPE html> <html> <head> <meta name=\"viewport\" content=\"initial-scale=1.0, user-scalable=no\" /> <style type=\"text/css\"> html { height: 100% } body { height: 100%; margin: 0; padding: 0 } #map_canvas { height: 100% } </style> <script type=\"text/javascript\" src=\"http://maps.googleapis.com/maps/api/js?key=")+
                settings.value("apiKey").toString()+
                QString("&sensor=false\"> </script> <script type=\"text/javascript\"> var map; var markers = []; var directionDisplay; var directionsService = new google.maps.DirectionsService(); var waypts = []; function initialize() { var myOptions = { center: new google.maps.LatLng(50.9801, 4.97517), zoom: 15, mapTypeId: google.maps.MapTypeId.ROADMAP, panControl: true }; map = new google.maps.Map(document.getElementById(\"map_canvas\"), myOptions); directionsDisplay = new google.maps.DirectionsRenderer(); directionsDisplay.setMap(map); var request = { origin:\"")+
                m_markers.first()->caption + //settings.value("startpunt").toString()+
                QString("\", destination:\"")+
                m_markers.first()->caption + //settings.value("startpunt").toString()+
                QString("\", waypoints: waypts, travelMode: google.maps.DirectionsTravelMode.DRIVING, avoidHighways: true }; directionsService.route(request, function(response, status) { if (status == google.maps.DirectionsStatus.OK) { directionsDisplay.setDirections(response); } }); ");

        /*  --> change HTML to forget the previous markers, but the put the route instead.
                Then new markers will be added as "markers", but not yet in the route.

                What will happen when we try to remove markers that are already in the route? Error?
        */

        QList <SMarker*> m_markers_minus_startingpoint = m_markers;
        m_markers_minus_startingpoint.removeFirst();

        foreach(SMarker* m, m_markers_minus_startingpoint)
        {
            QString str =
                    QString("waypts.push({") +
                    QString("location: new google.maps.LatLng(%1,%2),").arg(m->north).arg(m->east) +
//                    QString("location: \"%1\",").arg(m->caption) +
                    QString("stopover: true") +
                    QString("});");

            qDebug() << str;
            html += str;
        }

        html += QString("} </script> </head> <body onload=\"initialize()\"> <div id=\"map_canvas\" style=\"width:100%;height:100%\"> </div> </body> </html> ");

        ui->webView->setHtml(html);
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
            qDebug()<< m_markers.at(i)->caption;
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
    /**/qDebug() << "write out of m_markers BEFORE the reordering";
    /**/logOutputMarkers();

    for(int i = 0; i < ui->lwMarkers->count(); i++)
    {
        temp.push_back(link_lwMarkers_mmarkers[ui->lwMarkers->item(i)->text()]);
    }

    m_markers = temp; // does this leave garbage in memory? should I delete something??
                      // TODO <vvim> ask StackOverflow

    /**/qDebug() << "write out of m_markers AFTER the reordering";
    /**/logOutputMarkers();
}

void Form::logOutputMarkers()
{
    // function used for debugging
    qDebug() << "Debug information: output markers. Length: " << m_markers.length();
    qDebug() << "Are the distance matrices up to date?" << matrices_up_to_date;
    for(int i = 0; i < m_markers.length(); i++)
    {
        qDebug() << "Marker" << i << ":";
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
            qDebug() << ". type: Levering";
            qDebug() << "... anders bij levering dan bij ophaling, neen??? Levering wordt eerst gedaan, daarna is de camion leeg, dan de ophaling";
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

void Form::on_pbTransportationList_clicked()
{
    qDebug() << "Form::on_pbTransportationList_clicked()";
    qDebug() << "This is where we should work on making the Transportation List (using distance matrices and Document Writer)";
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
    transporationlistWriter.prepare(m_markers, distance_matrix_in_meters, distance_matrix_in_seconds);
    transporationlistWriter.show();
    return;
    qDebug() << "[Vervoerslijst]" << "<vvim> TODO: (porposal -> ) move building of Transportation List to a separate class";

    qDebug() << "[Vervoerslijst]" << "Vraag: verwachtte vertrektijd???";

    //QString currentdate = QDate::currentDate().toString("d MMMM yyyy"); // QLocale::setDefault(QLocale::Dutch);
    QString currentdate = QLocale().toString(QDateTime::currentDateTime(),"d MMMM yyyy - hh:mm:ss");
    DocumentWriter vervoersLijst("",
                                 "",
                                 currentdate);


    int previous_distance_matrix_i = -1; // we do not need the information of the starting point in the Transportation List
    int total_distance_in_meters = 0;
    int total_time_on_the_road_in_seconds = 0;
    int total_time_needed_in_seconds = 8 * 60 * 60; // 8 AM
    int STANDAARD_OPHAALTIJD_IN_SECONDEN = 30 * 60; // 30 minutes
    int counter = 1;
    int empty_bags_of_kurk_needed = 0;
    int empty_bags_of_kaarsresten_needed = 0;

    // 0. for debug only:
    for(int i = 0; i < m_markers.length(); i++)
    {
        qDebug() << "[Vervoerslijst]" << i << ":" << m_markers[i]->caption;
    }

    // 5. go to each SMarker, in order shown, and add its information to the Transportation List
    for(int i = 0; i < m_markers.length(); i++)
    {
        int current_distance_matrix_i = m_markers[i]->distancematrixindex;
        if(previous_distance_matrix_i > -1)
        {

            total_distance_in_meters += distance_matrix_in_meters[previous_distance_matrix_i][current_distance_matrix_i];
            total_time_on_the_road_in_seconds += distance_matrix_in_seconds[previous_distance_matrix_i][current_distance_matrix_i];
            total_time_needed_in_seconds += distance_matrix_in_seconds[previous_distance_matrix_i][current_distance_matrix_i];

            qDebug() << "[Vervoerslijst]" << "location:" << m_markers[i]->caption;
            qDebug() << "[Vervoerslijst]" << "arrival time:" << total_time_needed_in_seconds; // make human readable

            // dit zou een aparte functie kunnen zijn binnen DocumentWriter(SMarker) => m_markers[i]
            DocumentWriter::VisitLocation location;
            location.order = counter;
            location.distance_seconds = distance_matrix_in_seconds[previous_distance_matrix_i][current_distance_matrix_i];
            location.distance_meters = distance_matrix_in_meters[previous_distance_matrix_i][current_distance_matrix_i];
            location.Naam = m_markers[i]->caption;
            location.aankomsttijd = total_time_needed_in_seconds;
            if(m_markers[i]->ophaling)
            {
                // dit zou een aparte functie kunnen zijn binnen DocumentWriter(SOphaalpunt) => m_markers[i]->ophaalpunt
                location.Naam = m_markers[i]->ophaalpunt.naam;
                location.Adres = m_markers[i]->ophaalpunt.adres;
                location.Postcode = "een PC";
                location.Gemeente = "een gemeente";
                location.Telefoon = "een telefoonnr";
                location.Contactpersoon = "een contactpersoon";
                location.Openingsuren = "ma-vr ...";
                location.Speciale_opmerkingen = "speciale opmerk...";
                location.Soort_vervoer = "Ophaling";
                location.Kaartnr = "kaartnr?";
                location.Kurk_op_te_halen_zakken = QString("%1 zakken, %2 kg").arg(m_markers[i]->ophaalpunt.zakken_kurk).arg(m_markers[i]->ophaalpunt.kg_kurk);
                //location.Kaars_op_te_halen_zakken = QString("%1 zakken, %2 kg").arg(m_markers[i]->ophaalpunt.zakken_kaarsresten).arg(m_markers[i]->ophaalpunt.kg_kaarsresten);
                vervoersLijst.addVisit(location);
                total_time_needed_in_seconds += STANDAARD_OPHAALTIJD_IN_SECONDEN;
                empty_bags_of_kurk_needed += m_markers[i]->ophaalpunt.zakken_kurk;
                empty_bags_of_kaarsresten_needed += m_markers[i]->ophaalpunt.zakken_kaarsresten;
            }
            if(m_markers[i]->levering)
            {
                // dit zou een aparte functie kunnen zijn binnen DocumentWriter(SLevering) => m_markers[i]->levering
                location.Naam = m_markers[i]->leveringspunt.name;
                location.Adres = ""; location.Adres.append(m_markers[i]->leveringspunt.street).append(" ").append(m_markers[i]->leveringspunt.housenr);
                location.Postcode = m_markers[i]->leveringspunt.postalcode;
                location.Gemeente = m_markers[i]->leveringspunt.plaats;
                location.Telefoon = m_markers[i]->leveringspunt.telephone;
                location.Contactpersoon = m_markers[i]->leveringspunt.contactperson;
                location.Openingsuren = "ma-vr ...";
                location.Speciale_opmerkingen = "speciale opmerk...";
                location.Soort_vervoer = "Levering";
                location.Kaartnr = "kaartnr?";
                // GEWICHT, VOLUME + TIJD DIE NODIG IS
                vervoersLijst.addVisit(location);
                total_time_needed_in_seconds += m_markers[i]->leveringspunt.minutes_needed;
            }
            if((!m_markers[i]->ophaling)&&(!m_markers[i]->levering))
            {
                // enkel een adres?
                location.Soort_vervoer = "Adres";
                vervoersLijst.addVisit(location);
            }
            counter++;
            qDebug() << "[Vervoerslijst]" << "departure time:" << total_time_needed_in_seconds; // make human readable

        }
        previous_distance_matrix_i = current_distance_matrix_i;
    }

    if(m_markers.length() > 1)
    {
        int i = 0;
        qDebug() << "[Vervoerslijst]" << "We moeten de terugkeer naar het startpunt ook nog in de vervoerslijst steken";

        total_distance_in_meters += distance_matrix_in_meters[previous_distance_matrix_i][0];
        total_time_on_the_road_in_seconds += distance_matrix_in_seconds[previous_distance_matrix_i][0];
        total_time_needed_in_seconds += distance_matrix_in_seconds[previous_distance_matrix_i][0];

        qDebug() << "[Vervoerslijst]" << "location:" << m_markers[i]->caption;
        qDebug() << "[Vervoerslijst]" << "arrival time:" << total_time_needed_in_seconds; // make human readable

        // dit zou een aparte functie kunnen zijn binnen DocumentWriter(SMarker) => m_markers[i]
        DocumentWriter::VisitLocation location;
        location.order = counter;
        location.distance_seconds = distance_matrix_in_seconds[previous_distance_matrix_i][0];
        location.distance_meters = distance_matrix_in_meters[previous_distance_matrix_i][0];
        location.Naam = m_markers[i]->caption;
        location.aankomsttijd = total_time_needed_in_seconds;
        if(m_markers[i]->ophaling)
        {
            // dit zou een aparte functie kunnen zijn binnen DocumentWriter(SOphaalpunt) => m_markers[i]->ophaalpunt
            location.Naam = m_markers[i]->ophaalpunt.naam;
            location.Adres = m_markers[i]->ophaalpunt.adres;
            location.Postcode = "een PC";
            location.Gemeente = "een gemeente";
            location.Telefoon = "een telefoonnr";
            location.Contactpersoon = "een contactpersoon";
            location.Openingsuren = "ma-vr ...";
            location.Speciale_opmerkingen = "speciale opmerk...";
            location.Soort_vervoer = "Ophaling";
            location.Kaartnr = "kaartnr?";
            location.Kurk_op_te_halen_zakken = QString("%1 zakken, %2 kg").arg(m_markers[i]->ophaalpunt.zakken_kurk).arg(m_markers[i]->ophaalpunt.kg_kurk);
            //location.Kaars_op_te_halen_zakken = QString("%1 zakken, %2 kg").arg(m_markers[i]->ophaalpunt.zakken_kaarsresten).arg(m_markers[i]->ophaalpunt.kg_kaarsresten);
            vervoersLijst.addVisit(location);
            total_time_needed_in_seconds += STANDAARD_OPHAALTIJD_IN_SECONDEN;
            empty_bags_of_kurk_needed += m_markers[i]->ophaalpunt.zakken_kurk;
            empty_bags_of_kaarsresten_needed += m_markers[i]->ophaalpunt.zakken_kaarsresten;
        }
        if(m_markers[i]->levering)
        {
            // dit zou een aparte functie kunnen zijn binnen DocumentWriter(SLevering) => m_markers[i]->levering
            location.Naam = m_markers[i]->leveringspunt.name;
            location.Adres = ""; location.Adres.append(m_markers[i]->leveringspunt.street).append(" ").append(m_markers[i]->leveringspunt.housenr);
            location.Postcode = m_markers[i]->leveringspunt.postalcode;
            location.Gemeente = m_markers[i]->leveringspunt.plaats;
            location.Telefoon = m_markers[i]->leveringspunt.telephone;
            location.Contactpersoon = m_markers[i]->leveringspunt.contactperson;
            location.Openingsuren = "ma-vr ...";
            location.Speciale_opmerkingen = "speciale opmerk...";
            location.Soort_vervoer = "Levering";
            location.Kaartnr = "kaartnr?";
            // GEWICHT, VOLUME + TIJD DIE NODIG IS
            vervoersLijst.addVisit(location);
            total_time_needed_in_seconds += (m_markers[i]->leveringspunt.minutes_needed)* 60;
        }
        if((!m_markers[i]->ophaling)&&(!m_markers[i]->levering))
        {
            // enkel een adres?
            location.Soort_vervoer = "Adres";
            vervoersLijst.addVisit(location);
        }
        counter++;
        qDebug() << "[Vervoerslijst]" << "departure time:" << total_time_needed_in_seconds; // make human readable

    }

    // TELKENS

    vervoersLijst.write(QString("vervoerslijst-(%1).odt").arg(currentdate));
    qDebug() << "[Vervoerslijst]" << "Lege zakken voor kurk:" << empty_bags_of_kurk_needed << ", lege zakken voor kaarsresten:" << empty_bags_of_kaarsresten_needed;
    setTotalWeightTotalVolume();
    qDebug() << "[Vervoerslijst]" << "Totaal lading:" << ui->totalWeightEdit->text() << "kg and " << ui->totalVolumeEdit->text() << "liter";

}


void Form::logOutputLwMarkers()
{
    qDebug() << "\n\nSTART lwMarkers";
    // function used for debugging
    qDebug() << "Debug information: output lwmarkers. Count: " << ui->lwMarkers->count();
    qDebug() << "Current row:" << ui->lwMarkers->currentRow();
    for(int i = 0; i < ui->lwMarkers->count(); i++)
    {
        qDebug() << "Row" << i << ":" << ui->lwMarkers->item(i)->text();
    }
    qDebug() << "STOP\n\n";
}
