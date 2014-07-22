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

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);
    connect(ui->goButton, SIGNAL(clicked()), this, SLOT(goClicked()));
    connect(ui->lePostalAddress, SIGNAL(returnPressed()), this, SLOT(goClicked()));

    connect(&m_geocodeDataManager, SIGNAL(coordinatesReady(double,double,QString)), this, SLOT(showCoordinates(double,double,QString)));
    connect(&m_geocodeDataManager, SIGNAL(errorOccured(QString)), this, SLOT(errorOccured(QString)));

    connect(&m_distanceMatrix, SIGNAL(errorOccured(QString)), this, SLOT(errorOccured(QString)));
    connect(&m_distanceMatrix, SIGNAL(new_order_smarkers(QList<int> *)), this, SLOT(adapt_order_smarkers(QList<int> *)));

    QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);
    ui->lePostalAddress->setText("");
    //////    ui->webView->setUrl(QUrl("qrc:/html/google_maps.html"));
    ui->webView->setHtml("<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"initial-scale=1.0, user-scalable=no\" /><style type=\"text/css\">html { height: 100% } body { height: 100%; margin: 0; padding: 0 } #map_canvas { height: 100% } </style> <script type=\"text/javascript\" src=\"http://maps.googleapis.com/maps/api/js?key="+settings.value("apiKey").toString()+"&sensor=false\"> </script> <script type=\"text/javascript\"> var map; var markers = []; function initialize() { var myOptions = { center: new google.maps.LatLng(50.9801, 4.97517), zoom: 15, mapTypeId: google.maps.MapTypeId.ROADMAP, panControl: true }; map = new google.maps.Map(document.getElementById(\"map_canvas\"), myOptions); } </script> </head> <body onload=\"initialize()\"> <div id=\"map_canvas\" style=\"width:100%; height:100%\"></div> </body></html>");

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
}

Form::~Form()
{
    delete ui;
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
}

void Form::goClicked()
{
    QString address = ui->lePostalAddress->text();
    m_geocodeDataManager.getCoordinates(address.replace(" ", "+")); // !! move this replace action to geocode_data_manager
    //ui->lePostalAddress->clear(); // <vvim> this can maybe be re-instated?


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
    //<vvim> TODO: programma crasht als er maar 2 markers zijn en je wil de tweede verwijderen. Waarom?
    qDebug() << "<vvim> TODO: BUG: programma crasht als er maar 2 markers zijn en je wil de tweede verwijderen. Waarom?";

    if (ui->lwMarkers->currentRow() < 0) return;

    QString str =
            QString("markers[%1].setMap(null); markers.splice(%1, 1);").arg(ui->lwMarkers->currentRow());
    qDebug() << str;
    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);

    //deleteing caption from markers list
    delete m_markers.takeAt(ui->lwMarkers->currentRow());
qDebug() << "<vvim> DEBUG: ui->lwMarkerslength" << ui->lwMarkers->size();

    //deleteing caption from ListWidget
//<vvim> THIS ONE GOES WRONG when it is the next-to-last marker. why??
//""var newLoc = new google.maps.LatLng(50.9896, 5.05016); map.setCenter(newLoc);" " -> currentRowChanged
    delete ui->lwMarkers->takeItem(ui->lwMarkers->currentRow());
qDebug() << "<vvim> DEBUG: lwmarkers deleted";

}

void Form::on_zoomSpinBox_valueChanged(int arg1)
{
    QString str =
            QString("map.setZoom(%1);").arg(arg1);
    qDebug() << str;
    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);
}

void Form::on_pbDistanceMatrix_clicked()
{
    qDebug() << "<vvim> Button 'Route Planner' pressed";
    m_distanceMatrix.getDistances(m_markers);

    //ui->webView->setHtml("<!DOCTYPE html><h1>Hi mom!</h1></html>");
    /*  --> change HTML to forget the previous markers, but the put the route instead.
            Then new markers will be added as "markers", but not yet in the route.

            What will happen when we try to remove markers that are already in the route? Error?
    */

}

void Form::adapt_order_smarkers(QList<int> *tsp_order_smarkers)
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
            temp.push_back(m_markers.at(tsp_order_smarkers->at(i)));
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

        //<vvim> TODO:
        qDebug() << "<vvim> TODO: route tekenen: teken_route.clicked";
        qDebug() << "<vvim> TODO: ui->lwMarkers : enable dragging!!! QListWidgetItem";
        qDebug() << "<vvim> TODO: http://www.qtcentre.org/threads/40611-Accessing-and-changing-the-order-index-of-QListWidget-items";

        //hier de route-aanduiding op baseren

        //teken_route.clicked();

            // ui->lwMarkers : enable dragging!!! QListWidgetItem
}

void Form::add_aanmeldingen(QList<QString> *aanmeldingen)
{
    m_geocodeDataManager.pushListOfMarkers(aanmeldingen);
}

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


void Form::keyPressEvent( QKeyEvent *k )
{
    if(k->key() == Qt::Key_Delete)
        on_pbRemoveMarker_clicked();
}

